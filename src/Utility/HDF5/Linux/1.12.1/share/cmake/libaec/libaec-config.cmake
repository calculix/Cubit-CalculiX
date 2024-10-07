#-----------------------------------------------------------------------------
# LIBAEC Config file for compiling against LIBAEC build directory
#-----------------------------------------------------------------------------

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was libaec-config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

string(TOUPPER libaec LIBAEC_PACKAGE_NAME)

set (${LIBAEC_PACKAGE_NAME}_VALID_COMPONENTS static shared)

#-----------------------------------------------------------------------------
# User Options
#-----------------------------------------------------------------------------
set (${LIBAEC_PACKAGE_NAME}_ENABLE_ENCODING      )
set (${LIBAEC_PACKAGE_NAME}_BUILD_SHARED_LIBS    OFF)
set (${LIBAEC_PACKAGE_NAME}_EXPORT_LIBRARIES     aec-static;sz-static)

#-----------------------------------------------------------------------------
# Directories
#-----------------------------------------------------------------------------
set (${LIBAEC_PACKAGE_NAME}_INCLUDE_DIR "${PACKAGE_PREFIX_DIR}/include")

set (${LIBAEC_PACKAGE_NAME}_SHARE_DIR "${PACKAGE_PREFIX_DIR}/share/cmake")
set_and_check (${LIBAEC_PACKAGE_NAME}_BUILD_DIR "${PACKAGE_PREFIX_DIR}")

#-----------------------------------------------------------------------------
# Version Strings
#-----------------------------------------------------------------------------
set (${LIBAEC_PACKAGE_NAME}_VERSION_STRING 1.0)
set (${LIBAEC_PACKAGE_NAME}_VERSION_MAJOR  1.0)
set (${LIBAEC_PACKAGE_NAME}_VERSION_MINOR  4)

#-----------------------------------------------------------------------------
# Don't include targets if this file is being picked up by another
# project which has already build LIBAEC as a subproject
#-----------------------------------------------------------------------------
if (NOT TARGET "libaec")
  include (${PACKAGE_PREFIX_DIR}/share/cmake/libaec/libaec-targets.cmake)
endif ()

# Handle default component(static) :
if (NOT ${LIBAEC_PACKAGE_NAME}_FIND_COMPONENTS)
    set (${LIBAEC_PACKAGE_NAME}_FIND_COMPONENTS static)
    set (${LIBAEC_PACKAGE_NAME}_FIND_REQUIRED_static true)
endif ()

# Handle requested components:
list (REMOVE_DUPLICATES ${LIBAEC_PACKAGE_NAME}_FIND_COMPONENTS)
foreach (comp IN LISTS ${LIBAEC_PACKAGE_NAME}_FIND_COMPONENTS)
    list (FIND ${LIBAEC_PACKAGE_NAME}_EXPORT_LIBRARIES "aec-${comp}" HAVE_COMP)
    if (${HAVE_COMP} LESS 0)
      set (${LIBAEC_PACKAGE_NAME}_${comp}_FOUND 0)
    else ()
      set (${LIBAEC_PACKAGE_NAME}_${comp}_FOUND 1)
      string(TOUPPER ${LIBAEC_PACKAGE_NAME}_${comp}_LIBRARY COMP_LIBRARY)
      set (${COMP_LIBRARY} ${${COMP_LIBRARY}} aec-${comp})
    endif ()
endforeach ()

check_required_components (${LIBAEC_PACKAGE_NAME})
