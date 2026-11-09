#include "ResultColoring.hpp"
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
namespace
{
  struct ColorArray
  {
    vtkDataArray* values = nullptr;
    vtkDataArray* mask = nullptr;
    int component = -1;
  };
  bool selected_array(vtkDataSet* mesh,const SimResults::Field& field,
    const Postprocessor::FieldSelection& selection,ColorArray& input,std::string& error)
  {
    error.clear();
    if (!mesh || field.association == SimResults::Association::Table)
    {
      error = "The field is not a mesh field.";
      return false;
    }
    vtkDataSetAttributes* attributes = field.association == SimResults::Association::Element ?
      static_cast<vtkDataSetAttributes*>(mesh->GetCellData()) : static_cast<vtkDataSetAttributes*>(mesh->GetPointData());
    const auto tuples = field.association == SimResults::Association::Element ? mesh->GetNumberOfCells() : mesh->GetNumberOfPoints();
    input.values = attributes->GetArray(field.array_name.c_str());
    input.mask = field.validity_array.empty() ? nullptr : attributes->GetArray(field.validity_array.c_str());
    const auto component = std::find(field.components.begin(),field.components.end(),selection.component);
    if (!input.values || input.values->GetNumberOfTuples() != tuples ||
        input.values->GetNumberOfComponents() != static_cast<int>(field.components.size()) ||
        (!field.validity_array.empty() && (!input.mask || input.mask->GetNumberOfTuples() != tuples || input.mask->GetNumberOfComponents() != 1)) ||
        (!selection.magnitude && component == field.components.end()) || (selection.magnitude && !field.vector_components))
    {
      error = "The field array, validity mask or selected component is unavailable.";
      return false;
    }
    if (selection.magnitude && std::any_of(field.vector_components->begin(),field.vector_components->end(),[&](std::size_t i)
      {
        return i >= field.components.size();
      }))
    {
      error = "The vector component declaration is invalid.";
      return false;
    }
    if (!selection.magnitude)
    {
      input.component = static_cast<int>(component-field.components.begin());
    }
    return true;
  }
  double sample(const ColorArray& input,const SimResults::Field& field,const Postprocessor::FieldSelection& selection,vtkIdType i)
  {
    if (input.mask && (!std::isfinite(input.mask->GetComponent(i,0)) || input.mask->GetComponent(i,0) <= 0))
    {
      return std::numeric_limits<double>::quiet_NaN();
    }
    double value;
    if (selection.magnitude)
    {
      const auto& indices = *field.vector_components;
      value = std::hypot(input.values->GetComponent(i,static_cast<int>(indices[0])),
        input.values->GetComponent(i,static_cast<int>(indices[1])),input.values->GetComponent(i,static_cast<int>(indices[2])));
    }
    else
    {
      value = input.values->GetComponent(i,input.component);
    }
    return std::isfinite(value) ? value : std::numeric_limits<double>::quiet_NaN();
  }
}

namespace Postprocessor
{
  std::string field_selection_key(const FieldSelection& selection)
  {
    std::string key;
    auto append = [&key](const std::string& value)
    {
      key += std::to_string(value.size())+":"+value;
    };
    append(selection.dataset_id ? std::to_string(*selection.dataset_id) : "all");
    append(selection.array_name);
    append(std::to_string(static_cast<int>(selection.association)));
    append(selection.component);
    append(selection.magnitude ? "magnitude" : "component");
    append(selection.group_key);
    return key;
  }
  std::string field_group_key(const SimResults::Field& field)
  {
    std::string key;
    auto append = [&key](const std::string& value)
    {
      key += std::to_string(value.size()) + ":" + value;
    };
    append(field.name);
    append(std::to_string(static_cast<int>(field.role)));
    append(field.source);
    append(field.set_name);
    append(field.array_name);
    append(std::to_string(static_cast<int>(field.association)));
    append(std::to_string(field.components.size()));
    for (const auto& component : field.components)
    {
      append(component);
    }
    append(field.vector_components ? "vector" : "components");
    if (field.vector_components)
    {
      for (auto index : *field.vector_components)
      {
        append(std::to_string(index));
      }
    }
    return key;
  }
  std::vector<ResultFieldGroup> result_field_groups(const SimResults::VtkResultsData& data,
    std::optional<std::size_t> current)
  {
    // Identities exclude frame and dataset numbers. Source, association, set,
    // component layout and vector semantics remain distinct variants.
    std::map<std::string,ResultFieldGroup> catalogue;
    for (std::size_t frame = 0; frame < data.increments.size(); ++frame)
    {
      for (const auto& field : data.increments[frame].fields)
      {
        const auto key = field_group_key(field);
        auto found = catalogue.find(key);
        if (found == catalogue.end())
        {
          found = catalogue.emplace(key,ResultFieldGroup{field,key,{},false}).first;
        }
        auto& group = found->second;
        if (std::find(group.datasets.begin(),group.datasets.end(),field.dataset_id) == group.datasets.end())
        {
          group.datasets.push_back(field.dataset_id);
        }
        group.available = group.available || (current && *current == frame);
      }
    }
    std::vector<ResultFieldGroup> groups;
    for (auto& entry : catalogue)
    {
      groups.push_back(std::move(entry.second));
    }
    std::sort(groups.begin(),groups.end(),[](const ResultFieldGroup& a,const ResultFieldGroup& b)
    {
      return a.field.name == b.field.name ? a.key < b.key : a.field.name < b.field.name;
    });
    return groups;
  }
  bool same_field(const std::optional<FieldSelection>& a, const std::optional<FieldSelection>& b)
  {
    return a.has_value() == b.has_value() && (!a || (a->dataset_id == b->dataset_id &&
      a->array_name == b->array_name && a->association == b->association &&
      a->component == b->component && a->magnitude == b->magnitude && a->group_key == b->group_key));
  }
  bool same_coloring(const ResultColoring& a, const ResultColoring& b)
  {
    return same_field(a.field, b.field) && a.palette == b.palette && a.range_mode == b.range_mode &&
      a.fixed_range == b.fixed_range && a.reversed == b.reversed && a.legend_visible == b.legend_visible &&
      a.bands == b.bands && a.minimum.enabled == b.minimum.enabled && a.minimum.value == b.minimum.value &&
      a.minimum.color == b.minimum.color && a.maximum.enabled == b.maximum.enabled &&
      a.maximum.value == b.maximum.value && a.maximum.color == b.maximum.color;
  }
  bool valid_coloring(const ResultColoring& c, std::string& error)
  {
    auto limit = [](const ColorLimit& v)
    {
      return std::isfinite(v.value) && std::all_of(v.color.begin(), v.color.end(), [](double x)
      {
        return std::isfinite(x) && x >= 0 && x <= 1;
      });
    };
    const bool palette = c.palette == ColorPalette::Viridis || c.palette == ColorPalette::BlueWhiteRed ||
      c.palette == ColorPalette::Turbo || c.palette == ColorPalette::Rainbow || c.palette == ColorPalette::Grayscale;
    const bool mode = c.range_mode == ColorRangeMode::Automatic || c.range_mode == ColorRangeMode::Fixed || c.range_mode == ColorRangeMode::Symmetric || c.range_mode == ColorRangeMode::AllFrames;
    if (!palette || !mode || !limit(c.minimum) || !limit(c.maximum) ||
        (c.minimum.enabled && c.maximum.enabled && c.minimum.value >= c.maximum.value) ||
        !std::isfinite(c.fixed_range[0]) || !std::isfinite(c.fixed_range[1]) || c.fixed_range[0] >= c.fixed_range[1] ||
        (c.bands && (*c.bands < 2 || *c.bands > ColoringDefaults::maximum_bands)))
    {
      error = "Color ranges/limits must be finite and ordered, RGB values must be in [0,1], and bands must be 2..256.";
      return false;
    }
    error.clear();
    return true;
  }
  const SimResults::Field* find_field(const SimResults::Increment& frame, const FieldSelection& s)
  {
    for (const auto& field : frame.fields)
    {
      if ((!s.dataset_id || field.dataset_id == *s.dataset_id) && field.array_name == s.array_name &&
          field.association == s.association && (s.group_key.empty() || field_group_key(field) == s.group_key))
      {
        return &field;
      }
    }
    return nullptr;
  }
  bool color_sample(vtkDataSet* mesh, const SimResults::Field& field, const FieldSelection& selection,
    std::size_t index, double& value, std::string& error)
  {
    ColorArray input;
    if (!selected_array(mesh,field,selection,input,error) || index >= static_cast<std::size_t>(input.values->GetNumberOfTuples()))
    {
      if (error.empty())
      {
        error = "The requested result sample is unavailable.";
      }
      return false;
    }
    value = sample(input,field,selection,static_cast<vtkIdType>(index));
    return true;
  }

  bool color_values(vtkDataSet* mesh,const SimResults::Field& field,const FieldSelection& selection,
    std::vector<double>& values,std::string& error)
  {
    ColorArray input;
    if (!selected_array(mesh,field,selection,input,error))
    {
      return false;
    }
    values.resize(static_cast<std::size_t>(input.values->GetNumberOfTuples()));
    for (vtkIdType i = 0; i < input.values->GetNumberOfTuples(); ++i)
    {
      values[static_cast<std::size_t>(i)] = sample(input,field,selection,i);
    }
    return true;
  }
  bool accumulate_color_range(vtkDataSet* mesh,const SimResults::Field& field,const FieldSelection& selection,
    std::optional<std::array<double,2>>& range,std::string& error)
  {
    ColorArray input;
    if (!selected_array(mesh,field,selection,input,error))
    {
      return false;
    }
    for (vtkIdType i = 0; i < input.values->GetNumberOfTuples(); ++i)
    {
      const auto value = sample(input,field,selection,i);
      if (!std::isfinite(value))
      {
        continue;
      }
      if (!range)
      {
        range = std::array<double,2>{value,value};
      }
      else
      {
        (*range)[0] = std::min((*range)[0],value);
        (*range)[1] = std::max((*range)[1],value);
      }
    }
    return true;
  }
  const char* color_palette_name(ColorPalette palette)
  {
    switch (palette)
    {
      case ColorPalette::Viridis: return "viridis";
      case ColorPalette::BlueWhiteRed: return "bluewhitered";
      case ColorPalette::Turbo: return "turbo";
      case ColorPalette::Rainbow: return "rainbow";
      case ColorPalette::Grayscale: return "grayscale";
    }
    return "";
  }

  bool parse_color_palette(const std::string& name, ColorPalette& palette)
  {
    for (const auto candidate : {ColorPalette::Viridis, ColorPalette::BlueWhiteRed,
      ColorPalette::Turbo, ColorPalette::Rainbow, ColorPalette::Grayscale})
    {
      if (name == color_palette_name(candidate))
      {
        palette = candidate;
        return true;
      }
    }
    return false;
  }

  std::array<double, 3> palette_color(ColorPalette palette, double t)
  {
    t = std::clamp(t, 0.0, 1.0);
    // Sampled RGB control points owned by Core, interpolated in RGB space.
    using RGB = std::array<double, 3>;
    std::array<RGB, 5> points;
    switch (palette)
    {
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
        // Equally spaced RGB stops match the approved Classic rainbow preview.
        constexpr double half = 128.0 / 255.0;
        constexpr double sky = 191.0 / 255.0;
        const std::array<RGB,10> rainbow = {{{0,0,half},{0,0,1},{0,sky,1},{0,1,1},
          {0,1,half},{half,1,0},{1,1,0},{1,half,0},{1,0,0},{half,0,0}}};
        const double p = t * 9;
        const auto i = std::min(static_cast<std::size_t>(p),std::size_t{8});
        RGB value;
        for (std::size_t c = 0; c < 3; ++c)
        {
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
    for (std::size_t c = 0; c < 3; ++c)
    {
      value[c] = points[i][c] + (points[i+1][c] - points[i][c]) * (p - static_cast<double>(i));
    }
    return value;
  }
  std::array<double, 3> result_color(double value, const ResultColoring& c, const std::array<double, 2>& range)
  {
    if (!std::isfinite(value))
    {
      return ColoringDefaults::missing;
    }
    if (c.minimum.enabled && value < c.minimum.value)
    {
      return c.minimum.color;
    }
    if (c.maximum.enabled && value > c.maximum.value)
    {
      return c.maximum.color;
    }
    double t = 0.5;
    if (range[0] != range[1])
    {
      // Scaling also avoids overflow on MSVC, where long double is double.
      const double scale = std::max({std::abs(value),std::abs(range[0]),std::abs(range[1])});
      t = std::clamp((value/scale-range[0]/scale)/(range[1]/scale-range[0]/scale),0.0,1.0);
    }
    if (c.bands)
    {
      t = std::min(std::floor(t * *c.bands), static_cast<double>(*c.bands - 1)) / (*c.bands - 1);
    }
    return palette_color(c.palette, c.reversed ? 1 - t : t);
  }
}
