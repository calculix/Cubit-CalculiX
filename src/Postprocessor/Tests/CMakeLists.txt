cmake_minimum_required(VERSION 3.20)
project(PostprocessorDisplayTests LANGUAGES C CXX)
find_package(Threads REQUIRED)

get_filename_component(CUBITSIM_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)
if (WIN32)
  set(CUBIT_ROOT "C:/Program Files/Coreform Cubit 2026.11" CACHE PATH "Cubit installation")
  set(HDF5_ROOT "${CUBITSIM_SOURCE_ROOT}/Utility/HDF5/Win/1.14.6" CACHE PATH "HDF5 development files")
else()
  set(CUBIT_ROOT "$ENV{HOME}/Downloads/Coreform-Cubit-2026.11" CACHE PATH "Cubit installation")
  set(HDF5_ROOT "${CUBITSIM_SOURCE_ROOT}/Utility/HDF5/Linux/1.14.6" CACHE PATH "HDF5 development files")
endif ()
include("${CUBITSIM_SOURCE_ROOT}/cmake/CubitVtkSDK.cmake")
find_package(Cubit REQUIRED CONFIG PATHS "${CUBIT_ROOT}/cmake" NO_DEFAULT_PATH)
find_package(HDF5 REQUIRED COMPONENTS C CXX)
add_compile_definitions(H5_BUILT_AS_DYNAMIC_LIB)
include_directories("${CUBIT_ROOT}/include")
set(CUBITSIM_DEPLOY_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
add_subdirectory("${CUBITSIM_SOURCE_ROOT}/Framework/Reporting" reporting)
add_subdirectory("${CUBITSIM_SOURCE_ROOT}/Framework/Storage" storage)
add_subdirectory("${CUBITSIM_SOURCE_ROOT}/Framework/Core" model)
add_subdirectory("${CUBITSIM_SOURCE_ROOT}/Framework/Results" results)
add_subdirectory("${CUBITSIM_SOURCE_ROOT}/Framework/Media" media)

add_library(postprocessor_display_code STATIC
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/explosion/ExplosionLayout.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/explosion/ExplosionPulse.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/explosion/ExplosionRuntime.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/explosion/ExplosionCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/explosion/PostprocessorCoreExplosion.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorExplosionInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Explosion/PostprocessorExplosionControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/VectorPlotDefinition.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/ResultVectors.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/VectorPlotRuntime.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/VectorPlotCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/PostprocessorCoreVectors.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorVectorPlotInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/results/VectorCommandText.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/VectorPlot/PostprocessorVectorControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/VectorPlot/PostprocessorVectorActors.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/VectorPlot/PostprocessorVectorLegendProperties.cpp"

  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/capture/AnimationCaptureDefinition.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/capture/AnimationCapturePlan.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorAnimationCaptureInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/DatasetAppearance.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/PostprocessorCoreAppearance.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/persistence/DatasetAppearanceCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorAppearanceInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Properties/PostprocessorAppearanceControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/ResultsTree/PostprocessorStyleDelegate.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorDatasetAppearance.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/display/AppearanceCommandText.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorReportFrameSelection.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorReportExtrema.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorReportBlockSummary.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorReportResultImages.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorReportCapturePlan.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/application/PostprocessorCoreUnits.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/ResultColoringUnits.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/AnimationRotationCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/AnimationRotation.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/AnimationRotationRuntime.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/FilterDefinition.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/ThresholdDefinition.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/ThresholdFilter.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/ThresholdCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/filters/ThresholdCommandText.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Filters/Threshold/PostprocessorThresholdControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/PlaneOperations.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/ClipFilter.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/SectionFilter.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/FilterRuntime.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/PostprocessorCoreFilters.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/filters/FilterCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorFiltersInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Filters/PostprocessorPlaneOverlay.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Filters/Clip/PostprocessorClipControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Filters/Section/PostprocessorSectionControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/filters/FilterCommandText.cpp"

  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/tables/ResultTableSelector.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/tables/TableCommandText.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/ResultFieldSelector.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/FieldCommandText.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/capture/CaptureSnapshot.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/capture/CaptureFileWriter.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorCaptureInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Annotations/PostprocessorAnnotationPainter.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/plots/PlotData.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/plots/PlotAxisUnits.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/plots/PlotRangeUnits.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/plots/PlotHistory.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/plots/PlotTableData.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/plots/PostprocessorCorePlots.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/plots/PlotCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/plots/PostprocessorCorePlotLabels.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/plots/PlotCsvExport.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorPlotsInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/plots/PlotCommandText.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Plots/PostprocessorPlotControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Plots/PostprocessorPlotLabels.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Plots/PostprocessorPlotInteraction.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/annotations/PostprocessorCoreAnnotations.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/annotations/AnnotationCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorAnnotationsInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Annotations/PostprocessorAnnotationDisplay.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Annotations/PostprocessorAnnotationInteraction.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Annotations/PostprocessorAnnotationMenu.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/persistence/PostprocessorSessionCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/application/PostprocessorCorePersistence.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/tables/ResultTables.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/tables/TableFilters.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/tables/PostprocessorCoreTables.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/sources/VtkFileSourceProvider.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Tables/PostprocessorTableModel.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Tables/PostprocessorTableView.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Tables/PostprocessorTableFilters.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/application/PostprocessorCore.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/selection/ResultProbe.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/selection/PostprocessorCoreSelection.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/selection/PickCommandText.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Selection/PostprocessorPicking.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Selection/PostprocessorHighlight.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Selection/PostprocessorPickControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Resources/PostprocessorIconLoader.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Selection/PostprocessorProbeProperties.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/sources/PostprocessorCoreSources.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/ResultReferenceMesh.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/ResultMesh.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/ResultDeformation.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/PostprocessorCoreDeformation.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/ResultColoring.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/PostprocessorCoreRepresentations.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/PostprocessorCoreDatasetVisibility.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/PostprocessorCoreCategories.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/PostprocessorCoreAnimation.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/PostprocessorCoreColorRange.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/views/PostprocessorCoreViews.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/views/PostprocessorCoreSceneUnits.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/views/PostprocessorCoreCamera.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/views/CameraOperations.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/views/CameraCommandText.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/views/CameraCommandTiming.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorLegendInteraction.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/CameraInteraction.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/CameraTimings.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Options/PostprocessorMouseOptionsPanel.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Properties/PostprocessorDisplayProperties.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/options/PostprocessorUserOptions.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorApplicationInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorViewToolsInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorScene.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorScaleOverlay.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorColorMap.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorTriad.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Navigation/PostprocessorFrameNavigation.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Navigation/PostprocessorDeformationControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.cpp")
target_link_libraries(postprocessor_display_code PUBLIC cubitsim_media Threads::Threads cubitsim_reporting cubitsim_results cubitsim_results_io VTK::IOXML VTK::CommonDataModel
  VTK::FiltersGeneral VTK::FiltersCore VTK::FiltersGeometry VTK::RenderingCore VTK::RenderingAnnotation VTK::RenderingOpenGL2 Qt6::Core Qt6::Widgets)
target_compile_features(postprocessor_display_code PUBLIC cxx_std_17)
add_executable(postprocessor_display_tests DisplayTests.cpp)
target_link_libraries(postprocessor_display_tests PRIVATE postprocessor_display_code)
add_executable(postprocessor_camera_timings_tests CameraTimingsTests.cpp
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/CameraTimings.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/views/CameraCommandTiming.cpp")
target_include_directories(postprocessor_camera_timings_tests PRIVATE "${CUBITSIM_SOURCE_ROOT}")
target_compile_features(postprocessor_camera_timings_tests PRIVATE cxx_std_17)
enable_testing()
add_executable(postprocessor_selection_tests SelectionTests.cpp)
target_link_libraries(postprocessor_selection_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_selection COMMAND postprocessor_selection_tests)
set_tests_properties(postprocessor_selection PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
add_test(NAME postprocessor_camera_timings COMMAND postprocessor_camera_timings_tests)
add_test(NAME postprocessor_display COMMAND postprocessor_display_tests)
add_executable(postprocessor_camera_tests CameraTests.cpp)
target_link_libraries(postprocessor_camera_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_camera COMMAND postprocessor_camera_tests)
set_tests_properties(postprocessor_camera PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set(_test_path "${CUBIT_ROOT}/bin;$ENV{PATH}")
  string(REPLACE ";" "\\;" _test_path "${_test_path}")
  set_tests_properties(postprocessor_display PROPERTIES ENVIRONMENT "PATH=${_test_path}")
  set_property(TEST postprocessor_selection APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
  set_property(TEST postprocessor_camera APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_navigation_tests NavigationTests.cpp AnimationRotationTests.cpp)
target_link_libraries(postprocessor_navigation_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_navigation COMMAND postprocessor_navigation_tests)
set_tests_properties(postprocessor_navigation PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_navigation APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_coloring_tests ColoringTests.cpp)
target_link_libraries(postprocessor_coloring_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_coloring COMMAND postprocessor_coloring_tests)
if (WIN32)
  set_property(TEST postprocessor_coloring APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_deformation_tests DeformationTests.cpp)
target_link_libraries(postprocessor_deformation_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_deformation COMMAND postprocessor_deformation_tests)
set_tests_properties(postprocessor_deformation PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_deformation APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_display_properties_tests DisplayPropertiesTests.cpp)
target_link_libraries(postprocessor_display_properties_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_display_properties COMMAND postprocessor_display_properties_tests)
set_tests_properties(postprocessor_display_properties PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_display_properties APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_table_tests TableTests.cpp)
target_link_libraries(postprocessor_table_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_tables COMMAND postprocessor_table_tests)
set_tests_properties(postprocessor_tables PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_tables APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_project_tests ProjectPersistenceTests.cpp)
target_link_libraries(postprocessor_project_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_project COMMAND postprocessor_project_tests)
set_tests_properties(postprocessor_project PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_project APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_import_tests ImportTests.cpp
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Sources/PostprocessorSourceControls.cpp")
target_link_libraries(postprocessor_import_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_import COMMAND postprocessor_import_tests)
set_tests_properties(postprocessor_import PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_import APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_reference_mesh_tests ReferenceMeshTests.cpp)
target_link_libraries(postprocessor_reference_mesh_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_reference_mesh COMMAND postprocessor_reference_mesh_tests)
if (WIN32)
  set_property(TEST postprocessor_reference_mesh APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_source_defaults_tests SourceDefaultsTests.cpp)
target_link_libraries(postprocessor_source_defaults_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_source_defaults COMMAND postprocessor_source_defaults_tests)
set_tests_properties(postprocessor_source_defaults PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_source_defaults APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_table_pane_tests TablePaneTests.cpp)
target_link_libraries(postprocessor_table_pane_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_table_pane COMMAND postprocessor_table_pane_tests)
set_tests_properties(postprocessor_table_pane PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_table_pane APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

# Annotation cases share the selection fixture but keep feature tests separate.
target_sources(postprocessor_selection_tests PRIVATE AnnotationTests.cpp LegendTests.cpp)

target_sources(postprocessor_selection_tests PRIVATE PlotTests.cpp)

target_sources(postprocessor_selection_tests PRIVATE CaptureTests.cpp AnimationCaptureTests.cpp)

target_sources(postprocessor_selection_tests PRIVATE FieldSelectorTests.cpp)

target_sources(postprocessor_selection_tests PRIVATE TableSelectorTests.cpp)

add_executable(postprocessor_filter_tests FilterTests.cpp ThresholdTests.cpp VectorPlotTests.cpp)
target_link_libraries(postprocessor_filter_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_filters COMMAND postprocessor_filter_tests)
set_tests_properties(postprocessor_filters PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_filters APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_units_metadata_tests UnitsMetadataTests.cpp)
target_link_libraries(postprocessor_units_metadata_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_units_metadata COMMAND postprocessor_units_metadata_tests)
if (WIN32)
  set_property(TEST postprocessor_units_metadata APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_coloring_units_tests ColoringUnitsTests.cpp)
target_link_libraries(postprocessor_coloring_units_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_coloring_units COMMAND postprocessor_coloring_units_tests)
set_tests_properties(postprocessor_coloring_units PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_coloring_units APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_presentation_units_tests PresentationUnitsTests.cpp)
target_link_libraries(postprocessor_presentation_units_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_presentation_units COMMAND postprocessor_presentation_units_tests)
set_tests_properties(postprocessor_presentation_units PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_presentation_units APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_report_collection_tests ReportCollectionTests.cpp ReportCaptureTests.cpp)
target_link_libraries(postprocessor_report_collection_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_report_collection COMMAND postprocessor_report_collection_tests)

add_executable(postprocessor_appearance_tests AppearanceTests.cpp)
target_link_libraries(postprocessor_appearance_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_appearance COMMAND postprocessor_appearance_tests)
set_tests_properties(postprocessor_appearance PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
if (WIN32)
  set_property(TEST postprocessor_appearance APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif ()

add_executable(postprocessor_explosion_tests ExplosionTests.cpp)
target_link_libraries(postprocessor_explosion_tests PRIVATE postprocessor_display_code)
add_test(NAME postprocessor_explosion COMMAND postprocessor_explosion_tests)
if(WIN32)
  set_property(TEST postprocessor_explosion APPEND PROPERTY ENVIRONMENT "PATH=${_test_path}")
endif()
