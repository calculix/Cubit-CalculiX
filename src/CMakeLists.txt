cmake_minimum_required(VERSION 3.16)

project(Cubit-CalculiX)

### version
string(TIMESTAMP CCX_YEAR "%Y")
string(TIMESTAMP CCX_MONTH "%m")
string(REGEX REPLACE "^0" "" CCX_MONTH "${CCX_MONTH}") # 02 -> 2
add_compile_definitions(CCX_VERSION="${CCX_YEAR}.${CCX_MONTH}")

if (UNIX)
 set(CMAKE_PREFIX_PATH "~/Downloads/Coreform-Cubit-2026.10/cmake")
 include_directories("~/Downloads/Coreform-Cubit-2026.10/include")
 include_directories("~/Downloads/Coreform-Cubit-2026.10/include/python")
 include_directories("~/Downloads/Coreform-Cubit-2026.10/bin")
 # python for our swig wrapping
 include_directories("~/Downloads/Python-3.12.10/Include")
 include_directories("~/Downloads/Python-3.12.10") 
 set(HDF5_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Utility/HDF5/Linux/1.14.6)
endif (UNIX)

if (WIN32)
 SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
 set(CMAKE_PREFIX_PATH "C:/Program Files/Coreform Cubit 2026.10/cmake")
 include_directories("C:/Program Files/Coreform Cubit 2026.10/include")
 include_directories("C:/Program Files/Coreform Cubit 2026.10/include/python")
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
find_package(Cubit REQUIRED CONFIG)
find_package(CubitGUI REQUIRED CONFIG)
find_package(CubitPython REQUIRED CONFIG)
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Charts REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS C CXX HL)
ADD_DEFINITIONS(-DH5_BUILT_AS_DYNAMIC_LIB)

set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../boost_1_92_0")
set(Boost_NO_BOOST_CMAKE ON) # Discover the headers using CMake's FindBoost module.
set(CGAL_DISABLE_GMP ON) # Use Boost.Multiprecision instead of GMP/MPFR.
set(CGAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Utility/cgal-main")
find_package(CGAL REQUIRED CONFIG)

find_package(SWIG REQUIRED)
find_package(Threads REQUIRED)

set(CCX_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(CCX_DEPLOY_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

include_directories("${CCX_SOURCE_ROOT}")
add_subdirectory(CalculiX)
add_subdirectory(Framework)
