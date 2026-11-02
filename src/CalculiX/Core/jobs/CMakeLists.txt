target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/JobsSettings.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCoreJobs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreJobs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreJobs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreJobsPersistence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreJobsPersistence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreJobExportContext.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreJobInputPreparation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreJobInputPreparation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXJobsInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXJobsInterface.hpp
)
