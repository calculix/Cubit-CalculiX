target_sources(calculix_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXUnitPresentation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CalculiXUnitPresentation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/NativeNavigation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/NativeNavigation.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Highlight.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Highlight.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PanelTable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PanelTable.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TreeContextMenu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TreeContextMenu.hpp
)

if(CUBITSIM_DOMAIN_UNIT_TESTS)
  add_executable(calculix_unit_presentation_tests
    tests/UnitPresentationTests.cpp
    CalculiXUnitPresentation.cpp
    CalculiXUnitPresentation.hpp
  )
  set_target_properties(calculix_unit_presentation_tests PROPERTIES AUTOMOC ON)
  target_compile_features(calculix_unit_presentation_tests PRIVATE cxx_std_17)
  target_include_directories(calculix_unit_presentation_tests PRIVATE "${CUBITSIM_SOURCE_ROOT}" "${CUBITSIM_SOURCE_ROOT}/CalculiX")
  target_link_libraries(calculix_unit_presentation_tests PRIVATE calculix_core cubitsim_model Qt6::Widgets)
  add_test(NAME calculix_unit_presentation COMMAND calculix_unit_presentation_tests)
  set(_unit_presentation_environment "QT_QPA_PLATFORM=offscreen")
  if(TARGET Qt6::QOffscreenIntegrationPlugin)
    list(APPEND _unit_presentation_environment
      "QT_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>/.."
      "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>"
    )
  else()
    message(FATAL_ERROR "The presentation unit test requires Qt's offscreen platform plugin.")
  endif()
  if(WIN32)
    set(_unit_presentation_path "$<TARGET_FILE_DIR:Qt6::Core>;${CUBITSIM_DEPLOY_DIRECTORY};${CUBIT_ROOT}/bin;${CUBIT_ROOT}/acis/code/bin;$ENV{PATH}")
    string(REPLACE ";" "\\;" _unit_presentation_path "${_unit_presentation_path}")
    list(APPEND _unit_presentation_environment "PATH=${_unit_presentation_path}")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND _unit_presentation_environment "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:Qt6::Core>:${CUBITSIM_DEPLOY_DIRECTORY}:${CUBIT_ROOT}/bin:${CUBIT_ROOT}/acis/code/bin:$ENV{LD_LIBRARY_PATH}")
  endif()
  set_tests_properties(calculix_unit_presentation PROPERTIES ENVIRONMENT "${_unit_presentation_environment}")
endif()
