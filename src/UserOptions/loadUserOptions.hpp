#ifndef LOADUSEROPTIONS_HPP
#define LOADUSEROPTIONS_HPP

#include "UserOptions.hpp"

#ifdef WIN32
    #if COMPILING_THE_DLL
        #define DLLEXTERN __declspec(dllexport)
    #else
        #define DLLEXTERN __declspec(dllimport)
    #endif
    DLLEXTERN UserOptions ccx_uo;
#else
    extern UserOptions ccx_uo;
#endif

#endif // LOADUSEROPTIONS_HPP
