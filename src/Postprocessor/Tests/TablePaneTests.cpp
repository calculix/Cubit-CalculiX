#include "Postprocessor/GUI/Tables/PostprocessorTableView.hpp"
#include "Postprocessor/GUI/Tables/PostprocessorTableModel.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include <QApplication>
#include <QTableView>
#include <QHeaderView>
#include <QFontMetrics>
#include <QToolButton>
#include <vtkActor2DCollection.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarsToColors.h>
#include <vtkTextProperty.h>
#include <vtkOpenGLRenderer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkFieldData.h>
#include <vtkDoubleArray.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkSmartPointer.h>
#include <vtkCellType.h>
#include <iostream>
#include <algorithm>
namespace
{
  int failures = 0;
  void check(bool value,const char* message)
  {
    if (!value)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::string command_prefix() const override
    {
      return "fake";
    }
    bool open(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = std::to_string(reference.job_id);
      auto data = std::make_shared<SimResults::VtkResultsData>();
      data->job_id = reference.job_id; data->run_identity = reference.run_identity;
      SimResults::Increment frame;
      auto* mesh = vtkUnstructuredGrid::New();
      auto points = vtkSmartPointer<vtkPoints>::New();
      points->InsertNextPoint(0,0,0); points->InsertNextPoint(1,0,0);
      mesh->SetPoints(points); mesh->Allocate(1);
      vtkIdType ids[] = {0,1}; mesh->InsertNextCell(VTK_LINE,2,ids);
      auto values = vtkSmartPointer<vtkDoubleArray>::New();
      values->SetName("temperature");
      values->InsertNextValue(reference.job_id); values->InsertNextValue(reference.job_id*10);
      mesh->GetPointData()->AddArray(values);
      frame.dataset = std::shared_ptr<vtkDataObject>(mesh,[](vtkDataObject* value)
      {
        value->Delete();
      });
      SimResults::Field field;
      field.name = "Temperature"; field.array_name = "temperature"; field.components = {"Value"};
      frame.fields.push_back(field);
      auto* table = vtkTable::New();
      auto column = vtkSmartPointer<vtkDoubleArray>::New();
      column->SetName("A long descriptive result column header");
      column->InsertNextValue(-1.2345678901234567e-123);
      table->AddColumn(column);
      auto name = vtkSmartPointer<vtkStringArray>::New();
      name->SetName("TableName"); name->InsertNextValue("Summary");
      table->GetFieldData()->AddArray(name);
      data->tables.emplace_back(table,[](vtkTable* value)
      {
        value->Delete();
      });
      data->increments.push_back(frame);
      output = {reference,"job"+std::to_string(reference.job_id),data};
      error.clear(); return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      return open(request,output,error);
    }
  };
  class Registry : public ResultSourceRegistryInterface
  {
  public:
    mutable Provider provider;
    bool resolve_result_provider(const std::string&,ResolvedResultSourceProvider& output,std::string& error) const override
    {
      output = {"test",&provider}; error.clear(); return true;
    }
  };
}
int main(int argc,char** argv)
{
  QApplication application(argc,argv);
  Registry registry;
  auto active = std::make_shared<PostprocessorApplicationInterface>(registry);
  std::string error;
  check(active->open_job("fake",1,std::nullopt,error),"Cannot open spatial/table fixture");
  const auto settings = active->coloring("job1",error);
  check(settings && settings->field,"Opening did not select a result field");
  check(active->open_table("job1","Summary",false,error),"Cannot open result table");
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  PostprocessorScene scene(renderer,"view1");
  check(scene.synchronize(*active,error),"Opening a table prevented scene synchronization");
  scene.apply_appearance({1,1,1},{1,1,1});
  int legends = 0;
  renderer->GetActors2D()->InitTraversal();
  while (auto* actor = renderer->GetActors2D()->GetNextActor2D())
  {
    auto* legend = vtkScalarBarActor::SafeDownCast(actor);
    if (legend && legend->GetVisibility())
    {
      ++legends;
      check(legend->GetTitleTextProperty()->GetColor()[0] == 0 && legend->GetLabelTextProperty()->GetColor()[0] == 0,
        "Initial white background retained white legend text");
      check(std::string(legend->GetTitle()).find("job1") != std::string::npos,"Legend retained a stale source title");
      check(legend->GetLookupTable()->GetRange()[1] == 10,"Legend range did not match the opened job");
    }
  }
  check(legends == 1,"Table opening removed the viewport legend");
  if (settings)
  {
    check(active->set_coloring("job1",*settings,error) && active->session().views.at("view1").table,
      "Palette/settings update unexpectedly closed the table");
  }
  check(active->color_field("job1","Temperature","Value",false,std::nullopt,error) &&
    !active->session().views.at("view1").table,"Activating an unchanged spatial field retained the table");
  check(active->open_table("job1","Summary",false,error),"Cannot reopen result table");
  check(!active->color_field("job1","Temperature","missing",false,std::nullopt,error) &&
    active->session().views.at("view1").table,"Invalid field activation closed the table");
  int closes = 0;
  PostprocessorTableView pane([&]()
  {
    ++closes;
  },nullptr);
  pane.show_data(active->table_data());
  auto* table = pane.findChild<QTableView*>("Postprocessor.TableView");
  auto* close = pane.findChild<QToolButton*>("Postprocessor.CloseTable");
  check(table && close,"Table pane controls are missing");
  if (table && close)
  {
    const QFontMetrics cells(table->font());
    const QFontMetrics headers(table->horizontalHeader()->font());
    const auto value = table->model()->index(0,0).data().toString();
    const auto label = table->model()->headerData(0,Qt::Horizontal).toString();
    check(table->columnWidth(0) >= std::max(cells.horizontalAdvance(value),headers.horizontalAdvance(label)),
      "Table column did not fit its header/value");
    check(table->model()->index(0,0).data(Qt::ToolTipRole).toString() == value,"Full cell value tooltip is missing");
    table->setColumnWidth(0,table->columnWidth(0)+50);
    const int manual = table->columnWidth(0);
    pane.show_data(active->table_data());
    check(table->columnWidth(0) == manual,"Unchanged refresh discarded manual column width");
    close->click();
    check(closes == 1,"Table close button did not dispatch once");
  }
  check(active->open_job("fake",2,std::nullopt,error) && scene.synchronize(*active,error),"Cannot display a second job");
  scene.apply_appearance({1,1,1},{1,1,1});
  bool second = false;
  renderer->GetActors2D()->InitTraversal();
  while (auto* actor = renderer->GetActors2D()->GetNextActor2D())
  {
    auto* legend = vtkScalarBarActor::SafeDownCast(actor);
    if (legend && std::string(legend->GetTitle()).find("job2") != std::string::npos)
    {
      second = legend->GetLookupTable()->GetRange()[1] == 20;
    }
  }
  check(second,"Second job did not publish its own legend/range");
  return failures ? 1 : 0;
}
