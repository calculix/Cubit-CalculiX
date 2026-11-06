#ifndef CUBITSIM_DRAWINGEXPORT_HPP
#define CUBITSIM_DRAWINGEXPORT_HPP

#if defined(_WIN32)
  #if defined(CUBITSIM_DRAWING_BUILD)
    #define CUBITSIM_DRAWING_EXPORT __declspec(dllexport)
  #else
    #define CUBITSIM_DRAWING_EXPORT __declspec(dllimport)
  #endif
#else
  #define CUBITSIM_DRAWING_EXPORT
#endif

#endif
