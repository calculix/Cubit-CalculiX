add_library(postprocessor_core STATIC
  Core/tables/ResultTables.cpp
  Core/tables/PostprocessorCoreTables.cpp
  Core/persistence/PostprocessorSessionCodec.cpp
  Core/application/PostprocessorCorePersistence.cpp
  Core/application/PostprocessorCore.cpp
  Core/application/PostprocessorCore.hpp
  Core/selection/EntityReference.hpp
  Core/selection/SelectionState.hpp
  Core/selection/ResultProbe.hpp
  Core/selection/ResultProbe.cpp
  Core/selection/PostprocessorCoreSelection.cpp
  Core/representations/DisplayDefaults.hpp
  Core/representations/ResultReferenceMesh.cpp
  Core/representations/ResultReferenceMesh.hpp
  Core/representations/ResultMesh.cpp
  Core/representations/ResultMesh.hpp
  Core/representations/ResultDeformation.cpp
  Core/representations/ResultDeformation.hpp
  Core/representations/PostprocessorCoreDeformation.cpp
  Core/representations/PostprocessorCoreRepresentations.cpp
  Core/representations/PostprocessorCoreDatasetVisibility.cpp
  Core/representations/PostprocessorCoreCategories.cpp
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
  Core/representations/AnimationDefinition.hpp
  Core/representations/PostprocessorCoreAnimation.cpp
  Core/representations/PostprocessorCoreColorRange.cpp
  Core/representations/Representation.hpp
  Core/representations/ResultColoring.hpp
  Core/representations/ResultColoring.cpp
  Core/views/ViewDefinition.hpp
  Core/layouts/LayoutDefinition.hpp
)

target_link_libraries(postprocessor_core PUBLIC cubitsim_results PRIVATE Qt6::Core VTK::CommonDataModel VTK::FiltersCore)
target_include_directories(postprocessor_core PRIVATE "${CUBITSIM_SOURCE_ROOT}")
target_compile_features(postprocessor_core PUBLIC cxx_std_17)
set_target_properties(postprocessor_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(postprocessor_module SHARED
  Icons/Postprocessor.svg
  Icons/PostprocessorIntegrationPoint.svg
  Core/sources/VtkFileSourceProvider.cpp
  Commands/tables/PostTableCommand.cpp
  Commands/tables/PostViewTypeCommand.cpp
  GUI/Workspace/PostprocessorToolbar.cpp
  GUI/Sources/PostprocessorSourceControls.cpp
  GUI/Sources/PostprocessorImportDialog.cpp
  GUI/Tables/PostprocessorTableModel.cpp
  GUI/Tables/PostprocessorTableView.cpp
  Core/persistence/PostprocessorProjectPersistence.cpp
  Interfaces/PostprocessorProjectInterface.cpp
  PostprocessorModule.cpp
  PostprocessorModule.hpp
  EventObservers/PostprocessorModelObserver.cpp
  EventObservers/PostprocessorModelObserver.hpp
  Interfaces/PostprocessorApplicationInterface.cpp
  Interfaces/PostprocessorApplicationInterface.hpp
  Commands/PostprocessorCommands.cpp
  Commands/PostprocessorCommands.hpp
  Commands/selection/PostPickCommand.hpp
  Commands/selection/PostPickCommand.cpp
  Commands/selection/PostProbeCommand.hpp
  Commands/selection/PostProbeCommand.cpp
  Commands/selection/PickCommandText.hpp
  Commands/selection/PickCommandText.cpp
  Commands/results/PostAnimationCommand.cpp
  Commands/results/PostAnimationCommand.hpp
  Commands/results/PostFrameCommand.cpp
  Commands/results/PostFrameCommand.hpp
  Commands/display/PostColorCommand.cpp
  Commands/display/PostColorCommand.hpp
  Commands/display/PostDeformCommand.cpp
  Commands/display/PostDeformCommand.hpp
  Commands/display/PostLegendCommand.cpp
  Commands/display/PostLegendCommand.hpp
  Commands/display/PostShowCommand.cpp
  Commands/display/PostShowCommand.hpp
  Commands/display/DatasetCommandSupport.hpp
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
  Commands/sources/PostRefreshCommand.cpp
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
  GUI/Selection/PostprocessorPicking.hpp
  GUI/Selection/PostprocessorPicking.cpp
  GUI/Selection/PostprocessorHighlight.cpp
  GUI/Selection/PostprocessorPickControls.hpp
  GUI/Selection/PostprocessorPickControls.cpp
  GUI/Selection/PostprocessorProbeProperties.hpp
  GUI/Selection/PostprocessorProbeProperties.cpp
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
  GUI/Viewport/PostprocessorColorMap.cpp
  GUI/Viewport/PostprocessorColorMap.hpp
  GUI/Viewport/PostprocessorScene.cpp
  GUI/Viewport/PostprocessorScene.hpp
  GUI/Viewport/PostprocessorTriad.cpp
  GUI/Viewport/PostprocessorTriad.hpp
  GUI/Navigation/PostprocessorFrameNavigation.cpp
  GUI/Navigation/PostprocessorFrameNavigation.hpp
  GUI/Navigation/PostprocessorDeformationControls.cpp
  GUI/Navigation/PostprocessorDeformationControls.hpp
  GUI/ResultsTree/PostprocessorResultsTree.cpp
  GUI/ResultsTree/PostprocessorResultsTree.hpp
  GUI/Properties/PostprocessorColorControls.cpp
  GUI/Properties/PostprocessorColorControls.hpp
  GUI/Properties/PostprocessorDisplayProperties.cpp
  GUI/Properties/PostprocessorDisplayProperties.hpp
  GUI/Properties/PostprocessorProperties.cpp
  GUI/Properties/PostprocessorProperties.hpp
  GUI/Observers/PostprocessorGuiObserver.cpp
  GUI/Observers/PostprocessorGuiObserver.hpp
)

target_link_libraries(postprocessor_module PRIVATE postprocessor_core cubitsim_storage cubiti cubit_util clarogui clarofw
  Qt6::Widgets Qt6::OpenGLWidgets VTK::IOXML VTK::GUISupportQt VTK::RenderingCore VTK::RenderingAnnotation VTK::RenderingOpenGL2 VTK::FiltersCore VTK::FiltersGeometry)
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
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorIntegrationPoint.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorIntegrationPoint.svg"
  VERBATIM
)
add_dependencies(postprocessor_module postprocessor_icons)
