#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/sources/VtkFileSourceProvider.hpp"
#include "Postprocessor/GUI/Tables/PostprocessorTableModel.hpp"
#include "Postprocessor/GUI/Tables/PostprocessorTableView.hpp"
#include <QTableView>
#include <QItemSelectionModel>
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include <QTreeWidgetItemIterator>
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include <QApplication>
#include <QTemporaryDir>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkTypeInt64Array.h>
#include <vtkXMLTableWriter.h>
#include <iostream>
namespace
{
  int failures = 0;
  void check(bool condition, const char* message)
  {
    if (!condition)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  }
  std::shared_ptr<vtkTable> table(const std::string& name, double value)
  {
    auto result = std::shared_ptr<vtkTable>(vtkTable::New(),[](vtkTable* item)
    {
      item->Delete();
    });
    auto label = vtkSmartPointer<vtkStringArray>::New();
    label->SetName("TableName");
    label->InsertNextValue(name);
    result->GetFieldData()->AddArray(label);
    auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
    ids->SetName("Identifier");
    ids->InsertNextValue(9007199254740993LL);
    result->AddColumn(ids);
    auto values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("Value");
    values->InsertNextValue(value);
    result->AddColumn(values);
    auto strings = vtkSmartPointer<vtkStringArray>::New();
    strings->SetName("Label");
    strings->InsertNextValue("load_target");
    result->AddColumn(strings);
    return result;
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = std::make_shared<SimResults::VtkResultsData>();
    std::string command_prefix() const override
    {
      return "test";
    }
    bool open(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = "table-run";
      data->job_id = reference.job_id;
      data->run_identity = reference.run_identity;
      output = {reference,"Tables",data};
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
  Provider provider;
  provider.data->tables.push_back(table("Summary",10));
  provider.data->increments.resize(2);
  provider.data->increments[0].tables.push_back(table("Forces",20));
  provider.data->increments[1].tables.push_back(table("Forces",30));
  PostprocessorCore core;
  std::string error;
  check(core.open_result(provider,SimResults::JobSourceReference{"test","",1,""},std::nullopt,error),"Cannot open table-only source");
  const auto source = core.session().active_source_id;
  check(core.session().representations.empty(),"Table-only source fabricated a mesh representation");
  check(core.open_table(source,"view1","Forces",false,error),"Cannot open native DAT table");
  auto data = core.table_data("view1");
  check(data.rows == 2 && result_table_cell(data,0,4) == "9007199254740993", "History table lost rows or exact 64-bit identifier");
  check(result_table_cell(data,0,6) == "load_target", "Table string column changed");
  check(result_table_cell(data,0,0) == "1" && result_table_cell(data,1,0) == "2" &&
    result_table_cell(data,0,5) == "20" && result_table_cell(data,1,5) == "30","History table did not retain both frames");
  check(core.set_frame(source,"view1",1,error),"Table-only frame navigation failed");
  check(core.table_data("view1").rows == 2 && core.table_data("view1").active_frame == 1 &&
    result_table_cell(core.table_data("view1"),0,5) == "20", "Frame navigation replaced history rows");
  PostprocessorTableModel model(nullptr);
  model.set_data(core.table_data("view1"));
  check(model.rowCount() == 2 && model.columnCount() == 7 &&
    model.data(model.index(0,4),Qt::UserRole).toLongLong() == 9007199254740993LL,"Qt model lost integer sorting precision");
  check(!model.data(model.index(0,0),PostprocessorTableModel::ActiveFrameRole).toBool() &&
    model.data(model.index(1,0),PostprocessorTableModel::ActiveFrameRole).toBool(),"Active frame highlight is incorrect");
  int resets = 0;
  QObject::connect(&model,&QAbstractItemModel::modelReset,[&]()
  {
    ++resets;
  });
  model.set_active_frame(0);
  check(!resets && model.data(model.index(0,0),PostprocessorTableModel::ActiveFrameRole).toBool() &&
    !model.data(model.index(1,0),PostprocessorTableModel::ActiveFrameRole).toBool(),"Moving the highlight reset the model or retained old rows");
  PostprocessorTableView pane([]()
  {
  },nullptr);
  pane.show_data(core.table_data("view1"));
  auto* widget = pane.findChild<QTableView*>("Postprocessor.TableView");
  if (widget)
  {
    widget->selectRow(0);
    int paneResets = 0;
    QObject::connect(widget->model(),&QAbstractItemModel::modelReset,&pane,[&]()
    {
      ++paneResets;
    });
    auto moved = core.table_data("view1");
    moved.active_frame = 0;
    pane.show_data(std::move(moved));
    check(!paneResets && widget->selectionModel()->selectedRows().size() == 1 &&
      widget->selectionModel()->selectedRows().front().row() == 0,"Frame highlight reset the pane or overwrote user selection");
  }
  auto varied = std::make_shared<SimResults::VtkResultsData>();
  varied->increments.resize(3);
  varied->increments[0].tables.push_back(table("History",1));
  auto additional = table("History",2);
  auto energy = vtkSmartPointer<vtkDoubleArray>::New();
  energy->SetName("Energy"); energy->InsertNextValue(7);
  additional->AddColumn(energy);
  varied->increments[2].tables.push_back(additional);
  TableReference history;
  history.key = result_tables(*varied,std::nullopt).front().key;
  history.frame = 1;
  const auto combined = result_table_data(varied,history);
  check(combined.rows == 2 && combined.columns.size() == 8 &&
    result_table_cell(combined,0,7) == "Unavailable" && result_table_cell(combined,1,7) == "7" &&
    result_table_cell(combined,1,0) == "3","Schema union or missing-frame handling shifted history records");
  check(result_table_row_frame(combined,0) == 0 && result_table_row_frame(combined,1) == 2 &&
    !combined.status.empty(),"Missing active frame did not preserve row identities/status");
  PostprocessorResultsTree tree([](const QString&)
  {
  },[](const QString&,bool)
  {
  });
  tree.refresh(core.session(),[&](const std::string& id)
  {
    return core.result_data(id);
  },[&](const std::string& id,std::size_t frame)
  {
    return core.frame_has_mesh(id,frame);
  });
  int tableItems = 0;
  for (QTreeWidgetItemIterator item(&tree); *item; ++item)
  {
    if ((*item)->data(0,Qt::UserRole).toInt() == static_cast<int>(ResultTreeItemKind::Table))
    {
      ++tableItems;
      check((*item)->parent() && (*item)->parent()->text(0) == "Tables","Tree retained an unnecessary table-scope layer");
    }
  }
  check(tableItems == 2,"Tree duplicated a table family across frames");
  check(core.open_table(source,"view1","Summary",false,error) && core.set_frame(source,"view1",0,error),"Cannot navigate global table");
  check(result_table_cell(core.table_data("view1"),0,1) == "10", "Global table changed with the frame");
  check(core.show_3d("view1",error) && !core.session().views.at("view1").table,"3D switch retained table state");
  check(core.open_table(source,"view1","Forces",false,error) && core.close_source(source,error) &&
    !core.session().views.at("view1").table,"Source closure retained table reference");
  QTemporaryDir directory;
  const auto path = directory.filePath("forces.vtt").toUtf8().toStdString();
  auto original = table("External",1.23456789012345);
  auto writer = vtkSmartPointer<vtkXMLTableWriter>::New();
  writer->SetFileName(path.c_str());
  writer->SetInputData(original.get());
  check(writer->Write() != 0,"Cannot create VTT verification fixture");
  VtkFileSourceProvider reader;
  SimResults::PublishedResultSource publication;
  check(reader.open(SimResults::FileSourceReference{"vtk",{path}},publication,error) &&
    publication.data && publication.data->tables.size() == 1,"Standalone VTT did not publish a table source");
  if (publication.data)
  {
    const auto savedReference = publication.reference;
    SimResults::PublishedResultSource restored;
    check(reader.restore(savedReference,restored,error),"Unchanged VTT reference did not restore");
    auto* changed = vtkDoubleArray::SafeDownCast(original->GetColumnByName("Value"));
    changed->SetValue(0,999);
    writer->Write();
    check(!reader.restore(savedReference,restored,error),"Changed VTT file was silently substituted");
    const auto list = result_tables(*publication.data,std::nullopt);
    check(list.size() == 1,"VTT table catalogue missing");
    if (!list.empty())
    {
      TableReference reference;
      reference.key = list.front().key;
      data = result_table_data(publication.data,reference);
      check(result_table_cell(data,0,0) == "9007199254740993" && result_table_cell(data,0,1).size() > 10,
        "VTT reader/display lost integer or floating-point precision");
    }
  }
  return failures ? 1 : 0;
}
