#include "PostLegendCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>
PostLegendCommand::PostLegendCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}
std::vector<std::string> PostLegendCommand::get_syntax()
{
  const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
  return {"post legend {on|off}"+source,
    "post legend position <value:label='x'> <value:label='y'>"+source,
    "post legend position automatic"+source};
}
std::vector<std::string> PostLegendCommand::get_syntax_help()
{
  return {"post legend {on|off} [source \"name\"]",
    "post legend position X Y [source \"name\"]","post legend position automatic [source \"name\"]"};
}
std::vector<std::string> PostLegendCommand::get_help()
{
  return {"Show, hide, or position this source representation's result legend in the active view.",
    "Position uses normalized bottom-left viewport coordinates in [0,1]; automatic restores layout."};
}
bool PostLegendCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name, error;
    if (!active || (data.find_keyword("SOURCE") && (!data.get_string("source_name", name) || name.empty())))
    {
      PRINT_ERROR("Legend requires an available source.\n");
      return false;
    }
    auto c = active->coloring(name, error);
    if (!c)
    {
      PRINT_ERROR("Cannot change legend: %s\n", error.c_str());
      return false;
    }
    if (data.find_keyword("POSITION"))
    {
      if (data.find_keyword("AUTOMATIC"))
      {
        c->legend_position.reset();
      }
      else
      {
        std::array<double,2> position;
        if (!data.get_value("x",position[0]) || !data.get_value("y",position[1]))
        {
          PRINT_ERROR("Legend position requires X and Y coordinates.\n");
          return false;
        }
        c->legend_position = position;
      }
    }
    else
    {
      c->legend_visible = data.find_keyword("ON");
    }
    if (!active->set_coloring(name, *c, error))
    {
      PRINT_ERROR("Cannot change legend: %s\n", error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot change legend: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot change legend: unknown failure.\n");
    return false;
  }
}
