add_library(postprocessor_core STATIC
  Core/application/PostprocessorCore.cpp
  Core/application/PostprocessorCore.hpp
  Core/sources/PostprocessorCoreSources.cpp
  Core/sources/SourceRuntime.hpp
  Core/sources/DataSource.hpp
  Core/sources/SourceReference.hpp
  Core/session/PostprocessorSession.hpp
  Core/representations/Representation.hpp
  Core/views/ViewDefinition.hpp
  Core/layouts/LayoutDefinition.hpp
)

target_link_libraries(postprocessor_core PUBLIC cubitsim_results)
target_compile_features(postprocessor_core PUBLIC cxx_std_17)
set_target_properties(postprocessor_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(postprocessor_module SHARED
  PostprocessorModule.cpp
  PostprocessorModule.hpp
  Interfaces/PostprocessorApplicationInterface.cpp
  Interfaces/PostprocessorApplicationInterface.hpp
  Commands/PostprocessorCommands.cpp
  Commands/PostprocessorCommands.hpp
  Commands/sources/PostOpenCommand.cpp
  Commands/sources/PostOpenCommand.hpp
  Commands/sources/PostListCommand.cpp
  Commands/sources/PostListCommand.hpp
  Commands/sources/PostSelectCommand.cpp
  Commands/sources/PostSelectCommand.hpp
  Commands/sources/PostCloseCommand.cpp
  Commands/sources/PostCloseCommand.hpp
  GUI/PostprocessorGui.cpp
  GUI/PostprocessorGui.hpp
  GUI/Workspace/PostprocessorWorkspace.cpp
  GUI/Workspace/PostprocessorWorkspace.hpp
  GUI/Viewport/PostprocessorViewport.cpp
  GUI/Viewport/PostprocessorViewport.hpp
  GUI/ResultsTree/PostprocessorResultsTree.cpp
  GUI/ResultsTree/PostprocessorResultsTree.hpp
  GUI/Properties/PostprocessorProperties.cpp
  GUI/Properties/PostprocessorProperties.hpp
  GUI/Observers/PostprocessorGuiObserver.cpp
  GUI/Observers/PostprocessorGuiObserver.hpp
)

target_link_libraries(postprocessor_module PRIVATE postprocessor_core cubiti cubit_util
  Qt6::Widgets Qt6::OpenGLWidgets VTK::GUISupportQt VTK::RenderingCore VTK::RenderingOpenGL2)
target_compile_features(postprocessor_module PUBLIC cxx_std_17)
target_include_directories(postprocessor_module PUBLIC "${CUBITSIM_SOURCE_ROOT}")
set_target_properties(postprocessor_module PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  ARCHIVE_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  BUILD_RPATH_USE_ORIGIN TRUE
)
