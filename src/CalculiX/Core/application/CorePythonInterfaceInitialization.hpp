#ifndef COREPYTHONINTERFACEINITIALIZATION_HPP
#define COREPYTHONINTERFACEINITIALIZATION_HPP

#include <string>

class CubitPythonInterpreter;

class CorePythonInterfaceInitialization
{
public:
  static bool initialize(CubitPythonInterpreter* interpreter, const std::string& path);
};

#endif
