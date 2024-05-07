#ifndef CALCULIXPYTHONINTERFACE_HPP
#define CALCULIXPYTHONINTERFACE_HPP

/* don't forget to wrap with 'swig -python -c++ CalculiXPythonInterface.i'  */

//class CalculiXCoreInterface;

class CalculiXPythonInterface
{

public:
  CalculiXPythonInterface();
  ~CalculiXPythonInterface();
	
  std::string to_string_scientific(double value, int precision = 6); // converts a double to string with scientific notation, with optional precision
  
  //CalculiXCoreInterface *ccx_iface;
};

#endif // CALCULIXPYTHONINTERFACE_HPP
