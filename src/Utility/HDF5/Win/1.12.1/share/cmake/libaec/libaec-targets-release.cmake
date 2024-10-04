#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "hdf5::aec-static" for configuration "Release"
set_property(TARGET hdf5::aec-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(hdf5::aec-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libaec.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS hdf5::aec-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_hdf5::aec-static "${_IMPORT_PREFIX}/lib/libaec.lib" )

# Import target "hdf5::sz-static" for configuration "Release"
set_property(TARGET hdf5::sz-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(hdf5::sz-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libsz.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS hdf5::sz-static )
list(APPEND _IMPORT_CHECK_FILES_FOR_hdf5::sz-static "${_IMPORT_PREFIX}/lib/libsz.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
