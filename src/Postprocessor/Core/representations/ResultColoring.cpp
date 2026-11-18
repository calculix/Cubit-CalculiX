#include "ResultColoringUnits.hpp"
#include "ResultColoring.hpp"
#include <vtkMath.h>
#include "Framework/Results/ResultsUnits.hpp"
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <utility>
namespace
{
  struct ColorArray
  {
    vtkDataArray* values = nullptr;
    vtkDataArray* mask = nullptr;
    int component = -1;
    double unit_scale = 1.0;
    double unit_offset = 0.0;
  };
  bool selected_array(vtkDataSet* mesh,const SimResults::Field& field,
    const Postprocessor::FieldSelection& selection,ColorArray& input,std::string& error,const SimUnits::WorkspaceUnitContext* presentation) {
    error.clear();
    if (!mesh || field.association == SimResults::Association::Table) {
      error = "The field is not a mesh field.";
      return false;
    }
    vtkDataSetAttributes* attributes = mesh->GetPointData();
    auto tuples = mesh->GetNumberOfPoints();
    if (field.association == SimResults::Association::Element) {
      attributes = mesh->GetCellData();
      tuples = mesh->GetNumberOfCells();
    }
    input.values = attributes->GetArray(field.array_name.c_str());
    input.mask = nullptr;
    if (!field.validity_array.empty()) {
      input.mask = attributes->GetArray(field.validity_array.c_str());
    }
    const auto component = std::find(field.components.begin(),field.components.end(),selection.component);
    if (!input.values || input.values->GetNumberOfTuples() != tuples ||
        input.values->GetNumberOfComponents() != static_cast<int>(field.components.size()) ||
        (!field.validity_array.empty() && (!input.mask || input.mask->GetNumberOfTuples() != tuples || input.mask->GetNumberOfComponents() != 1)) ||
        (!selection.magnitude && component == field.components.end()) || (selection.magnitude && !field.vector_components)) {
      error = "The field array, validity mask or selected component is unavailable.";
      return false;
    }
    if (selection.magnitude && std::any_of(field.vector_components->begin(),field.vector_components->end(),[&](std::size_t i) {
        return i >= field.components.size();
      })) {
      error = "The vector component declaration is invalid.";
      return false;
    }
    if (!selection.magnitude) {
      input.component = static_cast<int>(component-field.components.begin());
    }
    double one = 1.0;
    std::string label;
    if (!SimResults::ResultsUnits::field_value(0.0, field, selection.component, selection.magnitude, "postprocessor", input.unit_offset, label, error, presentation) || !SimResults::ResultsUnits::field_value(1.0, field, selection.component, selection.magnitude, "postprocessor", one, label, error, presentation)) {
      return false;
    }
    input.unit_scale = one - input.unit_offset;
    return true;
  }
  double sample(const ColorArray& input,const SimResults::Field& field,const Postprocessor::FieldSelection& selection,vtkIdType i) {
    if (input.mask && (!std::isfinite(input.mask->GetComponent(i,0)) || input.mask->GetComponent(i,0) <= 0)) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    double value;
    if (selection.magnitude) {
      const auto& indices = *field.vector_components;
      value = std::hypot(input.values->GetComponent(i,static_cast<int>(indices[0])),
        input.values->GetComponent(i,static_cast<int>(indices[1])),input.values->GetComponent(i,static_cast<int>(indices[2])));
    }
    else {
      value = input.values->GetComponent(i,input.component);
    }
    if (!std::isfinite(value)) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return value * input.unit_scale + input.unit_offset;
  }
}

namespace Postprocessor
{
  std::string preferred_field_component(const SimResults::Field& field)
  {
    const auto mises = std::find_if(field.components.begin(), field.components.end(), [](const std::string& name)
    {
      const std::string expected = "MISES";
      return name.size() == expected.size() && std::equal(name.begin(), name.end(), expected.begin(), [](char value, char upper)
      {
        if (value >= 'a' && value <= 'z')
        {
          value = static_cast<char>(value - 'a' + 'A');
        }
        return value == upper;
      });
    });
    if (mises != field.components.end())
    {
      return *mises;
    }
    return field.components.empty() ? std::string{} : field.components.front();
  }

  FieldSelection preferred_field_selection(const SimResults::Field& field, const std::string& group_key)
  {
    FieldSelection selection;
    selection.array_name = field.array_name;
    selection.association = field.association;
    selection.group_key = group_key;
    selection.magnitude = field.vector_components.has_value();
    if (!selection.magnitude)
    {
      selection.component = preferred_field_component(field);
    }
    return selection;
  }

  std::string field_selection_key(const FieldSelection& selection) {
    std::string key;
    auto append = [&key](const std::string& value) {
      key += std::to_string(value.size())+":"+value;
    };
    if (selection.dataset_id) {
      append(std::to_string(*selection.dataset_id));
    } else {
      append("all");
    }
    append(selection.array_name);
    append(std::to_string(static_cast<int>(selection.association)));
    append(selection.component);
    if (selection.magnitude) {
      append("magnitude");
    } else {
      append("component");
    }
    append(selection.group_key);
    return key;
  }
  std::string field_group_key(const SimResults::Field& field) {
    std::string key;
    auto append = [&key](const std::string& value) {
      key += std::to_string(value.size()) + ":" + value;
    };
    append(field.name);
    append(std::to_string(static_cast<int>(field.role)));
    append(field.source);
    append(field.set_name);
    append(field.array_name);
    append(std::to_string(static_cast<int>(field.association)));
    append(std::to_string(field.components.size()));
    for (const auto& component : field.components) {
      append(component);
    }
    if (field.vector_components) {
      append("vector");
    } else {
      append("components");
    }
    if (field.vector_components) {
      for (auto index : *field.vector_components) {
        append(std::to_string(index));
      }
    }
    return key;
  }
  std::vector<ResultFieldGroup> result_field_groups(const SimResults::VtkResultsData& data,
    std::optional<std::size_t> current) {
    // Identities exclude frame and dataset numbers. Source, association, set,
    // component layout and vector semantics remain distinct variants.
    std::map<std::string,ResultFieldGroup> catalogue;
    for (std::size_t frame = 0; frame < data.increments.size(); ++frame) {
      for (const auto& field : data.increments[frame].fields) {
        if (field.categorical) {
          continue;
        }
        const auto key = field_group_key(field);
        auto found = catalogue.find(key);
        if (found == catalogue.end()) {
          found = catalogue.emplace(key,ResultFieldGroup{field,key,{},false}).first;
        }
        auto& group = found->second;
        const auto& basis = group.field.component_units;
        if (basis.size() != field.component_units.size())
        {
          group.unit_issue = "Result unit metadata changes across datasets or frames.";
        }
        else
        {
          for (std::size_t i = 0; i < basis.size(); ++i)
          {
            const auto& candidate = field.component_units[i];
            if (basis[i].quantity != candidate.quantity || basis[i].dimension != candidate.dimension ||
                basis[i].role != candidate.role || basis[i].unit.empty() != candidate.unit.empty() ||
                (basis[i].quantity.empty() && basis[i].unit != candidate.unit))
            {
              group.unit_issue = "Result physical quantity changes across datasets or frames.";
            }
          }
        }

        if (std::find(group.datasets.begin(),group.datasets.end(),field.dataset_id) == group.datasets.end()) {
          group.datasets.push_back(field.dataset_id);
        }
        group.available = group.available || (current && *current == frame);
      }
    }
    std::vector<ResultFieldGroup> groups;
    for (auto& entry : catalogue) {
      if (!entry.second.unit_issue.empty())
      {
        entry.second.available = false;
      }
      groups.push_back(std::move(entry.second));
    }
    std::sort(groups.begin(),groups.end(),[](const ResultFieldGroup& a,const ResultFieldGroup& b) {
      if (a.field.name == b.field.name) {
        return a.key < b.key;
      }
      return a.field.name < b.field.name;
    });
    return groups;
  }
  bool same_field(const std::optional<FieldSelection>& a, const std::optional<FieldSelection>& b) {
    return a.has_value() == b.has_value() && (!a || (a->dataset_id == b->dataset_id &&
      a->array_name == b->array_name && a->association == b->association &&
      a->component == b->component && a->magnitude == b->magnitude && a->group_key == b->group_key));
  }
  bool same_coloring(const ResultColoring& a, const ResultColoring& b) {
    return same_coloring_units(a.bounds_unit, b.bounds_unit) && same_field(a.field, b.field) && a.palette == b.palette && a.custom_palette == b.custom_palette && a.palette_hsv == b.palette_hsv && a.range_mode == b.range_mode &&
      a.fixed_range == b.fixed_range && a.reversed == b.reversed && a.legend_visible == b.legend_visible &&
      a.bands == b.bands && a.legend_position == b.legend_position && a.minimum.enabled == b.minimum.enabled && a.minimum.value == b.minimum.value &&
      a.minimum.color == b.minimum.color && a.maximum.enabled == b.maximum.enabled &&
      a.maximum.value == b.maximum.value && a.maximum.color == b.maximum.color;
  }
  bool valid_coloring(const ResultColoring& c, std::string& error) {
    if (!valid_coloring_units(c.bounds_unit, error))
    {
      return false;
    }
    if ((c.palette_hsv && c.custom_palette.size() != 2) || (!c.custom_palette.empty() && c.custom_palette.size() < 2) || c.custom_palette.size() > ColoringDefaults::maximum_bands ||
        !std::all_of(c.custom_palette.begin(),c.custom_palette.end(),[](const auto& rgb)
        {
          return std::all_of(rgb.begin(),rgb.end(),[](double v)
          {
            return std::isfinite(v) && v >= 0 && v <= 1;
          });
        }))
    {
      error = "Custom palettes require 2..256 finite RGB stops in [0,1].";
      return false;
    }
    if (c.legend_position && !std::all_of(c.legend_position->begin(),c.legend_position->end(),[](double value)
      {
        return std::isfinite(value) && value >= 0 && value <= 1;
      }))
    {
      error = "Legend position requires finite normalized coordinates in [0,1].";
      return false;
    }
    auto limit = [](const ColorLimit& v) {
      return std::isfinite(v.value) && std::all_of(v.color.begin(), v.color.end(), [](double x) {
        return std::isfinite(x) && x >= 0 && x <= 1;
      });
    };
    const bool palette = c.palette == ColorPalette::Viridis || c.palette == ColorPalette::BlueWhiteRed ||
      c.palette == ColorPalette::Turbo || c.palette == ColorPalette::Rainbow || c.palette == ColorPalette::Grayscale;
    const bool mode = c.range_mode == ColorRangeMode::Automatic || c.range_mode == ColorRangeMode::Fixed || c.range_mode == ColorRangeMode::Symmetric || c.range_mode == ColorRangeMode::AllFrames;
    if (!palette || !mode || !limit(c.minimum) || !limit(c.maximum) ||
        (c.minimum.enabled && c.maximum.enabled && c.minimum.value >= c.maximum.value) ||
        !std::isfinite(c.fixed_range[0]) || !std::isfinite(c.fixed_range[1]) || c.fixed_range[0] >= c.fixed_range[1] ||
        (c.bands && (*c.bands < 2 || *c.bands > ColoringDefaults::maximum_bands))) {
      error = "Color ranges/limits must be finite and ordered, RGB values must be in [0,1], and bands must be 2..256.";
      return false;
    }
    error.clear();
    return true;
  }
  const SimResults::Field* find_field(const SimResults::Increment& frame, const FieldSelection& s) {
    for (const auto& field : frame.fields) {
      if ((!s.dataset_id || field.dataset_id == *s.dataset_id) && field.array_name == s.array_name &&
          field.association == s.association && (s.group_key.empty() || field_group_key(field) == s.group_key)) {
        return &field;
      }
    }
    return nullptr;
  }
  bool color_values_range(vtkDataSet* mesh, const SimResults::Field& field,
    const FieldSelection& selection, std::size_t begin, std::size_t count,
    std::vector<double>& output, std::string& error)
  {
    ColorArray input;
    if (!selected_array(mesh, field, selection, input, error, nullptr))
    {
      return false;
    }
    const auto size = static_cast<std::size_t>(input.values->GetNumberOfTuples());
    if (begin > size || count > size - begin)
    {
      error = "Result sample chunk is outside the retained array.";
      return false;
    }
    std::vector<double> candidate(count);
    for (std::size_t index = 0; index < count; ++index)
    {
      candidate[index] = sample(input, field, selection, static_cast<vtkIdType>(begin + index));
    }
    output = std::move(candidate);
    error.clear();
    return true;
  }

  bool color_sample(vtkDataSet* mesh, const SimResults::Field& field, const FieldSelection& selection,
    std::size_t index, double& value, std::string& error, const SimUnits::WorkspaceUnitContext* presentation) {
    ColorArray input;
    if (!selected_array(mesh,field,selection,input,error,presentation) || index >= static_cast<std::size_t>(input.values->GetNumberOfTuples())) {
      if (error.empty()) {
        error = "The requested result sample is unavailable.";
      }
      return false;
    }
    value = sample(input,field,selection,static_cast<vtkIdType>(index));
    return true;
  }

  bool color_values(vtkDataSet* mesh,const SimResults::Field& field,const FieldSelection& selection,
    std::vector<double>& values,std::string& error, const SimUnits::WorkspaceUnitContext* presentation) {
    ColorArray input;
    if (!selected_array(mesh,field,selection,input,error,presentation)) {
      return false;
    }
    values.resize(static_cast<std::size_t>(input.values->GetNumberOfTuples()));
    for (vtkIdType i = 0; i < input.values->GetNumberOfTuples(); ++i) {
      values[static_cast<std::size_t>(i)] = sample(input,field,selection,i);
    }
    return true;
  }
  bool accumulate_color_range(vtkDataSet* mesh,const SimResults::Field& field,const FieldSelection& selection,
    std::optional<std::array<double,2>>& range,std::string& error, const SimUnits::WorkspaceUnitContext* presentation) {
    ColorArray input;
    if (!selected_array(mesh,field,selection,input,error,presentation)) {
      return false;
    }
    for (vtkIdType i = 0; i < input.values->GetNumberOfTuples(); ++i) {
      const auto value = sample(input,field,selection,i);
      if (!std::isfinite(value)) {
        continue;
      }
      if (!range) {
        range = std::array<double,2>{value,value};
      }
      else {
        (*range)[0] = std::min((*range)[0],value);
        (*range)[1] = std::max((*range)[1],value);
      }
    }
    return true;
  }
  const char* color_palette_name(ColorPalette palette) {
    switch (palette) {
      case ColorPalette::Viridis: return "viridis";
      case ColorPalette::BlueWhiteRed: return "bluewhitered";
      case ColorPalette::Turbo: return "turbo";
      case ColorPalette::Rainbow: return "rainbow";
      case ColorPalette::Grayscale: return "grayscale";
    }
    return "";
  }

  bool parse_color_palette(const std::string& name, ColorPalette& palette) {
    for (const auto candidate : {ColorPalette::Viridis, ColorPalette::BlueWhiteRed,
      ColorPalette::Turbo, ColorPalette::Rainbow, ColorPalette::Grayscale}) {
      if (name == color_palette_name(candidate)) {
        palette = candidate;
        return true;
      }
    }
    return false;
  }

  std::array<double, 3> palette_color(ColorPalette palette, double t) {
    t = std::clamp(t, 0.0, 1.0);
    // Sampled RGB control points owned by Core, interpolated in RGB space.
    using RGB = std::array<double, 3>;
    std::array<RGB, 5> points;
    switch (palette) {
      case ColorPalette::Viridis:
        points = {{{0.267,0.005,0.329},{0.230,0.322,0.546},{0.128,0.567,0.551},{0.369,0.789,0.383},{0.993,0.906,0.144}}};
        break;
      case ColorPalette::BlueWhiteRed:
        points = {{{0.1,0.2,0.8},{0.55,0.6,0.9},{1,1,1},{0.9,0.55,0.55},{0.8,0.1,0.1}}};
        break;
      case ColorPalette::Turbo:
        points = {{{0.19,0.07,0.23},{0.16,0.74,0.92},{0.64,0.99,0.23},{0.98,0.5,0.13},{0.48,0.016,0.01}}};
        break;
      case ColorPalette::Rainbow:
      {
        // Bright blue/red endpoints; retain the interior Classic rainbow stops.
        constexpr double half = 128.0 / 255.0;
        constexpr double sky = 191.0 / 255.0;
        const std::array<RGB,8> rainbow = {{{0,0,1},{0,sky,1},{0,1,1},
          {0,1,half},{half,1,0},{1,1,0},{1,half,0},{1,0,0}}};
        const double p = t * 7;
        const auto i = std::min(static_cast<std::size_t>(p),std::size_t{6});
        RGB value;
        for (std::size_t c = 0; c < 3; ++c) {
          value[c] = rainbow[i][c] + (rainbow[i+1][c] - rainbow[i][c]) * (p - static_cast<double>(i));
        }
        return value;
      }
      case ColorPalette::Grayscale:
        return {t,t,t};
      default:
        return ColoringDefaults::missing;
    }
    const double p = t * 4;
    const auto i = std::min(static_cast<std::size_t>(p), std::size_t{3});
    RGB value;
    for (std::size_t c = 0; c < 3; ++c) {
      value[c] = points[i][c] + (points[i+1][c] - points[i][c]) * (p - static_cast<double>(i));
    }
    return value;
  }
  std::array<double,3> coloring_palette_color(const ResultColoring& coloring,double t)
  {
    if (!std::isfinite(t))
    {
      return ColoringDefaults::missing;
    }
    if (coloring.custom_palette.empty())
    {
      return palette_color(coloring.palette,t);
    }
    const auto& stops = coloring.custom_palette;
    if (t <= 0)
    {
      return stops.front();
    }
    if (t >= 1)
    {
      return stops.back();
    }
    if (coloring.palette_hsv)
    {
      std::array<double,3> low, high, rgb;
      vtkMath::RGBToHSV(stops.front().data(),low.data());
      vtkMath::RGBToHSV(stops.back().data(),high.data());
      // Gray has no hue. Borrow the chromatic endpoint's hue, then take
      // the shortest path around the hue circle between the two colors.
      if (low[1] == 0)
      {
        low[0] = high[0];
      }
      if (high[1] == 0)
      {
        high[0] = low[0];
      }
      const double hue = low[0]+t*std::remainder(high[0]-low[0],1.0);
      const std::array<double,3> hsv = {hue-std::floor(hue),
        low[1]+t*(high[1]-low[1]),low[2]+t*(high[2]-low[2])};
      vtkMath::HSVToRGB(hsv.data(),rgb.data());
      return rgb;
    }
    const double p = std::clamp(t,0.0,1.0)*(stops.size()-1);
    const auto i = std::min(static_cast<std::size_t>(p),stops.size()-2);
    std::array<double,3> rgb;
    for (std::size_t c = 0; c < 3; ++c)
    {
      rgb[c] = stops[i][c]+(stops[i+1][c]-stops[i][c])*(p-i);
    }
    return rgb;
  }
  std::vector<std::array<double,3>> palette_endpoints(const ResultColoring& coloring)
  {
    auto low = coloring_palette_color(coloring,0);
    auto high = coloring_palette_color(coloring,1);
    if (coloring.reversed)
    {
      std::swap(low,high);
    }
    return {low,high};
  }

  bool set_palette_endpoint(ResultColoring& coloring,bool high,const std::array<double,3>& rgb,std::string& error)
  {
    if (!valid_coloring(coloring,error))
    {
      return false;
    }
    if (!std::all_of(rgb.begin(),rgb.end(),[](double channel)
      {
        return std::isfinite(channel) && channel >= 0 && channel <= 1;
      }))
    {
      error = "Palette endpoint RGB must be in [0,1].";
      return false;
    }
    auto stops = palette_endpoints(coloring);
    stops[static_cast<std::size_t>(high)] = rgb;
    if (coloring.reversed)
    {
      std::swap(stops.front(),stops.back());
    }
    coloring.custom_palette = std::move(stops);
    coloring.palette_hsv = true;
    return true;
  }

  void reset_custom_palette(ResultColoring& coloring)
  {
    coloring.custom_palette.clear();
    coloring.palette_hsv = false;
  }

  std::vector<std::array<double,3>> palette_samples(const ResultColoring& coloring)
  {
    std::size_t count = 8;
    if (!coloring.custom_palette.empty())
    {
      count = coloring.custom_palette.size();
    }
    if (coloring.bands)
    {
      count = *coloring.bands;
    }
    std::vector<std::array<double,3>> colors;
    for (std::size_t i = 0; i < count; ++i)
    {
      colors.push_back(coloring_palette_color(coloring,static_cast<double>(i)/(count-1)));
    }
    return colors;
  }
  bool set_palette_stop(ResultColoring& coloring,std::size_t stop,const std::array<double,3>& rgb,std::string& error)
  {
    error.clear();
    if (!valid_coloring(coloring,error))
    {
      return false;
    }
    auto colors = palette_samples(coloring);
    if (stop == 0 || stop > colors.size() || !std::all_of(rgb.begin(),rgb.end(),[](double c)
      {
        return std::isfinite(c) && c >= 0 && c <= 1;
      }))
    {
      error = "Palette stop must address an existing color (starting at 1), with RGB in [0,1].";
      return false;
    }
    colors[stop-1] = rgb;
    coloring.custom_palette = std::move(colors);
    coloring.palette_hsv = false;
    return true;
  }
  std::array<double, 3> result_color(double value, const ResultColoring& c, const std::array<double, 2>& range) {
    if (!std::isfinite(value)) {
      return ColoringDefaults::missing;
    }
    if (c.minimum.enabled && value < c.minimum.value) {
      return c.minimum.color;
    }
    if (c.maximum.enabled && value > c.maximum.value) {
      return c.maximum.color;
    }
    double t = 0.5;
    if (range[0] != range[1]) {
      // Scaling also avoids overflow on MSVC, where long double is double.
      const double scale = std::max({std::abs(value),std::abs(range[0]),std::abs(range[1])});
      t = std::clamp((value/scale-range[0]/scale)/(range[1]/scale-range[0]/scale),0.0,1.0);
    }
    if (c.bands) {
      t = std::min(std::floor(t * *c.bands), static_cast<double>(*c.bands - 1)) / (*c.bands - 1);
    }
    if (c.reversed) {
      t = 1 - t;
    }
    return coloring_palette_color(c, t);
  }
}
