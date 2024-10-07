cmake_minimum_required (VERSION 3.12)
PROJECT (HDF5Examples_C)

#-----------------------------------------------------------------------------
# Build the C Examples
#-----------------------------------------------------------------------------
add_subdirectory (${PROJECT_SOURCE_DIR}/H5D)
add_subdirectory (${PROJECT_SOURCE_DIR}/H5G)
add_subdirectory (${PROJECT_SOURCE_DIR}/H5T)
#add_subdirectory (${PROJECT_SOURCE_DIR}/Performance)
if (USE_SHARED_LIBS AND HDF_BUILD_FILTERS)
  add_subdirectory (${PROJECT_SOURCE_DIR}/H5Filters)
endif ()
