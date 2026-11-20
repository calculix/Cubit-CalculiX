#include "ccxJointCreateCommand.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxJointCreateCommand::get_syntax()
{
  const std::string prefix = "ccx create joint <value:label='id'> name <string:type='unquoted',number='1',label='name'> type <string:type='unquoted',number='1',label='type'> ";
  const std::string axis = " [axis <value:label='x'> <value:label='y'> <value:label='z'>]";
  const std::string history = " [history {all | off | step <value:label='history_step'>}]";
  return {prefix + "rp <value:label='a'> to rp <value:label='b'>" + axis + history,
    prefix + "rp <value:label='a'> to ground" + axis + history,
    prefix + "surface <value:label='a'> to surface <value:label='b'> coaxial" + history};
}

std::vector<std::string> ccxJointCreateCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ccxJointCreateCommand::get_help()
{
  return {"Create a fixed, spherical, revolute, prismatic or cylindrical joint. Axis defaults to global Z."};
}

bool ccxJointCreateCommand::execute(CubitCommandData& data)
{
  ConnectionJoint joint;
  int a = 0;
  int b = 0;
  data.get_value("id", joint.id);
  data.get_value("a", a);
  data.get_value("b", b);
  data.get_string("name", joint.name);
  data.get_string("type", joint.type);
  joint.coaxial = data.find_keyword("COAXIAL");
  if (!joint.coaxial)
  {
    joint.point_a = a;
    joint.point_b = b;
    if (data.find_keyword("AXIS"))
    {
      data.get_value("x", joint.axis[0]);
      data.get_value("y", joint.axis[1]);
      data.get_value("z", joint.axis[2]);
    }
  }
  if (data.find_keyword("OFF"))
  {
    joint.history_step = 0;
  }
  if (data.find_keyword("STEP"))
  {
    data.get_value("history_step", joint.history_step);
    if (joint.history_step <= 0)
    {
      PRINT_ERROR("Expected a positive history step ID.\n");
      return false;
    }
  }
  std::string error;
  if (!CalculiXConnectionsInterface().create_joint(joint, a, b, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return true;
}
