#include <type_traits>
#include "PickCommandText.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <algorithm>
namespace
{
  std::string quote(const std::string& text)
  {
    const char delimiter = [&]() -> char {
      if (text.find('"') == std::string::npos) {
        return '"';
      } else {
        return '\'';
      }
    }();
    if (text.empty() || text.find(delimiter) != std::string::npos ||
        std::any_of(text.begin(),text.end(),[](unsigned char c)
        {
          return c < 32 || c == 127;
        }))
    {
      return {};
    }
    return std::string(1,delimiter)+text+delimiter;
  }
}
namespace Postprocessor
{
  bool pick_command_text(const PostprocessorApplicationInterface& application,
    const EntityReference& entity, std::string& command, std::string& error)
  {
    error.clear();
    command.clear();
    const auto source = application.session().sources.find(entity.source_id);
    const auto name = [&]() -> std::common_type_t<decltype(std::string{}), decltype(quote(source->second.name))> {
      if (source == application.session().sources.end()) {
        return std::string{};
      } else {
        return quote(source->second.name);
      }
    }();
    if (name.empty())
    {
      error = "The picked source name is unavailable or cannot be quoted safely.";
      return false;
    }
    const auto catalogue = application.mesh_datasets(entity.source_id);
    const auto item = std::find_if(catalogue.begin(),catalogue.end(),[&](const SimResults::Dataset& value)
    {
      return value.id == entity.dataset_id;
    });
    if (item == catalogue.end())
    {
      error = "The picked dataset is no longer available.";
      return false;
    }
    auto dataset = std::to_string(entity.dataset_id);
    const auto label = quote(item->name);
    if (!label.empty() && std::count_if(catalogue.begin(),catalogue.end(),[&](const SimResults::Dataset& value)
        {
          return value.name == item->name;
        }) == 1)
    {
      dataset = label;
    }
    auto id = std::to_string(entity.entity_id);
    if (entity.entity_id > 9007199254740991LL)
    {
      id = "\""+id+"\"";
    }
    if (entity.kind == EntityKind::IntegrationPoint && entity.integration_point)
    {
      command = "post pick integrationpoint "+std::to_string(*entity.integration_point)+" element "+id;
    }
    else
    {
      command = std::string("post pick ")+([&]() -> const char* {
        if (entity.kind == EntityKind::Node) {
          return "node ";
        } else {
          return "element ";
        }
      }())+id;
    }
    command += " dataset "+dataset+" source "+name;
    if (entity.display_element)
    {
      command += " owner \""+std::to_string(*entity.display_element)+"\"";
    }
    return true;
  }
}
