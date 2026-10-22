target_sources(calculix_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxJobCreateCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxJobCreateCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxJobDeleteCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxJobDeleteCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxJobModifyCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxJobModifyCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxKillJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxKillJobCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxRunJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxRunJobCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxWaitJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxWaitJobCommand.hpp
)
