# Shared ownership contracts; no dependency on a solver, viewport or VTK runtime.
add_library(fea_results INTERFACE)
target_compile_features(fea_results INTERFACE cxx_std_17)
target_include_directories(fea_results INTERFACE "${CCX_SOURCE_ROOT}")
target_sources(fea_results INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/VtkResultsData.hpp")

# Optional serialization is separate from the in-memory ownership contract.
add_library(fea_results_io STATIC VtkResultsExporter.cpp VtkResultsExporter.hpp)
target_link_libraries(fea_results_io PUBLIC fea_results PRIVATE VTK::IOXML VTK::IOParallelXML)
set_target_properties(fea_results_io PROPERTIES POSITION_INDEPENDENT_CODE ON)
