target_sources(calculix_core PRIVATE
  CalculiXDomainUnits.cpp CalculiXDomainUnits.hpp
  CalculiXUnitsInterface.cpp CalculiXUnitsInterface.hpp
  CalculiXPhysicalModelUnits.cpp CalculiXPhysicalModelUnits.hpp
)

option(CUBITSIM_DOMAIN_UNIT_TESTS "Build CalculiX domain unit and persistence regression tests" OFF)
if (CUBITSIM_DOMAIN_UNIT_TESTS)
  enable_testing()
  add_subdirectory(tests "${CMAKE_CURRENT_BINARY_DIR}/domain-unit-tests")
endif()
