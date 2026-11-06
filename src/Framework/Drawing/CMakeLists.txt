# Shared renderer: no dependency on workspace modules or the framework host.
add_library(cubitsim_drawing SHARED
  DrawingExport.hpp DrawingTypes.hpp ../Interfaces/DrawingInterface.hpp
  DrawingGeometry.cpp DrawingGeometry.hpp
  DrawingSymbols.cpp DrawingSymbols.hpp
  DrawingMeshGeometry.cpp DrawingMeshGeometry.hpp
  DrawingService.cpp DrawingService.hpp
  DrawingGraphics.cpp DrawingGraphics.hpp
  DrawingPicker.cpp DrawingPicker.hpp
  DrawingHitTest.cpp DrawingHitTest.hpp
  DrawingHover.cpp DrawingHover.hpp DrawingPickData.hpp
  NativeDisplayRefreshObserver.cpp NativeDisplayRefreshObserver.hpp
)
target_compile_features(cubitsim_drawing PUBLIC cxx_std_17)
target_compile_definitions(cubitsim_drawing PRIVATE CUBITSIM_DRAWING_BUILD)
target_include_directories(cubitsim_drawing PUBLIC "${CUBITSIM_SOURCE_ROOT}")
target_link_libraries(cubitsim_drawing
  # DrawingService.hpp exposes picker types backed by Showviz and VTK headers.
  PUBLIC showviz_base VTK::CommonCore VTK::CommonDataModel
  PRIVATE cubiti cubit_util cubit_gui clarogui clarofw navigation
    showviz_cubit showviz_mesh showviz_geom Qt6::Widgets
)
set_target_properties(cubitsim_drawing PROPERTIES
  WINDOWS_EXPORT_ALL_SYMBOLS OFF
  RUNTIME_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  ARCHIVE_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  BUILD_RPATH_USE_ORIGIN TRUE
)
if (UNIX)
  set_target_properties(cubitsim_drawing PROPERTIES
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../../lib"
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH_USE_LINK_PATH FALSE
  )
endif()
