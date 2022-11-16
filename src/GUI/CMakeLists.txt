project(Cubit-CalculiX_component_gui)

set(SRC
    ${CMAKE_CURRENT_LIST_DIR}/ModelTree.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ModelTree.hpp
    ${CMAKE_CURRENT_LIST_DIR}/BlocksTree.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BlocksTree.hpp
    ${CMAKE_CURRENT_LIST_DIR}/MyPanel1.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MyPanel1.hpp
    ${CMAKE_CURRENT_LIST_DIR}/MyPanel2.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MyPanel2.hpp)

set(UI
    ${CMAKE_CURRENT_LIST_DIR}/MyPanel1.ui
    ${CMAKE_CURRENT_LIST_DIR}/MyPanel2.ui)

# Add the parent scope so that our parent cmake list can use them
set(GUI_SRC ${SRC} PARENT_SCOPE)
set(GUI_MOC ${MOC} PARENT_SCOPE)
set(GUI_UI  ${UI}  PARENT_SCOPE)
