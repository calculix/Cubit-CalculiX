#include "FieldSelectorTests.hpp"
#include "Postprocessor/Core/representations/ResultFieldSelector.hpp"
#include "Postprocessor/Commands/FieldCommandText.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <algorithm>
#include <iostream>
int field_selector_cases(const std::shared_ptr<PostprocessorApplicationInterface>& app)
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
  const auto group = [](const std::string& origin,SimResults::Association association,std::size_t dataset)
  {
    ResultFieldGroup result;
    result.field.name = "CONTACT";
    result.field.source = origin;
    result.field.association = association;
    result.field.array_name = origin+"_CONTACT";
    result.field.components = {"COPEN","CPRESS"};
    result.field.dataset_id = dataset;
    result.key = field_group_key(result.field);
    result.datasets = {dataset};
    result.available = true;
    return result;
  };
  const auto frd = group("FRD",SimResults::Association::Node,0);
  const auto dat = group("DAT",SimResults::Association::Node,0);
  const auto ip = group("FRD",SimResults::Association::IntegrationPoint,1);
  std::string error;
  ResultFieldGroup resolved;
  FieldSelection selected{0,frd.field.array_name,frd.field.association,"COPEN",false,frd.key};
  ResultFieldSelector readable;
  check(readable_result_field({frd},selected,readable,error) && readable.name == "CONTACT" &&
      !readable.qualifiers.origin && !readable.qualifiers.association && field_command_text(readable) == "field \"CONTACT\"",
    "Unique field emitted an opaque key or unnecessary qualifiers");
  check(readable_result_field({frd,dat},selected,readable,error) && readable.qualifiers.origin == "FRD" &&
      field_command_text(readable) == "field \"CONTACT\" origin \"FRD\"",
    "Ambiguous origins did not generate a readable origin qualifier");
  check(resolve_result_field({dat,frd},readable,0,"COPEN",false,false,resolved,error) && resolved.key == frd.key,
    "Readable field did not replay to the exact identity after catalogue reordering");
  check(!resolve_result_field({frd,dat},{"CONTACT",{}},0,"COPEN",false,false,resolved,error) &&
      error.find("FRD") != std::string::npos && error.find("DAT") != std::string::npos && error.find(frd.key) == std::string::npos,
    "Ambiguity silently chose a field or printed an opaque candidate key");
  check(resolve_result_field({frd,dat},{frd.key,{}},0,"COPEN",false,false,resolved,error) && resolved.key == frd.key,
    "Existing journal group key no longer replays");
  check(resolve_result_field({frd,dat},{frd.field.array_name,{}},0,"COPEN",false,false,resolved,error) && resolved.key == frd.key,
    "Existing array-name journal no longer replays");
  auto nodal = frd;
  auto sameDatasetIp = ip;
  sameDatasetIp.datasets = {0};
  check(readable_result_field({nodal,sameDatasetIp},selected,readable,error) &&
      readable.qualifiers.association == SimResults::Association::Node,
    "Association collision was not qualified");
  selected.dataset_id.reset();
  check(readable_result_field({frd,ip},selected,readable,error) && readable.qualifiers.association,
    "No-dataset field ignored an association collision");
  check(resolve_result_field({frd,ip},{"CONTACT",{}},1,"COPEN",false,false,resolved,error) && resolved.key == ip.key,
    "Dataset scope did not select integration-point field");
  auto differentComponent = dat;
  differentComponent.field.components = {"OTHER"};
  differentComponent.key = field_group_key(differentComponent.field);
  check(resolve_result_field({frd,differentComponent},{"CONTACT",{}},0,"COPEN",false,false,resolved,error) && resolved.key == frd.key,
    "Component scope did not disambiguate a field");
  auto duplicate = frd;
  duplicate.field.set_name = "other set";
  duplicate.key = field_group_key(duplicate.field);
  selected.dataset_id = 0;
  readable.name = "unchanged";
  check(!readable_result_field({frd,duplicate},selected,readable,error) && readable.name == "unchanged",
    "Unrepresentable ambiguity fell back to an opaque journal field or changed output");
  auto block1 = ip;
  block1.field.name = block1.field.array_name = "strains";
  block1.field.source = "DAT";
  block1.field.set_name = "ip_BLOCK_1";
  block1.field.components = {"exx","eyy"};
  block1.key = field_group_key(block1.field);
  auto block2 = block1;
  block2.field.set_name = "ip_BLOCK_2";
  block2.field.dataset_id = 2;
  block2.datasets = {2};
  block2.key = field_group_key(block2.field);
  FieldSelection blockSelection{std::nullopt,block1.field.array_name,block1.field.association,"exx",false,block1.key};
  check(readable_result_field({block1,block2},blockSelection,readable,error) &&
      readable.qualifiers.set_name == "ip_BLOCK_1" &&
      field_command_text(readable) == "field \"strains\" set \"ip_BLOCK_1\"",
    "Repeated DAT integration-point fields did not generate a readable set qualifier");
  check(resolve_result_field({block2,block1},readable,std::nullopt,"exx",false,false,resolved,error) && resolved.key == block1.key,
    "Set-qualified field did not replay to its exact identity after catalogue reordering");
  auto foreign = block1;
  foreign.field.source = "FRD";
  foreign.key = field_group_key(foreign.field);
  check(readable_result_field({block2,foreign,block1},blockSelection,readable,error) &&
      readable.qualifiers.set_name == "ip_BLOCK_1" && readable.qualifiers.origin == "DAT",
    "Origin and set collision was not jointly qualified");
  check(resolve_result_field({foreign,block2,block1},readable,std::nullopt,"exx",false,false,resolved,error) && resolved.key == block1.key,
    "Combined origin/set qualifiers resolved to another field");
  readable.qualifiers.set_name = "missing_set";
  check(!resolve_result_field({block1,block2},readable,std::nullopt,"exx",false,false,resolved,error),
    "An unknown set silently selected another result group");
  readable.qualifiers.set_name = "set \"with quotes\"";
  check(field_command_text(readable).contains("set 'set \"with quotes\"'"),"Set names were not safely quoted");
  const auto source = app->session().active_source_id;
  const auto settings = app->coloring("Model",error);
  if (settings && settings->field)
  {
    const auto before = *settings;
    const auto groups = app->field_groups(source,std::nullopt);
    ResultFieldSelector selector;
    check(app->readable_field(source,*settings->field,selector,error),"Application did not resolve a readable selection");
    check(app->color_field("Model",selector.name,settings->field->component,settings->field->magnitude,
        settings->field->dataset_id,error,selector.qualifiers) && same_coloring(before,*app->coloring("Model",error)),
      "Readable coloring changed the selected field identity");
    EntityReference entity{source,0,EntityKind::Node,9007199254740993LL,std::nullopt};
    const auto commands = plot_entity_commands(*app,entity);
    check(!commands.empty(),"Cannot generate readable history command");
    if (!commands.empty())
    {
      const auto& command = commands.back();
      check(command.contains(" field ") && !command.contains(QString::fromStdString(settings->field->group_key)),
        "History journal still contains the encoded field identity");
    }
  }
  return failures;
}
