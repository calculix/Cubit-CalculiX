.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

#########
Remeshing
#########

OpenPisco provides a Python interface to the remeshing software MMG [1]_ together
with several pre-processing and post-processing utilities.

The interface is available through:

* :py:mod:`OpenPisco.Unstructured.MmgMesher`

Supported MMG modules include:

* **mmg3d** for tetrahedral meshes,
* **mmgs** for surface triangular meshes,
* **mmg2d** for two-dimensional simplicial meshes.

Remeshing plays a central role in body-fitted topology optimization and mesh
adaptation workflows. As geometries evolve during optimization, remeshing helps
maintain mesh quality, capture interfaces accurately, and control computational
cost.

Remeshing Strategies
====================

OpenPisco supports two complementary remeshing approaches.

.. grid:: 2
   :gutter: 3

   .. grid-item-card:: 📐 Level-Set Remeshing

      Generate a mesh conforming to an implicit interface represented by
      a level-set function.

   .. grid-item-card:: 🔍 Adaptive Remeshing

      Adapt the mesh locally according to a user-defined metric field.

Level-Set Remeshing
===================

* :py:func:`OpenPisco.Unstructured.MmgMesher.MmgMesherActionLevelSet`

Purpose
--------

Generate an explicit mesh representation of an implicit geometry described
by a level-set function.

This operation is particularly useful for body-fitted approaches, where the
computational mesh must conform to the material interface.

Key Parameters
--------------

* **ls**: activates level-set mode and specifies the isovalue
  (default value: 0).
* **rmc**: removes connected components whose volume fraction falls below
  a user-defined threshold.

Generated Tags
--------------

The level-set split preserves existing mesh subregions and automatically
creates the following tags:

* **Outside3D**: elements located outside the structure.
* **Inside3D**: elements located inside the structure.
* **InterSurf**: triangles belonging to the extracted interface.
* **InterBars**: interface bars located on the bounding box.

Examples
--------

* :py:func:`OpenPisco.Unstructured.MmgMesher.CheckIntegrity_LevelSetRemeshingWithMetric`
* :py:func:`OpenPisco.Unstructured.MmgMesher.CheckIntegrity_LevelSetRemeshingWithLocalParams`
*  web documentation [2]_ 

Adaptive Remeshing
==================

* :py:func:`OpenPisco.Unstructured.MmgMesher.MmgMesherActionRemesh`

Purpose
--------

Adapt the mesh according to a user-defined metric field.

The metric may be isotropic or anisotropic and specifies the desired local
element size throughout the computational domain.

Benefits
--------

* Refines the mesh only where needed.
* Reduces computational cost.
* Improves numerical accuracy near critical regions.

Combined Adaptation and Interface Tracking
------------------------------------------

Adaptive remeshing and level-set conformal remeshing can be performed
simultaneously.

In this case:

* the level-set field is provided through the **-ls** option,
* the metric field is provided through the **-met** option.

Examples
--------

* :py:func:`OpenPisco.Unstructured.MmgMesher.CheckIntegrity_LevelSetRemeshingWithMetric`

Common MMG Parameters
=====================

The following parameters are available for both remeshing strategies.

+-----------+--------------------------------------------------+
| Parameter | Description                                      |
+===========+==================================================+
| hmin      | Minimum desired element size                     |
+-----------+--------------------------------------------------+
| hmax      | Maximum desired element size                     |
+-----------+--------------------------------------------------+
| hausd     | Geometrical approximation accuracy               |
+-----------+--------------------------------------------------+
| hgrad     | Maximum size variation between adjacent elements |
+-----------+--------------------------------------------------+
| opnbdy    | Preserve internal boundary surfaces              |
+-----------+--------------------------------------------------+
| nr        | Disable ridge detection                          |
+-----------+--------------------------------------------------+

Starting from these global requirements, MMG automatically performs local mesh
operations to improve mesh quality while satisfying the prescribed constraints.

Preserving Mesh Entities
========================

Certain mesh entities may need to remain unchanged during remeshing.
OpenPisco supports this through dedicated tags.

Required Tags
-------------

* **RequiredVertices**: preserves mesh vertices.
* **RequiredEdges**: preserves mesh edges.
* **RequiredTriangles**: preserves mesh triangles.

Additionally, entities of type ``Point0`` are automatically preserved during
remeshing operations.

Additional Features
===================

Overlapping Mesh Tags
---------------------

Dedicated pre-processing and post-processing routines are available to preserve
overlapping volumetric tags throughout the remeshing process.



.. [1] https://www.mmgtools.org/
.. [2] https://www.mmgtools.org/tutorials/tutorials_mmg3d_implicit.html

