#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkSmartPointer.h>
#include <cmath>
#include <iostream>
#include <limits>
namespace
{
  int failures = 0;
  void check(bool condition,const char* message)
  {
    if (!condition)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  }
}
int main()
{
  using namespace Postprocessor;
  ResultColoring c;
  c.palette = ColorPalette::Grayscale;
  c.minimum = {true,0,{0,0,1}};
  c.maximum = {true,100,{1,0,1}};
  std::string error;
  check(valid_coloring(c,error),"Valid stress limits rejected");
  check(result_color(-1,c,{-10,110}) == c.minimum.color,"Minimum override missing");
  check(result_color(101,c,{-10,110}) == c.maximum.color,"Maximum override missing");
  check(result_color(0,c,{-10,110}) != c.minimum.color && result_color(100,c,{-10,110}) != c.maximum.color,
    "Equality must retain palette color");
  const auto maximum = c.maximum;
  c.maximum.enabled = false;
  check(result_color(101,c,{-10,110}) != maximum.color && c.maximum.value == maximum.value && c.maximum.color == maximum.color,
    "Disabling a limit must restore palette coloring and retain its configuration");
  c.maximum.enabled = true;
  check(result_color(101,c,{-10,110}) == maximum.color,"Re-enabling did not restore limit color");
  check(result_color(std::numeric_limits<double>::quiet_NaN(),c,{-10,110}) == ColoringDefaults::missing,
    "Missing values were treated as limit violations");
  c.minimum.value = 101;
  check(!valid_coloring(c,error),"Crossed limits accepted");
  c.minimum.enabled = false;
  check(valid_coloring(c,error),"Disabled retained limits must not constrain the enabled limit");
  c.minimum.color[0] = 2;
  check(!valid_coloring(c,error),"Invalid RGB accepted");
  ResultColoring bands;
  bands.palette = ColorPalette::Grayscale;
  bands.bands = 4;
  check(result_color(0.01,bands,{0,1}) == result_color(0.24,bands,{0,1}),"Values in one band had different colors");
  bands.reversed = true;
  check(result_color(0,bands,{0,1})[0] == 1,"Palette reversal failed");
  check(std::isfinite(result_color(0,bands,{-std::numeric_limits<double>::max(),std::numeric_limits<double>::max()})[0]),
    "Large symmetric ranges overflowed normalization");
  auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint(0,0,0);
  points->InsertNextPoint(1,0,0);
  grid->SetPoints(points);
  auto array = vtkSmartPointer<vtkDoubleArray>::New();
  array->SetName("vector");
  array->SetNumberOfComponents(4);
  array->InsertNextTuple4(3,4,0,999);
  array->InsertNextTuple4(6,8,0,999);
  auto mask = vtkSmartPointer<vtkUnsignedCharArray>::New();
  mask->SetName("valid");
  mask->InsertNextValue(1);
  mask->InsertNextValue(0);
  grid->GetPointData()->AddArray(array);
  grid->GetPointData()->AddArray(mask);
  SimResults::Field field;
  field.array_name = "vector";
  field.validity_array = "valid";
  field.components = {"X","Y","Z","Derived"};
  field.vector_components = std::array<std::size_t,3>{0,1,2};
  FieldSelection selected{0,"vector",SimResults::Association::Node,"",true};
  std::vector<double> values;
  check(color_values(grid,field,selected,values,error) && values.size() == 2 && values[0] == 5 && std::isnan(values[1]),
    "Magnitude included derived components or ignored the validity mask");
  std::optional<std::array<double,2>> accumulated;
  check(accumulate_color_range(grid,field,selected,accumulated,error) && accumulated && (*accumulated)[0] == 5 && (*accumulated)[1] == 5,
    "Streaming range accumulation differs from masked vector extraction");
  field.vector_components.reset();
  check(!color_values(grid,field,selected,values,error),"Undeclared vector magnitude accepted");
  check(array->GetNumberOfComponents() == 4 && array->GetComponent(0,3) == 999 && grid->GetPointData()->GetNumberOfArrays() == 2,
    "Color extraction modified published arrays");
  SimResults::VtkResultsData result;
  result.increments.resize(3);
  field.name = "DISP";
  field.source = "FRD";
  field.dataset_id = 0;
  result.increments[0].fields = {field};
  field.dataset_id = 1;
  result.increments[0].fields.push_back(field);
  result.increments[2].fields = result.increments[0].fields;
  auto groups = result_field_groups(result,0);
  check(groups.size() == 1 && groups[0].datasets.size() == 2 && groups[0].available,"Compatible datasets/frames were not grouped");
  const auto key = groups[0].key;
  groups = result_field_groups(result,1);
  check(groups.size() == 1 && !groups[0].available && groups[0].key == key,"Frame changes replaced result group identity");
  field.association = SimResults::Association::Element;
  result.increments[0].fields.push_back(field);
  groups = result_field_groups(result,0);
  check(groups.size() == 2,"Incompatible associations were silently merged");
  FieldSelection grouped{std::nullopt,"vector",SimResults::Association::Node,"X",false};
  check(find_field(result.increments[0],grouped) != nullptr,"Grouped selection did not resolve a compatible field");
  grouped.dataset_id = 99;
  check(find_field(result.increments[0],grouped) == nullptr,"Explicit dataset scope was ignored");
  return failures == 0 ? 0 : 1;
}
