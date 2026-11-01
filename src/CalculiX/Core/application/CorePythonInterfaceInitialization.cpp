#include "Core/application/CorePythonInterfaceInitialization.hpp"
#include "CubitPythonInterpreter.hpp"
#include "CubitMessage.hpp"

bool CorePythonInterfaceInitialization::initialize(CubitPythonInterpreter* interpreter, const std::string& path)
{
  if (!interpreter || !interpreter->IsInitialized())
  {
    PRINT_ERROR("Cubit Python interpreter is not ready.\n");
    return false;
  }

  if (path.empty())
  {
    PRINT_ERROR("Path to Python Interface is empty.\n");
    return false;
  }

  interpreter->add_sys_path(path, true);

  if (interpreter->Run_SimpleString(
        "from calculix_pythoninterface import CalculiXPythonInterface\n"
        "ccx = CalculiXPythonInterface()") != 0)
  {
    PRINT_ERROR("Init CalculiX Python Interface Failed!\n");
    return false;
  }

  PRINT_INFO("%s", "CalculiX Python Interface is ready!\n");
  return true;
}
