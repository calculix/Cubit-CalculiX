****************************************************
Muscat Quick Reference
****************************************************

This document provides a quick reference for developers using the Muscat library, one of the core dependency of OpenPisco, with code examples for common operations. 
To actually contribute to OpenPisco or even understanding the core implementation, some level of familiarity with Muscat is recommended. 
After all, it is not possible to make (Open)Pisco, without Muscat. 

The aim of this section is not to provide an extensive guide regarding Muscat. For more details, we refer to the actual Muscat library documentation.
We merely collect here the Muscat module extensively used within OpenPisco and provide, in what follows, explanations regarding their classical use cases.
For convenience, we recommend the reader to refer first to the Muscat Cheat sheet within Muscat documentation `here`_.

.. _here: https://muscat.readthedocs.io/en/stable/rstFiles/Cheat_sheet.html



1. Mesh IO
====================================================

Muscat's ability to read/write meshes in various format, described in :py:module:`Muscat.IO`, is especially useful to interact with external 
tools (physical solvers, remeshing tools etc...). See the Muscat documentation `MuscatMeshIO`_.

.. _MuscatMeshIO: https://muscat.readthedocs.io/en/stable/rstFiles/MeshFileConverter.html 

However, what is paramount is its ability to make quite user friendly various manipulations of a mesh object as featured in :py:module:`Muscat.MeshContainers` and :py:module:`Muscat.MeshTools`.
All the operations related to mesh extraction, mesh/field input/output, tag creation, advance interfaces with external tools etc...Rely heavily on these features.
See the Muscat documentation `MuscatMesh`_.

.. _MuscatMesh: https://muscat.readthedocs.io/en/stable/rstFiles/Containers.html
 
2. Filters
====================================================

Filters (by nodes, elements and operators) are objects used to select nodes/elements of a mesh based on filter parameters. 
See the Muscat documentation `MuscatFilters`_.

.. _MuscatFilters: https://muscat.readthedocs.io/en/stable/rstFiles/Filters.html


3. Implicit Geometry
====================================================

This module provides a framework for manipulating implicit geometries. 
It offers efficient algorithms for computing distances to elementary geometric primitives (such as cylinders, planes, spheres, honeycombs) as well as to surfaces described by mesh representations. 

It is especially convenient to create a mesh tag 

.. code-block:: python

    from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometrySphere
    from Muscat.MeshContainers.Filters.FilterTools import ElementFilter
    from Muscat.MeshTools.MeshCreationTools import CreateCube

    length = np.array([8,4,4])
    dimensions = np.array([20, 10, 10])
    spacing = length/(dimensions-1)
    support = CreateCube(dimensions=dimensions,spacing=spacing,origin=[0.,0.,0.],ofTetras=True)

    ff = ElementFilter()
    zone = ImplicitGeometrySphere(center=[2.5, 2., 2.], radius=1)
    ff.AddZone(zone)
    ff.SetDimensionality(dim)
    for name,data,ids in ff(support):
        data.tags.CreateTag("myTag").SetIds(ids)


or even to initialize a levelset with an hole

.. code-block:: python

    from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometrySphere
    from Muscat.MeshTools.MeshCreationTools import CreateCube
    
    from OpenPisco.Unstructured.Levelset import LevelSet
    from OpenPisco.Unstructured.MmgMesher import MmgMesherActionLevelset

    length = np.array([8,4,4])
    dimensions = np.array([20, 10, 10])
    spacing = length/(dimensions-1)
    mesh = CreateCube(dimensions=dimensions,spacing=spacing,origin=[0.,0.,0.],ofTetras=True)
    ls = LevelSet(support=mesh)
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.5,"hmax":1,"nr":True,"keepGeneratedFiles":True}
    ls.conform = True
    implicitSphere = ImplicitGeometrySphere(center=[2.5, 2., 2.], radius=1)
    ls.phi = implicitSphere.ApplyVector(mesh)
    ls.originalSupport = ls.support
    MmgMesherActionLevelset(ls,opts)


5. Examples
====================================================

Muscat provides various features regarding fields manipulation and allows the user to implement a custom Finite element solver. See for example the nootebooks `MuscatFilters`_ and 
`MuscatFEM`_ . 

.. _MuscatPrePostExample: https://muscat.readthedocs.io/en/stable/notebooks/PrePostDeepLearning.html
.. _MuscatFEM: https://muscat.readthedocs.io/en/stable/notebooks/TwoInclusions.html