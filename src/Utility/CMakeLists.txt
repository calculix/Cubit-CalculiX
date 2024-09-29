project(Cubit-CalculiX_Utility)

# Include Eigen code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Eigen)

set(UTILITY_SRC
    ${CMAKE_CURRENT_LIST_DIR}/StopWatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StopWatch.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ThreadPool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ThreadPool.hpp
    ${CMAKE_CURRENT_LIST_DIR}/PlotChart.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlotChart.hpp
    ${CMAKE_CURRENT_LIST_DIR}/HDF5Tool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HDF5Tool.hpp
    PARENT_SCOPE)
