#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/sources/VtkFileSourceProvider.hpp"
#include "Postprocessor/GUI/Tables/PostprocessorTableModel.hpp"
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
  check(data.rows == 1 && result_table_cell(data,0,0) == "9007199254740993", "Table lost exact 64-bit identifier");
  check(result_table_cell(data,0,2) == "load_target", "Table string column changed");
  check(core.set_frame(source,"view1",1,error),"Table-only frame navigation failed");
  check(result_table_cell(core.table_data("view1"),0,1) == "30", "Native table retained stale frame values");
  PostprocessorTableModel model(nullptr);
  model.set_data(core.table_data("view1"));
  check(model.rowCount() == 1 && model.columnCount() == 3 &&
    model.data(model.index(0,0),Qt::UserRole).toLongLong() == 9007199254740993LL,"Qt model lost integer sorting precision");
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
