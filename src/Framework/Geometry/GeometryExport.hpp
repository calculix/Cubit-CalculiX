#ifndef CUBITSIM_GEOMETRYEXPORT_HPP
#define CUBITSIM_GEOMETRYEXPORT_HPP

#if defined(_WIN32)
  #if defined(CUBITSIM_GEOMETRY_BUILD)
    #define CUBITSIM_GEOMETRY_EXPORT __declspec(dllexport)
  #else
    #define CUBITSIM_GEOMETRY_EXPORT __declspec(dllimport)
  #endif
#else
  #define CUBITSIM_GEOMETRY_EXPORT
#endif

#endif
