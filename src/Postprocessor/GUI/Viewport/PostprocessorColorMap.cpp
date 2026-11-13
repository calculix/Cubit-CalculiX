#include "PostprocessorColorMap.hpp"
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
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
namespace
{
  constexpr const char* scalarName = "__CubitSimDisplayScalar";
}
PostprocessorColorMap::PostprocessorColorMap(vtkRenderer* target) : renderer(target)
{
  legend = vtkSmartPointer<vtkScalarBarActor>::New();
  legend->SetWidth(0.12);
  legend->SetHeight(0.65);
  legend->SetNumberOfLabels(5);
  legend->SetLabelFormat("%.6g");
  legend->UnconstrainedFontSizeOn();
  legend->GetTitleTextProperty()->SetFontSize(12);
  legend->GetLabelTextProperty()->SetFontSize(12);
  legend->GetTitleTextProperty()->SetOpacity(1.0);
  legend->GetLabelTextProperty()->SetOpacity(1.0);
  legend->SetVisibility(false);
  renderer->AddActor2D(legend);
  minimumLabel = vtkSmartPointer<vtkTextActor>::New();
  maximumLabel = vtkSmartPointer<vtkTextActor>::New();
  for (auto* label : {minimumLabel.GetPointer(),maximumLabel.GetPointer()})
  {
    label->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    label->GetTextProperty()->SetFontSize(12);
    label->SetVisibility(false);
    renderer->AddActor2D(label);
  }
}
PostprocessorColorMap::~PostprocessorColorMap()
{
  renderer->RemoveActor2D(legend);
  renderer->RemoveActor2D(minimumLabel);
  renderer->RemoveActor2D(maximumLabel);
}
void PostprocessorColorMap::apply(const Postprocessor::Representation& display, const SimResults::VtkResultsData& data,
  std::vector<PostprocessorColorSurface>& surfaces, unsigned int slot,
  const std::optional<std::array<double,2>>& rangeAcrossFrames, std::string& error)
{
  using namespace Postprocessor;
  const auto& c = display.coloring;
  const bool inputChanged = appliedData != &data || appliedFrame != display.increment_index ||
    appliedDisplayName != display.name;
  const bool changedField = inputChanged || !applied || !same_field(applied->field,c.field);
  appliedSlot = slot;
  if (inputChanged || !applied || !same_coloring(*applied, c) || allFrameRange != rangeAcrossFrames || hiddenDatasets != display.hidden_datasets)
  {
    if (changedField)
    {
      actualRange.reset();
      title = display.name;
      const auto* field = c.field && display.increment_index < data.increments.size() ?
        find_field(data.increments[display.increment_index], *c.field) : nullptr;
      for (auto& surface : surfaces)
      {
        surface.scalars = nullptr;
        surface.range.reset();
        if (surface.nodes)
        {
          surface.nodes->GetMapper()->ScalarVisibilityOff();
          surface.node_data->GetPointData()->RemoveArray(scalarName);
        }
        auto* mapper = vtkPolyDataMapper::SafeDownCast(surface.actor->GetMapper());
        mapper->ScalarVisibilityOff();
        mapper->SetInputData(surface.base);
        if (!c.field || (c.field->dataset_id && surface.dataset != *c.field->dataset_id))
        {
          continue;
        }
        auto selected = *c.field;
        selected.dataset_id = surface.dataset;
        const auto* partField = find_field(data.increments[display.increment_index],selected);
        if (!partField)
        {
          continue;
        }
        std::vector<double> values;
        std::string valueError;
        if (!color_values(surface.input, *partField, selected, values, valueError))
        {
          error += (error.empty() ? "" : "\n")+display.name+": "+valueError;
          continue;
        }
        for (double v : values)
        {
          if (std::isfinite(v))
          {
            if (!surface.range)
            {
              surface.range = std::array<double,2>{v,v};
            }
            else
            {
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
        for (std::size_t i = 0; i < values.size(); ++i)
        {
          scalar->SetValue(static_cast<vtkIdType>(i), values[i]);
        }
        if (partField->association == SimResults::Association::Element)
        {
          input->GetCellData()->AddArray(scalar);
        }
        else
        {
          input->GetPointData()->AddArray(scalar);
          if (surface.node_data)
          {
            surface.node_data->GetPointData()->AddArray(scalar);
          }
        }
        auto filter = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
        Postprocessor::configure_pick_provenance(filter);
        filter->SetInputData(input);
        filter->Update();
        surface.scalars = vtkSmartPointer<vtkPolyData>::New();
        surface.scalars->ShallowCopy(filter->GetOutput());
      }
      if (field)
      {
        title += "\n" + field->name + " / " + (c.field->magnitude ? "Magnitude" : c.field->component);
      }
    }
    // Visibility changes only recombine cached per-surface extrema; geometry and
    // scalar arrays remain intact while the selected field is unchanged.
    actualRange.reset();
    for (const auto& surface : surfaces)
    {
      if (display.hidden_datasets.count(surface.dataset) != 0 || !surface.range)
      {
        continue;
      }
      if (!actualRange)
      {
        actualRange = surface.range;
      }
      else
      {
        (*actualRange)[0] = std::min((*actualRange)[0],(*surface.range)[0]);
        (*actualRange)[1] = std::max((*actualRange)[1],(*surface.range)[1]);
      }
    }
    std::array<double,2> range = c.fixed_range;
    if (c.range_mode == ColorRangeMode::AllFrames && rangeAcrossFrames)
    {
      range = *rangeAcrossFrames;
    }
    else if (c.range_mode != ColorRangeMode::Fixed && c.range_mode != ColorRangeMode::AllFrames && actualRange)
    {
      range = *actualRange;
      if (c.range_mode == ColorRangeMode::Symmetric)
      {
        const auto m = std::max(std::abs(range[0]), std::abs(range[1]));
        range = {-m,m};
      }
    }
    // Map interpolated scalars, not interpolated RGB, so palette bands and
    // limit colors survive within nodal faces. The table resolution is finite.
    auto mappingRange = range;
    for (const auto& limit : {c.minimum,c.maximum})
    {
      if (limit.enabled && actualRange && limit.value > (*actualRange)[0] && limit.value < (*actualRange)[1])
      {
        mappingRange[0] = std::min(mappingRange[0],limit.value);
        mappingRange[1] = std::max(mappingRange[1],limit.value);
      }
    }
    if (mappingRange[0] == mappingRange[1])
    {
      const auto pad = std::max(1.0,std::abs(mappingRange[0])*1e-6);
      const auto low = mappingRange[0]-pad;
      const auto high = mappingRange[1]+pad;
      if (std::isfinite(low))
      {
        mappingRange[0] = low;
      }
      if (std::isfinite(high))
      {
        mappingRange[1] = high;
      }
    }
    auto mapping = vtkSmartPointer<vtkLookupTable>::New();
    mapping->SetNumberOfTableValues(4096);
    mapping->SetTableRange(mappingRange.data());
    mapping->SetNanColor(ColoringDefaults::missing[0],ColoringDefaults::missing[1],ColoringDefaults::missing[2],1);
    mapping->Build();
    for (vtkIdType i = 0; i < 4096; ++i)
    {
      const long double t = static_cast<long double>(i)/4095;
      const double value = actualRange && (*actualRange)[0] == (*actualRange)[1] ? (*actualRange)[0] :
        static_cast<double>((1-t)*mappingRange[0]+t*mappingRange[1]);
      const auto rgb = result_color(value,c,range);
      mapping->SetTableValue(i,rgb[0],rgb[1],rgb[2],1);
    }
    for (const auto& limit : {c.minimum,c.maximum})
    {
      if (limit.enabled && limit.value >= mappingRange[0] && limit.value <= mappingRange[1])
      {
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
    for (auto& surface : surfaces)
    {
      auto* mapper = vtkPolyDataMapper::SafeDownCast(surface.actor->GetMapper());
      if (surface.nodes)
      {
        surface.nodes->GetMapper()->ScalarVisibilityOff();
      }
      if (!c.field || !surface.scalars ||
          (c.range_mode == ColorRangeMode::AllFrames && actualRange && !rangeAcrossFrames))
      {
        mapper->ScalarVisibilityOff();
        mapper->SetInputData(surface.base);
        continue;
      }
      const bool cell = c.field->association == SimResults::Association::Element;
      mapper->SetInputData(surface.scalars);
      if (cell)
      {
        mapper->SetScalarModeToUseCellFieldData();
      }
      else
      {
        mapper->SetScalarModeToUsePointFieldData();
      }
      mapper->SelectColorArray(scalarName);
      mapper->SetColorModeToMapScalars();
      mapper->SetLookupTable(mapping);
      mapper->UseLookupTableScalarRangeOn();
      mapper->InterpolateScalarsBeforeMappingOn();
      mapper->ScalarVisibilityOn();
      if (surface.nodes && !cell)
      {
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
    table->SetNumberOfTableValues(c.bands ? *c.bands : 256);
    auto legendRange = range;
    if (legendRange[0] == legendRange[1])
    {
      legendRange[0] = std::nextafter(range[0], -std::numeric_limits<double>::infinity());
      legendRange[1] = std::nextafter(range[1], std::numeric_limits<double>::infinity());
      if (!std::isfinite(legendRange[0]))
      {
        legendRange[0] = range[0];
      }
      if (!std::isfinite(legendRange[1]))
      {
        legendRange[1] = range[1];
      }
    }
    table->SetTableRange(legendRange.data());
    table->Build();
    auto paletteOnly = c;
    paletteOnly.minimum.enabled = false;
    paletteOnly.maximum.enabled = false;
    const auto count = table->GetNumberOfTableValues();
    for (vtkIdType i = 0; i < count; ++i)
    {
      const auto rgb = result_color(static_cast<double>(i)/(count-1),paletteOnly,{0,1});
      table->SetTableValue(i,rgb[0],rgb[1],rgb[2],1);
    }
    legend->SetLookupTable(table);
    legend->SetMaximumNumberOfColors(static_cast<int>(count));
    const bool rangeAvailable = actualRange.has_value() && (c.range_mode != ColorRangeMode::AllFrames || rangeAcrossFrames.has_value());
    const auto caption = title + (rangeAvailable ? "" : " (unavailable)");
    legend->SetTitle(caption.c_str());
    legend->SetDrawColorBar(rangeAvailable);
    legend->SetDrawTickLabels(rangeAvailable);
    auto labelLimit = [](vtkTextActor* actor,const ColorLimit& limit,const char* direction)
    {
      std::ostringstream text;
      text << std::setprecision(10) << direction << limit.value;
      actor->SetInput(text.str().c_str());
      actor->GetTextProperty()->SetBackgroundColor(limit.color[0],limit.color[1],limit.color[2]);
      actor->GetTextProperty()->SetBackgroundOpacity(1);
      const double ink = (limit.color[0]+limit.color[1]+limit.color[2]) > 1.5 ? 0.0 : 1.0;
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
    std::any_of(surfaces.begin(),surfaces.end(),[&](const PostprocessorColorSurface& surface)
    {
      return dataset_visible(display,surface.dataset) && (!c.field->dataset_id || *c.field->dataset_id == surface.dataset);
    });
  legend->SetVisibility(visible);
  const bool valuesAvailable = actualRange.has_value() && (c.range_mode != ColorRangeMode::AllFrames || allFrameRange.has_value());
  minimumLabel->SetVisibility(visible && valuesAvailable && c.minimum.enabled);
  maximumLabel->SetVisibility(visible && valuesAvailable && c.maximum.enabled);
}

void PostprocessorColorMap::update_appearance()
{
  const int viewportWidth = std::max(1,renderer->GetSize()[0]);
  const double legendWidth = std::clamp(150.0*captureScale/viewportWidth,0.12,0.45);
  const double x = std::max(0.02,0.98-legendWidth-(legendWidth+0.02)*appliedSlot);
  legend->SetWidth(legendWidth);
  legend->SetPosition(x, 0.22);
  minimumLabel->SetPosition(x, 0.04);
  maximumLabel->SetPosition(x, 0.10);
  // Scene appearance is applied after CPU-side source synchronization. Refresh
  // text independently of coloring caches so initial white backgrounds cannot
  // retain text colors chosen against VTK's default black background.
  const auto* background = renderer->GetBackground();
  const auto* top = renderer->GetGradientBackground() ? renderer->GetBackground2() : background;
  const double brightness = (background[0]+background[1]+background[2]+top[0]+top[1]+top[2])/6.0;
  const double ink = brightness > 0.5 ? 0.0 : 1.0;
  for (auto* text : {legend->GetTitleTextProperty(),legend->GetLabelTextProperty()})
  {
    text->SetColor(ink,ink,ink);
    text->SetOpacity(1.0);
    text->ShadowOff();
    // A light/dark backing keeps labels readable across gradient backgrounds.
    text->SetBackgroundColor(1.0-ink,1.0-ink,1.0-ink);
    text->SetBackgroundOpacity(renderer->GetGradientBackground() ? 0.75 : 0.0);
  }
}

void PostprocessorColorMap::set_capture_scale(double scale)
{
  const double ratio = scale/captureScale;
  for (auto* text : {legend->GetTitleTextProperty(),legend->GetLabelTextProperty(),
      minimumLabel->GetTextProperty(),maximumLabel->GetTextProperty()})
  {
    text->SetFontSize(std::max(1,static_cast<int>(std::lround(text->GetFontSize()*ratio))));
  }
  captureScale = scale;
  update_appearance();
}
