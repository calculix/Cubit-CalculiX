#ifndef CALCULIXPYTHONINTERFACE_HPP
#define CALCULIXPYTHONINTERFACE_HPP

#include <vector>
#include <string>

/* don't forget to wrap with 'swig -python -py3 -c++ CalculiXPythonInterface.i'  

and replace Python.h with, if compiling fails because of qt

#pragma push_macro("slots")
#undef slots
#include "Python.h"
#pragma pop_macro("slots")

cubit:
#!python
import sys
sys.path.append('/home/user/git/Cubit-CalculiX/build')
import calculix_pythoninterface as ccx
print(ccx.CalculiXPythonInterface.to_string_scientific(1.5,3))
print(ccx.CalculiXPythonInterface.test_int())
print(ccx.CalculiXPythonInterface.test_double())
print(ccx.CalculiXPythonInterface.test_string())
*/

class CalculiXCoreInterface;

class CalculiXPythonInterface
{

public:
  CalculiXPythonInterface();
  ~CalculiXPythonInterface();
	
  void help(); // prints out python interface help
  int test_int(); // test int datatype
  double test_double(); // test double datatype
  std::string test_string(); // test string datatype
  std::string to_string_scientific(double value, int precision = 6); // converts a double to string with scientific notation, with optional precision
  
  CalculiXCoreInterface *ccx_iface;
};

#endif // CALCULIXPYTHONINTERFACE_HPP
