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
   %template(vector2i) vector<vector<int>>;
   %template(vector3i) vector<vector<vector<int>>>;
   %template(vectorf) vector<float>;
   %template(vector2f) vector<vector<float>>;
   %template(vector3f) vector<vector<vector<float>>>;
   %template(vectord) vector<double>;
   %template(vector2d) vector<vector<double>>;
   %template(vector3d) vector<vector<vector<double>>>;
   %template(vectors) vector<string>;
   %template(vector2s) vector<vector<string>>;
   %template(vector3s) vector<vector<vector<string>>>;
   %template(pairi_i) std::pair<int, int>;
   %template(VectorPairi) std::vector<std::pair<int, int> >;
};

%include "CalculiXPythonInterface.hpp"