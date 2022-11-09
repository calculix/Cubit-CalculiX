project(Cubit-CalculiX_core)

set(CORE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXCore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXCore.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXCoreInterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXCoreInterface.hpp
    ${CMAKE_CURRENT_LIST_DIR}/loadCalculiXCore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/loadCalculiXCore.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CoreBlocks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CoreBlocks.hpp
    PARENT_SCOPE)
