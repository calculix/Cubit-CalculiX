#-----------------------------------------------------------------------------
# Define Sources, one file per application
#-----------------------------------------------------------------------------
set (examples
  h5ex_d_alloc.f90
  h5ex_d_checksum.f90
  h5ex_d_chunk.f90
  h5ex_d_compact.f90
  h5ex_d_extern.f90
  h5ex_d_fillval.f90
  h5ex_d_hyper.f90
  h5ex_d_nbit.f90
  h5ex_d_rdwr.f90
  h5ex_d_rdwr_kind.f90
  h5ex_d_soint.f90
  h5ex_d_transform.f90
  h5ex_d_unlimmod.f90
)
if (HDF5_ENABLE_Z_LIB_SUPPORT)
  set (examples ${examples} h5ex_d_gzip.f90)
endif ()
if (HDF5_ENABLE_SZIP_SUPPORT)
  set (examples ${examples} h5ex_d_szip.f90)
endif ()
