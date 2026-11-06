# CalculiX-specific build and resources.
if (UNIX)
set(CCXPYTHONINTERFACE_WRAP
     ${CMAKE_CURRENT_BINARY_DIR}/CalculiXPythonInterface_wrap.cxx )
endif(UNIX)
if (WIN32)
set(CCXPYTHONINTERFACE_WRAP
     ${CMAKE_BINARY_DIR}/Release/CalculiXPythonInterface_wrap.cxx )
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
         ${CMAKE_BINARY_DIR}/Release/calculix_pythoninterface.py 
    COMMAND  ${SWIG_EXECUTABLE} 
        -c++
        -python
         -I${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface 
        -outdir ${CMAKE_BINARY_DIR}/Release
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
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../../lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/GUI
                    ${CUBITSIM_SOURCE_ROOT}/Utility
                    ${CMAKE_CURRENT_BINARY_DIR})

# Core domain sources are registered after calculix_core is created.



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

# Re-stage the approved toolbar SVGs when their source assets change.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/CCXCGALWrap.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/CCXDrawingSize.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/CCXTopologyThreshold.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/CCXDisplay.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/CCXDisplayHidden.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/CCXPickSymbols.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/CCXPickSymbolsActive.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/CCXDrawAll.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/Icons/CCXDrawAllHidden.svg")

# copy pythoninterface and icons to build directory
if (WIN32)
 #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/calculix_pythoninterface.py DESTINATION ${CMAKE_BINARY_DIR}/Release/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Icons/ DESTINATION ${CMAKE_BINARY_DIR}/Release/Icons/)
endif (WIN32)
if (UNIX)
 #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/PythonInterface/calculix_pythoninterface.py DESTINATION ${CMAKE_BINARY_DIR}/)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Icons/ DESTINATION ${CMAKE_BINARY_DIR}/Icons/)
 #file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/calculix_gui.py DESTINATION ${CMAKE_BINARY_DIR}/)
endif (UNIX)

# copy missing libraries into build directory
# Qt6Charts is currently not shipped with Cubit 2025.8
if (WIN32)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../Library/Win/Qt6Charts.dll DESTINATION ${CMAKE_BINARY_DIR}/Release/)
endif (WIN32)
if (UNIX)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../Library/Linux/libQt6Charts.so.6.2.4 DESTINATION ${CMAKE_BINARY_DIR}/)
endif (UNIX)

#copy material default library into build directory
if (WIN32)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libmaterial.hdf5 DESTINATION ${CMAKE_BINARY_DIR}/Release/)
endif (WIN32)
if (UNIX)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/libmaterial.hdf5 DESTINATION ${CMAKE_BINARY_DIR}/)
endif (UNIX)

#copy setup scripts
if (WIN32)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../set_cubit_plugin_dir.bat DESTINATION ${CMAKE_BINARY_DIR}/Release/)
endif (WIN32)
if (UNIX)
 file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../set_cubit_plugin_dir.sh DESTINATION ${CMAKE_BINARY_DIR}/)
endif (UNIX)



set(CCXPLUGIN_SRCS
    Commands/CalculiXCommands.cpp
    Commands/CalculiXCommands.hpp)

set(CCXCORE_SRCS
    ${UTILITY_SRC})

set(CCXUSEROPTIONS_SRCS
    ${UO_SRC}) 

set(CCXPYTHONINTERFACE_SRCS
    ${PYTHONINTERFACE_SRC})

set(CCXCOMP_SRCS
    GUI/CalculiXGui.cpp
    GUI/CalculiXGui.hpp
    GUI/properties/CalculiXPropertiesDock.cpp
    GUI/properties/CalculiXPropertiesDock.hpp
    ${EO_COMP_SRC}
    GUI/MenuManager.cpp
    GUI/MenuManager.hpp
    GUI/ToolbarManager.cpp
    GUI/ToolbarManager.hpp
    GUI/cmdPanelManager.cpp
    GUI/cmdPanelManager.hpp
    GUI/MyCmdWidgetFactory.cpp
    GUI/MyCmdWidgetFactory.hpp
    GUI/CCXDockWindowModelTree.cpp
    GUI/CCXDockWindowModelTree.hpp
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

    add_library(calculix_commands SHARED ${CCXPLUGIN_SRCS})
    #set_target_properties(calculix_commands PROPERTIES SUFFIX ".ccm")

    add_library(calculix_core SHARED ${CCXCORE_SRCS})
    target_link_libraries(calculix_core cubiti cubit_util cubit_gui cubit_python)
    #target_link_libraries(calculix_core clarogui navigation pickwidget clarofw Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_core clarogui clarofw navigation Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_core cubitsim_storage)
    set_target_properties(calculix_core PROPERTIES AUTOMOC 1)

    add_library(calculix_gui SHARED ${CCXCOMP_SRCS})
    #set_target_properties(calculix_gui PROPERTIES SUFFIX ".ccl")

    #target_link_libraries(calculix_gui clarogui navigation pickwidget cubiti clarofw Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_gui clarogui navigation cubiti clarofw Qt6::Charts Qt6::Widgets component)
    set_target_properties(calculix_gui PROPERTIES AUTOMOC 1)

    add_library(calculix_pythoninterface SHARED ${CCXPYTHONINTERFACE_SRCS})
    set_target_properties(calculix_pythoninterface PROPERTIES SUFFIX ".pyd" PREFIX "_")
    target_link_libraries(calculix_pythoninterface cubiti cubit_util)

    target_link_libraries(calculix_core calculix_useroptions)
    target_link_libraries(calculix_commands calculix_core)
    target_link_libraries(calculix_pythoninterface calculix_core)
    target_link_libraries(calculix_gui calculix_core)
endif (WIN32)

if (UNIX)
    add_library(calculix_useroptions SHARED ${CCXUSEROPTIONS_SRCS})
    target_link_libraries(calculix_useroptions clarogui cubiti clarofw Qt6::Widgets)
    set_target_properties(calculix_useroptions PROPERTIES AUTOMOC 1)

    add_library(calculix_commands SHARED ${CCXPLUGIN_SRCS})
    #set_target_properties(calculix_commands PROPERTIES SUFFIX ".ccm")

    add_library(calculix_core SHARED ${CCXCORE_SRCS})
    set_target_properties(calculix_core PROPERTIES SUFFIX ".so")
    target_link_libraries(calculix_core cubiti cubit_util cubit_gui cubit_python)
    #target_link_libraries(calculix_core clarogui clarofw navigation pickwidget Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_core clarogui clarofw navigation Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_core cubitsim_storage)
    set_target_properties(calculix_core PROPERTIES AUTOMOC 1)

    add_library(calculix_gui SHARED ${CCXCOMP_SRCS})
    #set_target_properties(calculix_gui PROPERTIES SUFFIX ".ccl")

    #target_link_libraries(calculix_gui clarogui navigation pickwidget cubiti clarofw Qt6::Charts Qt6::Widgets)
    target_link_libraries(calculix_gui clarogui navigation cubiti clarofw Qt6::Charts Qt6::Widgets component)
    set_target_properties(calculix_gui PROPERTIES AUTOMOC 1)

    add_library(calculix_pythoninterface SHARED ${CCXPYTHONINTERFACE_SRCS})
    set_target_properties(calculix_pythoninterface PROPERTIES SUFFIX ".so" PREFIX "_")
    target_link_libraries(calculix_pythoninterface cubiti cubit_util)

    target_link_libraries(calculix_core calculix_useroptions)
    target_link_libraries(calculix_pythoninterface calculix_core)
    target_link_libraries(calculix_commands calculix_core)
    target_link_libraries(calculix_gui calculix_core)
endif (UNIX)

# The wrapping command directly consumes the shared geometry API.
target_link_libraries(calculix_commands cubitsim_geometry)
target_link_libraries(calculix_core cubitsim_drawing)
target_link_libraries(calculix_gui cubitsim_drawing)

# The material-library GUI also uses the shared storage tool directly.
target_link_libraries(calculix_gui cubitsim_storage)

# Register feature sources after both platform branches create the targets.
add_subdirectory(Core)
add_subdirectory(Commands)
add_subdirectory(GUI)


# Result readers use header-only Boost.Asio on both platforms.
find_package(Threads REQUIRED)
target_include_directories(calculix_core PRIVATE "${BOOST_ROOT}")
target_compile_features(calculix_core PRIVATE cxx_std_17)
target_link_libraries(calculix_core Threads::Threads)
target_link_libraries(calculix_core showviz_base showviz_cubit showviz_mesh showviz_geom)

# copy external software
if (WIN32)
 set(CCX_WINDOWS_SOLVER_DIRECTORY "${CUBITSIM_SOURCE_ROOT}/Solvers/CalculiX/Win")
 add_custom_target(calculix_solver_runtime ALL
   COMMAND "${CMAKE_COMMAND}" -E make_directory "${CUBITSIM_DEPLOY_DIRECTORY}/Solver"
   COMMAND "${CMAKE_COMMAND}" -E copy_if_different
     "${CCX_WINDOWS_SOLVER_DIRECTORY}/ccx_2.23.exe"
     "${CCX_WINDOWS_SOLVER_DIRECTORY}/libiomp5md.dll"
     "${CCX_WINDOWS_SOLVER_DIRECTORY}/README.md"
     "${CUBITSIM_DEPLOY_DIRECTORY}/Solver"
   COMMAND "${CMAKE_COMMAND}" -E copy_directory
     "${CCX_WINDOWS_SOLVER_DIRECTORY}/licenses"
     "${CUBITSIM_DEPLOY_DIRECTORY}/Solver/licenses"
   VERBATIM
 )
 #file(COPY ${CUBITSIM_SOURCE_ROOT}/Postprocessor/CGX/Win/ DESTINATION ${CMAKE_BINARY_DIR}/Release/Postprocessor/CGX/)
 # if missing -> please download paraview binaries from https://www.paraview.org/download/
 #file(COPY ${CUBITSIM_SOURCE_ROOT}/../../ParaView-6.0.0-MPI-Windows-Python3.12-msvc2017-AMD64/ DESTINATION ${CMAKE_BINARY_DIR}/Release/Postprocessor/ParaView/)
endif (WIN32)
if (UNIX)
 file(COPY ${CUBITSIM_SOURCE_ROOT}/Solvers/CalculiX/Linux/ DESTINATION ${CMAKE_BINARY_DIR}/Solver/)
 #file(COPY ${CUBITSIM_SOURCE_ROOT}/Postprocessor/CGX/Linux/ DESTINATION ${CMAKE_BINARY_DIR}/Postprocessor/CGX/)
 # if missing -> please download paraview binaries from https://www.paraview.org/download/
 #file(COPY ${CUBITSIM_SOURCE_ROOT}/../../ParaView-6.0.0-MPI-Linux-Python3.12-x86_64/ DESTINATION ${CMAKE_BINARY_DIR}/Postprocessor/ParaView/)
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

add_library(calculix_module SHARED CalculiXModule.cpp CalculiXModule.hpp)
if (WIN32)
  add_dependencies(calculix_module calculix_solver_runtime)
endif()
target_link_libraries(calculix_module PRIVATE
  calculix_commands calculix_gui calculix_core calculix_useroptions cubiti cubit_util
)

foreach (CCX_TARGET calculix_module calculix_commands calculix_gui calculix_core
    calculix_useroptions calculix_pythoninterface)
  target_compile_features(${CCX_TARGET} PRIVATE cxx_std_17)
  set_target_properties(${CCX_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
    LIBRARY_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${CUBITSIM_DEPLOY_DIRECTORY}"
  )
endforeach()


if (WIN32)
  set(CCXPYTHONINTERFACE_PROXY "${CMAKE_BINARY_DIR}/Release/calculix_pythoninterface.py")
else()
  set(CCXPYTHONINTERFACE_PROXY "${CMAKE_CURRENT_BINARY_DIR}/calculix_pythoninterface.py")
endif()

if (NOT WIN32)
add_custom_command(TARGET calculix_pythoninterface POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CCXPYTHONINTERFACE_PROXY}"
    "$<TARGET_FILE_DIR:calculix_pythoninterface>/calculix_pythoninterface.py"
  VERBATIM
)

endif()
