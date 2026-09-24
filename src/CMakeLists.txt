cmake_minimum_required(VERSION 3.5)

project(Cubit-CalculiX)

if (UNIX)
 set(CMAKE_PREFIX_PATH "~/Downloads/Coreform-Cubit-2026.9/cmake")
 include_directories("~/Downloads/Coreform-Cubit-2026.9/include")
 include_directories("~/Downloads/Coreform-Cubit-2026.9/include/python")
 include_directories("~/Downloads/Coreform-Cubit-2026.9/bin")
 # python for our swig wrapping
 include_directories("~/Downloads/Python-3.12.10/Include")
 include_directories("~/Downloads/Python-3.12.10") 
 set(HDF5_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Utility/HDF5/Linux/1.14.4.3)
endif (UNIX)

if (WIN32)
 SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
 set(CMAKE_PREFIX_PATH "C:/Program Files/Coreform Cubit 2026.9/cmake")
 include_directories("C:/Program Files/Coreform Cubit 2026.9/include")
 set(Qt6_DIR "C:/Qt/6.8.3/msvc2022_64/lib/cmake/Qt6") 
 set(Qt6Core_DIR "C:/Qt/6.8.3/msvc2022_64/lib/cmake/Qt6Core") 
 set(Qt6Charts_DIR "C:/Qt/6.8.3/msvc2022_64/lib/cmake/Qt6Charts")
 set(Qt6Widgets_DIR "C:/Qt/6.8.3/msvc2022_64/lib/cmake/Qt6Widgets")
 set(HDF5_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Utility/HDF5/Win/1.14.4.3) 
 # python and swig for our wrapping
 include_directories("C:/Program Files/Python312/include")
 link_directories("C:/Program Files/Python312/libs")
 set(SWIG_EXECUTABLE
    "C:/Program Files/swigwin-4.5.1/swig.exe"
    CACHE FILEPATH "Path to SWIG executable"
    )
endif (WIN32)

# On Linux, we need to add some addition compiler flags to get our static
# library to link with our swig module (which acts like a shared library)
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

# Get the packages needed to use the SDK
find_package(Cubit REQUIRED CONFIG)
find_package(CubitGUI REQUIRED CONFIG)
find_package(CubitPython REQUIRED CONFIG)
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Charts REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS C CXX HL)
ADD_DEFINITIONS(-DH5_BUILT_AS_DYNAMIC_LIB)


# SWIG WRAPPING
find_package(SWIG REQUIRED)

if (UNIX)
set(CCXCOMP_WRAP
    ${CMAKE_CURRENT_BINARY_DIR}/calculix_comp_wrap.cxx)
set(CCXCOMP_PY
    ${CMAKE_CURRENT_BINARY_DIR}/calculix_comp.py)
endif(UNIX)
if (WIN32)
set(CCXCOMP_WRAP
    ${CMAKE_CURRENT_BINARY_DIR}/Release/calculix_comp_wrap.cxx)
set(CCXCOMP_PY
    ${CMAKE_CURRENT_BINARY_DIR}/Release/calculix_comp.py)
endif(WIN32)

if(UNIX)
add_custom_command(
    OUTPUT ${CCXCOMP_WRAP} ${CCXCOMP_PY}
    COMMAND  ${SWIG_EXECUTABLE} 
        -c++
        -python
         -I${CMAKE_CURRENT_SOURCE_DIR} 
        -outdir  ${CMAKE_CURRENT_BINARY_DIR} 
        -o  ${CCXCOMP_WRAP} 
         ${CMAKE_CURRENT_SOURCE_DIR}/calculix_comp.i 
    DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/calculix_comp.i 
         ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXComp.hpp 
    COMMENT "Generating CalculiXComp Python bindings"
    VERBATIM
)
endif(UNIX)
if(WIN32)
add_custom_command(
    OUTPUT ${CCXCOMP_WRAP} ${CCXCOMP_PY}
    COMMAND  ${SWIG_EXECUTABLE} 
        -c++
        -python
         -I${CMAKE_CURRENT_SOURCE_DIR} 
        -outdir  ${CMAKE_CURRENT_BINARY_DIR}/Release
        -o  ${CCXCOMP_WRAP} 
         ${CMAKE_CURRENT_SOURCE_DIR}/calculix_comp.i 
    DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/calculix_comp.i 
         ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXComp.hpp 
    COMMENT "Generating CalculiXComp Python bindings"
    VERBATIM
)
endif(WIN32)

if (UNIX)
set(CCXPYTHONINTERFACE_WRAP
     ${CMAKE_CURRENT_BINARY_DIR}/CalculiXPythonInterface_wrap.cxx )
endif(UNIX)
if (WIN32)
set(CCXPYTHONINTERFACE_WRAP
     ${CMAKE_CURRENT_BINARY_DIR}/Release/CalculiXPythonInterface_wrap.cxx )
endif(WIN32)

if(UNIX)
add_custom_command(
    OUTPUT
         ${CCXPYTHONINTERFACE_WRAP} 
         ${CMAKE_CURRENT_BINARY_DIR}/calculix_pythoninterface.py 
    COMMAND  ${SWIG_EXECUTABLE} 
        -c++
        -python
         -I${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface 
        -outdir  ${CMAKE_CURRENT_BINARY_DIR}
        -o  ${CCXPYTHONINTERFACE_WRAP} 
         ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/CalculiXPythonInterface.i 
    DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/CalculiXPythonInterface.i 
         ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/CalculiXPythonInterface.hpp 
    COMMENT "Generating CalculiXPythonInterface Python bindings"
    VERBATIM
)
endif(UNIX)

if(WIN32)
add_custom_command(
    OUTPUT
         ${CCXPYTHONINTERFACE_WRAP} 
         ${CMAKE_CURRENT_BINARY_DIR}/Release/calculix_pythoninterface.py 
    COMMAND  ${SWIG_EXECUTABLE} 
        -c++
        -python
         -I${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface 
        -outdir ${CMAKE_CURRENT_BINARY_DIR}/Release
        -o  ${CCXPYTHONINTERFACE_WRAP} 
         ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/CalculiXPythonInterface.i 
    DEPENDS
         ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/CalculiXPythonInterface.i 
         ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/CalculiXPythonInterface.hpp 
    COMMENT "Generating CalculiXPythonInterface Python bindings"
    VERBATIM
)
endif(WIN32)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# this will link our libraries with a relative path
SET(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

# Include our Core code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core)
add_subdirectory(Core)

# Include our Commands code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Commands)
add_subdirectory(Commands)

# Include our GUI code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/GUI)
add_subdirectory(GUI)

# Include our EventObserver code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/EventObservers)
add_subdirectory(EventObservers)

# Include our UserOptions code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/UserOptions)
add_subdirectory(UserOptions)

# Include our Utility code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Utility)
add_subdirectory(Utility)

# Include our Python Interface code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface)
add_subdirectory(PythonInterface)

# copy pythoninterface and icons to build directory
if (WIN32)
 #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/calculix_pythoninterface.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Icons/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/Icons/)
endif (WIN32)
if (UNIX)
 #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/calculix_pythoninterface.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Icons/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Icons/)
 #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/calculix_comp.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
endif (UNIX)

# copy missing libraries into build directory
# Qt6Charts is currently not shipped with Cubit 2025.8
if (WIN32)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Win/Qt6Charts.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Win/hdf5_cpp.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Win/hdf5.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
endif (WIN32)
if (UNIX)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Linux/libQt6Charts.so.6.2.4 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Linux/libhdf5_cpp.so DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Linux/libhdf5_cpp.so.310 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Linux/libhdf5_cpp.so.310.0.4 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
endif (UNIX)

#copy material default library into build directory
if (WIN32)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libmaterial.hdf5 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
endif (WIN32)
if (UNIX)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libmaterial.hdf5 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
endif (UNIX)


set(CCXPLUGIN_SRCS
    CalculiXPlugin.cpp
    CalculiXPlugin.hpp
    ${CMD_SRC})

set(CCXCORE_SRCS
    ${CORE_SRC}
    ${UTILITY_SRC})

set(CCXUSEROPTIONS_SRCS
    ${UO_SRC}) 

set(CCXPYTHONINTERFACE_SRCS
    ${PYTHONINTERFACE_SRC})

set(CCXCOMP_SRCS
    CalculiXComp.cpp
    CalculiXComp.hpp
    ${CCXCOMP_WRAP}
    ${EO_COMP_SRC}
    ${GUI_SRC}
    MenuManager.cpp
    MenuManager.hpp
    ToolbarManager.cpp
    ToolbarManager.hpp
    cmdPanelManager.cpp
    cmdPanelManager.hpp
    MyCmdWidgetFactory.cpp
    MyCmdWidgetFactory.hpp
    CCXDockWindowModelTree.cpp
    CCXDockWindowModelTree.hpp
    # never worked, also deprecated 20260822
    #ExportManager.cpp
    #ExportManager.hpp
    #deprecated 20260822
    #loadCalculiXComp.cpp
    )

if (WIN32)
    add_library(calculix_useroptions SHARED ${CCXUSEROPTIONS_SRCS})
    target_link_libraries(calculix_useroptions clarogui cubiti clarofw Qt6::Widgets)
    set_target_properties(calculix_useroptions PROPERTIES AUTOMOC 1)

    add_library(calculix_plugin MODULE ${CCXPLUGIN_SRCS})
    #set_target_properties(calculix_plugin PROPERTIES SUFFIX ".ccm")

    add_library(calculix_core SHARED ${CCXCORE_SRCS})
    target_link_libraries(calculix_core cubiti cubit_util cubit_gui)
    #target_link_libraries(calculix_core clarogui navigation pickwidget clarofw Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_core clarogui clarofw navigation Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_core HDF5::HDF5)
    set_target_properties(calculix_core PROPERTIES AUTOMOC 1)

    add_library(calculix_comp SHARED ${CCXCOMP_SRCS})
    #set_target_properties(calculix_comp PROPERTIES SUFFIX ".ccl")
    set_target_properties(calculix_comp PROPERTIES SUFFIX ".pyd" PREFIX "_")
    #target_link_libraries(calculix_comp clarogui navigation pickwidget cubiti clarofw Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_comp clarogui navigation cubiti clarofw Qt6::Charts Qt6::Widgets component)
    set_target_properties(calculix_comp PROPERTIES AUTOMOC 1)

    add_library(calculix_pythoninterface SHARED ${CCXPYTHONINTERFACE_SRCS})
    set_target_properties(calculix_pythoninterface PROPERTIES SUFFIX ".pyd" PREFIX "_")
    target_link_libraries(calculix_pythoninterface cubiti cubit_util)

    target_link_libraries(calculix_core calculix_useroptions)
    target_link_libraries(calculix_plugin calculix_core)
    target_link_libraries(calculix_pythoninterface calculix_core)
    target_link_libraries(calculix_comp calculix_core)
endif (WIN32)

if (UNIX)
    add_library(calculix_useroptions SHARED ${CCXUSEROPTIONS_SRCS})
    target_link_libraries(calculix_useroptions clarogui cubiti clarofw Qt6::Widgets)
    set_target_properties(calculix_useroptions PROPERTIES AUTOMOC 1)

    add_library(calculix_plugin MODULE ${CCXPLUGIN_SRCS})
    #set_target_properties(calculix_plugin PROPERTIES SUFFIX ".ccm")

    add_library(calculix_core SHARED ${CCXCORE_SRCS})
    set_target_properties(calculix_core PROPERTIES SUFFIX ".so")
    target_link_libraries(calculix_core cubiti cubit_util cubit_gui)
    #target_link_libraries(calculix_core clarogui clarofw navigation pickwidget Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_core clarogui clarofw navigation Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_core HDF5::HDF5)
    set_target_properties(calculix_core PROPERTIES AUTOMOC 1)

    add_library(calculix_comp SHARED ${CCXCOMP_SRCS})
    #set_target_properties(calculix_comp PROPERTIES SUFFIX ".ccl")
    set_target_properties(calculix_comp PROPERTIES SUFFIX ".so" PREFIX "_")
    #target_link_libraries(calculix_comp clarogui navigation pickwidget cubiti clarofw Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_comp clarogui navigation cubiti clarofw Qt6::Charts Qt6::Widgets component)
    set_target_properties(calculix_comp PROPERTIES AUTOMOC 1)

    add_library(calculix_pythoninterface SHARED ${CCXPYTHONINTERFACE_SRCS})
    set_target_properties(calculix_pythoninterface PROPERTIES SUFFIX ".so" PREFIX "_")
    target_link_libraries(calculix_pythoninterface cubiti cubit_util)

    target_link_libraries(calculix_core calculix_useroptions)
    target_link_libraries(calculix_pythoninterface calculix_core)
    target_link_libraries(calculix_plugin calculix_core)
    target_link_libraries(calculix_comp calculix_core)
endif (UNIX)

# copy external software
if (WIN32)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Solver/CalculiX/Win/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/Solver/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Postprocessor/CGX/Win/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/Postprocessor/CGX/)
 # if missing -> please download paraview binaries from https://www.paraview.org/download/
 #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../ParaView-6.0.0-MPI-Windows-Python3.12-msvc2017-AMD64/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/Postprocessor/ParaView/)
endif (WIN32)
if (UNIX)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Solver/CalculiX/Linux/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Solver/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Postprocessor/CGX/Linux/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Postprocessor/CGX/)
 # if missing -> please download paraview binaries from https://www.paraview.org/download/
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../ParaView-6.0.0-MPI-Linux-Python3.12-x86_64/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Postprocessor/ParaView/)
endif (UNIX)

set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()
if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()