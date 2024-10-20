cmake_minimum_required(VERSION 3.5)

project(Cubit-CalculiX)

if (UNIX)
 set(CMAKE_PREFIX_PATH "/opt/Coreform-Cubit-2024.8/cmake")
 # python for our swig wrapping
 include_directories("/opt/Coreform-Cubit-2024.8/bin/python3/include/python3.10") 
 set(HDF5_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Utility/HDF5/Linux/1.12.1) 
endif (UNIX)

if (WIN32)
 SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
 set(CMAKE_PREFIX_PATH "C:/Program Files/Coreform Cubit 2024.8/cmake")
 set(Qt5_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5") 
 set(Qt5Core_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5Core") 
 set(Qt5Charts_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5Charts")
 set(Qt5Widgets_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5Widgets")
 set(HDF5_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Utility/HDF5/Win/1.12.1) 
 # python for our swig wrapping
 include_directories("C:/Program Files/Python310/include")
 link_directories("C:/Program Files/Python310/libs")
endif (WIN32)

# On Linux, we need to add some addition compiler flags to get our static
# library to link with our swig module (which acts like a shared library)
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")

# Get the packages needed to use the SDK
find_package(Cubit REQUIRED CONFIG)
find_package(Qt5Core REQUIRED)
find_package(Qt5Charts REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS C CXX HL)
ADD_DEFINITIONS(-DH5_BUILT_AS_DYNAMIC_LIB)

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
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/calculix_pythoninterface.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Icons/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/Icons/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Solver/CalculiX/Win/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/Solver/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Postprocessor/CGX/Win/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/Postprocessor/CGX/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Postprocessor/ParaView/Win/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/Postprocessor/ParaView/)
endif (WIN32)
if (UNIX)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/calculix_pythoninterface.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Icons/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Icons/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Solver/CalculiX/Linux/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Solver/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Postprocessor/CGX/Linux/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Postprocessor/CGX/)
 # if missing -> please download paraview binaries from https://www.paraview.org/download/
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../../ParaView-5.13.1-MPI-Linux-Python3.10-x86_64/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Postprocessor/ParaView/)
endif (UNIX)

# copy missing libraries into build directory
# Qt5Charts is currently not shipped with Cubit 2024.8
if (WIN32)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Win/Qt5Charts.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Win/hdf5_cpp.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Win/hdf5.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
endif (WIN32)
if (UNIX)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Linux/libQt5Charts.so.5 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Linux/libhdf5_cpp.so DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Linux/libhdf5_cpp.so.200 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Library/Linux/libhdf5_cpp.so.200.1.0 DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
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
    ExportManager.cpp
    ExportManager.hpp
    loadCalculiXComp.cpp
    )

if (WIN32)
add_library(calculix_useroptions SHARED ${CCXUSEROPTIONS_SRCS})
target_link_libraries(calculix_useroptions clarogui cubiti clarofw Qt5::Widgets)
set_target_properties(calculix_useroptions PROPERTIES AUTOMOC 1)

add_library(calculix_plugin MODULE ${CCXPLUGIN_SRCS})
set_target_properties(calculix_plugin PROPERTIES SUFFIX ".ccm")

add_library(calculix_core SHARED ${CCXCORE_SRCS})
target_link_libraries(calculix_core cubiti cubit_util)
target_link_libraries(calculix_core clarogui navigation pickwidget clarofw Qt5::Charts Qt5::Widgets)
target_link_libraries(calculix_core HDF5::HDF5)
set_target_properties(calculix_core PROPERTIES AUTOMOC 1)

add_library(calculix_comp SHARED ${CCXCOMP_SRCS})
set_target_properties(calculix_comp PROPERTIES SUFFIX ".ccl")
target_link_libraries(calculix_comp clarogui navigation pickwidget cubiti clarofw Qt5::Charts Qt5::Widgets)
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
target_link_libraries(calculix_useroptions clarogui cubiti clarofw Qt5::Widgets)
set_target_properties(calculix_useroptions PROPERTIES AUTOMOC 1)

add_library(calculix_plugin MODULE ${CCXPLUGIN_SRCS})
set_target_properties(calculix_plugin PROPERTIES SUFFIX ".ccm")

add_library(calculix_core SHARED ${CCXCORE_SRCS})
set_target_properties(calculix_core PROPERTIES SUFFIX ".so")
target_link_libraries(calculix_core cubiti cubit_util)
target_link_libraries(calculix_core clarogui clarofw navigation pickwidget Qt5::Charts Qt5::Widgets)
target_link_libraries(calculix_core HDF5::HDF5)
set_target_properties(calculix_core PROPERTIES AUTOMOC 1)

add_library(calculix_comp SHARED ${CCXCOMP_SRCS})
set_target_properties(calculix_comp PROPERTIES SUFFIX ".ccl")
target_link_libraries(calculix_comp clarogui navigation pickwidget cubiti clarofw Qt5::Charts Qt5::Widgets)
set_target_properties(calculix_comp PROPERTIES AUTOMOC 1)

add_library(calculix_pythoninterface SHARED ${CCXPYTHONINTERFACE_SRCS})
set_target_properties(calculix_pythoninterface PROPERTIES SUFFIX ".so" PREFIX "_")
target_link_libraries(calculix_pythoninterface cubiti cubit_util)

target_link_libraries(calculix_core calculix_useroptions)
target_link_libraries(calculix_pythoninterface calculix_core)
target_link_libraries(calculix_plugin calculix_core)
target_link_libraries(calculix_comp calculix_core)
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
