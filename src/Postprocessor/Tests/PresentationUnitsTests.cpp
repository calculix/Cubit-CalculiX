#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/plots/PlotAxisUnits.hpp"
#include "Postprocessor/Core/plots/PlotCodec.hpp"
#include "Postprocessor/Core/persistence/PostprocessorSessionCodec.hpp"
#include <limits>
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include <QApplication>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkCellType.h>
#include <vtkSmartPointer.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
namespace
{
  int failures = 0;
  bool check(bool value, const char* message)
  {
    if (!value)
    {
      ++failures;
      std::cerr << message << '\n';
    }
    return value;
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = std::make_shared<SimResults::VtkResultsData>();
    Provider()
    {
      auto mesh = vtkSmartPointer<vtkUnstructuredGrid>::New();
      auto points = vtkSmartPointer<vtkPoints>::New();
      points->SetDataTypeToDouble();
      points->InsertNextPoint(0,0,0);
      points->InsertNextPoint(1,0,0);
      mesh->SetPoints(points);
      const vtkIdType nodes[] = {0,1};
      mesh->InsertNextCell(VTK_LINE,2,nodes);
      auto values = vtkSmartPointer<vtkDoubleArray>::New();
      values->SetName("STRESS");
      values->InsertNextValue(100);
      values->InsertNextValue(200);
      mesh->GetPointData()->AddArray(values);
      auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
      ids->SetName("OriginalNodeId");
      ids->InsertNextValue(1);
      ids->InsertNextValue(2);
      mesh->GetPointData()->AddArray(ids);
      SimResults::Increment frame;
      mesh->Register(nullptr);
      frame.dataset = std::shared_ptr<vtkDataObject>(mesh.GetPointer(),[](vtkDataObject* object)
      {
        object->Delete();
      });
      frame.coordinate_unit = "mm";
      frame.independent_variable = SimUnits::QuantityUnit("time","s",SimUnits::Time);
      SimResults::Field field;
      field.name = field.array_name = "STRESS";
      field.source = "VTK";
      field.components = {"XX"};
      field.component_units.emplace_back("stress","MPa",SimUnits::Stress);
      frame.fields.push_back(field);
      data->increments.push_back(frame);
      data->datasets.push_back({0,"Block_1","Blocks",1});
      data->job_id = 1;
      data->run_identity = "presentation-test";
      data->source_units.coordinate_unit = "mm";
    }
    std::string command_prefix() const override
    {
      return "test";
    }
    bool open(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = data->run_identity;
      output = {reference,"Presentation",data};
      error.clear();
      return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override
    {
      return open(request,output,error);
    }
  };
}
int main(int argc, char** argv)
{
  QApplication application(argc,argv);
  using namespace Postprocessor;
  std::string error;
  check(framework_units().set_display_unit("postprocessor","stress","MPa",error),"Cannot set initial stress unit");
  Provider provider;
  PostprocessorCore live;
  if (!check(live.open_result(provider,SimResults::JobSourceReference{"test","",1,""},std::nullopt,error),"Cannot open fixture"))
  {
    return EXIT_FAILURE;
  }
  const auto source = live.session().active_source_id;
  const auto display = live.session().active_representation_id;
  const auto field = *live.session().representations.at(display).coloring.field;
  EntityReference node;
  node.source_id = source;
  node.dataset_id = 0;
  node.kind = EntityKind::Node;
  node.entity_id = 1;
  ResultAnnotation label;
  label.name = "Probe";
  label.entity = node;
  label.field = field;
  std::string assigned;
  check(live.add_label("view1",label,assigned,error),"Cannot create labeled probe");
  check(live.set_labels("view1",true,true,error),"Cannot enable extrema");
  check(live.create_plot("History",error),"Cannot create plot");
  const auto plot = live.session().active_plot_id;
  PlotEntityReference history;
  history.entity = node;
  history.dataset_kind = "Blocks";
  history.dataset_name = "Block_1";
  history.model_id = 1;
  history.field = field;
  PlotCurveDefinition curve;
  curve.name = "Node 1";
  curve.input = history;
  check(live.add_plot_curve(plot,curve,error),"Cannot create history curve");
  PlotAxes manual;
  manual.y = std::array<double,2>{50,250};
  check(live.set_plot_axes(plot,manual,error),"Cannot bind manual ranges");
  check(live.set_scale_visible(true,error),"Cannot enable scale");
  check(live.session().views.at("view1").scene_unit == "mm","Initial scene unit was not bound");
  CaptureSnapshot snapshot;
  if (!check(live.capture_snapshot(CaptureDefinition(),snapshot,error),"Cannot freeze capture state"))
  {
    return EXIT_FAILURE;
  }
  PostprocessorCore frozen;
  frozen.install_capture_snapshot(snapshot);
  check(snapshot.provenance.contains("presentation_units"),"Capture provenance has no units");
  check(live.configure_units("","stress","Pa",error),"Cannot change live presentation units");
  PlotData currentPlot;
  check(live.plot_data(plot,currentPlot,error),"Cannot evaluate live plot in Pa");
  const auto convertedAxes = presented_plot_axes(live.session().plots.at(plot),currentPlot);
  check(convertedAxes.y && (*convertedAxes.y)[0] == 50e6 && (*convertedAxes.y)[1] == 250e6,
    "Manual range was reinterpreted after a unit change");
  std::map<std::string,PlotDefinition> decodedPlots;
  check(plots_from_json(plots_to_json(live.session().plots),decodedPlots) && decodedPlots.at(plot).axes.y_unit &&
    decodedPlots.at(plot).axes.y_unit->unit == "MPa","Plot range basis did not survive serialization");
  PostprocessorProjectState project, decodedProject;
  check(live.capture_project(project,error) && project_from_json(project_to_json(project,"."),".",decodedProject,error) &&
    decodedProject.session.views.at("view1").scene_unit == "mm" && decodedProject.session.views.at("view1").scale_visible,
    "Scene unit or scale visibility did not survive serialization");
  auto range = frozen.all_frame_range(source,field,error);
  check(range && (*range)[0] == 100 && (*range)[1] == 200,"Frozen color range followed live units");
  auto probe = frozen.probe_entity("view1",node,error);
  check(probe && probe->values && (*probe->values)[0] == 100 && probe->value_unit == "MPa","Frozen probe followed live units");
  PlotData plotted;
  check(frozen.plot_data(plot,plotted,error) && plotted.curves.size() == 1 &&
    plotted.curves[0].samples[0].y == 100 && plotted.curves[0].y_unit == "MPa","Frozen plot followed live units");
  const auto labels = frozen.annotations("view1");
  check(labels.size() == 3,"Frozen probe/extrema labels are missing");
  for (const auto& value : labels)
  {
    check(value.text.find("[MPa]") != std::string::npos,"Result/extrema label omitted frozen units");
  }
  check(!frozen.configure_units("","stress","Pa",error),"Frozen capture allowed its context to mutate");
  check(provider.data->increments[0].fields[0].component_units[0].unit == "MPa","Presentation changed source metadata");

  PlotDefinition definition;
  definition.curves.resize(2);
  definition.curves[0].name = "Stress Pa";
  definition.curves[1].name = "Stress MPa";
  PlotData data;
  data.curves.resize(2);
  for (auto& value : data.curves)
  {
    value.time_axis = true;
    value.samples.resize(1);
    value.samples[0].valid = true;
    value.x_scope = "same/time";
    value.y_scope = "same/stress";
  }
  data.curves[0].x_quantity = SimUnits::QuantityUnit("time","s",SimUnits::Time);
  data.curves[0].y_quantity = SimUnits::QuantityUnit("stress","Pa",SimUnits::Stress);
  data.curves[0].samples[0].x = .1;
  data.curves[0].samples[0].y = 100e6;
  data.curves[1].x_quantity = SimUnits::QuantityUnit("time","ms",SimUnits::Time);
  data.curves[1].y_quantity = SimUnits::QuantityUnit("stress","MPa",SimUnits::Stress);
  data.curves[1].samples[0].x = 100;
  data.curves[1].samples[0].y = 100;
  check(normalize_plot_axes(definition,data,error) && std::abs(data.curves[1].samples[0].x-.1) < 1e-12 &&
    data.curves[1].samples[0].y == 100e6,"Compatible axes were not normalized");
  data.curves[1].y_quantity = SimUnits::QuantityUnit("force","N",SimUnits::Force);
  const auto previous = data.curves[1].samples[0].y;
  check(!normalize_plot_axes(definition,data,error) && data.curves[1].samples[0].y == previous,"Incompatible axes were accepted or partially published");
  definition.curves[1].visible = false;
  check(normalize_plot_axes(definition,data,error),"Hidden incompatible curve blocked plotting");
  definition.curves[1].visible = true;
  data.curves[1].y_quantity = data.curves[0].y_quantity;
  data.curves[1].frame_axis = true;
  check(!normalize_plot_axes(definition,data,error),"Frame and physical axes were mixed");
  data.curves[1].frame_axis = false;
  data.curves[0].y_quantity.unit.clear();
  data.curves[1].y_quantity.unit.clear();
  check(normalize_plot_axes(definition,data,error),"Same scoped unknown raw basis was rejected");
  data.curves[1].y_scope = "other/field";
  check(!normalize_plot_axes(definition,data,error),"Different unknown raw bases were mixed");

  auto changed = std::make_shared<SimResults::VtkResultsData>(*provider.data);
  changed->increments[0].coordinate_unit = "m";
  FilterDefinition plane;
  plane.id = plane.name = "Plane";
  plane.input = {OutputOwner::Source,source,0};
  plane.coordinate_unit = "mm";
  EvaluatedOutput output;
  check(!evaluate_filter(plane,live.session().representations.at(display),changed,output,error),"A changed native coordinate basis silently moved the plane");
  check(scale_interval(4.9) == 2 && scale_interval(5) == 5 && scale_interval(23) == 20 &&
    scale_interval(0) == 0 && scale_interval(std::numeric_limits<double>::infinity()) == 0,"Invalid ruler interval");
  Provider metric;
  metric.data->job_id = 2;
  metric.data->increments[0].coordinate_unit = "m";
  metric.data->source_units.coordinate_unit = "m";
  auto* metricMesh = vtkUnstructuredGrid::SafeDownCast(metric.data->increments[0].dataset.get());
  metricMesh->GetPoints()->SetPoint(1,0.001,0,0);
  metricMesh->Modified();
  check(live.open_result(metric,SimResults::JobSourceReference{"test","",2,""},std::string("Metres"),error),"Cannot open metre source");
  const auto& metricDisplay = live.session().representations.at(live.session().active_representation_id);
  double factor = 0;
  bool known = false;
  check(live.scene_scale(metricDisplay,factor,known,error) && known && factor == 1000,"Metres were not transformed to fixed millimetre scene");
  std::array<double,6> fittedBounds;
  check(live.visible_bounds("view1",fittedBounds,error) && std::abs(fittedBounds[1]-1) < 1e-12,
    "Fit bounds mixed native metre and millimetre coordinates");
  check(live.standard_view("top",error) && live.standard_view("parallel",error),"Cannot set standard view");
  const auto parallel = *live.session().views.at("view1").camera;
  check(live.standard_view("perspective",error),"Cannot return to perspective");
  const auto perspective = *live.session().views.at("view1").camera;
  const double distance = std::hypot(perspective.position[0]-perspective.focal_point[0],
    perspective.position[1]-perspective.focal_point[1],perspective.position[2]-perspective.focal_point[2]);
  check(std::abs(distance*std::tan(perspective.vertical_view_angle_degrees*std::acos(-1.0)/360)-parallel.parallel_scale) < 1e-12,
    "Projection change lost focal-plane scale");
  auto grouped = *provider.data;
  auto otherFrame = grouped.increments.front();
  otherFrame.fields[0].component_units[0].unit = "Pa";
  grouped.increments.push_back(otherFrame);
  auto groups = result_field_groups(grouped,0);
  check(groups.size() == 1 && groups[0].unit_issue.empty(),"Compatible stress units split a stable result group");
  grouped.increments.back().fields[0].component_units[0] = SimUnits::QuantityUnit("length","mm",SimUnits::Length);
  groups = result_field_groups(grouped,0);
  check(groups.size() == 1 && !groups[0].unit_issue.empty() && !groups[0].available,
    "Incompatible physical quantities shared result coloring");
  if (failures != 0)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
