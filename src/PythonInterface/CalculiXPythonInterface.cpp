#include "CalculiXPythonInterface.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"

#include <iostream>

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
  PRINT_INFO("%s", log.c_str());

  std::cout << log;
}

std::string CalculiXPythonInterface::to_string_scientific(double value, int precision)
{
  return ccx_iface->to_string_scientific(value, precision);
}