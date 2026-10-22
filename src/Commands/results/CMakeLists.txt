target_sources(calculix_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultCGXJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultCGXJobCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultConvertJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultConvertJobCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultLoadJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultLoadJobCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultParaViewJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultParaViewJobCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultProjectJobCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxResultProjectJobCommand.hpp
)

add_subdirectory(frd)
add_subdirectory(dat)
