#include "Core/CalculiXCore.hpp"
#include "CoreIconLoader.hpp"
#include "loadUserOptions.hpp"

QIcon* CalculiXCore::getIcon(std::string name)
{
  return new QIcon(CoreIconLoader::load(ccx_uo.mPathIcons, name));
}

QIcon CalculiXCore::getIcon2(std::string name)
{
  return CoreIconLoader::load(ccx_uo.mPathIcons, name);
}
