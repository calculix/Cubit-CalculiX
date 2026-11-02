project(Cubit-CalculiX_useroptions)

set(UO_SRC
    ${CMAKE_CURRENT_LIST_DIR}/UserOptionsExport.hpp
    ${CMAKE_CURRENT_LIST_DIR}/DrawingOptions.hpp
    ${CMAKE_CURRENT_LIST_DIR}/DrawingOptionsWidget.hpp
    ${CMAKE_CURRENT_LIST_DIR}/DrawingOptionsWidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UserOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UserOptions.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXConfigFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXConfigFile.hpp
    ${CMAKE_CURRENT_LIST_DIR}/loadUserOptions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loadUserOptions.hpp
    ${CMAKE_CURRENT_LIST_DIR}/UserOptionsPanel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UserOptionsPanel.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXSettings.hpp
    PARENT_SCOPE)
