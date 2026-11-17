# Reusable media encoding/output; no workspace, Cubit or VTK dependencies.
add_library(cubitsim_media SHARED
  MediaExport.hpp
  AnimationExportDefinition.hpp AnimationExportDefinition.cpp
  AnimationOutput.hpp AnimationOutput.cpp
  PngSequenceWriter.hpp PngSequenceWriter.cpp
  GifAnimation.hpp GifAnimation.cpp
  Mp4Animation.hpp Mp4Animation.cpp
  AnimationExportJob.hpp AnimationExportJob.cpp
)
target_compile_features(cubitsim_media PUBLIC cxx_std_17)
target_compile_definitions(cubitsim_media PRIVATE CUBITSIM_MEDIA_BUILD)
target_include_directories(cubitsim_media PUBLIC "${CUBITSIM_SOURCE_ROOT}")
target_link_libraries(cubitsim_media PUBLIC Qt6::Core Qt6::Gui PRIVATE Threads::Threads)
set_target_properties(cubitsim_media PROPERTIES
  WINDOWS_EXPORT_ALL_SYMBOLS OFF
  RUNTIME_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  ARCHIVE_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  BUILD_RPATH_USE_ORIGIN TRUE
)
if (UNIX)
  set_target_properties(cubitsim_media PROPERTIES BUILD_RPATH "$ORIGIN" INSTALL_RPATH "$ORIGIN")
endif()
