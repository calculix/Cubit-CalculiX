#include "PostPickCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <charconv>
#include <cmath>
#include <exception>
#include <utility>

PostPickCommand::PostPickCommand(std::weak_ptr<PostprocessorApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> PostPickCommand::get_syntax()
{
  const std::string id = "{<value:label='entity_id'>|<string:type='quoted',number='1',label='entity_text'>}";
  const std::string scope = " dataset {<value:label='dataset_id'>|<string:type='quoted',number='1',label='dataset_name'>}"
    " [source <string:type='quoted',number='1',label='source_name'>]";
  return {"post pick mode {node|element|integrationpoint}","post pick clear",
    "post pick node "+id+scope,"post pick element "+id+scope,
    "post pick integrationpoint <value:label='ip'> element "+id+scope};
}
std::vector<std::string> PostPickCommand::get_syntax_help()
{
  return {"post pick mode {node|element|integrationpoint}","post pick clear",
    "post pick {node|element} ID dataset {ID|\"name\"} [source \"name\"]",
    "post pick integrationpoint N element ID dataset {ID|\"name\"} [source \"name\"]"};
}
std::vector<std::string> PostPickCommand::get_help()
{
  return {"Select one original result entity in the active view and make its source active.",
    "Quote decimal entity IDs above 9007199254740991 for exact 64-bit transport. GUI picks do this automatically.",
    "Pick clear retains source selection; mode defaults to Element. Missing frame entities remain selected but unavailable."};
}
bool PostPickCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    if (!active)
    {
      PRINT_ERROR("Postprocessor selection is unavailable.\n");
      return false;
    }
    using Kind = Postprocessor::EntityKind;
    const auto kind = data.find_keyword("INTEGRATIONPOINT") ? Kind::IntegrationPoint :
      data.find_keyword("NODE") ? Kind::Node : Kind::Element;
    std::string error;
    bool accepted = false;
    if (data.find_keyword("CLEAR"))
    {
      accepted = active->clear_pick(error);
    }
    else if (data.find_keyword("MODE"))
    {
      accepted = active->set_pick_mode(kind,error);
    }
    else
    {
      std::int64_t id = -1;
      std::string exact;
      double number = 0;
      if (data.get_string("entity_text",exact))
      {
        const auto parsed = std::from_chars(exact.data(),exact.data()+exact.size(),id);
        if (parsed.ec != std::errc{} || parsed.ptr != exact.data()+exact.size() || id < 0)
        {
          PRINT_ERROR("Entity ID requires a nonnegative decimal 64-bit integer.\n");
          return false;
        }
      }
      else if (data.get_value("entity_id",number) && std::isfinite(number) && number >= 0 &&
          number <= 9007199254740991.0 && number == std::floor(number))
      {
        id = static_cast<std::int64_t>(number);
      }
      else
      {
        PRINT_ERROR("Entity ID is invalid; quote large decimal IDs for exact transport.\n");
        return false;
      }
      std::optional<std::int64_t> ip;
      if (kind == Kind::IntegrationPoint)
      {
        if (!data.get_value("ip",number) || !std::isfinite(number) || number < 1 ||
            number > 2147483647 || number != std::floor(number))
        {
          PRINT_ERROR("Integration-point index must be an integer in [1,2147483647].\n");
          return false;
        }
        ip = static_cast<std::int64_t>(number);
      }
      std::string datasetName, source;
      PostprocessorApplicationInterface::DatasetSelector dataset;
      if (data.get_string("dataset_name",datasetName))
      {
        dataset = datasetName;
      }
      else if (data.get_value("dataset_id",number) && std::isfinite(number) && number >= 0 &&
          number <= 2147483647 && number == std::floor(number))
      {
        dataset = static_cast<std::size_t>(number);
      }
      else
      {
        PRINT_ERROR("Dataset requires a numeric ID or a quoted published name.\n");
        return false;
      }
      if (data.find_keyword("SOURCE") && (!data.get_string("source_name",source) || source.empty()))
      {
        PRINT_ERROR("Source requires a nonempty quoted name.\n");
        return false;
      }
      accepted = active->pick_entity(source,dataset,kind,id,ip,error);
    }
    if (!accepted)
    {
      PRINT_ERROR("Cannot change result selection: %s\n",error.c_str());
    }
    return accepted;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot change result selection: %s\n",exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot change result selection: unknown failure.\n");
  }
  return false;
}
