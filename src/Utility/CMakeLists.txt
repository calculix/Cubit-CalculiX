project(Cubit-CalculiX_Utility)

# Include our Eigen code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Eigen)


set(UTILITY_SRC
    ${CMAKE_CURRENT_LIST_DIR}/StopWatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StopWatch.hpp
    PARENT_SCOPE)
