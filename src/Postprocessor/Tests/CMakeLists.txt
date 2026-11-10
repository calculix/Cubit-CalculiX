cmake_minimum_required(VERSION 3.20)
project(PostprocessorDisplayTests LANGUAGES CXX)

get_filename_component(CUBITSIM_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)
if (WIN32)
  set(CUBIT_ROOT "C:/Program Files/Coreform Cubit 2026.11" CACHE PATH "Cubit installation")
endif ()
include("${CUBITSIM_SOURCE_ROOT}/cmake/CubitVtkSDK.cmake")
add_subdirectory("${CUBITSIM_SOURCE_ROOT}/Framework/Results" results)

add_library(postprocessor_display_code STATIC
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/persistence/PostprocessorSessionCodec.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/application/PostprocessorCorePersistence.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/tables/ResultTables.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/tables/PostprocessorCoreTables.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/sources/VtkFileSourceProvider.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Tables/PostprocessorTableModel.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Tables/PostprocessorTableView.cpp"
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
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/views/PostprocessorCoreCamera.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/views/CameraOperations.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/views/CameraCommandText.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Commands/views/CameraCommandTiming.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/CameraInteraction.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/CameraTimings.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Options/PostprocessorMouseOptionsPanel.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Properties/PostprocessorDisplayProperties.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/options/PostprocessorUserOptions.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorApplicationInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorScene.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorColorMap.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorTriad.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Navigation/PostprocessorFrameNavigation.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Navigation/PostprocessorDeformationControls.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.cpp")
target_link_libraries(postprocessor_display_code PUBLIC cubitsim_results VTK::IOXML VTK::CommonDataModel
  VTK::FiltersCore VTK::FiltersGeometry VTK::RenderingCore VTK::RenderingAnnotation VTK::RenderingOpenGL2 Qt6::Core Qt6::Widgets)
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

add_executable(postprocessor_navigation_tests NavigationTests.cpp)
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
