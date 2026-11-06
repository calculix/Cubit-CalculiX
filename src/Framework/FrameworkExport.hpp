#ifndef FRAMEWORKEXPORT_HPP
#define FRAMEWORKEXPORT_HPP

#if defined(_WIN32)
  #if defined(CUBITSIM_FRAMEWORK_BUILD)
    #define CUBITSIM_FRAMEWORK_EXPORT __declspec(dllexport)
  #else
    #define CUBITSIM_FRAMEWORK_EXPORT __declspec(dllimport)
  #endif
#else
  #define CUBITSIM_FRAMEWORK_EXPORT
#endif

#endif // FRAMEWORKEXPORT_HPP
