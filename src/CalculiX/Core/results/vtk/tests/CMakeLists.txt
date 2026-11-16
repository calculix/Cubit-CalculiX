add_executable(calculix_inp_surface_helper_tests InpSurfaceHelperTests.cpp)
target_compile_features(calculix_inp_surface_helper_tests PRIVATE cxx_std_17)
target_include_directories(calculix_inp_surface_helper_tests PRIVATE
  "${CUBITSIM_SOURCE_ROOT}" "${CUBITSIM_SOURCE_ROOT}/CalculiX" "${CUBITSIM_SOURCE_ROOT}/CalculiX/Utility")
target_link_libraries(calculix_inp_surface_helper_tests PRIVATE calculix_core cubitsim_results Qt6::Core)
add_test(NAME calculix_inp_surface_helpers COMMAND calculix_inp_surface_helper_tests)

if(WIN32)
  set(_inp_helper_test_path "$<TARGET_FILE_DIR:Qt6::Core>;${CUBITSIM_DEPLOY_DIRECTORY};${CUBIT_ROOT}/bin;${CUBIT_ROOT}/acis/code/bin;$ENV{PATH}")
  string(REPLACE ";" "\\;" _inp_helper_test_path "${_inp_helper_test_path}")
  set_tests_properties(calculix_inp_surface_helpers PROPERTIES ENVIRONMENT "PATH=${_inp_helper_test_path}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set_tests_properties(calculix_inp_surface_helpers PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:Qt6::Core>:${CUBITSIM_DEPLOY_DIRECTORY}:${CUBIT_ROOT}/bin:${CUBIT_ROOT}/acis/code/bin:$ENV{LD_LIBRARY_PATH}")
endif()
