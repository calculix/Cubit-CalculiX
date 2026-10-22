target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreSteps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreSteps.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreInitialConditions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreInitialConditions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreFieldOutputs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreFieldOutputs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreHistoryOutputs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreHistoryOutputs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDamping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreDamping.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CorePhysicalConstants.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CorePhysicalConstants.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXStepsInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXStepsInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreStepsStructs.hpp
)
