target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCoreFrd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsFrd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsFrd.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsFrdPersistence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsFrdPersistence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsFrdQuery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsFrdQuery.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXFrdInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXFrdInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsFrdStructs.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsFrdFormat.hpp
)
