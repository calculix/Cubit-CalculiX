target_sources(postprocessor_module PRIVATE
  PostprocessorPlaneOverlay.cpp
)

add_subdirectory(Clip)
add_subdirectory(Section)
add_subdirectory(Threshold)
