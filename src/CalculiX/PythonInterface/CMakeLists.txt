project(Cubit-CalculiX_pythoninterface)

set(PYTHONINTERFACE_SRC
    ${CCXPYTHONINTERFACE_WRAP}
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXPythonInterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXPythonInterface.hpp
    PARENT_SCOPE
)