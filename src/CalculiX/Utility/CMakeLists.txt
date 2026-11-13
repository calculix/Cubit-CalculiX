project(cubit-sim_Utility)

# Include Eigen code
include_directories(${CUBITSIM_SOURCE_ROOT}/Utility/Eigen)

set(UTILITY_SRC
    ${CMAKE_CURRENT_LIST_DIR}/PersistenceTable.hpp
    ${CMAKE_CURRENT_LIST_DIR}/StopWatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StopWatch.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ThreadPool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ThreadPool.hpp
    ${CMAKE_CURRENT_LIST_DIR}/PlotChart.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlotChart.hpp
    ${CMAKE_CURRENT_LIST_DIR}/TopologyHdf5.hpp
    ${CMAKE_CURRENT_LIST_DIR}/TopologyIterationIds.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ScientificNumber.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ElementNodeOrdering.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ElementInterpolation.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ElementInterpolation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ElementPhysicalMapping.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ElementPhysicalMapping.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ElementIntegration.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ElementIntegration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GifAnimation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GifAnimation.hpp
    ${CUBITSIM_SOURCE_ROOT}/Utility/gif.h
    PARENT_SCOPE)
