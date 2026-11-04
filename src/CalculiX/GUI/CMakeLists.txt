add_subdirectory(application)
add_subdirectory(common)
add_subdirectory(mesh)
add_subdirectory(materials)
add_subdirectory(sections)
add_subdirectory(interactions)
add_subdirectory(loads)
add_subdirectory(bcs)
add_subdirectory(amplitudes)
add_subdirectory(steps)
add_subdirectory(jobs)
add_subdirectory(topology)
add_subdirectory(cgal)
add_subdirectory(export)

add_subdirectory(properties)

add_subdirectory(drawing)

target_sources(calculix_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/options/UserOptionsPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/options/UserOptionsPanel.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/options/DrawingOptionsWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/options/DrawingOptionsWidget.hpp
)
