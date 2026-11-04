# Shared storage foundation: no dependency on the host or workspace modules.
add_library(fea_storage SHARED
  UserOptionsStore.cpp
  UserOptionsStore.hpp
  ../Utility/FrameworkPaths.cpp
  ../Utility/FrameworkPaths.hpp
  ../Interfaces/UserOption.hpp
  ../Interfaces/UserOptionsInterface.hpp
  HDF5Tool.cpp
  HDF5Tool.hpp
  StorageExport.hpp
)

target_compile_features(fea_storage PUBLIC cxx_std_17)
target_compile_definitions(fea_storage PRIVATE FEA_STORAGE_BUILD)
target_include_directories(fea_storage PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CCX_SOURCE_ROOT}")
target_link_libraries(fea_storage PUBLIC HDF5::HDF5 PRIVATE cubit_util Qt6::Core)
if (CMAKE_DL_LIBS)
  target_link_libraries(fea_storage PRIVATE ${CMAKE_DL_LIBS})
endif()

set_target_properties(fea_storage PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CCX_DEPLOY_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${CCX_DEPLOY_DIRECTORY}"
  ARCHIVE_OUTPUT_DIRECTORY "${CCX_DEPLOY_DIRECTORY}"
  BUILD_RPATH_USE_ORIGIN TRUE
)

if (UNIX)
  set_target_properties(fea_storage PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../../lib"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH FALSE
  )
endif()

# Preserve the existing deployment locations for the bundled HDF5 runtime.
if (WIN32)
  file(COPY
    "${CCX_SOURCE_ROOT}/Library/Win/hdf5_cpp.dll"
    "${CCX_SOURCE_ROOT}/Library/Win/hdf5.dll"
    DESTINATION "${CMAKE_BINARY_DIR}/Release/"
  )
endif()

if (UNIX)
  file(COPY
    "${CCX_SOURCE_ROOT}/Library/Linux/libhdf5.so"
    "${CCX_SOURCE_ROOT}/Library/Linux/libhdf5.so.310"
    "${CCX_SOURCE_ROOT}/Library/Linux/libhdf5.so.310.5.1"
    "${CCX_SOURCE_ROOT}/Library/Linux/libhdf5_cpp.so"
    "${CCX_SOURCE_ROOT}/Library/Linux/libhdf5_cpp.so.310"
    "${CCX_SOURCE_ROOT}/Library/Linux/libhdf5_cpp.so.310.0.6"
    DESTINATION "${CMAKE_BINARY_DIR}/"
  )
endif()
