# Shared ownership contracts; no dependency on a solver, viewport or VTK runtime.
add_library(cubitsim_results INTERFACE)
target_compile_features(cubitsim_results INTERFACE cxx_std_17)
target_include_directories(cubitsim_results INTERFACE "${CUBITSIM_SOURCE_ROOT}")
target_sources(cubitsim_results INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/VtkResultsData.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ResultSourceReference.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PublishedResultSource.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../Interfaces/ResultSourceProviderInterface.hpp"
)

# Optional serialization is separate from the in-memory ownership contract.
find_package(Threads REQUIRED)
add_library(cubitsim_results_io STATIC VtkResultsExporter.cpp VtkResultsExporter.hpp)
target_link_libraries(cubitsim_results_io PUBLIC cubitsim_results PRIVATE VTK::IOXML VTK::IOParallelXML Threads::Threads)
set_target_properties(cubitsim_results_io PROPERTIES POSITION_INDEPENDENT_CODE ON)
