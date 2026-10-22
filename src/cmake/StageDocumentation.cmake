# Standalone packaging script: no plugin build or documentation toolchain required.
cmake_minimum_required(VERSION 3.5)

foreach(_variable CCX_DOCUMENTATION_SOURCE CCX_DOCUMENTATION_DESTINATION)
  if (NOT DEFINED ${_variable} OR "${${_variable}}" STREQUAL "")
    message(FATAL_ERROR "${_variable} must name an absolute directory")
  endif()
  if (NOT IS_ABSOLUTE "${${_variable}}")
    message(FATAL_ERROR "${_variable} must be absolute: ${${_variable}}")
  endif()
endforeach()

get_filename_component(_source "${CCX_DOCUMENTATION_SOURCE}" REALPATH)
get_filename_component(_destination "${CCX_DOCUMENTATION_DESTINATION}" REALPATH)
if (NOT EXISTS "${_source}/index.html" OR IS_DIRECTORY "${_source}/index.html")
  message(FATAL_ERROR "Generated manual missing: ${_source}/index.html. Build docs/build.py first.")
endif()

string(FIND "${_destination}/" "${_source}/" _inside_source)
if (_inside_source EQUAL 0)
  message(FATAL_ERROR "Documentation destination must be outside the generated source directory")
endif()
if (EXISTS "${_destination}" AND NOT IS_DIRECTORY "${_destination}")
  message(FATAL_ERROR "Documentation destination is not a directory: ${_destination}")
endif()

file(MAKE_DIRECTORY "${_destination}")
file(COPY "${_source}/" DESTINATION "${_destination}")
message(STATUS "Staged Cubit-CalculiX documentation: ${_destination}/index.html")
