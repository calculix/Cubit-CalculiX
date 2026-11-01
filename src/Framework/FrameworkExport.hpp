#ifndef FRAMEWORKEXPORT_HPP
#define FRAMEWORKEXPORT_HPP

#if defined(_WIN32)
  #if defined(FEA_FRAMEWORK_BUILD)
    #define FEA_FRAMEWORK_EXPORT __declspec(dllexport)
  #else
    #define FEA_FRAMEWORK_EXPORT __declspec(dllimport)
  #endif
#else
  #define FEA_FRAMEWORK_EXPORT
#endif

#endif // FRAMEWORKEXPORT_HPP
