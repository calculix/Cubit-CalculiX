add_executable(report_tests
  ReportTests.cpp ReportTestProviders.cpp ReportTestProviders.hpp
  ${CUBITSIM_SOURCE_ROOT}/CalculiX/Core/reporting/CalculiXReportInputReader.cpp
)
target_compile_features(report_tests PRIVATE cxx_std_17)
target_include_directories(report_tests PRIVATE "${CUBITSIM_SOURCE_ROOT}")
target_link_libraries(report_tests PRIVATE report_module cubitsim_reporting cubitsim_storage Qt6::Widgets)
add_test(NAME report_contracts_and_exports COMMAND report_tests)
set_tests_properties(report_contracts_and_exports PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
