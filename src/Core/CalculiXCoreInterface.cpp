#include "CalculiXCoreInterface.hpp"
#include "loadCalculiXCore.hpp"

CalculiXCoreInterface::CalculiXCoreInterface()
{}

CalculiXCoreInterface::~CalculiXCoreInterface()
{}

std::string CalculiXCoreInterface::test()
{
  return ccx_core.test();
}

