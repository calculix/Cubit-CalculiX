#include "CalculiXGuiResourcesInterface.hpp"
#include <string>

#include "Core/loadCalculiXCore.hpp"

QIcon* CalculiXGuiResourcesInterface::getIcon(std::string name)
{
  return ccx_core.getIcon(name);
}

QIcon CalculiXGuiResourcesInterface::getIcon2(std::string name)
{
  return ccx_core.getIcon2(name);
}
