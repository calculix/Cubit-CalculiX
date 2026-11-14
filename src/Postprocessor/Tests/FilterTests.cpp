#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/filters/FilterCodec.hpp"
#include "Postprocessor/Core/persistence/PostprocessorSessionCodec.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include <QApplication>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkVariant.h>
#include <vtkCellType.h>
#include <iostream>
#include <cmath>
namespace
{
  int failures = 0;
  void check(bool value,const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  }
  std::shared_ptr<SimResults::VtkResultsData> fixture(bool quadratic)
  {
    auto data = std::make_shared<SimResults::VtkResultsData>();
    data->job_id = 1;
    data->run_identity = "filter-fixture";
    auto mesh = std::shared_ptr<vtkUnstructuredGrid>(vtkUnstructuredGrid::New(),[](auto* value)
    {
      value->Delete();
    });
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    const double coordinates[][3] = {{0,0,0},{1,0,0},{1,1,0},{0,1,0},{0,0,1},{1,0,1},{1,1,1},{0,1,1},
      {.5,0,0},{1,.5,0},{.5,1,0},{0,.5,0},{.5,0,1},{1,.5,1},{.5,1,1},{0,.5,1},{0,0,.5},{1,0,.5},{1,1,.5},{0,1,.5}};
    const int count = quadratic ? 20 : 8;
    auto values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
    ids->SetName("OriginalNodeId");
    for (int i = 0; i < count; ++i)
    {
      points->InsertNextPoint(coordinates[i]);
      values->InsertNextValue(coordinates[i][0]);
      ids->InsertNextValue(9007199254740993LL+i);
    }
    mesh->SetPoints(points);
    vtkIdType vertices[20];
    for (int i = 0; i < count; ++i)
    {
      vertices[i] = i;
    }
    mesh->InsertNextCell(quadratic ? VTK_QUADRATIC_HEXAHEDRON : VTK_HEXAHEDRON,count,vertices);
    mesh->GetPointData()->AddArray(values);
    mesh->GetPointData()->AddArray(ids);
    auto elements = vtkSmartPointer<vtkTypeInt64Array>::New();
    elements->SetName("OriginalElementId");
    elements->InsertNextValue(9007199254741093LL);
    mesh->GetCellData()->AddArray(elements);
    SimResults::Increment frame;
    frame.dataset = mesh;
    frame.step = frame.increment = 1;
    frame.time = .1;
    SimResults::Field field;
    field.name = field.array_name = "VALUE";
    field.source = "VTK";
    field.components = {"X"};
    frame.fields.push_back(field);
    data->increments.push_back(frame);
    data->datasets.push_back({0,"Block_1","Blocks",1});
    return data;
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = fixture(false);
    std::string command_prefix() const override
    {
      return "test";
    }
    bool open(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      auto resolved = std::get<SimResults::JobSourceReference>(request);
      resolved.run_identity = data->run_identity;
      output = {resolved,"Mesh",data};
      error.clear();
      return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      return open(request,output,error);
    }
  };
}
int main(int argc,char** argv)
{
  QApplication application(argc,argv);
  using namespace Postprocessor;
  std::string error;
  PlaneDefinition plane;
  plane.normal = {1e300,0,0};
  check(normalize_plane(plane,error) && plane.normal[0] == 1,"Normal normalization overflowed");
  plane.normal = {0,0,0};
  check(!normalize_plane(plane,error),"Zero plane normal was accepted");
  for (bool quadratic : {false,true})
  {
    const auto source = fixture(quadratic);
    FilterDefinition filter;
    filter.id = "test";
    filter.name = "Cutaway";
    filter.input = {OutputOwner::Source,"s",0};
    filter.plane.origin = {.5,0,0};
    Representation display;
    display.id = "r";
    EvaluatedOutput clipped;
    check(evaluate_filter(filter,display,source,clipped,error),"Cannot clip linear/quadratic hexahedron");
    if (!clipped.parts.empty())
    {
      const auto& part = clipped.parts.front();
      check(part.mesh->GetNumberOfCells() > 0 && part.nodes->GetNumberOfPoints() > 0,"Clip lost retained geometry/nodes");
      check(!part.mesh->GetPointData()->HasArray("OriginalNodeId"),"Generated points retained interpolated solver node IDs");
      auto* elements = part.mesh->GetCellData()->GetArray("OriginalElementId");
      check(elements && elements->GetVariantValue(0).ToLongLong() == 9007199254741093LL,"Clip lost exact 64-bit element ID");
      auto* nodes = part.nodes->GetPointData()->GetArray("OriginalNodeId");
      check(nodes && nodes->GetVariantValue(0).ToLongLong() > 9007199254740992LL,"Retained node marker lost exact solver ID");
    }
    filter.kind = FilterKind::Section;
    EvaluatedOutput section;
    check(evaluate_filter(filter,display,source,section,error),"Cannot section linear/quadratic hexahedron");
    if (!section.parts.empty())
    {
      auto* mesh = section.parts.front().mesh.get();
      check(mesh->GetNumberOfCells() > 0,"Section unexpectedly empty");
      for (vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i)
      {
        double point[3];
        mesh->GetPoint(i,point);
        check(std::abs(point[0]-.5) < 1e-8,"Section generated a point off the plane");
      }
    }
    filter.plane.origin = {2,0,0};
    check(evaluate_filter(filter,display,source,section,error) && !section.parts.empty() && section.parts.front().mesh->GetNumberOfCells() == 0,
      "Empty section did not succeed as an empty output");
    auto* original = vtkDataSet::SafeDownCast(source->increments.front().dataset.get());
    check(original->GetNumberOfCells() == 1 && original->GetPointData()->HasArray("OriginalNodeId"),"Filter modified source mesh/IDs");
  }
  Provider provider;
  PostprocessorCore core;
  check(core.open_result(provider,SimResults::JobSourceReference{"test","",1,""},std::nullopt,error),"Cannot open filter source fixture");
  const auto source = core.session().active_source_id;
  FilterDefinition requested;
  requested.name = "Cutaway";
  requested.input = {OutputOwner::Source,source,0};
  requested.plane.origin = {.5,0,0};
  check(core.create_filter(requested,"view1",error),"Cannot create named clip");
  if (failures != 0 || core.session().representations.count(core.session().active_representation_id) == 0)
  {
    return failures == 0 ? 1 : failures;
  }
  const auto display = core.session().representations.at(core.session().active_representation_id);
  std::shared_ptr<const EvaluatedOutput> first, second;
  check(core.evaluate_output(display,first,error) && core.evaluate_output(display,second,error) && first == second,"Filter cache missed unchanged context");
  PostprocessorProjectState saved, decoded;
  check(core.capture_project(saved,error) && project_from_json(project_to_json(saved,"."),".",decoded,error) &&
      decoded.session.filters.size() == 1,"Project did not preserve filter definitions/feature representations");
  const auto revision = core.display_revision();
  plane.normal = {0,0,0};
  check(!core.change_filter("Cutaway","plane","view1",plane,"",error) && core.display_revision() == revision,"Invalid plane mutated committed session");
  check(core.change_filter("Cutaway","hide","view1",{},"",error) && !core.session().representations.at(display.id).visible,"Filter visibility not view-local");
  check(core.change_filter("Cutaway","show","view1",{},"",error),"Cannot restore filter visibility");
  check(core.close_source(source,error) && core.session().filters.empty(),"Source closure left dependent filters");
  check(core.reset(error) && core.session().filters.empty(),"Reset did not clear filters");
  std::map<std::string,FilterDefinition> legacy;
  check(filters_from_json(QJsonValue(QJsonValue::Undefined),legacy,error) && legacy.empty(),"Legacy project filter defaults failed");
  return failures;
}
