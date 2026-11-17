#include "VectorPlotTests.hpp"
#include "Postprocessor/Core/representations/VectorPlotRuntime.hpp"
#include "Postprocessor/Core/representations/VectorPlotCodec.hpp"
#include "Postprocessor/Core/representations/ResultVectors.hpp"
#include "Postprocessor/Commands/results/VectorCommandText.hpp"
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkCellType.h>
#include <vtkNew.h>
#include <cmath>
#include <limits>
#include <iostream>
int vector_plot_tests()
{
  using namespace Postprocessor;
  int failures = 0;
  const auto check = [&](bool success,const char* message)
  {
    if (!success)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  auto mesh = std::shared_ptr<vtkUnstructuredGrid>(vtkUnstructuredGrid::New(),[](auto* value) { value->Delete(); });
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0,0,0);
  points->InsertNextPoint(1,0,0);
  points->InsertNextPoint(1,1,0);
  points->InsertNextPoint(0,1,0);
  mesh->SetPoints(points);
  const vtkIdType quad[] = {0,1,2,3};
  mesh->InsertNextCell(VTK_QUAD,4,quad);
  vtkNew<vtkDoubleArray> values;
  values->SetName("V");
  values->SetNumberOfComponents(4);
  const double tuples[][4] = {{3,4,0,999},{0,0,0,999},
    {std::numeric_limits<double>::quiet_NaN(),4,0,999},{3,4,0,999}};
  vtkNew<vtkDoubleArray> mask;
  mask->SetName("Valid/V");
  vtkNew<vtkTypeInt64Array> ids;
  ids->SetName("OriginalNodeId");
  for (int i = 0; i < 4; ++i)
  {
    values->InsertNextTuple(tuples[i]);
    double valid = 1;
    if (i == 3)
    {
      valid = 0;
    }
    mask->InsertNextValue(valid);
    ids->InsertNextValue(9007199254740993LL+i);
  }
  mesh->GetPointData()->AddArray(values);
  mesh->GetPointData()->AddArray(mask);
  mesh->GetPointData()->AddArray(ids);
  SimResults::Field field;
  field.name = "DISP";
  field.source = "FRD";
  field.array_name = "V";
  field.validity_array = "Valid/V";
  field.components = {"D1","D2","D3","ALL"};
  field.vector_components = std::array<std::size_t,3>{0,1,2};
  field.role = SimResults::FieldRole::Displacement;
  SimResults::VtkResultsData source;
  SimResults::Increment frame;
  frame.fields.push_back(field);
  frame.dataset = mesh;
  source.increments.push_back(frame);
  source.datasets.push_back({0,"Block","Blocks",1});
  Representation display;
  display.input = {OutputOwner::Source,"source",0};
  display.vectors.enabled = true;
  display.vectors.field = preferred_field_selection(field,field_group_key(field));
  VectorPlotData output;
  std::string error;
  check(prepare_vector_plot(source,source,display,10,output,error),"Vector sample preparation failed");
  check(output.samples.size() == 1,"Zero, masked and nonfinite vectors were not excluded");
  if (output.samples.size() == 1)
  {
    const auto& sample = output.samples.front();
    check(sample.magnitude == 5 && std::abs(sample.direction[0]-0.6) < 1e-12,
      "Vector direction/magnitude included the derived ALL column");
    check(sample.entity == 9007199254740993LL,"Vector provenance lost 64-bit node identity");
    check(std::abs(sample.length-output.reference_length*0.5) < 1e-12,"All-frame magnitude reference was not used");
  }
  display.deformation.enabled = true;
  display.deformation.field_mode = DeformationFieldMode::Explicit;
  display.deformation.field = display.vectors.field;
  check(prepare_vector_plot(source,source,display,10,output,error) && !output.samples.empty() &&
    output.samples.front().position == std::array<double,3>{3,4,0},"Vector anchors did not follow deformation");
  double original[3];
  mesh->GetPoint(0,original);
  check(original[0] == 0 && original[1] == 0,"Vector preparation changed source geometry");
  display.deformation.enabled = false;
  display.vectors.proportional = false;
  check(prepare_vector_plot(source,source,display,10,output,error) && !output.samples.empty() &&
    output.samples.front().length == output.reference_length,"Uniform vector scaling is incorrect");
  display.hidden_datasets.insert(0);
  check(prepare_vector_plot(source,source,display,10,output,error) && output.samples.empty(),"Hidden dataset still has vectors");
  display.hidden_datasets.clear();
  auto encoded = vector_plot_to_json(display.vectors);
  VectorPlotDefinition decoded;
  check(vector_plot_from_json(encoded,decoded,error) && same_vector_plot(decoded,display.vectors),"Vector settings did not round trip");
  encoded.insert("stride",0);
  check(!vector_plot_from_json(encoded,decoded,error) && same_vector_plot(decoded,display.vectors),"Malformed vector settings replaced committed state");
  check(vector_plot_from_json(QJsonValue(QJsonValue::Undefined),decoded,error) && !decoded.enabled,
    "Legacy projects did not default vectors off");
  auto tensor = field;
  tensor.vector_components.reset();
  check(!spatial_vector(tensor),"Undeclared three-column fields were treated as vectors");
  field.association = SimResults::Association::Element;
  check(spatial_vector(field),"Element-associated vectors were excluded by deformation rules");
  vtkNew<vtkDoubleArray> cellValues;
  cellValues->SetName("V");
  cellValues->SetNumberOfComponents(4);
  cellValues->InsertNextTuple(tuples[0]);
  mesh->GetCellData()->AddArray(cellValues);
  field.validity_array.clear();
  source.increments.front().fields = {field};
  display.vectors.field = preferred_field_selection(field,field_group_key(field));
  check(prepare_vector_plot(source,source,display,10,output,error) && output.samples.size() == 1 &&
    output.samples.front().position == std::array<double,3>{0.5,0.5,0},"Element vector was not placed at its cell center");
  check(vector_command_text("on","first_run") == "post vector on source \"first_run\"",
    "Vector GUI command did not target the source");
  check(vector_command_text("on","bad\nsource").isEmpty(),"Vector command accepted a multiline source");
  return failures;
}
