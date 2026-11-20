#include "ccxRigidBodyCreateCommand.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxRigidBodyCreateCommand::get_syntax()
{
  return {"ccx create rigid body <value:label='id'> name <string:type='unquoted',number='1',label='name'> curve <value:label='entity'> center",
    "ccx create rigid body <value:label='id'> name <string:type='unquoted',number='1',label='name'> surface <value:label='entity'> center",
    "ccx create rigid body <value:label='id'> name <string:type='unquoted',number='1',label='name'> volume <value:label='entity'> center",
    "ccx create rigid body <value:label='id'> name <string:type='unquoted',number='1',label='name'> curve <value:label='entity'> rp <value:label='point'>",
    "ccx create rigid body <value:label='id'> name <string:type='unquoted',number='1',label='name'> surface <value:label='entity'> rp <value:label='point'>",
    "ccx create rigid body <value:label='id'> name <string:type='unquoted',number='1',label='name'> volume <value:label='entity'> rp <value:label='point'>",
    "ccx create rigid body <value:label='id'> name <string:type='unquoted',number='1',label='name'> block <value:label='entity'> rp <value:label='point'>",
    "ccx create rigid body <value:label='id'> name <string:type='unquoted',number='1',label='name'> nodeset <value:label='entity'> rp <value:label='point'>"};
}

std::vector<std::string> ccxRigidBodyCreateCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ccxRigidBodyCreateCommand::get_help()
{
  return {"Create a rigid body attached to a named RP."};
}

bool ccxRigidBodyCreateCommand::execute(CubitCommandData& data)
{
  CalculiXConnectionsInterface iface;
  ConnectionRigidBody body;
  data.get_value("id", body.id);
  data.get_value("entity", body.entity);
  data.get_value("point", body.point);
  data.get_string("name", body.name);
  if (data.find_keyword("CURVE"))
  {
    body.geometry = "curve";
  }
  else if (data.find_keyword("SURFACE"))
  {
    body.geometry = "surface";
  }
  else if (data.find_keyword("VOLUME"))
  {
    body.geometry = "volume";
  }
  else if (data.find_keyword("BLOCK"))
  {
    body.geometry = "block";
  }
  else if (data.find_keyword("NODESET"))
  {
    body.geometry = "nodeset";
  }
  const bool created = data.find_keyword("CENTER") ? iface.create_at_center(body) : iface.create(body);
  if (!created)
  {
    PRINT_ERROR("Cannot create rigid body %d. Check the attachment and RP assignment.\n", body.id);
    return false;
  }
  return true;
}
