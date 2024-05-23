%module calculix_pythoninterface
%{
    #include "CalculiXPythonInterface.hpp"
%}

%include "cpointer.i"
%include "std_string.i"
%include "std_pair.i"
%include "std_vector.i"

// instantiate templates
namespace std {
   %template(vectori) vector<int>;
   %template(vectorf) vector<float>;
   %template(vectord) vector<double>;
   %template(vectors) vector<string>;
   %template(pairi_i) std::pair<int, int>;
   %template(VectorPairi) std::vector<std::pair<int, int> >;
};

%include "CalculiXPythonInterface.hpp"