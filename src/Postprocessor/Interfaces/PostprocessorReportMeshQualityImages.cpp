#include <utility>
#include "PostprocessorReportMeshQualityImages.hpp"
#include "PostprocessorApplicationInterface.hpp"
#include "Framework/MeshQuality/MeshQualitySnapshot.hpp"
#include "Framework/MeshQuality/MeshQualitySettings.hpp"
#include "Framework/MeshQuality/MeshQualityStatistics.hpp"
#include "Framework/MeshQuality/MeshQualityPresentation.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkSmartPointer.h>
#include <vtkCellType.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
namespace
{
  vtkSmartPointer<vtkUnstructuredGrid> quality_grid(const MeshQualityBlock& block,
    vtkPoints* points, const std::map<std::int64_t, vtkIdType>& nodes, const std::vector<double>& values,
    const MeshQualityMetric& metric, bool failingOnly, ReportProgress* progress, std::string& error)
  {
    auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->SetPoints(points);
    int corners = 3, type = VTK_TRIANGLE;
    if (block.family == "hex")
    {
      corners = 8;
      type = VTK_HEXAHEDRON;
    }
    else if (block.family == "tet")
    {
      corners = 4;
      type = VTK_TETRA;
    }
    else if (block.family == "quad")
    {
      corners = 4;
      type = VTK_QUAD;
    }
    auto scalars = vtkSmartPointer<vtkDoubleArray>::New();
    scalars->SetName("MeshQuality");
    auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
    ids->SetName("OriginalElementId");
    grid->Allocate(static_cast<vtkIdType>(block.element_ids.size()));
    for (std::size_t index = 0; index < block.element_ids.size(); ++index)
    {
      if (index % 2048 == 0 && !ReportProgress::advance(progress, ReportProgressStage::Images,
            "Composing quality mesh / block " + std::to_string(block.block_id), index, block.element_ids.size(), error))
      {
        return {};
      }
      if (failingOnly && !MeshQualityStatistics::outside(values[index], metric))
      {
        continue;
      }
      vtkIdType connectivity[8];
      for (int node = 0; node < corners; ++node)
      {
        connectivity[node] = nodes.at(block.connectivity[index * block.nodes_per_element + node]);
      }
      grid->InsertNextCell(type, corners, connectivity);
      scalars->InsertNextValue(values[index]);
      ids->InsertNextValue(block.element_ids[index]);
    }
    grid->GetCellData()->AddArray(scalars);
    grid->GetCellData()->AddArray(ids);
    return grid;
  }
}
PostprocessorReportMeshQualityImages::PostprocessorReportMeshQualityImages() = default;
bool PostprocessorReportMeshQualityImages::collect(const PostprocessorApplicationInterface& application,
  const ReportImageRequest& request, ReportDocument& output, std::string& error) const
{
  if (!request.mesh_quality || !request.mesh_quality->validate(error))
  {
    if (error.empty())
    {
      error = "The mesh quality image snapshot is unavailable.";
    }
    return false;
  }
  MeshQualitySettings settings;
  if (!MeshQualitySettings::decode(request.settings, settings, error))
  {
    return false;
  }
  const auto& mesh = *request.mesh_quality;
  Postprocessor::CaptureDefinition definition;
  const auto option = [&](const std::string& key)
  {
    const auto found = request.settings.values.find(key);
    if (found != request.settings.values.end())
    {
      return found->second;
    }
    return std::string();
  };
  definition.name = option("post.images.view");
  if (request.width != 0 || request.height != 0)
  {
    definition.width = request.width;
    definition.height = request.height;
  }
  Postprocessor::CaptureSnapshot presentation;
  if (!application.capture_snapshot(definition, presentation, error))
  {
    return false;
  }
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToDouble();
  std::map<std::int64_t, vtkIdType> nodes;
  auto nodeIds = vtkSmartPointer<vtkTypeInt64Array>::New();
  nodeIds->SetName("OriginalNodeId");
  for (std::size_t index = 0; index < mesh.node_ids.size(); ++index)
  {
    if (index % 2048 == 0 && !ReportProgress::advance(request.progress, ReportProgressStage::Images,
          "Preparing quality image coordinates", index, mesh.node_ids.size(), error))
    {
      return false;
    }
    nodes.emplace(mesh.node_ids[index], points->InsertNextPoint(mesh.coordinates[index].data()));
    nodeIds->InsertNextValue(mesh.node_ids[index]);
  }
  ReportDocument candidate = output;
  auto root = std::find_if(candidate.sections.begin(), candidate.sections.end(), [](const auto& section)
  {
    return section.id == "mesh.quality";
  });
  if (root == candidate.sections.end())
  {
    error = "The quality image section has not been prepared.";
    return false;
  }
  std::size_t completed = 0;
  const auto total = mesh.blocks.size() * settings.metrics.size() * 2;
  for (const auto& block : mesh.blocks)
  {
    if (block.nodes_per_element < 3 || (block.family == "hex" && block.nodes_per_element < 8) ||
        ((block.family == "tet" || block.family == "quad") && block.nodes_per_element < 4))
    {
      candidate.diagnostics.emplace_back("mesh.quality", "unavailable", "Unsupported corner topology for quality image.");
      continue;
    }
    const auto sectionId = "mesh.quality.block." + std::to_string(block.block_id) + "." + block.family + "." + std::to_string(block.nodes_per_element);
    auto section = std::find_if(root->children.begin(), root->children.end(), [&](const auto& value)
    {
      return value.id == sectionId;
    });
    if (section == root->children.end())
    {
      continue;
    }
    Postprocessor::DatasetAppearance appearance;
    std::map<std::string, Postprocessor::DatasetAppearance> appearances;
    // Match by original model block, never by a temporary quality dataset index.
    for (const auto& entry : presentation.session.representations)
    {
      if (entry.second.view_id != presentation.target_id || !entry.second.visible)
      {
        continue;
      }
      auto input = entry.second.input;
      std::set<std::string> visited;
      while (input.owner == Postprocessor::OutputOwner::Feature)
      {
        const auto filter = presentation.session.filters.find(input.object_id);
        if (filter == presentation.session.filters.end() || !visited.insert(input.object_id).second)
        {
          input.object_id.clear();
          break;
        }
        input = filter->second.input;
      }
      const auto source = presentation.sources.find(input.object_id);
      if (source == presentation.sources.end() || (request.source.kind == "job" &&
          source->second->solver_launch_id != request.source.solver_launch_id))
      {
        continue;
      }
      for (const auto& dataset : source->second->datasets)
      {
        if (dataset.kind == "Blocks" && dataset.model_id == block.block_id)
        {
          auto style = Postprocessor::default_appearance(dataset);
          const auto stored = entry.second.dataset_appearance.find(dataset.id);
          if (stored != entry.second.dataset_appearance.end())
          {
            style = stored->second;
          }
          appearances.emplace(entry.first, style);
        }
      }
    }
    const auto preferred = appearances.find(presentation.session.active_representation_id);
    if (preferred != appearances.end())
    {
      appearance = preferred->second;
    }
    else if (appearances.size() == 1)
    {
      appearance = appearances.begin()->second;
    }
    for (const auto& configured : settings.metrics)
    {
      const auto measured = block.values.find(configured.id);
      if (configured.family != block.family || measured == block.values.end())
      {
        continue;
      }
      if (configured.quantity != "dimensionless" && mesh.coordinate_unit.empty())
      {
        continue; // The model provider already recorded the missing-unit diagnostic.
      }
      MeshQualityPresentation qualityPresentation;
      if (!qualityPresentation.prepare(configured, mesh.coordinate_unit, request.settings, measured->second,
            request.progress, ReportProgressStage::Images, error))
      {
        return false;
      }
      const auto& metric = qualityPresentation.metric;
      const auto& samples = qualityPresentation.values(measured->second);
      MeshQualityStatistics statistics;
      if (!statistics.calculate(block.element_ids, samples, metric, error))
      {
        return false;
      }
      for (bool failingOnly : {false, true})
      {
        if ((!failingOnly && !settings.images) ||
            (failingOnly && (!settings.failed_images || statistics.failed == 0)))
        {
          continue;
        }
        if (!ReportProgress::advance(request.progress, ReportProgressStage::Images,
              "Rendering quality / block " + std::to_string(block.block_id) + " / " + metric.label,
              completed, total, error))
        {
          return false;
        }
        auto snapshot = presentation;
        const std::string sourceId = "report.mesh_quality";
        auto data = std::make_shared<SimResults::VtkResultsData>();
        data->result_identity = mesh.mesh_fingerprint;
        data->layout = SimResults::DatasetLayout::Collection;
        data->source_units.coordinate_unit = mesh.coordinate_unit;
        auto collection = std::shared_ptr<vtkPartitionedDataSetCollection>(vtkPartitionedDataSetCollection::New(), [](auto* object)
        {
          object->Delete();
        });
        auto grid = quality_grid(block, points, nodes, samples, metric, false, request.progress, error);
        if (!grid)
        {
          return false;
        }
        grid->GetPointData()->AddArray(nodeIds);
        auto part = vtkSmartPointer<vtkPartitionedDataSet>::New();
        part->SetPartition(0, grid);
        collection->SetPartitionedDataSet(0, part);
        SimResults::Dataset dataset;
        dataset.id = 0;
        dataset.name = block.block_name;
        dataset.kind = "Blocks";
        dataset.model_id = block.block_id;
        data->datasets.push_back(dataset);
        SimResults::Increment frame;
        frame.coordinate_unit = mesh.coordinate_unit;
        frame.step = 1;
        frame.dataset = collection;
        SimResults::Field field;
        field.name = metric.label;
        field.array_name = "MeshQuality";
        field.association = SimResults::Association::Element;
        field.components = {"Value"};
        field.component_units = {qualityPresentation.quantity};
        if (failingOnly)
        {
          auto failures = quality_grid(block, points, nodes, samples, metric, true, request.progress, error);
          if (!failures)
          {
            return false;
          }
          failures->GetPointData()->AddArray(nodeIds);
          auto failurePart = vtkSmartPointer<vtkPartitionedDataSet>::New();
          failurePart->SetPartition(0, failures);
          collection->SetPartitionedDataSet(1, failurePart);
          dataset.id = 1;
          dataset.name += " failing elements";
          data->datasets.push_back(dataset);
          field.dataset_id = 1;
        }
        frame.fields = {field};
        data->increments.push_back(std::move(frame));
        Postprocessor::Representation display;
        display.id = "report.quality";
        display.view_id = snapshot.target_id;
        display.input.object_id = sourceId;
        display.dataset_appearance[field.dataset_id] = appearance;
        display.coloring.palette = Postprocessor::ColorPalette::Viridis;
        display.coloring.range_mode = Postprocessor::ColorRangeMode::Automatic;
        display.coloring.legend_visible = true;
        display.coloring.bounds_unit = field.component_units.front();
        Postprocessor::FieldSelection selection;
        selection.dataset_id = field.dataset_id;
        selection.array_name = field.array_name;
        selection.association = field.association;
        selection.component = "Value";
        display.coloring.field = selection;
        if (metric.minimum && metric.maximum && *metric.minimum < *metric.maximum)
        {
          display.coloring.range_mode = Postprocessor::ColorRangeMode::Fixed;
          display.coloring.fixed_range = {*metric.minimum, *metric.maximum};
        }
        if (metric.minimum)
        {
          display.coloring.minimum.enabled = true;
          display.coloring.minimum.value = *metric.minimum;
          display.coloring.minimum.color = {1, 0, 0};
        }
        if (metric.maximum)
        {
          display.coloring.maximum.enabled = true;
          display.coloring.maximum.value = *metric.maximum;
          display.coloring.maximum.color = {1, 0, 0};
        }
        auto view = snapshot.session.views.at(snapshot.target_id);
        if (view.scene_unit.empty() || option("post.images.camera") == "fit")
        {
          view.camera.reset();
          view.scene_unit = mesh.coordinate_unit;
        }
        view.fit_pending = false;
        view.plot_active = false;
        view.plot_id.reset();
        view.table.reset();
        view.explosion = {};
        view.annotations.labels.clear();
        view.annotations.visible = true;
        view.annotations.extrema_visible = true;
        snapshot.session.sources.clear();
        snapshot.session.sources[sourceId] = Postprocessor::DataSource{sourceId, "Mesh quality", {}};
        snapshot.sources.clear();
        snapshot.sources[sourceId] = data;
        snapshot.session.filters.clear();
        snapshot.session.plots.clear();
        snapshot.session.active_plot_id.clear();
        snapshot.session.views.clear();
        snapshot.session.views[snapshot.target_id] = view;
        snapshot.session.representations.clear();
        if (failingOnly)
        {
          auto context = display;
          context.id = "report.quality.context";
          context.hidden_datasets.insert(1);
          context.coloring.field.reset();
          context.dataset_appearance.clear();
          auto& style = context.dataset_appearance[0];
          style.style = Postprocessor::DisplayStyle::Transparent;
          style.opacity = .1;
          style.solid_color = std::array<double, 3>{.7, .7, .7};
          snapshot.session.representations[context.id] = context;
          display.hidden_datasets.insert(0);
        }
        snapshot.session.representations[display.id] = display;
        snapshot.session.active_source_id = sourceId;
        snapshot.session.active_representation_id = display.id;
        if (!framework_units().workspace_context("report", snapshot.presentation_units, error))
        {
          return false;
        }
        if (request.settings.units)
        {
          request.settings.units->apply_to(snapshot.presentation_units);
        }
        snapshot.presentation_units.workspace = "postprocessor";
        if (option("post.images.background") == "white")
        {
          snapshot.request.background = Postprocessor::CaptureBackground::White;
          snapshot.options.backgroundTop = snapshot.options.backgroundBottom = {1, 1, 1};
        }
        else if (option("post.images.background") == "transparent")
        {
          snapshot.request.background = Postprocessor::CaptureBackground::Transparent;
        }
        snapshot.provenance = QJsonObject{{"report_capture", QJsonObject{{"kind", "mesh_quality"}}},
          {"mesh_fingerprint", QString::fromStdString(mesh.mesh_fingerprint)}, {"block", block.block_id},
          {"metric", QString::fromStdString(metric.id)}, {"failing_only", failingOnly},
          {"geometry", "undeformed corner topology"}, {"evaluator", QString::fromStdString(mesh.evaluator_version)}};
        QJsonObject limits{{"unit", QString::fromStdString(metric.unit)}};
        if (metric.minimum)
        {
          limits.insert("minimum", *metric.minimum);
        }
        if (metric.maximum)
        {
          limits.insert("maximum", *metric.maximum);
        }
        snapshot.provenance.insert("acceptance_limits", limits);
        ReportImage image;
        std::string captureError;
        if (!application.render_report_result(snapshot, image, captureError))
        {
          candidate.diagnostics.emplace_back(sectionId, "unavailable", metric.label + " image: " + captureError);
          ++completed;
          continue;
        }
        image.id = sectionId + "." + metric.id;
        image.caption = metric.label + " / block " + std::to_string(block.block_id) + " / native " + block.family +
          " " + std::to_string(block.nodes_per_element) + "-node metric / corner-topology overview";
        if (failingOnly)
        {
          image.id += ".failures";
          image.caption += " / failing elements with transparent model context";
        }
        section->images.push_back(std::move(image));
        ++completed;
      }
    }
  }
  if (!ReportProgress::advance(request.progress, ReportProgressStage::Images,
        "Quality images complete", completed, completed, error))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
