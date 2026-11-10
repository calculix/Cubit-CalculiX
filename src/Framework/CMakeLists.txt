# One external command plugin/component hosts the internal application modules.
add_library(cubitsim_framework SHARED
  Core/CubitSimVersion.hpp
  Core/ProjectStateCoordinator.cpp
  Core/ProjectStateCoordinator.hpp
  Core/FrameworkContext.cpp
  Core/FrameworkContext.hpp
  Core/Monitoring/FrameworkUserOptions.cpp
  Core/Monitoring/FrameworkUserOptions.hpp
  Core/Monitoring/ResourceMonitor.cpp
  Core/Monitoring/ResourceMonitor.hpp
  Core/Monitoring/ResourceAttribution.cpp
  Core/Monitoring/ResourceAttribution.hpp
  Interfaces/JobMonitorProviderInterface.hpp
  GUI/Status/ResourceStatusWidget.cpp
  GUI/Status/ResourceStatusWidget.hpp
  GUI/Status/ResourceUsageButton.cpp
  GUI/Status/ResourceUsageButton.hpp
  GUI/Status/FrameworkOptionsPanel.cpp
  GUI/Status/FrameworkOptionsPanel.hpp
  Core/FrameworkModules.cpp
  Core/FrameworkModules.hpp
  Core/ModuleManager.cpp
  Core/ModuleManager.hpp
  FrameworkExport.hpp
  Utility/StartupTrace.hpp
  Interfaces/DrawingInterface.hpp
  Interfaces/ModuleInterface.hpp
  Interfaces/CommandProviderInterface.hpp
  Interfaces/ComponentProviderInterface.hpp
  Interfaces/WorkspaceProviderInterface.hpp
  Interfaces/ResultSourceRegistryInterface.hpp
  GUI/DocumentationManager.cpp
  GUI/DocumentationManager.hpp
  GUI/UserOptionsManager.cpp
  GUI/UserOptionsManager.hpp
  GUI/WorkspaceManager.cpp
  GUI/WorkspaceManager.hpp
  GUI/WorkspaceHeader.cpp
  GUI/WorkspaceHeader.hpp
  GUI/CubitWorkspaceHost.cpp
  GUI/CubitWorkspaceHost.hpp
  GUI/WorkspaceGraphicsStack.cpp
  GUI/WorkspaceGraphicsStack.hpp
  GUI/CommandDispatcher.cpp
  GUI/CommandDispatcher.hpp
)

target_compile_features(cubitsim_framework PUBLIC cxx_std_17)
target_link_libraries(cubitsim_framework PUBLIC Qt6::Core)
target_link_libraries(cubitsim_framework PRIVATE cubitsim_system Threads::Threads)
target_link_libraries(cubitsim_framework PRIVATE cubitsim_storage cubitsim_drawing calculix_module postprocessor_module report_module clarogui clarofw navigation cubiti cubit_util Qt6::Widgets)
set_target_properties(cubitsim_framework PROPERTIES AUTOMOC ON)
target_compile_definitions(cubitsim_framework PRIVATE CUBITSIM_FRAMEWORK_BUILD)
target_include_directories(cubitsim_framework PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_library(cubitsim_plugin MODULE
  CubitSimPlugin.cpp CubitSimPlugin.hpp
  Commands/WorkspaceCommand.cpp Commands/WorkspaceCommand.hpp
  Commands/UserOptionsCommand.cpp Commands/UserOptionsCommand.hpp
  Commands/DocumentationCommand.cpp Commands/DocumentationCommand.hpp
  Commands/CubitSimVersionCommand.cpp Commands/CubitSimVersionCommand.hpp
)
target_link_libraries(cubitsim_plugin PRIVATE cubitsim_framework cubiti cubit_util)

set(CUBITSIM_COMPONENT_WRAPPER "${CMAKE_CURRENT_BINARY_DIR}/cubitsim_comp_wrap.cxx")
set(CUBITSIM_COMPONENT_PROXY "${CMAKE_CURRENT_BINARY_DIR}/cubitsim_comp.py")

add_custom_command(
  OUTPUT "${CUBITSIM_COMPONENT_WRAPPER}" "${CUBITSIM_COMPONENT_PROXY}"
  # Startup can re-enter Cubit's interpreter; release the calling Python GIL.
  COMMAND "${SWIG_EXECUTABLE}" -c++ -python -threads
    "-I${CMAKE_CURRENT_SOURCE_DIR}"
    -outdir "${CMAKE_CURRENT_BINARY_DIR}"
    -o "${CUBITSIM_COMPONENT_WRAPPER}"
    "${CMAKE_CURRENT_SOURCE_DIR}/cubitsim_comp.i"
  DEPENDS cubitsim_comp.i CubitSimComp.hpp
  VERBATIM
)

add_library(cubitsim_comp SHARED CubitSimComp.cpp CubitSimComp.hpp "${CUBITSIM_COMPONENT_WRAPPER}")
target_include_directories(cubitsim_comp PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(cubitsim_comp PRIVATE
  cubitsim_framework component clarogui clarofw cubiti cubit_util cubit_python Qt6::Widgets
)
set_target_properties(cubitsim_comp PROPERTIES PREFIX "_")

if (WIN32)
  set_target_properties(cubitsim_comp PROPERTIES SUFFIX ".pyd")
endif()

# Share the deployment directory with module libraries and their resources.
set(CUBITSIM_OUTPUT "${CUBITSIM_DEPLOY_DIRECTORY}")

foreach (CUBITSIM_TARGET cubitsim_framework cubitsim_plugin cubitsim_comp)
  set_target_properties(${CUBITSIM_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CUBITSIM_OUTPUT}"
    LIBRARY_OUTPUT_DIRECTORY "${CUBITSIM_OUTPUT}"
    ARCHIVE_OUTPUT_DIRECTORY "${CUBITSIM_OUTPUT}"
    BUILD_RPATH_USE_ORIGIN TRUE
  )
endforeach()

if (UNIX)
  set_target_properties(cubitsim_plugin cubitsim_comp PROPERTIES BUILD_RPATH "$ORIGIN")
endif()

add_custom_command(TARGET cubitsim_comp POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${CUBITSIM_COMPONENT_PROXY}" "$<TARGET_FILE_DIR:cubitsim_comp>/cubitsim_comp.py"
  VERBATIM
)

# Package an existing manual explicitly; normal builds do not generate or stage it.
set(CUBITSIM_DOCUMENTATION_SOURCE "${CUBITSIM_SOURCE_ROOT}/../docs/_build/html" CACHE PATH "Generated manual directory")
if (WIN32)
  set(_cubitsim_documentation_destination "${CMAKE_BINARY_DIR}/Release/documentation")
else()
  set(_cubitsim_documentation_destination "${CMAKE_BINARY_DIR}/documentation")
endif()
set(CUBITSIM_DOCUMENTATION_DESTINATION "${_cubitsim_documentation_destination}" CACHE PATH "Packaged manual directory beside the plugin")
add_custom_target(stage_documentation
  COMMAND "${CMAKE_COMMAND}"
    "-DCUBITSIM_DOCUMENTATION_SOURCE=${CUBITSIM_DOCUMENTATION_SOURCE}"
    "-DCUBITSIM_DOCUMENTATION_DESTINATION=${CUBITSIM_DOCUMENTATION_DESTINATION}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/StageDocumentation.cmake"
  COMMENT "Staging the existing Cubit cubit-sim manual"
  VERBATIM)
