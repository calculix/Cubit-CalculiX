#ifndef CUBITSIM_STORAGEEXPORT_HPP
#define CUBITSIM_STORAGEEXPORT_HPP

#if defined(_WIN32)
  #if defined(CUBITSIM_STORAGE_BUILD)
    #define CUBITSIM_STORAGE_EXPORT __declspec(dllexport)
  #else
    #define CUBITSIM_STORAGE_EXPORT __declspec(dllimport)
  #endif
#else
  #define CUBITSIM_STORAGE_EXPORT
#endif

#endif // CUBITSIM_STORAGEEXPORT_HPP
