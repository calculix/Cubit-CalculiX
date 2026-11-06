#include "PostListCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"

#include <exception>
#include <type_traits>
#include <utility>
#include <variant>

PostListCommand::PostListCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostListCommand::get_syntax()
{
  return {"post list"};
}

std::vector<std::string> PostListCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> PostListCommand::get_help()
{
  return {"List opened sources by name and origin. An asterisk marks the active source."};
}

bool PostListCommand::execute(CubitCommandData&)
{
  const auto active = application.lock();

  if (!active)
  {
    PRINT_ERROR("The Postprocessor application is unavailable.\n");
    return false;
  }

  try
  {
    const auto& session = active->session();

    if (session.sources.empty())
    {
      PRINT_INFO("No Postprocessor sources are open.\n");
      return true;
    }

    PRINT_INFO("Postprocessor sources (* active):\n");

    for (const auto& entry : session.sources)
    {
      const auto& source = entry.second;
      const std::string origin = std::visit(
        [](const auto& reference) -> std::string
        {
          using Reference = std::decay_t<decltype(reference)>;

          if constexpr (std::is_same_v<Reference, Postprocessor::JobSourceReference>)
          {
            return "job: " + reference.application_id + " " + std::to_string(reference.job_id);
          }
          else if constexpr (std::is_same_v<Reference, Postprocessor::FileSourceReference>)
          {
            std::string result = "files: " + reference.provider_id;

            for (const auto& path : reference.paths)
            {
              result += " \"" + path + "\"";
            }

            return result;
          }
          else
          {
            return "Cubit geometry";
          }
        }, source.reference);

      PRINT_INFO("%s \"%s\" | %s\n", entry.first == session.active_source_id ? "*" : " ",
        source.name.c_str(), origin.c_str());
    }

    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Postprocessor listing failed: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Postprocessor listing failed with an unknown exception.\n");
    return false;
  }
}
