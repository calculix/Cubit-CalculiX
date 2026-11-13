target_sources(calculix_commands PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxCaptureCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxCaptureCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxDisplayCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxDisplayCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxAnimateTopologyCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxAnimateTopologyCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxDrawCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxDrawCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxDrawStepCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxDrawStepCommand.hpp
)
