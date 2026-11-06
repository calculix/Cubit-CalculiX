#include "FrameworkModules.hpp"
#include "ModuleManager.hpp"
#include "CalculiX/CalculiXModule.hpp"
#include "Postprocessor/PostprocessorModule.hpp"
#include "Framework/GUI/CommandDispatcher.hpp"
#include "Claro.hpp"
#include <QString>

#include <memory>

bool register_framework_modules(ModuleManager& modules, std::string& error)
{
  if (!modules.add_module(std::make_unique<CalculiXModule>(), error))
  {
    return false;
  }

  auto submit = [](const std::string& command, std::string& error)
  {
    return CommandDispatcher::submit(Claro::instance(), QString::fromUtf8(command.c_str()), error);
  };
  return modules.add_module(std::make_unique<PostprocessorModule>(modules, submit), error);
}
