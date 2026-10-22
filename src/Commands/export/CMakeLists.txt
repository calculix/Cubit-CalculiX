target_sources(calculix_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxCustomLineCreateCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxCustomLineCreateCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxCustomLineDeleteCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxCustomLineDeleteCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxCustomLineModifyCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxCustomLineModifyCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxExportCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxExportCommand.hpp
)
