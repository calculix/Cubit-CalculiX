#include "ccxDistributingAttachmentCommand.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxDistributingAttachmentCommand::get_syntax()
{
  return {"ccx create distributing attachment <value:label='id'> name <string:type='unquoted',number='1',label='name'> "
    "{surface <value:label='entity'> | sideset <value:label='entity'>} {rp <value:label='rp'> | center} "
    "[moments <value:label='mask'>] [no_resultant] [mean_displacement]"};
}

std::vector<std::string> ccxDistributingAttachmentCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ccxDistributingAttachmentCommand::get_help()
{
  return {"Distribute RP loads over coplanar solid faces. Moment mask: X=1, Y=2, Z=4."};
}

bool ccxDistributingAttachmentCommand::execute(CubitCommandData& data)
{
  DistributingAttachment a;
  data.get_value("id",a.id);
  data.get_string("name",a.name);
  data.get_value("entity",a.entity);
  data.get_value("rp",a.point);
  if (data.find_keyword("MOMENTS"))
  {
    data.get_value("mask",a.moments);
  }
  a.geometry = data.find_keyword("SIDESET") ? "sideset" : "surface";
  a.resultant = !data.find_keyword("NO_RESULTANT");
  a.mean = data.find_keyword("MEAN_DISPLACEMENT");
  std::string error;
  if (!CalculiXConnectionsInterface().create_attachment(a,data.find_keyword("CENTER"),error))
  {
    PRINT_ERROR("%s\n",error.c_str());
    return false;
  }
  return true;
}
