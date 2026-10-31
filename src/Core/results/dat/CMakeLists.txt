target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCoreDat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsDat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsDat.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsDatPersistence.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsDatPersistence.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsDatQuery.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsDatQuery.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXDatInterface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXDatInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CoreResultsDatStructs.hpp
)
