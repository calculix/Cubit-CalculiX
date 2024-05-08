project(Cubit-CalculiX_pythoninterface)

set(PYTHONINTERFACE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXPythonInterface_wrap.cxx    
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXPythonInterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CalculiXPythonInterface.hpp    
    PARENT_SCOPE)
