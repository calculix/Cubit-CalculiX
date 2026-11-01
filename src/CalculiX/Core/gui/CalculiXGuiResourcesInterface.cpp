#include "CalculiXGuiResourcesInterface.hpp"
#include <string>

#include "Core/gui/CoreIconLoader.hpp"
#include "loadUserOptions.hpp"

QIcon* CalculiXGuiResourcesInterface::getIcon(std::string name)
{
  return new QIcon(CoreIconLoader::load(ccx_uo.mPathIcons, name));
}

QIcon CalculiXGuiResourcesInterface::getIcon2(std::string name)
{
  return CoreIconLoader::load(ccx_uo.mPathIcons, name);
}
