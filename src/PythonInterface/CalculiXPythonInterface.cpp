#include "CalculiXPythonInterface.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"

CalculiXPythonInterface::CalculiXPythonInterface()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  std::string log = "CalculiX Python Interface loaded!\n";
  PRINT_INFO("%s", log.c_str());
}

CalculiXPythonInterface::~CalculiXPythonInterface()
{
  delete ccx_iface;
}

void CalculiXPythonInterface::help()
{
  std::string log = "CalculiX Python Interface HELP:\n";
  log.append("write some help here....\n");
  log.append("write some help here....\n");
  log.append("write some help here....\n");
  log.append("write some help here....\n");
  PRINT_INFO("%s", log.c_str());
}

int CalculiXPythonInterface::test_int()
{
  return 999;
}

double CalculiXPythonInterface::test_double()
{
  return 1.125359;
}

std::string CalculiXPythonInterface::test_string()
{
 return "test string";
}
  

std::string CalculiXPythonInterface::to_string_scientific(double value, int precision)
{
  return ccx_iface->to_string_scientific(value, precision);
}