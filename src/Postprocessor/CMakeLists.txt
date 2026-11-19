find_package(Threads REQUIRED)
add_library(postprocessor_core STATIC)

target_link_libraries(postprocessor_core PUBLIC cubitsim_results cubitsim_results_io cubitsim_media PRIVATE Qt6::Core VTK::CommonDataModel VTK::FiltersCore VTK::FiltersGeneral VTK::FiltersGeometry)
target_include_directories(postprocessor_core PRIVATE "${CUBITSIM_SOURCE_ROOT}")
target_compile_features(postprocessor_core PUBLIC cxx_std_17)
set_target_properties(postprocessor_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(postprocessor_module SHARED)

target_sources(postprocessor_module PRIVATE
  PostprocessorModule.cpp
  PostprocessorModule.hpp
)

add_subdirectory(Commands)
add_subdirectory(Core)
add_subdirectory(EventObservers)
add_subdirectory(GUI)
add_subdirectory(Icons)
add_subdirectory(Interfaces)

target_link_libraries(postprocessor_module PRIVATE Threads::Threads VTK::ChartsCore VTK::ViewsContext2D VTK::RenderingContext2D VTK::RenderingContextOpenGL2 postprocessor_core cubitsim_storage cubitsim_reporting cubiti cubit_util clarogui clarofw
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
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorAnimationLoop.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorAnimationLoop.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorAnimationRotationX.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorAnimationRotationX.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorAnimationRotationY.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorAnimationRotationY.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorAnimationRotationZ.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorAnimationRotationZ.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorIntegrationPoint.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorIntegrationPoint.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorMinMax.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorMinMax.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorAddResultLabel.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorAddResultLabel.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorShowLabels.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorShowLabels.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorPlotHistory.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorPlotHistory.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorCaptureViewport.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorCaptureViewport.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorRenderImage.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorRenderImage.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorRenderAnimation.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorRenderAnimation.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorAddToReport.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorAddToReport.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorLegend.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorLegend.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorThreshold.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorThreshold.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorVectors.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorVectors.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorExplode.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorExplode.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorExplodeMesh.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorExplodeMesh.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorClip.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorClip.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorSection.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorSection.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorScale.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorScale.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorViewFront.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorViewFront.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorViewBack.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorViewBack.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorViewTop.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorViewTop.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorViewBottom.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorViewBottom.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorViewLeft.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorViewLeft.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorViewRight.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorViewRight.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorViewIso.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorViewIso.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorViewOrtho.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorViewOrtho.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorViewPerspective.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorViewPerspective.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorTreeSource.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorTreeSource.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorTreeIntegrationPoints.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorTreeIntegrationPoints.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorTreeResults.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorTreeResults.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorTreeField.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorTreeField.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorTreeTables.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorTreeTables.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorTreeTable.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorTreeTable.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorDeform.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorDeform.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/PostprocessorUndeformed.svg"
    "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/PostprocessorUndeformed.svg"
  VERBATIM
)
add_dependencies(postprocessor_module postprocessor_icons)
