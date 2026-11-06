project(cubit-sim_useroptions)

set(UO_SRC
    ${CMAKE_CURRENT_LIST_DIR}/UserOptionsExport.hpp
    ${CMAKE_CURRENT_LIST_DIR}/DrawingOptions.hpp
    ${CMAKE_CURRENT_LIST_DIR}/UserOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UserOptions.hpp
    ${CMAKE_CURRENT_LIST_DIR}/loadUserOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loadUserOptions.hpp
    PARENT_SCOPE)
