target_sources(calculix_commands PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultExportJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultExportJobCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultConvertJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultConvertJobCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultLoadJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultLoadJobCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultProjectJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultProjectJobCommand.hpp
)

add_subdirectory(frd)
add_subdirectory(dat)
