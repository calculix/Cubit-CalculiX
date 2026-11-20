#include "ccxPointMassCommand.hpp"
#include "DiscreteCommandInput.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxPointMassCommand::get_syntax()
{
  const std::string id = " <value:label='id'>";
  const std::string name = "name <string:type='quoted',number='1',label='name'>";
  const std::string first = "{vertex <value:label='vertex_a'> | rp <value:label='point_a'>}";
  const std::string ends = first;
  const std::string value = "mass <value:label='amount'> [unit <string:type='quoted',number='1',label='unit_name'>]";
  const std::string history = " [history {all | off | step <value:label='step'>}]";
  return {"ccx create point mass" + id + " " + name + " " + ends + " " + value + history,
    "ccx modify point mass" + id + " [" + name + "] [" + ends + "] [" + value + "]" + history};
}

std::vector<std::string> ccxPointMassCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ccxPointMassCommand::get_help()
{
  return {"Create or modify a point mass. Values use the active display unit unless a unit is specified."};
}

bool ccxPointMassCommand::execute(CubitCommandData& data)
{
  CalculiXConnectionsInterface iface;
  ConnectionPointMass record;
  data.get_value("id",record.id);
  const bool modify = data.find_keyword("MODIFY");
  if (modify && !iface.point_mass(record.id,record))
  {
    PRINT_ERROR("Unknown point mass %d.\n",record.id);
    return false;
  }
  std::string name;
  if (data.get_string("name",name))
  {
    record.name = name;
  }
  DiscreteCommandInput::endpoint(data,"vertex_a","point_a",record.endpoint);
  std::string error;
  if (!DiscreteCommandInput::quantity(data,"mass",record.mass_si,error) ||
      !DiscreteCommandInput::history(data,record.history_step,error) || !iface.save(record,modify,error))
  {
    PRINT_ERROR("%s\n",error.c_str());
    return false;
  }
  return true;
}
