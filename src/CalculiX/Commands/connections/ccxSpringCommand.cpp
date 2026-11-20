#include "ccxSpringCommand.hpp"
#include "DiscreteCommandInput.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxSpringCommand::get_syntax()
{
  const std::string id = " <value:label='id'>";
  const std::string name = "name <string:type='quoted',number='1',label='name'>";
  const std::string first = "{vertex <value:label='vertex_a'> | rp <value:label='point_a'>}";
  const std::string ends = "from " + first + " to {vertex <value:label='vertex_b'> | rp <value:label='point_b'> | ground direction {x | y | z}}";
  const std::string value = "stiffness <value:label='amount'> [unit <string:type='quoted',number='1',label='unit_name'>]";
  const std::string history = " [history {all | off | step <value:label='step'>}]";
  return {"ccx create spring" + id + " " + name + " " + ends + " " + value + history,
    "ccx modify spring" + id + " [" + name + "] [" + ends + "] [" + value + "]" + history};
}

std::vector<std::string> ccxSpringCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ccxSpringCommand::get_help()
{
  return {"Create or modify a spring. Values use the active display unit unless a unit is specified."};
}

bool ccxSpringCommand::execute(CubitCommandData& data)
{
  CalculiXConnectionsInterface iface;
  ConnectionSpring record;
  data.get_value("id",record.id);
  const bool modify = data.find_keyword("MODIFY");
  if (modify && !iface.spring(record.id,record))
  {
    PRINT_ERROR("Unknown spring %d.\n",record.id);
    return false;
  }
  std::string name;
  if (data.get_string("name",name))
  {
    record.name = name;
  }
  DiscreteCommandInput::endpoint(data,"vertex_a","point_a",record.first);
  ConnectionEndpoint end;
  if (DiscreteCommandInput::endpoint(data,"vertex_b","point_b",end))
  {
    record.second = end;
    record.direction = 0;
  }
  else if (data.find_keyword("GROUND"))
  {
    record.second.reset();
    record.direction = data.find_keyword("X") ? 1 : (data.find_keyword("Y") ? 2 : 3);
  }
  std::string error;
  if (!DiscreteCommandInput::quantity(data,"force_per_length",record.stiffness_si,error) ||
      !DiscreteCommandInput::history(data,record.history_step,error) || !iface.save(record,modify,error))
  {
    PRINT_ERROR("%s\n",error.c_str());
    return false;
  }
  return true;
}
