target_sources(postprocessor_module PRIVATE
  PostprocessorGui.cpp
  PostprocessorGui.hpp
)

add_subdirectory(Annotations)
add_subdirectory(Capture)
add_subdirectory(Explosion)
add_subdirectory(Filters)
add_subdirectory(Navigation)
add_subdirectory(Observers)
add_subdirectory(Options)
add_subdirectory(Plots)
add_subdirectory(Properties)
add_subdirectory(Resources)
add_subdirectory(ResultsTree)
add_subdirectory(Selection)
add_subdirectory(Sources)
add_subdirectory(Tables)
add_subdirectory(VectorPlot)
add_subdirectory(Viewport)
add_subdirectory(Workspace)
