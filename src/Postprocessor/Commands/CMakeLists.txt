target_sources(postprocessor_module PRIVATE
  FieldCommandText.cpp
  PostprocessorCommands.cpp
  PostprocessorCommands.hpp
)

add_subdirectory(annotations)
add_subdirectory(capture)
add_subdirectory(display)
add_subdirectory(explosion)
add_subdirectory(filters)
add_subdirectory(options)
add_subdirectory(plots)
add_subdirectory(results)
add_subdirectory(selection)
add_subdirectory(sources)
add_subdirectory(tables)
add_subdirectory(units)
add_subdirectory(views)
