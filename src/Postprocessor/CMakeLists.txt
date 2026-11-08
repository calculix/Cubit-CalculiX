add_library(postprocessor_core STATIC
  Core/application/PostprocessorCore.cpp
  Core/application/PostprocessorCore.hpp
  Core/representations/DisplayDefaults.hpp
  Core/representations/ResultMesh.cpp
  Core/representations/ResultMesh.hpp
  Core/representations/PostprocessorCoreRepresentations.cpp
  Core/views/PostprocessorCoreViews.cpp
  Core/views/PostprocessorCoreCamera.cpp
  Core/views/CameraOperations.cpp
  Core/views/CameraOperations.hpp
  Core/options/PostprocessorUserOptions.cpp
  Core/options/PostprocessorUserOptions.hpp
  Core/sources/PostprocessorCoreSources.cpp
  Core/sources/SourceRuntime.hpp
  Core/sources/DataSource.hpp
  Core/sources/SourceReference.hpp
  Core/session/PostprocessorSession.hpp
  Core/representations/Representation.hpp
  Core/views/ViewDefinition.hpp
  Core/layouts/LayoutDefinition.hpp
)

target_link_libraries(postprocessor_core PUBLIC cubitsim_results PRIVATE Qt6::Core VTK::CommonDataModel)
target_include_directories(postprocessor_core PRIVATE "${CUBITSIM_SOURCE_ROOT}")
target_compile_features(postprocessor_core PUBLIC cxx_std_17)
set_target_properties(postprocessor_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(postprocessor_module SHARED
  Icons/Postprocessor.svg
  PostprocessorModule.cpp
  PostprocessorModule.hpp
  EventObservers/PostprocessorModelObserver.cpp
  EventObservers/PostprocessorModelObserver.hpp
  Interfaces/PostprocessorApplicationInterface.cpp
  Interfaces/PostprocessorApplicationInterface.hpp
  Commands/PostprocessorCommands.cpp
  Commands/PostprocessorCommands.hpp
  Commands/display/PostShowCommand.cpp
  Commands/display/PostShowCommand.hpp
  Commands/display/PostHideCommand.cpp
  Commands/display/PostHideCommand.hpp
  Commands/views/PostViewFitCommand.cpp
  Commands/views/PostViewFitCommand.hpp
  Commands/views/CameraCommandText.cpp
  Commands/views/CameraCommandText.hpp
  Commands/views/CameraCommandTiming.cpp
  Commands/views/CameraCommandTiming.hpp
  Commands/views/PostViewCameraCommand.cpp
  Commands/views/PostViewCameraCommand.hpp
  Commands/views/PostViewRotateCommand.cpp
  Commands/views/PostViewRotateCommand.hpp
  Commands/views/PostViewPanCommand.cpp
  Commands/views/PostViewPanCommand.hpp
  Commands/views/PostViewZoomCommand.cpp
  Commands/views/PostViewZoomCommand.hpp
  Commands/options/PostUserOptionsCommand.cpp
  Commands/options/PostUserOptionsCommand.hpp
  Commands/options/PostprocessorOptionCommands.hpp
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
  GUI/Options/PostprocessorOptionsPanel.cpp
  GUI/Options/PostprocessorOptionsPanel.hpp
  GUI/Options/PostprocessorMouseOptionsPanel.cpp
  GUI/Options/PostprocessorMouseOptionsPanel.hpp
  GUI/Resources/PostprocessorIconLoader.cpp
  GUI/Resources/PostprocessorIconLoader.hpp
  GUI/Workspace/PostprocessorWorkspace.cpp
  GUI/Workspace/PostprocessorWorkspace.hpp
  GUI/Viewport/PostprocessorViewport.cpp
  GUI/Viewport/PostprocessorViewport.hpp
  GUI/Viewport/CameraInteraction.cpp
  GUI/Viewport/CameraInteraction.hpp
  GUI/Viewport/CameraTimings.cpp
  GUI/Viewport/CameraTimings.hpp
  GUI/Viewport/PostprocessorScene.cpp
  GUI/Viewport/PostprocessorScene.hpp
  GUI/Viewport/PostprocessorTriad.cpp
  GUI/Viewport/PostprocessorTriad.hpp
  GUI/ResultsTree/PostprocessorResultsTree.cpp
  GUI/ResultsTree/PostprocessorResultsTree.hpp
  GUI/Properties/PostprocessorProperties.cpp
  GUI/Properties/PostprocessorProperties.hpp
  GUI/Observers/PostprocessorGuiObserver.cpp
  GUI/Observers/PostprocessorGuiObserver.hpp
)

target_link_libraries(postprocessor_module PRIVATE postprocessor_core cubiti cubit_util
  Qt6::Widgets Qt6::OpenGLWidgets VTK::GUISupportQt VTK::RenderingCore VTK::RenderingOpenGL2 VTK::FiltersCore VTK::FiltersGeometry)
target_compile_features(postprocessor_module PUBLIC cxx_std_17)
target_include_directories(postprocessor_module PUBLIC "${CUBITSIM_SOURCE_ROOT}")
set_target_properties(postprocessor_module PROPERTIES
  AUTORCC OFF
  RUNTIME_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  ARCHIVE_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  BUILD_RPATH_USE_ORIGIN TRUE
)

# Deploy standalone SVGs beside the module, as for the CalculiX application.
add_custom_target(postprocessor_icons ALL
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${CUBITSIM_DEPLOY_DIRECTORY}/Icons"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/Postprocessor.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/Postprocessor.svg"
  VERBATIM
)
add_dependencies(postprocessor_module postprocessor_icons)
