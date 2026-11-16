#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/persistence/PostprocessorSessionCodec.hpp"
#include "Postprocessor/Core/representations/ResultColoringUnits.hpp"
#include "Postprocessor/GUI/Tables/PostprocessorTableView.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <QApplication>
#include <QJsonArray>
#include <QTableView>
#include <QAbstractItemModel>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkTable.h>
#include <vtkCellType.h>
#include <vtkSmartPointer.h>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace
{
  int failures = 0;
  bool check(bool success, const char* message)
  {
    if (!success)
    {
      std::cerr << message << '\n';
      ++failures;
    }
    return success;
  }
  bool near(double a, double b)
  {
    return std::abs(a-b) <= 1e-8*std::max(1.0, std::abs(b));
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = std::make_shared<SimResults::VtkResultsData>();
    Provider()
    {
      auto mesh = vtkSmartPointer<vtkUnstructuredGrid>::New();
      auto points = vtkSmartPointer<vtkPoints>::New();
      points->InsertNextPoint(0,0,0);
      points->InsertNextPoint(1,0,0);
      mesh->SetPoints(points);
      const vtkIdType ids[] = {0,1};
      mesh->InsertNextCell(VTK_LINE,2,ids);
      auto values = vtkSmartPointer<vtkDoubleArray>::New();
      values->SetName("STRESS");
      values->InsertNextValue(100);
      values->InsertNextValue(200);
      mesh->GetPointData()->AddArray(values);
      mesh->Register(nullptr);
      SimResults::Increment frame;
      frame.dataset = std::shared_ptr<vtkDataObject>(mesh.GetPointer(),[](vtkDataObject* object)
      {
        object->Delete();
      });
      SimResults::Field field;
      field.name = field.array_name = "STRESS";
      field.source = "VTK";
      field.components = {"XX"};
      field.component_units.emplace_back("stress","MPa",SimUnits::Stress);
      frame.fields.push_back(field);
      data->increments.push_back(frame);
      frame.fields.clear();
      frame.increment = 2;
      data->increments.push_back(frame);
      data->datasets.push_back({0,"Block_1","Blocks",1});
      data->job_id = 1;
      data->run_identity = "unit-fixture";
    }
    std::string command_prefix() const override
    {
      return "test";
    }
    bool open(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = data->run_identity;
      output = {reference,"Units",data};
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
  if (!check(framework_units().set_display_unit("postprocessor","stress","MPa",error),"Cannot set initial stress unit"))
  {
    return EXIT_FAILURE;
  }
  Provider provider;
  PostprocessorCore core;
  if (!check(core.open_result(provider,SimResults::JobSourceReference{"test","",1,""},std::nullopt,error),"Cannot open unit fixture"))
  {
    return EXIT_FAILURE;
  }
  const auto source = core.session().active_source_id;
  const auto display = core.session().active_representation_id;
  auto coloring = core.session().representations.at(display).coloring;
  if (!check(coloring.bounds_unit && coloring.bounds_unit->unit == "MPa","Initial coloring did not bind its presentation unit"))
  {
    return EXIT_FAILURE;
  }
  coloring.fixed_range = {0,300};
  coloring.maximum.enabled = true;
  coloring.maximum.value = 200;
  if (!check(core.set_coloring(source,"view1",coloring,error),"Cannot set fixed bounds"))
  {
    return EXIT_FAILURE;
  }
  PostprocessorProjectState saved, decoded;
  if (!check(core.capture_project(saved,error) && project_from_json(project_to_json(saved,"."),".",decoded,error),"Cannot round-trip bound units"))
  {
    return EXIT_FAILURE;
  }
  check(same_coloring_units(decoded.session.representations.at(display).coloring.bounds_unit,coloring.bounds_unit),"Persistence lost bound unit metadata");
  auto json = project_to_json(saved,".");
  auto displays = json.value("representations").toArray();
  if (!check(!displays.isEmpty(),"Saved fixture has no representations"))
  {
    return EXIT_FAILURE;
  }
  auto item = displays[0].toObject();
  auto colorJson = item.value("coloring").toObject();
  const auto unitJson = colorJson.value("bounds_unit");
  colorJson.remove("bounds_unit");
  item.insert("coloring",colorJson);
  displays[0] = item;
  json.insert("representations",displays);
  PostprocessorProjectState legacyJson;
  check(project_from_json(json,".",legacyJson,error) &&
    !legacyJson.session.representations.at(display).coloring.bounds_unit,"Legacy JSON did not retain an unbound unit");
  auto invalidUnit = unitJson.toObject();
  invalidUnit.insert("role",999);
  colorJson.insert("bounds_unit",invalidUnit);
  item.insert("coloring",colorJson);
  displays[0] = item;
  json.insert("representations",displays);
  check(!project_from_json(json,".",decoded,error),"Malformed bounds-unit metadata was accepted");
  check(same_coloring_units(decoded.session.representations.at(display).coloring.bounds_unit,coloring.bounds_unit),"Malformed metadata modified the published project output");

  PreparedPostprocessorProject missing;
  missing.definitions = decoded;
  missing.runtime[source].unavailable_reason = "External result unavailable";
  if (!check(core.restore_project(std::move(missing),error),"Cannot restore unavailable source"))
  {
    return EXIT_FAILURE;
  }
  check(!core.result_data(source),"Unavailable-source fixture unexpectedly has arrays");
  check(core.configure_units("","stress","Pa",error),"Cannot convert unavailable-source settings");
  const auto& converted = core.session().representations.at(display).coloring;
  check(near(converted.maximum.value,200e6) && near(converted.fixed_range[1],300e6),"Unavailable-source bounds changed physical meaning");
  check(core.configure_units("","stress","MPa",error),"Cannot convert bounds back");
  check(near(core.session().representations.at(display).coloring.maximum.value,200),"Bounds round-trip changed physical limit");

  // Legacy settings without a source cannot safely infer their original unit.
  PreparedPostprocessorProject legacy;
  legacy.definitions = decoded;
  legacy.definitions.session.representations.at(display).coloring.bounds_unit.reset();
  legacy.runtime[source].unavailable_reason = "Unavailable legacy result";
  if (!check(core.restore_project(std::move(legacy),error),"Cannot restore legacy fixture"))
  {
    return EXIT_FAILURE;
  }
  const auto generation = framework_units().generation();
  const auto revision = core.display_revision();
  check(!core.configure_units("","stress","Pa",error),"Unresolved legacy bounds allowed a unit change");
  check(generation == framework_units().generation() && revision == core.display_revision(),"Rejected conversion published partial state");

  // Recover legacy descriptors from another frame, not just the active frame.
  PreparedPostprocessorProject otherFrame;
  otherFrame.definitions = decoded;
  auto& otherDisplay = otherFrame.definitions.session.representations.at(display);
  otherDisplay.coloring.bounds_unit.reset();
  otherDisplay.increment_index = 1;
  otherFrame.runtime.emplace(source,SourceRuntime(provider.data,1,0));
  if (!check(core.restore_project(std::move(otherFrame),error),"Cannot restore missing-field frame"))
  {
    return EXIT_FAILURE;
  }
  check(core.configure_units("","stress","Pa",error),"Cannot recover unit metadata from another frame");
  check(near(core.session().representations.at(display).coloring.maximum.value,200e6),"Legacy field recovery did not convert limit");

  // Temperature differences must not acquire an absolute-temperature offset.
  ResultColoring difference = coloring;
  difference.bounds_unit = SimUnits::QuantityUnit("temperature_difference","K",SimUnits::Temperature,SimUnits::UnitRole::TemperatureDifference);
  difference.fixed_range = {0,100};
  difference.maximum.value = 10;
  auto staged = framework_units();
  check(staged.set_display_unit("postprocessor","temperature_difference","delta_degF",error),"Cannot set temperature-difference unit");
  check(convert_coloring_units(difference,staged,error) && near(difference.maximum.value,18),"Temperature-difference conversion applied an offset or rejected K");

  // Explicitly unknown units stay unknown and are not guessed from preferences.
  ResultColoring unknown = coloring;
  unknown.bounds_unit = SimUnits::QuantityUnit();
  check(convert_coloring_units(unknown,staged,error) && unknown.maximum.value == coloring.maximum.value,"Unknown unit settings were rescaled");

  // Reusing the same table must emit value/header notifications after a unit change.
  check(framework_units().set_display_unit("postprocessor","stress","MPa",error),"Cannot restore table presentation unit");
  auto values = vtkSmartPointer<vtkDoubleArray>::New();
  values->SetName("Stress");
  values->InsertNextValue(200);
  auto table = vtkSmartPointer<vtkTable>::New();
  table->AddColumn(values);
  table->Register(nullptr);
  ResultTableData data;
  data.owner = provider.data;
  data.table = std::shared_ptr<vtkTable>(table.GetPointer(),[](vtkTable* object)
  {
    object->Delete();
  });
  data.title = "Stress";
  data.columns = {"Stress"};
  data.column_units.emplace_back("stress","MPa",SimUnits::Stress);
  data.numeric = {true};
  data.rows = 1;
  PostprocessorTableView pane([]()
  {
  },nullptr);
  pane.show_data(data);
  auto* view = pane.findChild<QTableView*>();
  if (!check(view != nullptr,"Table view not found"))
  {
    return EXIT_FAILURE;
  }
  int headers = 0;
  int cells = 0;
  int resets = 0;
  QObject::connect(view->model(),&QAbstractItemModel::headerDataChanged,[&]()
  {
    ++headers;
  });
  QObject::connect(view->model(),&QAbstractItemModel::dataChanged,[&]()
  {
    ++cells;
  });
  QObject::connect(view->model(),&QAbstractItemModel::modelReset,[&]()
  {
    ++resets;
  });
  const auto selected = view->model()->index(0,0);
  view->setCurrentIndex(selected);
  view->setColumnWidth(0,180);
  check(framework_units().set_display_unit("postprocessor","stress","Pa",error),"Cannot change table units");
  pane.show_data(data);
  check(headers > 0 && cells > 0 && resets == 0,"Unit change did not notify table presentation without a reset");
  check(view->currentIndex() == selected && view->columnWidth(0) == 180,"Unit refresh lost selection or manual column width");
  check(near(view->model()->index(0,0).data().toDouble(),200e6),"Table still returns old-unit values");
  check(view->model()->headerData(0,Qt::Horizontal).toString().contains("[Pa]"),"Table still returns old-unit header");
  if (failures != 0)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
