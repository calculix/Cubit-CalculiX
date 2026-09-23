! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
!   Copyright by The HDF Group.                                               *
!   All rights reserved.                                                      *
!                                                                             *
!   This file is part of HDF5.  The full HDF5 copyright notice, including     *
!   terms governing use, modification, and redistribution, is contained in    *
!   the COPYING file, which can be found at the root of the source code       *
!   distribution tree, or in https://www.hdfgroup.org/licenses.               *
!   If you do not have access to either file, you may request a copy from     *
!   help@hdfgroup.org.                                                        *
! * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
! fortran/H5config_f.inc. Generated from fortran/src/H5config_f.inc.cmake by CMake

! Define if there is parallel support
#define CMAKE_H5_HAVE_PARALLEL 0
#if CMAKE_H5_HAVE_PARALLEL == 0
#undef H5_HAVE_PARALLEL
#else
#define H5_HAVE_PARALLEL
#endif

! Define if MPI supports mpi_f08 module
#define CMAKE_H5_HAVE_MPI_F08 0
#if CMAKE_H5_HAVE_MPI_F08 == 0
#undef H5_HAVE_MPI_F08
#else
#define H5_HAVE_MPI_F08
#endif

! Define if there is subfiling support
#define CMAKE_H5_HAVE_SUBFILING_VFD 0
#if CMAKE_H5_HAVE_SUBFILING_VFD == 0
#undef H5_HAVE_SUBFILING_VFD
#else
#define H5_HAVE_SUBFILING_VFD
#endif

! Define if on APPLE
#define CMAKE_H5_HAVE_DARWIN 0
#if CMAKE_H5_HAVE_DARWIN == 0
#undef H5_HAVE_DARWIN
#else
#define H5_HAVE_DARWIN
#endif

! Define if the intrinsic function STORAGE_SIZE exists
#define CMAKE_H5_FORTRAN_HAVE_STORAGE_SIZE 1
#if CMAKE_H5_FORTRAN_HAVE_STORAGE_SIZE == 0
#undef H5_FORTRAN_HAVE_STORAGE_SIZE
#else
#define H5_FORTRAN_HAVE_STORAGE_SIZE
#endif

! Define if the intrinsic function SIZEOF exists
#define CMAKE_H5_FORTRAN_HAVE_SIZEOF 1
#if CMAKE_H5_FORTRAN_HAVE_SIZEOF == 0
#undef H5_FORTRAN_HAVE_SIZEOF
#else
#define H5_FORTRAN_HAVE_SIZEOF
#endif

! Define if the intrinsic function C_SIZEOF exists
#define CMAKE_H5_FORTRAN_HAVE_C_SIZEOF 1
#if CMAKE_H5_FORTRAN_HAVE_C_SIZEOF == 0
#undef H5_FORTRAN_HAVE_C_SIZEOF
#else
#define H5_FORTRAN_HAVE_C_SIZEOF
#endif

! Define if allocatable character is supported
#define H5_FORTRAN_HAVE_CHAR_ALLOC 1

! Define if the intrinsic function C_LONG_DOUBLE exists
#define H5_FORTRAN_HAVE_C_LONG_DOUBLE 1

! Define if Fortran C_LONG_DOUBLE is different from C_DOUBLE
#define H5_FORTRAN_C_LONG_DOUBLE_IS_UNIQUE 0

! Define if Fortran C_BOOL is different from default LOGICAL
#define H5_FORTRAN_C_BOOL_IS_UNIQUE 1

! Define  MPI Fortran KIND of LOGICAL
#define CMAKE_H5_MPI_LOGICAL_KIND 0
#if CMAKE_H5_MPI_LOGICAL_KIND == 0
#undef H5_MPI_LOGICAL_KIND
#else
#define H5_MPI_LOGICAL_KIND 
#endif

! Define if Fortran supports ISO_FORTRAN_ENV (F08)
#define CMAKE_H5_HAVE_ISO_FORTRAN_ENV 1
#if CMAKE_H5_HAVE_ISO_FORTRAN_ENV == 0
#undef H5_HAVE_ISO_FORTRAN_ENV
#else
#define H5_HAVE_ISO_FORTRAN_ENV
#endif

! Define the size of C's double
#define H5_SIZEOF_DOUBLE 8

! Define the size of C's long double
#define H5_SIZEOF_LONG_DOUBLE 8

! Define the maximum decimal precision for reals
#define H5_PAC_FC_MAX_REAL_PRECISION 33

! If C has quad precision
#define CMAKE_H5_HAVE_FLOAT128 1
#if CMAKE_H5_HAVE_FLOAT128 == 0
#undef H5_HAVE_FLOAT128
#else
#define H5_HAVE_FLOAT128
#endif

! Define if INTEGER*16 is available
#define H5_HAVE_Fortran_INTEGER_SIZEOF_16 0

! Maximum decimal precision for C
#define H5_PAC_C_MAX_REAL_PRECISION 0

! number of valid REAL KINDs
#define H5_H5CONFIG_F_NUM_RKIND INTEGER, PARAMETER :: num_rkinds = 3

! valid REAL KINDs (need to have a matching C counter-part)
#define H5_H5CONFIG_F_RKIND INTEGER, DIMENSION(1:num_rkinds) :: rkind = (/4,8,16/)

! valid REAL KINDs (need to have a matching C counter-part)
#define H5_H5CONFIG_F_RKIND_SIZEOF INTEGER, DIMENSION(1:num_rkinds) :: rkind_sizeof = (/4,8,16/)

! number of valid INTEGER KINDs
#define H5_H5CONFIG_F_NUM_IKIND INTEGER, PARAMETER :: num_ikinds = 4

! valid INTEGER KINDs (need to have a matching C counter-part)
#define H5_H5CONFIG_F_IKIND INTEGER, DIMENSION(1:num_ikinds) :: ikind = (/1,2,4,8/)

! Fortran compiler id
#define H5_Fortran_COMPILER_ID IntelLLVM

! Define if deprecated public API symbols are disabled
#define CMAKE_NO_DEPRECATED_SYMBOLS 1
#if CMAKE_NO_DEPRECATED_SYMBOLS == 0
#undef H5_NO_DEPRECATED_SYMBOLS
#else
#define H5_NO_DEPRECATED_SYMBOLS
#endif

! For major interface/format changes
#define H5_VERS_MAJOR 1

! For minor interface/format changes
#define H5_VERS_MINOR 14

! For tweaks, bug-fixes, or development
#define H5_VERS_RELEASE 4

! macros for comparing versions
#define H5_VERSION_GE(Maj, Min, Rel)                                                   \
    (((H5_VERS_MAJOR == Maj) && (H5_VERS_MINOR == Min) && (H5_VERS_RELEASE >= Rel)) || \
     ((H5_VERS_MAJOR == Maj) && (H5_VERS_MINOR > Min)) || (H5_VERS_MAJOR > Maj))

#define H5_VERSION_LE(Maj, Min, Rel)                                                   \
    (((H5_VERS_MAJOR == Maj) && (H5_VERS_MINOR == Min) && (H5_VERS_RELEASE <= Rel)) || \
     ((H5_VERS_MAJOR == Maj) && (H5_VERS_MINOR < Min)) || (H5_VERS_MAJOR < Maj))

