target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreTopology.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreTopology.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreTopologyOptimization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreTopologyOptimization.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXTopologyInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXTopologyInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXTopologyRunInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXTopologyRunInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreTopologyStructs.hpp
)
