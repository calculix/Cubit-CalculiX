project(Cubit-CalculiX_useroptions)

set(UO_COMP_SRC
    ${CMAKE_CURRENT_LIST_DIR}/UserOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UserOptions.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ConfigFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ConfigFile.hpp
    ${CMAKE_CURRENT_LIST_DIR}/loadUserOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loadUserOptions.hpp
    ${CMAKE_CURRENT_LIST_DIR}/UserOptionsPanel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UserOptionsPanel.hpp
    PARENT_SCOPE)
