#include "PostUserOptionsCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Framework/Commands/UserOptionCommandSupport.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>

PostUserOptionsCommand::PostUserOptionsCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostUserOptionsCommand::get_syntax()
{
  const auto active = application.lock();
  return active ? UserOptionCommandSupport::syntax("post useroptions", active->user_option_definitions()) : std::vector<std::string>{};
}

std::vector<std::string> PostUserOptionsCommand::get_syntax_help()
{
  auto result = get_syntax();
  for (auto& line : result)
  {
    const auto argument = line.find(" <");
    if (argument != std::string::npos)
    {
      line.erase(argument);
      line += " <value>";
    }
  }
  return result;
}

std::vector<std::string> PostUserOptionsCommand::get_help()
{
  return {"Set and persist one named option. Booleans accept on/off. Quote paths and colors. Use list for saved, replayable settings and restart notices."};
}

bool PostUserOptionsCommand::execute(CubitCommandData& data)
{
  const auto owner = application.lock();
  if (!owner)
  {
    return false;
  }
  auto& active = *owner;
  try
  {
    const auto schema = active.user_option_definitions();
    std::string error;
    if (data.find_keyword("LIST"))
    {
      UserOptionValues values;
      if (!active.user_option_values(values, error))
      {
        PRINT_ERROR("Cannot list user options: %s\n", error.c_str());
        return false;
      }
      std::vector<std::string> commands;
      for (const auto& item : schema)
      {
        std::string command;
        if (!UserOptionCommandSupport::format("post useroptions", item, values.at(item.key), command, error))
        {
          PRINT_ERROR("Cannot list user options: %s\n", error.c_str());
          return false;
        }
        commands.push_back(std::move(command));
      }
      for (const auto& command : commands)
      {
        PRINT_INFO("%s\n", command.c_str());
      }
      if (active.user_options_restart_required())
      {
        PRINT_INFO("Restart required: Postprocessor icons directory.\n");
      }
      return true;
    }
    UserOptionValues changes;
    if (!UserOptionCommandSupport::read(data, schema, changes, error) || !active.apply_user_options(changes, error))
    {
      PRINT_ERROR("Cannot set user option: %s\n", error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot access user options: %s\n", exception.what());
    return false;
  }
}
