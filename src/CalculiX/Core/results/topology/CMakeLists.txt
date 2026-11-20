target_sources(calculix_core PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXTopologyResultSource.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TopologyElementPartitioner.cpp
)
target_link_libraries(calculix_core VTK::FiltersCore)
