target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultReadBuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultReadNumbers.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultReadTimings.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultsReaderPipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResults.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResults.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsProject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsProject.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsVtkWriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsVtkWriter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXResultsInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXResultsInterface.hpp
)

add_subdirectory(frd)
add_subdirectory(dat)
