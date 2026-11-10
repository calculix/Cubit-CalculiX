find_package(Qt6 COMPONENTS Svg REQUIRED)

add_library(report_module SHARED
  Commands/ReportSourceCommand.cpp Commands/ReportSourceCommand.hpp
  Commands/ReportCommands.cpp
  Commands/ReportConfigureCommand.cpp
  Commands/ReportExportCommand.cpp
  Commands/ReportGenerateCommand.cpp
  Commands/ReportPreviewCommand.cpp
  Commands/ReportProfileCommand.cpp
  Commands/ReportUpdateCommand.cpp
  Core/ReportLogo.cpp Core/ReportLogo.hpp
  Document/ReportPageDecoration.cpp Document/ReportPageDecoration.hpp
  Core/ReportConfigurationResolver.cpp
  Core/ReportCore.cpp
  Core/ReportProjectPersistence.cpp
  Core/ReportSession.cpp
  Core/ReportUserOptions.cpp
  Document/ReportDocumentLayout.cpp
  Document/ReportOdtExporter.cpp
  Document/ReportPdfExporter.cpp
  EventObservers/ReportModelObserver.cpp
  GUI/ReportCommandText.cpp
  GUI/ReportContentTree.cpp
  GUI/ReportGui.cpp
  GUI/ReportGuiObserver.cpp
  GUI/ReportPreviewWidget.cpp
  GUI/ReportPreviewWindow.cpp
  GUI/ReportIconLoader.cpp GUI/ReportIconLoader.hpp
  Icons/Report.svg
  GUI/ReportSettingsDialog.cpp
  GUI/ReportSettingsCommandText.cpp GUI/ReportSettingsCommandText.hpp
  GUI/ReportWorkspace.cpp
  GUI/ReportToolbar.cpp GUI/ReportToolbar.hpp
  GUI/ReportExportDialog.cpp GUI/ReportExportDialog.hpp
  Icons/ExportPdf.svg Icons/ExportOdt.svg
  Interfaces/ReportApplicationInterface.cpp
  ReportModule.cpp
  Commands/ReportCommands.hpp
  Commands/ReportConfigureCommand.hpp
  Commands/ReportExportCommand.hpp
  Commands/ReportGenerateCommand.hpp
  Commands/ReportPreviewCommand.hpp
  Commands/ReportProfileCommand.hpp
  Commands/ReportUpdateCommand.hpp
  Core/ReportConfigurationResolver.hpp
  Core/ReportCore.hpp
  Core/ReportProjectPersistence.hpp
  Core/ReportSession.hpp
  Core/ReportUserOptions.hpp
  Document/ReportDocumentLayout.hpp
  Document/ReportOdtExporter.hpp
  Document/ReportPdfExporter.hpp
  EventObservers/ReportModelObserver.hpp
  GUI/ReportCommandText.hpp
  GUI/ReportContentTree.hpp
  GUI/ReportGui.hpp
  GUI/ReportGuiObserver.hpp
  GUI/ReportPreviewWidget.hpp
  GUI/ReportPreviewWindow.hpp
  GUI/ReportSettingsDialog.hpp
  GUI/ReportWorkspace.hpp
  Interfaces/ReportApplicationInterface.hpp
  ReportModule.hpp
)
target_compile_features(report_module PUBLIC cxx_std_17)
target_include_directories(report_module PUBLIC "${CUBITSIM_SOURCE_ROOT}")
target_link_libraries(report_module PRIVATE cubitsim_reporting cubitsim_storage cubiti cubit_util clarogui clarofw Qt6::Widgets Qt6::Svg)
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
