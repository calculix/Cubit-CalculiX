#include "FrameworkModules.hpp"
#include "ModuleManager.hpp"
#include "CalculiX/CalculiXModule.hpp"

#include <memory>

bool register_framework_modules(ModuleManager& modules, std::string& error)
{
  return modules.add_module(std::make_unique<CalculiXModule>(), error);
}
