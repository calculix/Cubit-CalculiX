#ifndef LOADEDLIBRARYDIRECTORY_HPP
#define LOADEDLIBRARYDIRECTORY_HPP

#include <filesystem>
#include <stdexcept>
#include <string>

#ifdef _WIN32
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif
  #include <windows.h>
#else
  #include <dlfcn.h>
#endif

namespace LoadedLibraryDirectory
{
inline std::string from_address(const void* address)
{
#ifdef _WIN32
  HMODULE module = nullptr;

  if (!GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
          | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(address), &module))
  {
    throw std::runtime_error("Cannot locate the loaded module library.");
  }

  wchar_t filename[32768] = {};
  const DWORD length = GetModuleFileNameW(module, filename, 32768);

  if (length == 0 || length >= 32768)
  {
    throw std::runtime_error("Cannot determine the loaded module directory.");
  }

  return std::filesystem::path(filename).parent_path().u8string();
#else
  Dl_info information = {};

  if (dladdr(address, &information) == 0 || information.dli_fname == nullptr)
  {
    throw std::runtime_error("Cannot determine the loaded module directory.");
  }

  return std::filesystem::absolute(information.dli_fname).parent_path().string();
#endif
}
}

#endif // LOADEDLIBRARYDIRECTORY_HPP
