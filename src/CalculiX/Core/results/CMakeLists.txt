target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsExternalSource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsExternalSource.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsIdentity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsIdentity.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultReadBuffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultReadNumbers.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultReadTimings.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ResultsReaderPipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCoreResults.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResults.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsReaderData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResults.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsPersistence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsPersistence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsCsv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsCsv.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsMath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsMath.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsPlot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsPlot.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsProject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsProject.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsVtkStructs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsVtkConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsVtkConverter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsVtkPipeline.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsVtkElements.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsVtkElements.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsInpReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsInpReader.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsVtkLink.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vtk/CoreResultsVtkLink.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXResultsInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXResultsInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXResultSourceProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXResultSourceProvider.hpp
)

# legacy/CoreResultsVtkWriterLegacy.* is intentionally not built or called.
target_link_libraries(calculix_core cubitsim_results cubitsim_results_io)
# Dataset conversion only. Serialization is linked through cubitsim_results_io;
# CubitVtkSDK supplies the host DLL targets without a viewport dependency.
target_link_libraries(calculix_core VTK::CommonCore VTK::CommonDataModel)

add_subdirectory(frd)
add_subdirectory(dat)
