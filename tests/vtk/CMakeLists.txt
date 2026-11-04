cmake_minimum_required(VERSION 3.20)
project(CubitVtkDatasetTests LANGUAGES CXX)

get_filename_component(CCX_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../src" ABSOLUTE)
set(CUBIT_ROOT "C:/Program Files/Coreform Cubit 2026.11" CACHE PATH "Cubit installation")
include("${CCX_SOURCE_ROOT}/cmake/CubitVtkSDK.cmake")
find_package(CubitUtil CONFIG REQUIRED PATHS "${CUBIT_ROOT}/cmake" NO_DEFAULT_PATH)
find_package(Threads REQUIRED)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory("${CCX_SOURCE_ROOT}/Framework/Results" results)

# Compile production conversion code into a DLL to test ownership across a real
# C++ module boundary. No workspace, rendering, parser mocks or live model.
add_library(vtk_converter_test_code SHARED
  "${CCX_SOURCE_ROOT}/CalculiX/Core/results/CoreResultsReaderData.cpp"
  "${CCX_SOURCE_ROOT}/CalculiX/Core/results/vtk/CoreResultsVtkConverter.cpp"
  "${CCX_SOURCE_ROOT}/CalculiX/Core/results/vtk/CoreResultsVtkElements.cpp"
  "${CCX_SOURCE_ROOT}/CalculiX/Utility/ElementIntegration.cpp"
  "${CCX_SOURCE_ROOT}/CalculiX/Utility/ElementInterpolation.cpp")
target_include_directories(vtk_converter_test_code PUBLIC
  "${CCX_SOURCE_ROOT}/CalculiX" "${CCX_SOURCE_ROOT}/CalculiX/Utility"
  "${CCX_SOURCE_ROOT}/Utility" "${CCX_SOURCE_ROOT}/../../boost_1_92_0")
target_link_libraries(vtk_converter_test_code PUBLIC fea_results cubit_util VTK::CommonCore VTK::CommonDataModel Threads::Threads)

add_executable(vtk_dataset_tests VtkDatasetTests.cpp)
target_link_libraries(vtk_dataset_tests PRIVATE vtk_converter_test_code fea_results_io VTK::IOXML)
enable_testing()
add_test(NAME vtk_datasets COMMAND vtk_dataset_tests "${CMAKE_CURRENT_BINARY_DIR}/test-output")
set(_vtk_test_path "${CUBIT_ROOT}/bin;$<TARGET_FILE_DIR:vtk_converter_test_code>;$ENV{PATH}")
string(REPLACE ";" "\\;" _vtk_test_path "${_vtk_test_path}")
set_tests_properties(vtk_datasets PROPERTIES ENVIRONMENT "PATH=${_vtk_test_path}")
