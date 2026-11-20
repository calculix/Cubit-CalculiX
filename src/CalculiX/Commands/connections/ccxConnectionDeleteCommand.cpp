#include "ccxConnectionDeleteCommand.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxConnectionDeleteCommand::get_syntax()
{
  return {"ccx delete point mass <value:label='id'>", "ccx delete spring <value:label='id'>", "ccx delete distributing attachment <value:label='id'>", "ccx delete joint <value:label='id'>", "ccx delete rp <value:label='id'>",
    "ccx delete rigid body <value:label='id'>"};
}

std::vector<std::string> ccxConnectionDeleteCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ccxConnectionDeleteCommand::get_help()
{
  return {"Delete a reference point, rigid body, joint, distributing attachment, point mass or spring."};
}

bool ccxConnectionDeleteCommand::execute(CubitCommandData& data)
{
  CalculiXConnectionsInterface iface;
  int id = 0;
  data.get_value("id", id);
  bool removed = false;
  if (data.find_keyword("MASS"))
  {
    removed = iface.remove_point_mass(id);
  }
  else if (data.find_keyword("SPRING"))
  {
    removed = iface.remove_spring(id);
  }
  else if (data.find_keyword("DISTRIBUTING"))
  {
    removed = iface.remove_attachment(id);
  }
  else if (data.find_keyword("JOINT"))
  {
    removed = iface.remove_joint(id);
  }
  else
  {
    removed = data.find_keyword("RP") ? iface.remove_point(id) : iface.remove_body(id);
  }
  if (!removed)
  {
    PRINT_ERROR("Cannot delete connection %d. Check its ID and dependencies.\n", id);
    return false;
  }
  return true;
}
