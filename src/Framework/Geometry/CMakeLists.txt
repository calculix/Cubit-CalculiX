# Shared geometry tools depend on CGAL and the Cubit SDK, not workspace modules.
add_library(cubitsim_geometry SHARED
  CGALTool.cpp
  CGALTool.hpp
  CubitWrapInput.cpp
  CubitWrapInput.hpp
  GeometryExport.hpp
)

target_compile_features(cubitsim_geometry PUBLIC cxx_std_17)
target_compile_definitions(cubitsim_geometry PRIVATE CUBITSIM_GEOMETRY_BUILD)
target_include_directories(cubitsim_geometry PUBLIC "${CUBITSIM_SOURCE_ROOT}")
target_link_libraries(cubitsim_geometry PRIVATE CGAL::CGAL cubiti cubit_util)

set_target_properties(cubitsim_geometry PROPERTIES
  WINDOWS_EXPORT_ALL_SYMBOLS OFF
  RUNTIME_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  ARCHIVE_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  BUILD_RPATH_USE_ORIGIN TRUE
)

if (UNIX)
  set_target_properties(cubitsim_geometry PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../../lib"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH FALSE
  )
endif()
