target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreCsvWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreCsvWriter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCoreCustomLines.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreCustomLines.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreCustomLines.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreCustomLinesPersistence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreCustomLinesPersistence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXExportInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXExportInterface.hpp
)
