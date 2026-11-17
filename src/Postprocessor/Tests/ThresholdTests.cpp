#include "ThresholdTests.hpp"
#include "Postprocessor/Core/filters/FilterRuntime.hpp"
#include "Postprocessor/Core/filters/FilterCodec.hpp"
#include "Postprocessor/Core/filters/ThresholdFilter.hpp"
#include "Postprocessor/Commands/filters/ThresholdCommandText.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <QJsonObject>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkDoubleArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkNew.h>
#include <iostream>
#include <limits>
#include <cmath>

int threshold_tests()
{
  using namespace Postprocessor;
  int failures = 0;
  const auto check = [&](bool accepted,const char* message)
  {
    if (!accepted)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  auto mesh = std::shared_ptr<vtkUnstructuredGrid>(vtkUnstructuredGrid::New(),[](auto* value)
  {
    value->Delete();
  });
  vtkNew<vtkPoints> points;
  vtkNew<vtkDoubleArray> values;
  vtkNew<vtkTypeInt64Array> nodes;
  vtkNew<vtkTypeInt64Array> elements;
  values->SetName("VALUE");
  nodes->SetName("OriginalNodeId");
  elements->SetName("OriginalElementId");
  const double samples[] = {20,40,70,90,40,60,70,50};
  for (int index = 0; index < 8; ++index)
  {
    const int local = index%4;
    points->InsertNextPoint(index/4*2+(local == 1 || local == 2),local >= 2,0);
    values->InsertNextValue(samples[index]);
    nodes->InsertNextValue(9007199254740993LL+index);
  }
  mesh->SetPoints(points);
  for (vtkIdType cell = 0; cell < 2; ++cell)
  {
    const vtkIdType ids[] = {4*cell,4*cell+1,4*cell+2,4*cell+3};
    mesh->InsertNextCell(VTK_QUAD,4,ids);
    elements->InsertNextValue(9007199254741093LL+cell);
  }
  mesh->GetPointData()->AddArray(values);
  mesh->GetPointData()->AddArray(nodes);
  mesh->GetCellData()->AddArray(elements);
  auto source = std::make_shared<SimResults::VtkResultsData>();
  SimResults::Increment frame;
  frame.dataset = mesh;
  SimResults::Field field;
  field.name = field.array_name = "VALUE";
  field.components = {"X"};
  frame.fields.push_back(field);
  source->increments.push_back(frame);
  source->datasets.push_back({0,"Block","Blocks",1});
  FilterDefinition filter;
  filter.id = "threshold";
  filter.name = "Threshold 1";
  filter.kind = FilterKind::Threshold;
  filter.input = {OutputOwner::Source,"source",0};
  filter.threshold.field = preferred_field_selection(field,field_group_key(field));
  filter.threshold.lower = 30;
  filter.threshold.upper = 80;
  std::string error, warning;
  check(bind_threshold_units(filter.threshold,field,error),"Cannot bind threshold units");
  std::vector<vtkIdType> retained;
  check(ThresholdFilter::select_cells(mesh,frame,0,filter.threshold,retained,warning,error) &&
    retained == std::vector<vtkIdType>{1},"All nodes did not retain only the fully matching element");
  filter.threshold.node_rule = ThresholdNodeRule::Any;
  check(ThresholdFilter::select_cells(mesh,frame,0,filter.threshold,retained,warning,error) &&
    retained == std::vector<vtkIdType>({0,1}),"Any node did not retain both matching elements");
  check(ThresholdFilter::matches(30,filter.threshold) && ThresholdFilter::matches(80,filter.threshold),
    "Threshold bounds were not inclusive");
  check(!ThresholdFilter::matches(std::numeric_limits<double>::quiet_NaN(),filter.threshold) &&
    !ThresholdFilter::matches(std::numeric_limits<double>::infinity(),filter.threshold),"Invalid samples matched");
  filter.threshold.node_rule = ThresholdNodeRule::All;
  Representation display;
  display.input = {OutputOwner::Feature,filter.id,0};
  filter.reference_visible = true;
  EvaluatedOutput output;
  check(evaluate_filter(filter,display,source,output,error),"Cannot evaluate threshold output");
  if (!output.parts.empty())
  {
    const auto& part = output.parts.front();
    check(part.mesh->GetNumberOfCells() == 1 && part.nodes->GetNumberOfPoints() == 4 &&
      part.removed && part.removed->GetNumberOfCells() == 1,"Threshold geometry/reference counts differ");
    auto* ids = vtkTypeInt64Array::SafeDownCast(part.mesh->GetCellData()->GetArray("OriginalElementId"));
    check(ids && ids->GetValue(0) == 9007199254741094LL,"Threshold lost exact original element IDs");
    auto* nodeIds = vtkTypeInt64Array::SafeDownCast(part.nodes->GetPointData()->GetArray("OriginalNodeId"));
    check(nodeIds && nodeIds->GetValue(0) == 9007199254740997LL,"Threshold lost exact original node IDs");
  }
  check(mesh->GetNumberOfCells() == 2 && values->GetValue(0) == 20,"Threshold mutated source buffers");
  auto encoded = filters_to_json({{filter.id,filter}});
  std::map<std::string,FilterDefinition> decoded;
  check(filters_from_json(encoded,decoded,error) && decoded.at(filter.id).threshold.lower == 30 &&
    decoded.at(filter.id).threshold.node_rule == ThresholdNodeRule::All,"Threshold persistence round trip failed");
  auto malformed = encoded[0].toObject();
  auto settings = malformed.value("threshold").toObject();
  settings.insert("nodes","invalid");
  malformed.insert("threshold",settings);
  check(!filters_from_json(QJsonArray{malformed},decoded,error) && decoded.count(filter.id) == 1,
    "Invalid threshold restore replaced committed definitions");
  filter.threshold.lower = 90;
  filter.threshold.upper = 10;
  check(!valid_threshold(filter.threshold,error),"Reversed threshold bounds were accepted");
  filter.threshold.lower = 200;
  filter.threshold.upper.reset();
  check(evaluate_filter(filter,display,source,output,error) && output.parts.front().mesh->GetNumberOfCells() == 0,
    "Empty threshold output was not valid");
  filter.enabled = false;
  check(evaluate_filter(filter,display,source,output,error) && output.parts.front().mesh->GetNumberOfCells() == 2 &&
    !output.parts.front().removed,"Bypassed threshold did not restore the source geometry");
  filter.enabled = true;
  filter.threshold.lower = 30;
  filter.threshold.upper = 80;
  vtkNew<vtkDoubleArray> validity;
  validity->SetName("VALID");
  validity->SetNumberOfTuples(8);
  validity->FillComponent(0,1);
  validity->SetValue(4,0);
  mesh->GetPointData()->AddArray(validity);
  frame.fields[0].validity_array = "VALID";
  check(ThresholdFilter::select_cells(mesh,frame,0,filter.threshold,retained,warning,error) && retained.empty(),
    "All nodes accepted an invalid node");
  filter.threshold.node_rule = ThresholdNodeRule::Any;
  check(ThresholdFilter::select_cells(mesh,frame,0,filter.threshold,retained,warning,error) && retained.size() == 2,
    "Any node rejected other valid matching nodes");
  auto elemental = field;
  elemental.association = SimResults::Association::Element;
  vtkNew<vtkDoubleArray> cellValues;
  cellValues->SetName("VALUE");
  cellValues->InsertNextValue(10);
  cellValues->InsertNextValue(50);
  mesh->GetCellData()->AddArray(cellValues);
  frame.fields = {elemental};
  filter.threshold.field = preferred_field_selection(elemental,field_group_key(elemental));
  check(ThresholdFilter::select_cells(mesh,frame,0,filter.threshold,retained,warning,error) &&
    retained == std::vector<vtkIdType>{1},"Element threshold used nodal values");
  frame.fields.clear();
  check(ThresholdFilter::select_cells(mesh,frame,0,filter.threshold,retained,warning,error) &&
    retained.empty() && !warning.empty(),"Missing threshold field was not reported");
  // Only declared vector columns contribute to magnitude, not derived extras.
  auto vectorField = field;
  vectorField.array_name = "VECTOR";
  vectorField.components = {"X","Y","Z","EXTRA"};
  vectorField.vector_components = std::array<std::size_t,3>{0,1,2};
  vtkNew<vtkDoubleArray> vectorValues;
  vectorValues->SetName("VECTOR");
  vectorValues->SetNumberOfComponents(4);
  for (int index = 0; index < 8; ++index)
  {
    const double tuple[] = {3,4,0,10000};
    vectorValues->InsertNextTuple(tuple);
  }
  mesh->GetPointData()->AddArray(vectorValues);
  frame.fields = {vectorField};
  auto vectorThreshold = filter.threshold;
  vectorThreshold.field = preferred_field_selection(vectorField,field_group_key(vectorField));
  vectorThreshold.lower = 5;
  vectorThreshold.upper = 5;
  vectorThreshold.node_rule = ThresholdNodeRule::All;
  check(ThresholdFilter::select_cells(mesh,frame,0,vectorThreshold,retained,warning,error) && retained.size() == 2,
    "Threshold magnitude used derived columns or rejected equal bounds");
  // Integration points are individual vertex cells with point-associated values.
  vtkNew<vtkUnstructuredGrid> ipMesh;
  vtkNew<vtkPoints> ipPoints;
  vtkNew<vtkDoubleArray> ipValues;
  vtkNew<vtkTypeInt64Array> ipIds;
  ipValues->SetName("VALUE");
  ipIds->SetName("IntegrationPointId");
  for (vtkIdType index = 0; index < 2; ++index)
  {
    ipPoints->InsertNextPoint(index,0,0);
    ipMesh->InsertNextCell(VTK_VERTEX,1,&index);
    ipValues->InsertNextValue(20+30*index);
    ipIds->InsertNextValue(index+1);
  }
  ipMesh->SetPoints(ipPoints);
  ipMesh->GetPointData()->AddArray(ipValues);
  ipMesh->GetPointData()->AddArray(ipIds);
  auto ipField = field;
  ipField.association = SimResults::Association::IntegrationPoint;
  frame.fields = {ipField};
  auto ipThreshold = filter.threshold;
  ipThreshold.field = preferred_field_selection(ipField,field_group_key(ipField));
  check(ThresholdFilter::select_cells(ipMesh,frame,0,ipThreshold,retained,warning,error) &&
    retained == std::vector<vtkIdType>{1},"Integration-point threshold invented a surface or mixed values");
  // Native stress values in Pa, thresholds retained in MPa.
  SimUnits::QuantityUnit stress;
  check(framework_units().quantity("stress",stress,error),"Stress quantity is unavailable");
  stress.unit = "Pa";
  elemental.component_units = {stress};
  frame.fields = {elemental};
  filter.threshold.bounds_unit = stress;
  filter.threshold.bounds_unit->unit = "MPa";
  filter.threshold.lower = 30;
  filter.threshold.upper = 80;
  cellValues->SetValue(0,20e6);
  cellValues->SetValue(1,50e6);
  check(ThresholdFilter::select_cells(mesh,frame,0,filter.threshold,retained,warning,error) &&
    retained == std::vector<vtkIdType>{1},"Threshold did not convert native field units");
  auto units = framework_units();
  check(units.set_display_unit("postprocessor","stress","Pa",error) &&
    convert_threshold_units(filter.threshold,units,error) &&
    std::abs(*filter.threshold.lower-30e6) < 1e-6,"Threshold bound conversion changed physical limits");
  auto open = filter.threshold;
  open.upper.reset();
  check(convert_threshold_units(open,units,error) && !open.upper,"Unit conversion enabled an open threshold bound");
  auto exact = filter.threshold;
  exact.upper = exact.lower;
  check(convert_threshold_units(exact,units,error) && exact.upper == exact.lower,
    "Unit conversion rejected equal threshold bounds");
  ResultFieldSelector selector{"Stress",{}};
  const auto command = threshold_command_text(selector,filter.threshold);
  check(command.contains("nodes any") && command.contains("component \"X\"") && command.contains("lower "),
    "Threshold command text lost settings");
  return failures;
}
