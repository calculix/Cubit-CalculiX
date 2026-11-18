#include "PostExplodeCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <cctype>
#include <exception>
#include <utility>
PostExplodeCommand::PostExplodeCommand(std::weak_ptr<PostprocessorApplicationInterface> app)
  : application(std::move(app))
{
}
std::vector<std::string> PostExplodeCommand::get_syntax()
{
  const std::string scope = " [source <string:type='quoted',number='1',label='source_name'>] [view <string:type='quoted',number='1',label='view_name'>]";
  const std::string block = " [block <string:type='quoted',number='1',label='block_name'>]";
  const std::string pulseScope = " [view <string:type='quoted',number='1',label='view_name'>]";
  return {"post explode"+scope,"post explode {on|off|reset|recalculate}"+scope,
    "post explode pulse {on|off}"+pulseScope,
    "post explode pulse period <value:label='pulse_value'>"+pulseScope,
    "post explode pulse advance <value:label='pulse_value'>"+pulseScope,
    "post explode pulse range <value:label='pulse_min'> <value:label='pulse_max'>"+pulseScope,
    "post explode pulse target {components|mesh|both}"+pulseScope,
    "post explode components {on|off}"+scope,
    "post explode amount <value:label='amount'>"+scope,
    "post explode gap <value:label='amount'>"+scope,
    "post explode grouping {auto|sources|blocks}"+scope,
    "post explode direction {auto|x|y|z|radial}"+scope,
    "post explode mesh {on|off}"+block+scope,
    "post explode mesh amount <value:label='amount'>"+block+scope,
    "post explode offset block <string:type='quoted',number='1',label='block_name'> <value:label='offset_x'> <value:label='offset_y'> <value:label='offset_z'> [unit <string:type='quoted',number='1',label='unit_name'>]"+scope};
}
std::vector<std::string> PostExplodeCommand::get_syntax_help()
{
  return {"post explode [on|off|reset|recalculate] [source \"name\"] [view \"name\"]",
    "post explode pulse on|off; post explode pulse period SECONDS; post explode pulse range MIN MAX",
    "post explode pulse target components|mesh|both [view \"name\"]",
    "post explode components on|off; post explode amount VALUE; post explode gap FRACTION",
    "post explode grouping auto|sources|blocks; post explode direction auto|x|y|z|radial",
    "post explode mesh on|off|amount VALUE [block \"name\"] [source \"name\"]",
    "post explode offset block \"name\" X Y Z [unit \"mm\"] source \"name\""};
}
std::vector<std::string> PostExplodeCommand::get_help()
{
  return {"Presentation only: preserve solver values and IDs. The default is off.",
    "Bare post explode separates visible sources, then blocks for one source, then elements for a single block.",
    "Automatic axis layout minimizes squared movement with a 5% assembly-diagonal gap; component amount defaults to 1.",
    "Mesh amount defaults to 0.1 times the undeformed element-center offset from its block center.",
    "Offsets use fixed scene length units unless an explicit compatible unit is supplied. Reset clears both explosion modes."};
}
bool PostExplodeCommand::execute(CubitCommandData& data)
{
  std::string error;
  try
  {
    const auto app = application.lock();
    if (!app)
    {
      PRINT_ERROR("Exploded view is unavailable.\n");
      return false;
    }
    std::string source, view, block, unit, option;
    data.get_string("source_name",source);
    data.get_string("view_name",view);
    data.get_string("block_name",block);
    data.get_string("unit_name",unit);
    std::string action = "on";
    double amount = 0;
    std::array<double,3> offset = {0,0,0};
    if (data.find_keyword("PULSE"))
    {
      action = "pulse-on";
      if (data.find_keyword("OFF"))
      {
        action = "pulse-off";
      }
      if (data.find_keyword("PERIOD") || data.find_keyword("ADVANCE"))
      {
        action = "pulse-period";
        if (data.find_keyword("ADVANCE"))
        {
          action = "pulse-advance";
        }
        if (!data.get_value("pulse_value",amount))
        {
          PRINT_ERROR("Pulse requires a numeric time in seconds.\n");
          return false;
        }
      }
      if (data.find_keyword("RANGE"))
      {
        action = "pulse-range";
        if (!data.get_value("pulse_min",offset[0]) || !data.get_value("pulse_max",offset[1]))
        {
          PRINT_ERROR("Pulse range requires minimum and maximum multipliers.\n");
          return false;
        }
      }
      if (data.find_keyword("TARGET"))
      {
        action = "pulse-target";
        for (const auto* name : {"components","mesh","both"})
        {
          std::string keyword(name);
          for (auto& c : keyword)
          {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
          }
          if (data.find_keyword(keyword.c_str()))
          {
            option = name;
          }
        }
      }
      if (app->change_explosion({},view,action,amount,option,{},offset,{},error))
      {
        return true;
      }
      PRINT_ERROR("Cannot change explosion pulse: %s\n",error.c_str());
      return false;
    }
    if (data.find_keyword("OFF"))
    {
      action = "off";
    }
    for (const auto* name : {"amount","gap","grouping","direction","offset","reset","recalculate"})
    {
      std::string keyword(name);
      for (auto& c : keyword)
      {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
      }
      if (data.find_keyword(keyword.c_str()))
      {
        action = name;
      }
    }
    if (data.find_keyword("MESH"))
    {
      action = "mesh-"+action;
    }
    else if (data.find_keyword("COMPONENTS"))
    {
      action = "components-"+action;
    }
    if (data.find_keyword("AMOUNT") || data.find_keyword("GAP"))
    {
      if (!data.get_value("amount",amount))
      {
        PRINT_ERROR("Explosion requires a numeric amount.\n");
        return false;
      }
    }
    if (action == "offset" && (!data.get_value("offset_x",offset[0]) ||
        !data.get_value("offset_y",offset[1]) || !data.get_value("offset_z",offset[2])))
    {
      PRINT_ERROR("Offset requires three scene-length coordinates.\n");
      return false;
    }
    if (action == "grouping" || action == "direction")
    {
      for (const auto* name : {"auto","sources","blocks","x","y","z","radial"})
      {
        std::string keyword(name);
        for (auto& c : keyword)
        {
          c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        if (data.find_keyword(keyword.c_str()))
        {
          option = name;
        }
      }
    }
    if (app->change_explosion(source,view,action,amount,option,block,offset,unit,error))
    {
      return true;
    }
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  PRINT_ERROR("Cannot change exploded view: %s\n",error.c_str());
  return false;
}
