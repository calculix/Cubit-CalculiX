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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR})

# Include hdf5 files for the .trelis file example. It is recommended that you
# use your own hdf5 libraries to do this
option(SDK_BUILD_HDF5 "Use hdf5 libraries when building the SDK examples." OFF)
if(SDK_BUILD_HDF5)
  include_directories(${HDF5_SOURCE_DIR}/src
                      ${HDF5_SOURCE_DIR}/hl/src
                      ${HDF5_SOURCE_DIR}/hl/c++/src
                      ${HDF5_INCLUDE_DIR}
                      ${HDF5_BINARY_DIR})
  add_definitions(-DSDK_BUILD_HDF5)
endif()


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

qt5_wrap_ui(ui_src ${GUI_UI})

set(CCXCOMP_SRCS
    CalculiXComp.cpp
    CalculiXComp.hpp
    ${EO_COMP_SRC}
    ${GUI_SRC}
    ${ui_src}
    MenuManager.cpp
    MenuManager.hpp
    ToolbarManager.cpp
    ToolbarManager.hpp
    PanelManager.cpp
    PanelManager.hpp
    MyWidgetFactory.cpp
    MyWidgetFactory.hpp
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

add_library(calculix_comp SHARED ${CCXCOMP_SRCS})
set_target_properties(calculix_comp PROPERTIES SUFFIX ".ccl")
target_link_libraries(calculix_comp clarogui navigation pickwidget cubiti clarofw)
set_target_properties(calculix_comp PROPERTIES AUTOMOC 1)

if(SDK_BUILD_HDF5)
  if( USE_MPI_HDF5 )
    if( NOT HDF5_C_LIBRARY OR NOT HDF5_LIBRARIES )
      message( FATAL_ERROR "HDF5_C_LIBRARY not defined." )
    endif()
    ADD_DEFINITIONS(-DH5_BUILT_AS_DYNAMIC_LIB)
    target_link_libraries(calculix_comp ${HDF5_LIBRARIES} )
  else()
    target_link_libraries(calculix_comp hdf5_hl-static hdf5-static)
  endif()
endif()

target_link_libraries(calculix_comp Qt5::Widgets)

target_link_libraries(calculix_comp calculix_plugin)

if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()
if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()



