.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

############################
Lagrangian Mesh Displacement
############################

OpenPisco provides a Python interface to the remeshing software **MMG** [1]_.
Among the available capabilities is the support for **Lagrangian mesh displacement**,
which allows the nodes of a mesh to be moved according to a prescribed displacement
field while preserving the mesh connectivity.

This functionality is implemented in the
:py:mod:`OpenPisco.Unstructured.MmgMeshMotion` module and relies on the
``-lag`` option provided by MMG [2]_.

A displacement field is typically defined on a boundary or interface and
propagated throughout the volume mesh. As a result, the geometry can be updated
without modifying the mesh topology.

In the context of shape optimization, Lagrangian mesh displacement provides an
efficient alternative to level-set advection and remeshing when the desired
shape evolution remains compatible with the existing mesh connectivity.

Typical applications include:

* Shape optimization workflows.
* Design updates driven by shape sensitivities.
* Geometry morphing and mesh deformation.
* Parametric studies involving moderate geometric changes.

The following example illustrates the use of the Python API for mesh
displacement:

* :py:func:`OpenPisco.Unstructured.MmgMeshMotion.CheckIntegrity`


.. [1] https://www.mmgtools.org/
.. [2] https://www.mmgtools.org/tutorials/tutorials_mmg2d_lagrangian.html

