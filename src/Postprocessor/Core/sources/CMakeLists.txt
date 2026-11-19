target_sources(postprocessor_core PRIVATE
  PostprocessorCoreSources.cpp
  SourceRuntime.hpp
  DataSource.hpp
  SourceReference.hpp
  SourceProjectRuntime.hpp
  SourceProjectRuntime.cpp
)

target_sources(postprocessor_module PRIVATE
  VtkFileSourceProvider.cpp
)
