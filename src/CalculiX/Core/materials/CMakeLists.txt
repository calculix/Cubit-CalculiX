target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreMaterialsTopologyGrouping.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreMaterialsTopologyGrouping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MaterialProperties.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCoreMaterials.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreMaterials.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreMaterials.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreMaterialsTreeData.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreMaterialsTreeData.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCoreMaterialsLibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreMaterialsLibrary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreMaterialsLibrary.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXMaterialsInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXMaterialsInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreMaterialsStructs.hpp
)
