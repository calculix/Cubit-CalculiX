cmake_minimum_required(VERSION 3.5)

project(Cubit-CalculiX)

if (UNIX)
 set(CMAKE_PREFIX_PATH "/opt/Coreform-Cubit-2024.3/cmake")
 # python for our swig wrapping
 include_directories("/opt/Coreform-Cubit-2024.3/bin/python3/include/python3.10")
endif (UNIX)

if (WIN32)
 set(CMAKE_PREFIX_PATH "C:/Program Files/Coreform Cubit 2024.3/cmake")
 set(Qt5_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5") 
 set(Qt5Core_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5Core") 
 set(Qt5Widgets_DIR "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5Widgets")
 # python for our swig wrapping
 include_directories("C:/Program Files/Coreform Cubit 2024.3/bin/python3/include/python3.10")
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
find_package(Qt5Widgets REQUIRED)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Utiliy)
add_subdirectory(Utility)

# Include our Python Interface code
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface)
add_subdirectory(PythonInterface)

# copy our icons to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Icons/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Icons/)

# copy pythoninterface to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/calculix_pythoninterface.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

#qt5_wrap_ui(ui_src ${GUI_UI})

set(CCXPLUGIN_SRCS
    CalculiXPlugin.cpp
    CalculiXPlugin.hpp
    ${CMD_SRC}
    ${CORE_SRC}
    ${UTILITY_SRC})

set(CCXPYTHONINTERFACE_SRCS
    ${PYTHONINTERFACE_SRC})

set(CCXCOMP_SRCS
    CalculiXComp.cpp
    CalculiXComp.hpp
    ${EO_COMP_SRC}
    ${GUI_SRC}
    #${ui_src}
    ${UO_SRC}
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
add_library(calculix_plugin SHARED ${CCXPLUGIN_SRCS})
set_target_properties(calculix_plugin PROPERTIES SUFFIX ".ccm")
target_link_libraries(calculix_plugin cubiti cubit_util)
target_link_libraries(calculix_plugin clarofw Qt5::Widgets)
set_target_properties(calculix_plugin PROPERTIES AUTOMOC 1)

#set_target_properties(calculix_plugin PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

add_library(calculix_comp SHARED ${CCXCOMP_SRCS})
set_target_properties(calculix_comp PROPERTIES SUFFIX ".ccl")
target_link_libraries(calculix_comp clarogui navigation pickwidget cubiti clarofw)
set_target_properties(calculix_comp PROPERTIES AUTOMOC 1)
target_link_libraries(calculix_comp Qt5::Widgets)

add_library(calculix_pythoninterface SHARED ${CCXPYTHONINTERFACE_SRCS})
set_target_properties(calculix_pythoninterface PROPERTIES SUFFIX ".dll" PREFIX "_")
target_link_libraries(calculix_pythoninterface calculix_plugin cubiti cubit_util)
#target_link_libraries(calculix_plugin calculix_comp)
target_link_libraries(calculix_comp calculix_plugin)
endif (WIN32)

if (UNIX)
add_library(calculix_plugin SHARED ${CCXPLUGIN_SRCS})
set_target_properties(calculix_plugin PROPERTIES SUFFIX ".ccm")
target_link_libraries(calculix_plugin cubiti cubit_util)
#target_link_libraries(calculix_plugin clarofw Qt5::Widgets)
#set_target_properties(calculix_plugin PROPERTIES AUTOMOC 1)
#set_target_properties(calculix_plugin PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

add_library(calculix_comp SHARED ${CCXCOMP_SRCS})
set_target_properties(calculix_comp PROPERTIES SUFFIX ".ccl")
target_link_libraries(calculix_comp clarogui navigation pickwidget cubiti clarofw)
set_target_properties(calculix_comp PROPERTIES AUTOMOC 1)

add_library(calculix_pythoninterface SHARED ${CCXPYTHONINTERFACE_SRCS})
set_target_properties(calculix_pythoninterface PROPERTIES SUFFIX ".so" PREFIX "_")
target_link_libraries(calculix_pythoninterface calculix_plugin cubiti cubit_util)

#target_link_libraries(calculix_comp calculix_plugin)
target_link_libraries(calculix_plugin calculix_comp)
target_link_libraries(calculix_comp Qt5::Widgets)
endif (UNIX)


if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()
if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()
