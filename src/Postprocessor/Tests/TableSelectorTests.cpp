#include "TableSelectorTests.hpp"
#include "Postprocessor/Core/tables/ResultTableSelector.hpp"
#include "Postprocessor/Core/plots/PlotCodec.hpp"
#include "Postprocessor/Commands/tables/TableCommandText.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QJsonArray>
#include <algorithm>
#include <iostream>
int table_selector_cases(const std::shared_ptr<PostprocessorApplicationInterface>& app)
{
  using namespace Postprocessor;
  int failures = 0;
  const auto check = [&](bool value,const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  const ResultTableDescriptor frame{"encoded-frame-key","forces / LOAD_TARGET","Frame tables",false,"Forces",1};
  const ResultTableDescriptor global{"encoded-global-key",frame.name,"Global tables",true,"Forces",1};
  ResultTableDescriptor resolved;
  ResultTableSelector selector;
  check(readable_result_table({frame},frame.key,selector,error) && selector.name == frame.name &&
      !selector.qualifiers.global && !selector.qualifiers.type && !selector.qualifiers.entry,
    "Unique table required an opaque key or unnecessary qualifiers");
  check(readable_result_table({frame,global},frame.key,selector,error) && selector.qualifiers.global.has_value() &&
      !*selector.qualifiers.global && resolve_result_table({global,frame},selector,resolved,error) && resolved.key == frame.key,
    "Scope-qualified table did not replay independently of catalogue order");
  auto typed = frame;
  typed.key = "typed-key";
  typed.type = "Displacements";
  check(readable_result_table({typed,frame},frame.key,selector,error) && selector.qualifiers.type == "Forces" &&
      resolve_result_table({frame,typed},selector,resolved,error) && resolved.key == frame.key,
    "Table type did not distinguish identical names in one scope");
  auto second = frame;
  second.key = "second-key";
  second.entry = 2;
  check(readable_result_table({frame,second},second.key,selector,error) && selector.qualifiers.entry == 2 &&
      resolve_result_table({second,frame},selector,resolved,error) && resolved.key == second.key,
    "Repeated table family did not preserve its entry identity");
  check(!resolve_result_table({frame,global},{frame.name,{}},resolved,error) &&
      error.find("scope frame") != std::string::npos && error.find("scope global") != std::string::npos &&
      error.find(frame.key) == std::string::npos,"Ambiguity selected a first table or exposed encoded keys");
  check(resolve_result_table({global,frame},{frame.key,{}},resolved,error) && resolved.key == frame.key,
    "Legacy encoded table key no longer resolves");
  auto emptyType = frame;
  emptyType.type.clear();
  emptyType.key = "empty-type-key";
  check(readable_result_table({frame,emptyType},emptyType.key,selector,error) && selector.qualifiers.type &&
      selector.qualifiers.type->empty(),"Empty TableType metadata could not be qualified");
  ResultTableSelector invalid{frame.name,{}};
  invalid.qualifiers.entry = 0;
  resolved.key = "unchanged";
  check(!resolve_result_table({frame},invalid,resolved,error) && resolved.key == "unchanged",
    "Invalid entry mutated selector output");

  const auto source = app->session().active_source_id;
  const auto coloring = app->coloring("Model",error);
  if (!coloring || !coloring->field)
  {
    check(false,"Missing spatial table test field");
    return failures;
  }
  auto field = *coloring->field;
  // Tables expose the complete field, so uniqueness must not rely on a chosen component.
  field.component.clear();
  field.magnitude = false;
  field.dataset_id.reset();
  ResultFieldSelector readable;
  check(app->readable_field(source,field,readable,error),"Cannot obtain readable spatial table name");
  const TableReference reference{source,field.group_key,true,0};
  const auto command = table_open_command_text(*app,reference,error);
  check(command.startsWith("post table field ") && command.contains(QString::fromStdString(readable.name)) &&
      !command.contains(QString::fromStdString(field.group_key)),"Spatial table command still emits an encoded key");
  check(app->open_table("Model",readable.name,true,error,{},readable.qualifiers) &&
      app->session().views.at(app->session().active_view_id).table->key == field.group_key,
    "Readable spatial table did not restore exact field identity");
  const auto opened = *app->session().views.at(app->session().active_view_id).table;
  const auto revision = app->display_revision();
  check(!app->open_table("Model","missing table",false,error) &&
      app->session().views.at(app->session().active_view_id).table->key == opened.key && app->display_revision() == revision,
    "Failed table resolution changed the view");
  check(app->show_3d(error),"Cannot close spatial table fixture");
  check(app->create_plot("Field table fixture",error),"Cannot create spatial table plot fixture");
  const auto plotId = app->session().active_plot_id;
  check(app->add_plot_table("Field table fixture","nodal field","Model",readable.name,"Time","SXX","Node ID",
      "9007199254740993",error,{},true,readable.qualifiers),"Spatial field table was treated as a native table");
  if (!app->session().plots.at(plotId).curves.empty())
  {
    const auto curve = app->session().plots.at(plotId).curves.front();
    const auto& table = std::get<PlotTableReference>(curve.input);
    check(table.field && table.key == field.group_key,"Field plot lost its exact reference kind/key");
    PlotData data;
    check(app->plot_data(plotId,data,error) && !data.curves.empty() && !data.curves.front().samples.empty() &&
        data.curves.front().samples.front().y == 10,"Spatial field table plot did not use original nodal values");
    auto native = curve;
    std::get<PlotTableReference>(native.input).field = false;
    check(!same_plot_input(curve,native),"Plot cache ignored native/field table distinction");
    const auto encoded = plots_to_json(app->session().plots);
    std::map<std::string,PlotDefinition> restored;
    check(plots_from_json(encoded,restored) && std::get<PlotTableReference>(restored.at(plotId).curves.front().input).field,
      "Plot project roundtrip lost field-table flag");
    auto legacy = encoded;
    auto plot = legacy[0].toObject();
    auto curves = plot.value("curves").toArray();
    auto item = curves[0].toObject();
    auto input = item.value("input").toObject();
    input.remove("field");
    item.insert("input",input);
    curves[0] = item;
    plot.insert("curves",curves);
    legacy[0] = plot;
    check(plots_from_json(legacy,restored) && !std::get<PlotTableReference>(restored.at(plotId).curves.front().input).field,
      "Legacy plot without a field-table flag did not default to native table");
    input.insert("field","invalid");
    item.insert("input",input);
    curves[0] = item;
    plot.insert("curves",curves);
    legacy[0] = plot;
    check(!plots_from_json(legacy,restored),"Plot codec accepted a non-boolean field-table flag");
  }
  check(!app->add_plot_table("Field table fixture","mixed nodes","Model",readable.name,"Time","SXX",{}, {},
      error,{},true,readable.qualifiers),"Field table plot connected different node identities into a time history");
  check(app->change_plot("Field table fixture","remove","",error),"Cannot remove spatial table plot fixture");
  check(app->color_field("Model",field.group_key,coloring->field->component,coloring->field->magnitude,
      coloring->field->dataset_id,error),"Cannot restore active representation after table test");
  return failures;
}
