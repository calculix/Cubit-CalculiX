target_sources(calculix_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryCreateGroupCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryCreateGroupCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryCreateMaterialCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryCreateMaterialCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryDeleteGroupCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryDeleteGroupCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryDeleteMaterialCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryDeleteMaterialCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryExportMaterialCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryExportMaterialCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryImportMaterialCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryImportMaterialCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryModifyGroupCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryModifyGroupCommand.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryModifyMaterialCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ccxMaterialLibraryModifyMaterialCommand.hpp
)
