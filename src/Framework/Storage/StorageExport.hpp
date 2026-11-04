#ifndef FEA_STORAGEEXPORT_HPP
#define FEA_STORAGEEXPORT_HPP

#if defined(_WIN32)
  #if defined(FEA_STORAGE_BUILD)
    #define FEA_STORAGE_EXPORT __declspec(dllexport)
  #else
    #define FEA_STORAGE_EXPORT __declspec(dllimport)
  #endif
#else
  #define FEA_STORAGE_EXPORT
#endif

#endif // FEA_STORAGEEXPORT_HPP
