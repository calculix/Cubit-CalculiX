target_sources(calculix_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxBCsDisplacementsModifyCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxBCsDisplacementsModifyCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxBCsTemperaturesModifyCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxBCsTemperaturesModifyCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxHBCAddBCCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxHBCAddBCCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxHBCRemoveBCCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxHBCRemoveBCCommand.hpp
)
