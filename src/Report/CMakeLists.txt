find_package(Qt6 COMPONENTS Svg REQUIRED)

add_library(report_module SHARED
  ReportModule.cpp
  ReportModule.hpp
)

add_subdirectory(Commands)
add_subdirectory(Core)
add_subdirectory(Document)
add_subdirectory(EventObservers)
add_subdirectory(GUI)
add_subdirectory(Icons)
add_subdirectory(Interfaces)

target_compile_features(report_module PUBLIC cxx_std_17)
target_include_directories(report_module PUBLIC "${CUBITSIM_SOURCE_ROOT}")
target_link_libraries(report_module PRIVATE cubitsim_reporting cubitsim_storage cubitsim_model cubiti cubit_util clarogui clarofw Qt6::Widgets Qt6::Svg)
set_target_properties(report_module PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  LIBRARY_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  ARCHIVE_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  BUILD_RPATH_USE_ORIGIN TRUE
)

option(CUBITSIM_REPORT_TESTS "Build the reporting contract/storage/export tests" OFF)
if (CUBITSIM_REPORT_TESTS)
  enable_testing()
  add_subdirectory(Tests)
endif()

add_custom_target(report_icons ALL
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${CUBITSIM_DEPLOY_DIRECTORY}/Icons"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/Report.svg" "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/Report.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/ExportPdf.svg" "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/ExportPdf.svg"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/ExportOdt.svg" "${CUBITSIM_DEPLOY_DIRECTORY}/Icons/ExportOdt.svg"
  VERBATIM
)
add_dependencies(report_module report_icons)
