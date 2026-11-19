target_sources(report_module PRIVATE
  ReportConfigurationResolver.cpp
  ReportConfigurationResolver.hpp
  ReportCore.cpp
  ReportCore.hpp
  ReportImages.cpp
  ReportLogo.cpp
  ReportLogo.hpp
  ReportProjectPersistence.cpp
  ReportProjectPersistence.hpp
  ReportSession.cpp
  ReportSession.hpp
  ReportUnitValidation.cpp
  ReportUnitValidation.hpp
  ReportUserOptions.cpp
  ReportUserOptions.hpp
)

add_subdirectory(configuration)
add_subdirectory(generation)
add_subdirectory(sources)
