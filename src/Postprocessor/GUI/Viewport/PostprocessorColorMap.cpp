#include "PostprocessorColorMap.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorPicking.hpp"
#include <vtkActor.h>
#include <vtkCoordinate.h>
#include <vtkDataSet.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTextRenderer.h>
#include <vtkRenderWindow.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
namespace
{
  constexpr const char* scalarName = "__CubitSimDisplayScalar";
}
PostprocessorColorMap::PostprocessorColorMap(vtkRenderer* target) : renderer(target) {
  legend = vtkSmartPointer<vtkScalarBarActor>::New();
  legend->SetBarRatio(Postprocessor::ColoringDefaults::legend_bar_pixels/Postprocessor::ColoringDefaults::legend_width_pixels);
  legend->SetNumberOfLabels(5);
  legend->SetLabelFormat("%.6g");
  legend->UnconstrainedFontSizeOn();
  legend->GetTitleTextProperty()->SetFontSize(Postprocessor::ColoringDefaults::legend_font_pixels);
  legend->GetLabelTextProperty()->SetFontSize(Postprocessor::ColoringDefaults::legend_font_pixels);
  legend->GetTitleTextProperty()->BoldOff();
  legend->GetTitleTextProperty()->ItalicOff();
  legend->GetLabelTextProperty()->BoldOff();
  legend->GetLabelTextProperty()->ItalicOff();
  legend->GetTitleTextProperty()->SetOpacity(1.0);
  legend->GetLabelTextProperty()->SetOpacity(1.0);
  legend->SetVisibility(false);
  renderer->AddActor2D(legend);
  minimumLabel = vtkSmartPointer<vtkTextActor>::New();
  maximumLabel = vtkSmartPointer<vtkTextActor>::New();
  for (auto* label : {minimumLabel.GetPointer(),maximumLabel.GetPointer()}) {
    label->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    label->GetTextProperty()->SetFontSize(12);
    label->SetVisibility(false);
    renderer->AddActor2D(label);
  }
}
PostprocessorColorMap::~PostprocessorColorMap() {
  renderer->RemoveActor2D(legend);
  renderer->RemoveActor2D(minimumLabel);
  renderer->RemoveActor2D(maximumLabel);
}
void PostprocessorColorMap::apply(const Postprocessor::Representation& display, const SimResults::VtkResultsData& data,
  std::vector<PostprocessorColorSurface>& surfaces, unsigned int slot,
  const std::optional<std::array<double,2>>& rangeAcrossFrames, std::string& error, const SimUnits::WorkspaceUnitContext* presentation) {
  using namespace Postprocessor;
  const auto& c = display.coloring;
  const bool inputChanged = unitGeneration != framework_units().generation() || appliedData != &data || appliedFrame != display.increment_index ||
    appliedDisplayName != display.name;
  unitGeneration = framework_units().generation();
  const bool changedField = inputChanged || !applied || !same_field(applied->field,c.field);
  appliedSlot = slot;
  if (inputChanged || !applied || !same_coloring(*applied, c) || allFrameRange != rangeAcrossFrames || hiddenDatasets != display.hidden_datasets) {
    if (changedField) {
      actualRange.reset();
      title = display.name;
      const SimResults::Field* field = nullptr;
      if (c.field && display.increment_index < data.increments.size()) {
        field = find_field(data.increments[display.increment_index], *c.field);
      }
      for (auto& surface : surfaces) {
        surface.scalars = nullptr;
        surface.range.reset();
        if (surface.nodes) {
          surface.nodes->GetMapper()->ScalarVisibilityOff();
          surface.node_data->GetPointData()->RemoveArray(scalarName);
        }
        auto* mapper = vtkPolyDataMapper::SafeDownCast(surface.actor->GetMapper());
        mapper->ScalarVisibilityOff();
        mapper->SetInputData(surface.base);
        if (!c.field || (c.field->dataset_id && surface.dataset != *c.field->dataset_id)) {
          continue;
        }
        auto selected = *c.field;
        selected.dataset_id = surface.dataset;
        const auto* partField = find_field(data.increments[display.increment_index],selected);
        if (!partField) {
          continue;
        }
        std::vector<double> values;
        std::string valueError;
        if (!color_values(surface.input, *partField, selected, values, valueError, presentation)) {
          if (!error.empty()) {
            error += "\n";
          }
          error += display.name + ": " + valueError;
          continue;
        }
        for (double v : values) {
          if (std::isfinite(v)) {
            if (!surface.range) {
              surface.range = std::array<double,2>{v,v};
            }
            else {
              (*surface.range)[0] = std::min((*surface.range)[0], v);
              (*surface.range)[1] = std::max((*surface.range)[1], v);
            }
          }
        }
        vtkSmartPointer<vtkDataSet> input;
        input.TakeReference(surface.input->NewInstance());
        input->ShallowCopy(surface.input);
        auto scalar = vtkSmartPointer<vtkDoubleArray>::New();
        scalar->SetName(scalarName);
        scalar->SetNumberOfTuples(static_cast<vtkIdType>(values.size()));
        for (std::size_t i = 0; i < values.size(); ++i) {
          scalar->SetValue(static_cast<vtkIdType>(i), values[i]);
        }
        if (partField->association == SimResults::Association::Element) {
          input->GetCellData()->AddArray(scalar);
        }
        else {
          input->GetPointData()->AddArray(scalar);
          if (surface.node_data) {
            std::vector<double> nodeValues;
            std::string nodeError;
            if (color_values(surface.node_data,*partField,selected,nodeValues,nodeError,presentation)) {
              auto nodeScalar = vtkSmartPointer<vtkDoubleArray>::New();
              nodeScalar->SetName(scalarName);
              for (double value : nodeValues) {
                nodeScalar->InsertNextValue(value);
              }
              surface.node_data->GetPointData()->AddArray(nodeScalar);
            }
          }
        }
        auto filter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
        Postprocessor::configure_pick_provenance(filter);
        filter->SetInputData(input);
        filter->Update();
        surface.scalars = vtkSmartPointer<vtkPolyData>::New();
        surface.scalars->ShallowCopy(filter->GetOutput());
      }
      if (field) {
        std::string component = c.field->component;
        if (c.field->magnitude) {
          component = "Magnitude";
        }
        title += "\n" + field->name + " / " + component;
        double value = 0.0;
        std::string unit;
        if (SimResults::ResultsUnits::field_value(0.0, *field, c.field->component, c.field->magnitude, "postprocessor", value, unit, error, presentation)) {
          title += " [" + unit + "]";
        }
      }
    }
    // Visibility changes only recombine cached per-surface extrema; geometry and
    // scalar arrays remain intact while the selected field is unchanged.
    actualRange.reset();
    for (const auto& surface : surfaces) {
      if (display.hidden_datasets.count(surface.dataset) != 0 || !surface.range) {
        continue;
      }
      if (!actualRange) {
        actualRange = surface.range;
      }
      else {
        (*actualRange)[0] = std::min((*actualRange)[0],(*surface.range)[0]);
        (*actualRange)[1] = std::max((*actualRange)[1],(*surface.range)[1]);
      }
    }
    std::array<double,2> range = c.fixed_range;
    if (c.range_mode == ColorRangeMode::AllFrames && rangeAcrossFrames) {
      range = *rangeAcrossFrames;
    }
    else if (c.range_mode != ColorRangeMode::Fixed && c.range_mode != ColorRangeMode::AllFrames && actualRange) {
      range = *actualRange;
      if (c.range_mode == ColorRangeMode::Symmetric) {
        const auto m = std::max(std::abs(range[0]), std::abs(range[1]));
        range = {-m,m};
      }
    }
    // Map interpolated scalars, not interpolated RGB, so palette bands and
    // limit colors survive within nodal faces. The table resolution is finite.
    auto mappingRange = range;
    for (const auto& limit : {c.minimum,c.maximum}) {
      if (limit.enabled && actualRange && limit.value > (*actualRange)[0] && limit.value < (*actualRange)[1]) {
        mappingRange[0] = std::min(mappingRange[0],limit.value);
        mappingRange[1] = std::max(mappingRange[1],limit.value);
      }
    }
    if (mappingRange[0] == mappingRange[1]) {
      const auto pad = std::max(1.0,std::abs(mappingRange[0])*1e-6);
      const auto low = mappingRange[0]-pad;
      const auto high = mappingRange[1]+pad;
      if (std::isfinite(low)) {
        mappingRange[0] = low;
      }
      if (std::isfinite(high)) {
        mappingRange[1] = high;
      }
    }
    auto mapping = vtkSmartPointer<vtkLookupTable>::New();
    mapping->SetNumberOfTableValues(4096);
    mapping->SetTableRange(mappingRange.data());
    mapping->SetNanColor(ColoringDefaults::missing[0],ColoringDefaults::missing[1],ColoringDefaults::missing[2],1);
    mapping->Build();
    for (vtkIdType i = 0; i < 4096; ++i) {
      const long double t = static_cast<long double>(i)/4095;
      double value = static_cast<double>((1 - t) * mappingRange[0] + t * mappingRange[1]);
      if (actualRange && (*actualRange)[0] == (*actualRange)[1]) {
        value = (*actualRange)[0];
      }
      const auto rgb = result_color(value,c,range);
      mapping->SetTableValue(i,rgb[0],rgb[1],rgb[2],1);
    }
    for (const auto& limit : {c.minimum,c.maximum}) {
      if (limit.enabled && limit.value >= mappingRange[0] && limit.value <= mappingRange[1]) {
        const auto index = mapping->GetIndex(limit.value);
        const auto rgb = result_color(limit.value,c,range);
        mapping->SetTableValue(index,rgb[0],rgb[1],rgb[2],1);
      }
    }
    const auto below = result_color(-std::numeric_limits<double>::max(),c,range);
    const auto above = result_color(std::numeric_limits<double>::max(),c,range);
    mapping->SetBelowRangeColor(below[0],below[1],below[2],1);
    mapping->SetAboveRangeColor(above[0],above[1],above[2],1);
    mapping->UseBelowRangeColorOn();
    mapping->UseAboveRangeColorOn();
    for (auto& surface : surfaces) {
      auto* mapper = vtkPolyDataMapper::SafeDownCast(surface.actor->GetMapper());
      if (surface.nodes) {
        surface.nodes->GetMapper()->ScalarVisibilityOff();
      }
      if (!c.field || !surface.scalars ||
          (c.range_mode == ColorRangeMode::AllFrames && actualRange && !rangeAcrossFrames)) {
        mapper->ScalarVisibilityOff();
        mapper->SetInputData(surface.base);
        continue;
      }
      const bool cell = c.field->association == SimResults::Association::Element;
      mapper->SetInputData(surface.scalars);
      if (cell) {
        mapper->SetScalarModeToUseCellFieldData();
      }
      else {
        mapper->SetScalarModeToUsePointFieldData();
      }
      mapper->SelectColorArray(scalarName);
      mapper->SetColorModeToMapScalars();
      mapper->SetLookupTable(mapping);
      mapper->UseLookupTableScalarRangeOn();
      mapper->InterpolateScalarsBeforeMappingOn();
      mapper->ScalarVisibilityOn();
      if (surface.nodes && !cell) {
        auto* nodeMapper = vtkPolyDataMapper::SafeDownCast(surface.nodes->GetMapper());
        nodeMapper->SetScalarModeToUsePointFieldData();
        nodeMapper->SelectColorArray(scalarName);
        nodeMapper->SetColorModeToMapScalars();
        nodeMapper->SetLookupTable(mapping);
        nodeMapper->UseLookupTableScalarRangeOn();
        nodeMapper->ScalarVisibilityOn();
      }
    }
    auto table = vtkSmartPointer<vtkLookupTable>::New();
    unsigned int bands = 256;
    if (c.bands) {
      bands = *c.bands;
    }
    table->SetNumberOfTableValues(bands);
    auto legendRange = range;
    if (legendRange[0] == legendRange[1]) {
      legendRange[0] = std::nextafter(range[0], -std::numeric_limits<double>::infinity());
      legendRange[1] = std::nextafter(range[1], std::numeric_limits<double>::infinity());
      if (!std::isfinite(legendRange[0])) {
        legendRange[0] = range[0];
      }
      if (!std::isfinite(legendRange[1])) {
        legendRange[1] = range[1];
      }
    }
    table->SetTableRange(legendRange.data());
    table->Build();
    auto paletteOnly = c;
    paletteOnly.minimum.enabled = false;
    paletteOnly.maximum.enabled = false;
    const auto count = table->GetNumberOfTableValues();
    for (vtkIdType i = 0; i < count; ++i) {
      const auto rgb = result_color(static_cast<double>(i)/(count-1),paletteOnly,{0,1});
      table->SetTableValue(i,rgb[0],rgb[1],rgb[2],1);
    }
    legend->SetLookupTable(table);
    legend->SetMaximumNumberOfColors(static_cast<int>(count));
    const bool rangeAvailable = actualRange.has_value() && (c.range_mode != ColorRangeMode::AllFrames || rangeAcrossFrames.has_value());
    std::string caption = title;
    if (!rangeAvailable) {
      caption += " (unavailable)";
    }
    if (display.input.owner == OutputOwner::Feature && c.range_mode == ColorRangeMode::AllFrames) {
      caption += "\nSource all frames";
    }
    legend->SetTitle(caption.c_str());
    legend->SetDrawColorBar(rangeAvailable);
    legend->SetDrawTickLabels(rangeAvailable);
    auto labelLimit = [](vtkTextActor* actor,const ColorLimit& limit,const char* direction) {
      std::ostringstream text;
      text << std::setprecision(10) << direction << limit.value;
      actor->SetInput(text.str().c_str());
      actor->GetTextProperty()->SetBackgroundColor(limit.color[0],limit.color[1],limit.color[2]);
      actor->GetTextProperty()->SetBackgroundOpacity(1);
      double ink = 1.0;
      if ((limit.color[0] + limit.color[1] + limit.color[2]) > 1.5) {
        ink = 0.0;
      }
      actor->GetTextProperty()->SetColor(ink,ink,ink);
    };
    labelLimit(minimumLabel,c.minimum,"Below ");
    labelLimit(maximumLabel,c.maximum,"Above ");
    applied = c;
    allFrameRange = rangeAcrossFrames;
    hiddenDatasets = display.hidden_datasets;
    appliedData = &data;
    appliedFrame = display.increment_index;
    appliedDisplayName = display.name;
    legend->Modified();
  }
  update_appearance();
  const bool visible = display.visible && c.field && c.legend_visible &&
    std::any_of(surfaces.begin(),surfaces.end(),[&](const PostprocessorColorSurface& surface) {
      return dataset_visible(display,surface.dataset) && (!c.field->dataset_id || *c.field->dataset_id == surface.dataset);
    });
  legend->SetVisibility(visible);
  const bool valuesAvailable = actualRange.has_value() && (c.range_mode != ColorRangeMode::AllFrames || allFrameRange.has_value());
  minimumLabel->SetVisibility(visible && valuesAvailable && c.minimum.enabled);
  maximumLabel->SetVisibility(visible && valuesAvailable && c.maximum.enabled);
}

std::array<double,4> PostprocessorColorMap::legend_frame() const
{
  const auto* position = legend->GetPosition();
  return {position[0],position[1],legend->GetWidth(),legend->GetHeight()};
}
bool PostprocessorColorMap::legend_contains(double x,double y) const
{
  if (!legend->GetVisibility())
  {
    return false;
  }
  const auto frame = legend_frame();
  double left = frame[0], right = frame[0]+frame[2];
  // Unconstrained titles can extend horizontally beyond the scalar-bar frame.
  int bounds[4];
  auto* text = vtkTextRenderer::GetInstance();
  auto* window = renderer->GetRenderWindow();
  if (text && window && legend->GetTitle() &&
      text->GetBoundingBox(legend->GetTitleTextProperty(),legend->GetTitle(),bounds,window->GetDPI()))
  {
    const double halfWidth = 0.5*(bounds[1]-bounds[0]+1)/std::max(1,renderer->GetSize()[0]);
    const double center = frame[0]+0.5*frame[2];
    left = std::min(left,center-halfWidth);
    right = std::max(right,center+halfWidth);
  }
  return x >= left && x <= right && y >= frame[1] && y <= frame[1]+frame[3];
}
void PostprocessorColorMap::update_appearance()
{
  const int viewportWidth = std::max(1,renderer->GetSize()[0]);
  const int viewportHeight = std::max(1,renderer->GetSize()[1]);
  const double legendWidth = std::min(0.8,Postprocessor::ColoringDefaults::legend_width_pixels*captureScale/viewportWidth);
  const double legendHeight = std::min(0.7,Postprocessor::ColoringDefaults::legend_height_pixels*captureScale/viewportHeight);
  double x = std::max(0.02,0.98-legendWidth-(legendWidth+0.02)*appliedSlot);
  double y = 0.06;
  if (applied && applied->legend_position)
  {
    x = (*applied->legend_position)[0];
    y = (*applied->legend_position)[1];
  }
  x = std::clamp(x,0.0,1.0-legendWidth);
  y = std::clamp(y,0.0,1.0-legendHeight);
  legend->SetWidth(legendWidth);
  legend->SetHeight(legendHeight);
  legend->SetPosition(x,y);
  minimumLabel->SetPosition(x,std::max(0.0,y-0.05));
  maximumLabel->SetPosition(x,std::max(0.0,y-0.025));
  // Scene appearance is applied after CPU-side source synchronization. Refresh
  // text independently of coloring caches so initial white backgrounds cannot
  // retain text colors chosen against VTK's default black background.
  const auto* background = renderer->GetBackground();
  const auto* top = background;
  if (renderer->GetGradientBackground()) {
    top = renderer->GetBackground2();
  }
  const double brightness = (background[0]+background[1]+background[2]+top[0]+top[1]+top[2])/6.0;
  double ink = 1.0;
  if (brightness > 0.5) {
    ink = 0.0;
  }
  for (auto* text : {legend->GetTitleTextProperty(),legend->GetLabelTextProperty()}) {
    text->SetColor(ink,ink,ink);
    text->SetOpacity(1.0);
    text->ShadowOff();
    // A light/dark backing keeps labels readable across gradient backgrounds.
    text->SetBackgroundColor(1.0-ink,1.0-ink,1.0-ink);
    double opacity = 0.0;
    if (renderer->GetGradientBackground()) {
      opacity = 0.75;
    }
    text->SetBackgroundOpacity(opacity);
  }
}

void PostprocessorColorMap::set_capture_scale(double scale) {
  const double ratio = scale/captureScale;
  for (auto* text : {legend->GetTitleTextProperty(),legend->GetLabelTextProperty(),
      minimumLabel->GetTextProperty(),maximumLabel->GetTextProperty()}) {
    text->SetFontSize(std::max(1,static_cast<int>(std::lround(text->GetFontSize()*ratio))));
  }
  captureScale = scale;
  update_appearance();
}
