#ifndef LOADCALCULIXCORE_HPP
#define LOADCALCULIXCORE_HPP

#include "CalculiXCore.hpp"

#ifdef WIN32
    #if COMPILING_THE_DLL
        #define DLLEXTERN __declspec(dllexport)
    #else
        #define DLLEXTERN __declspec(dllimport)
    #endif
    DLLEXTERN CalculiXCore ccx_core;
#else
    extern CalculiXCore ccx_core;
#endif

#endif // LOADCALCULIXCORE_HPP
