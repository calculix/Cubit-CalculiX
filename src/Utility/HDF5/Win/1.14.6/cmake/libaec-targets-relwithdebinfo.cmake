#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "hdf5::aec-static" for configuration "RelWithDebInfo"
set_property(TARGET hdf5::aec-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(hdf5::aec-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libaec.lib"
  )

list(APPEND _cmake_import_check_targets hdf5::aec-static )
list(APPEND _cmake_import_check_files_for_hdf5::aec-static "${_IMPORT_PREFIX}/lib/libaec.lib" )

# Import target "hdf5::szaec-static" for configuration "RelWithDebInfo"
set_property(TARGET hdf5::szaec-static APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(hdf5::szaec-static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "C"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libszaec.lib"
  )

list(APPEND _cmake_import_check_targets hdf5::szaec-static )
list(APPEND _cmake_import_check_files_for_hdf5::szaec-static "${_IMPORT_PREFIX}/lib/libszaec.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
