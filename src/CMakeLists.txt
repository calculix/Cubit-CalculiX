cmake_minimum_required(VERSION 3.16)

project(cubit-sim)

# The framework owns the application version used by every workspace.
include("${CMAKE_CURRENT_SOURCE_DIR}/Framework/CubitSimVersion.cmake")

if (UNIX)
 set(CUBIT_ROOT "$ENV{HOME}/Downloads/Coreform-Cubit-2026.11" CACHE PATH "Cubit installation used by this plugin")
 list(PREPEND CMAKE_PREFIX_PATH "${CUBIT_ROOT}/cmake")
 include_directories("${CUBIT_ROOT}/include")
 include_directories("${CUBIT_ROOT}/include/python")
 include_directories("${CUBIT_ROOT}/bin")
 # python for our swig wrapping
 include_directories("~/Downloads/Python-3.12.10/Include")
 include_directories("~/Downloads/Python-3.12.10") 
 set(HDF5_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Utility/HDF5/Linux/1.14.6)
endif (UNIX)

if (WIN32)
 SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
 set(CUBIT_ROOT "C:/Program Files/Coreform Cubit 2026.11" CACHE PATH "Cubit installation used by this plugin")
 list(PREPEND CMAKE_PREFIX_PATH "${CUBIT_ROOT}/cmake")
 include_directories("${CUBIT_ROOT}/include")
 include_directories("${CUBIT_ROOT}/include/python")
 set(Qt6_DIR "C:/Qt/6.8.3/msvc2022_64/lib/cmake/Qt6") 
 set(Qt6Core_DIR "C:/Qt/6.8.3/msvc2022_64/lib/cmake/Qt6Core") 
 set(Qt6Charts_DIR "C:/Qt/6.8.3/msvc2022_64/lib/cmake/Qt6Charts")
 set(Qt6Widgets_DIR "C:/Qt/6.8.3/msvc2022_64/lib/cmake/Qt6Widgets")
 set(HDF5_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Utility/HDF5/Win/1.14.6)
#   for checking the cubit version
#   import h5py 
#   print("HDF5 runtime:", h5py.version.hdf5_version)
#   print("h5py:", h5py.__version__)

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
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CubitVtkSDK.cmake")
find_package(Cubit REQUIRED CONFIG PATHS "${CUBIT_ROOT}/cmake" NO_DEFAULT_PATH)
find_package(CubitGUI REQUIRED CONFIG PATHS "${CUBIT_ROOT}/cmake" NO_DEFAULT_PATH)
find_package(CubitPython REQUIRED CONFIG PATHS "${CUBIT_ROOT}/cmake" NO_DEFAULT_PATH)
get_filename_component(_ccx_selected_cubit_cmake "${CUBIT_ROOT}/cmake" REALPATH)
foreach(_ccx_cubit_package Cubit CubitGUI CubitPython)
 get_filename_component(_ccx_cubit_package_dir "${${_ccx_cubit_package}_DIR}" REALPATH)
 if(NOT _ccx_cubit_package_dir STREQUAL _ccx_selected_cubit_cmake)
  message(FATAL_ERROR "${_ccx_cubit_package}_DIR belongs to a different CUBIT_ROOT. Clear the stale package cache entry.")
 endif()
endforeach()
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Charts REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS OpenGLWidgets REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS C CXX)
ADD_DEFINITIONS(-DH5_BUILT_AS_DYNAMIC_LIB)

set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../boost_1_92_0")
set(Boost_NO_BOOST_CMAKE ON) # Discover the headers using CMake's FindBoost module.
set(CGAL_DISABLE_GMP ON) # Use Boost.Multiprecision instead of GMP/MPFR.
set(CGAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Utility/cgal-main")
find_package(CGAL REQUIRED CONFIG)

find_package(SWIG REQUIRED)
find_package(Threads REQUIRED)

set(CUBITSIM_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(CUBITSIM_DEPLOY_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

include_directories("${CUBITSIM_SOURCE_ROOT}")
add_subdirectory(Framework/Reporting)
add_subdirectory(Framework/Storage)
add_subdirectory(Framework/Results)
add_subdirectory(Framework/Geometry)
add_subdirectory(Framework/Drawing)
add_subdirectory(Framework/System)
add_subdirectory(CalculiX)
add_subdirectory(Postprocessor)
add_subdirectory(Report)
add_subdirectory(Framework)
