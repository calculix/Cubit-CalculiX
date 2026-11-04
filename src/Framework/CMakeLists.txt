# One external command plugin and component host the internal CalculiX module.
add_library(fea_framework SHARED
  Core/FrameworkContext.cpp
  Core/FrameworkContext.hpp
  Core/FrameworkModules.cpp
  Core/FrameworkModules.hpp
  Core/ModuleManager.cpp
  Core/ModuleManager.hpp
  FrameworkExport.hpp
  Utility/StartupTrace.hpp
  Interfaces/ModuleInterface.hpp
  Interfaces/CommandProviderInterface.hpp
  Interfaces/ComponentProviderInterface.hpp
  Interfaces/WorkspaceProviderInterface.hpp
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
  GUI/CommandDispatcher.cpp
  GUI/CommandDispatcher.hpp
)

target_compile_features(fea_framework PUBLIC cxx_std_17)
target_link_libraries(fea_framework PRIVATE fea_storage calculix_module clarogui clarofw navigation cubiti cubit_util Qt6::Widgets)
set_target_properties(fea_framework PROPERTIES AUTOMOC ON)
target_compile_definitions(fea_framework PRIVATE FEA_FRAMEWORK_BUILD)
target_include_directories(fea_framework PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_library(fea_plugin MODULE
  FEAPlugin.cpp FEAPlugin.hpp
  Commands/WorkspaceCommand.cpp Commands/WorkspaceCommand.hpp
  Commands/DocumentationCommand.cpp Commands/DocumentationCommand.hpp
)
target_link_libraries(fea_plugin PRIVATE fea_framework cubiti cubit_util)

set(FEA_COMPONENT_WRAPPER "${CMAKE_CURRENT_BINARY_DIR}/fea_comp_wrap.cxx")
set(FEA_COMPONENT_PROXY "${CMAKE_CURRENT_BINARY_DIR}/fea_comp.py")

add_custom_command(
  OUTPUT "${FEA_COMPONENT_WRAPPER}" "${FEA_COMPONENT_PROXY}"
  # Startup can re-enter Cubit's interpreter; release the calling Python GIL.
  COMMAND "${SWIG_EXECUTABLE}" -c++ -python -threads
    "-I${CMAKE_CURRENT_SOURCE_DIR}"
    -outdir "${CMAKE_CURRENT_BINARY_DIR}"
    -o "${FEA_COMPONENT_WRAPPER}"
    "${CMAKE_CURRENT_SOURCE_DIR}/fea_comp.i"
  DEPENDS fea_comp.i FEAComp.hpp
  VERBATIM
)

add_library(fea_comp SHARED FEAComp.cpp FEAComp.hpp "${FEA_COMPONENT_WRAPPER}")
target_include_directories(fea_comp PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(fea_comp PRIVATE
  fea_framework component clarogui clarofw cubiti cubit_util cubit_python Qt6::Widgets
)
set_target_properties(fea_comp PROPERTIES PREFIX "_")

if (WIN32)
  set_target_properties(fea_comp PROPERTIES SUFFIX ".pyd")
endif()

# Share the deployment directory with module libraries and their resources.
set(FEA_OUTPUT "${CCX_DEPLOY_DIRECTORY}")

foreach (FEA_TARGET fea_framework fea_plugin fea_comp)
  set_target_properties(${FEA_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${FEA_OUTPUT}"
    LIBRARY_OUTPUT_DIRECTORY "${FEA_OUTPUT}"
    ARCHIVE_OUTPUT_DIRECTORY "${FEA_OUTPUT}"
    BUILD_RPATH_USE_ORIGIN TRUE
  )
endforeach()

if (UNIX)
  set_target_properties(fea_plugin fea_comp PROPERTIES BUILD_RPATH "$ORIGIN")
endif()

add_custom_command(TARGET fea_comp POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${FEA_COMPONENT_PROXY}" "$<TARGET_FILE_DIR:fea_comp>/fea_comp.py"
  VERBATIM
)

# Package an existing manual explicitly; normal builds do not generate or stage it.
set(FEA_DOCUMENTATION_SOURCE "${CCX_SOURCE_ROOT}/../docs/_build/html" CACHE PATH "Generated manual directory")
if (WIN32)
  set(_fea_documentation_destination "${CMAKE_BINARY_DIR}/Release/documentation")
else()
  set(_fea_documentation_destination "${CMAKE_BINARY_DIR}/documentation")
endif()
set(FEA_DOCUMENTATION_DESTINATION "${_fea_documentation_destination}" CACHE PATH "Packaged manual directory beside the plugin")
add_custom_target(stage_documentation
  COMMAND "${CMAKE_COMMAND}"
    "-DFEA_DOCUMENTATION_SOURCE=${FEA_DOCUMENTATION_SOURCE}"
    "-DFEA_DOCUMENTATION_DESTINATION=${FEA_DOCUMENTATION_DESTINATION}"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/StageDocumentation.cmake"
  COMMENT "Staging the existing Cubit FEA manual"
  VERBATIM)
