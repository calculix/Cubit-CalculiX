target_sources(calculix_core PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXCore.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loadCalculiXCore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loadCalculiXCore.hpp
)

add_subdirectory(application)
add_subdirectory(mesh)
add_subdirectory(materials)
add_subdirectory(sections)
add_subdirectory(interactions)
add_subdirectory(loads)
add_subdirectory(bcs)
add_subdirectory(amplitudes)
add_subdirectory(steps)
add_subdirectory(jobs)
add_subdirectory(results)
add_subdirectory(topology)
add_subdirectory(drawing)
add_subdirectory(export)
add_subdirectory(gui)
