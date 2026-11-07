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
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/application/PostprocessorCore.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/sources/PostprocessorCoreSources.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/ResultMesh.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/representations/PostprocessorCoreRepresentations.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/views/PostprocessorCoreViews.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Core/options/PostprocessorUserOptions.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/Interfaces/PostprocessorApplicationInterface.cpp"
  "${CUBITSIM_SOURCE_ROOT}/Postprocessor/GUI/Viewport/PostprocessorScene.cpp")
target_link_libraries(postprocessor_display_code PUBLIC cubitsim_results VTK::CommonDataModel
  VTK::FiltersGeometry VTK::RenderingCore VTK::RenderingOpenGL2 Qt6::Core)
target_compile_features(postprocessor_display_code PUBLIC cxx_std_17)
add_executable(postprocessor_display_tests DisplayTests.cpp)
target_link_libraries(postprocessor_display_tests PRIVATE postprocessor_display_code)
enable_testing()
add_test(NAME postprocessor_display COMMAND postprocessor_display_tests)
if (WIN32)
  set(_test_path "${CUBIT_ROOT}/bin;$ENV{PATH}")
  string(REPLACE ";" "\\;" _test_path "${_test_path}")
  set_tests_properties(postprocessor_display PROPERTIES ENVIRONMENT "PATH=${_test_path}")
endif ()
