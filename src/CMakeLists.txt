project(Cubit-CalculiX)

set(CMAKE_PREFIX_PATH
"/opt/Coreform-Cubit-2022.4/bin"
"C:/Program Files/Coreform Cubit 2022.4/bin")

cmake_minimum_required(VERSION 3.1)

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

qt5_wrap_ui(ui_src ${GUI_UI})

set(CCXCOMP_SRCS
    CalculiXComp.cpp
    CalculiXComp.hpp
    ${EO_COMP_SRC}
    ${UO_COMP_SRC}
    ${GUI_SRC}
    ${ui_src}
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
    loadCalculiXComp.cpp)

set(CCXPLUGIN_SRCS
    CalculiXPlugin.cpp
    CalculiXPlugin.hpp
    ${CMD_SRC}
    ${CORE_SRC})

add_library(calculix_plugin SHARED ${CCXPLUGIN_SRCS})
target_link_libraries(calculix_plugin cubiti cubit_util)
#set_target_properties(calculix_plugin PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})

add_library(calculix_comp SHARED ${CCXCOMP_SRCS})
set_target_properties(calculix_comp PROPERTIES SUFFIX ".ccl")
target_link_libraries(calculix_comp clarogui navigation pickwidget cubiti clarofw)
set_target_properties(calculix_comp PROPERTIES AUTOMOC 1)

target_link_libraries(calculix_comp Qt5::Widgets)

#target_link_libraries(calculix_comp calculix_plugin)
target_link_libraries(calculix_plugin calculix_comp)

if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()
if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()