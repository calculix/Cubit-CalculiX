project(Cubit-CalculiX_commands)

set(CMD_SRC
    ${CMAKE_CURRENT_LIST_DIR}/ccxVersionCommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ccxVersionCommand.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ccxExportCommand.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ccxExportCommand.hpp
    PARENT_SCOPE)
