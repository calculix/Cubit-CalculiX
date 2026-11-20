#include "ccxReferencePointCreateCommand.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxReferencePointCreateCommand::get_syntax()
{
  return {"ccx create rp <value:label='id'> name <string:type='unquoted',number='1',label='name'> vertex <value:label='entity'> [rotation_vertex <value:label='rotation_vertex'>]",
    "ccx create rp <value:label='id'> name <string:type='unquoted',number='1',label='name'> center curve <value:label='entity'>",
    "ccx create rp <value:label='id'> name <string:type='unquoted',number='1',label='name'> center surface <value:label='entity'>",
    "ccx create rp <value:label='id'> name <string:type='unquoted',number='1',label='name'> center volume <value:label='entity'>",
    "ccx create rp <value:label='id'> name <string:type='unquoted',number='1',label='name'> xyz <value:label='x'> <value:label='y'> <value:label='z'>"};
}

std::vector<std::string> ccxReferencePointCreateCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ccxReferencePointCreateCommand::get_help()
{
  return {"Create a named reference point with an explicit journal ID."};
}

bool ccxReferencePointCreateCommand::execute(CubitCommandData& data)
{
  CalculiXConnectionsInterface iface;
  ConnectionPoint point;
  data.get_value("id", point.id);
  data.get_string("name", point.name);
  if (data.find_keyword("VERTEX"))
  {
    point.geometry = "vertex";
  }
  else if (data.find_keyword("CURVE"))
  {
    point.geometry = "curve";
  }
  else if (data.find_keyword("SURFACE"))
  {
    point.geometry = "surface";
  }
  else if (data.find_keyword("VOLUME"))
  {
    point.geometry = "volume";
  }
  else if (data.find_keyword("XYZ"))
  {
    point.geometry = "xyz";
  }
  if (point.geometry == "xyz")
  {
    data.get_value("x", point.position[0]);
    data.get_value("y", point.position[1]);
    data.get_value("z", point.position[2]);
  }
  else
  {
    data.get_value("entity", point.entity);
  }
  data.get_value("rotation_vertex", point.rotation_vertex);
  if (!iface.create(point))
  {
    PRINT_ERROR("Cannot create RP %d. Check the ID, name and placement.\n", point.id);
    return false;
  }
  return true;
}
