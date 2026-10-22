target_sources(calculix_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxInitPythonInterfaceCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxInitPythonInterfaceCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxLogCoreCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxLogCoreCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxPrintCoreCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxPrintCoreCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResetCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResetCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxUseAutoCleanupCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxUseAutoCleanupCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxUseLogCoreCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxUseLogCoreCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxVersionCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxVersionCommand.hpp
)
