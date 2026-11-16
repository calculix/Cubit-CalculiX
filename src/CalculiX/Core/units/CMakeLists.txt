target_sources(calculix_core PRIVATE
  CalculiXDofUnits.cpp CalculiXDofUnits.hpp
  CalculiXDomainUnits.cpp CalculiXDomainUnits.hpp
  CalculiXUnitsInterface.cpp CalculiXUnitsInterface.hpp
  CalculiXPhysicalModelUnits.cpp CalculiXPhysicalModelUnits.hpp
)

option(CUBITSIM_DOMAIN_UNIT_TESTS "Build CalculiX domain unit and persistence regression tests" ON)
if (CUBITSIM_DOMAIN_UNIT_TESTS)
  enable_testing()
  add_subdirectory(tests "${CMAKE_CURRENT_BINARY_DIR}/domain-unit-tests")
endif()
