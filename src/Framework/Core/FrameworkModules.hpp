#ifndef FRAMEWORKMODULES_HPP
#define FRAMEWORKMODULES_HPP

#include <string>

class ModuleManager;

bool register_framework_modules(ModuleManager& modules, std::string& error);

#endif // FRAMEWORKMODULES_HPP
