# Shared storage foundation: no dependency on the host or workspace modules.
add_library(cubitsim_storage SHARED
  Hdf5FileTransaction.cpp Hdf5FileTransaction.hpp
  ReportProfileStore.cpp ReportProfileStore.hpp
  UserOptionsStore.cpp
  UserOptionsStore.hpp
  ../Utility/FrameworkPaths.cpp
  ../Utility/FrameworkPaths.hpp
  ../Interfaces/UserOption.hpp
  ../Interfaces/UserOptionsInterface.hpp
  ModelFileMetadata.cpp
  ModelFileMetadata.hpp
  HDF5Tool.cpp
  HDF5Tool.hpp
  StorageExport.hpp
)

target_compile_features(cubitsim_storage PUBLIC cxx_std_17)
target_compile_definitions(cubitsim_storage PRIVATE CUBITSIM_STORAGE_BUILD)
target_include_directories(cubitsim_storage PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CUBITSIM_SOURCE_ROOT}")
target_link_libraries(cubitsim_storage PUBLIC HDF5::HDF5 cubitsim_reporting PRIVATE cubit_util Qt6::Core)
if (CMAKE_DL_LIBS)
  target_link_libraries(cubitsim_storage PRIVATE ${CMAKE_DL_LIBS})
endif()

set_target_properties(cubitsim_storage PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  ARCHIVE_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  BUILD_RPATH_USE_ORIGIN TRUE
)

if (UNIX)
  set_target_properties(cubitsim_storage PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../../lib"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH FALSE
  )
endif()

# Preserve the existing deployment locations for the bundled HDF5 runtime.
if (WIN32)
  file(COPY
    "${CUBITSIM_SOURCE_ROOT}/Library/Win/hdf5_cpp.dll"
    "${CUBITSIM_SOURCE_ROOT}/Library/Win/hdf5.dll"
    DESTINATION "${CMAKE_BINARY_DIR}/Release/"
  )
endif()

if (UNIX)
  file(COPY
    "${CUBITSIM_SOURCE_ROOT}/Library/Linux/libhdf5.so"
    "${CUBITSIM_SOURCE_ROOT}/Library/Linux/libhdf5.so.310"
    "${CUBITSIM_SOURCE_ROOT}/Library/Linux/libhdf5.so.310.5.1"
    "${CUBITSIM_SOURCE_ROOT}/Library/Linux/libhdf5_cpp.so"
    "${CUBITSIM_SOURCE_ROOT}/Library/Linux/libhdf5_cpp.so.310"
    "${CUBITSIM_SOURCE_ROOT}/Library/Linux/libhdf5_cpp.so.310.0.6"
    DESTINATION "${CMAKE_BINARY_DIR}/"
  )
endif()
