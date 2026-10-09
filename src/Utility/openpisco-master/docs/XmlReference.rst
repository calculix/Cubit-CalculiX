****************************************************
XML Reference Guide
****************************************************

.. _XML Reference Guide:

This document provides a reference for the XML-based Domain Specific Language (DSL) used to define and run optimization studies in OpenPisco with the applications.

Introduction
====================================================

The primary way to interact with the OpenPisco command-line application is through an XML input file. This file defines all the components of the optimization study, including the geometry, meshes, level-sets, physical problems, optimization setup, and the sequence of actions to perform.

The root element of the file is always ``<data>``.

.. code-block:: xml

    <data dim="3" debug="False">
        <!-- All other tags go here -->
    </data>

**Root Attributes:**

*   ``dim``: The dimensionality of the problem (2 or 3).
*   ``debug``: A boolean to enable verbose logging.

---

Core Component Tags
====================================================

The XML file is organized into several main sections, each corresponding to a core component of the OpenPisco platform. The order of these sections in the file is important, as components are often defined in terms of previously defined ones.

Zones
----------------------------------------------------
The ``<Zones>`` block is used to define geometric primitives that can be combined to describe the design space, boundary condition regions, and initial shapes. Unless specified otherwise, a zone is usually an ImplicitGeometry object by the terminology of Muscat.

*   **Primitives**: ``<Plane>``, ``<Sphere>``, ``<Cylinder>``, ``<Box>``, etc.
*   **Operators**: ``<Union>``, ``<Intersection>``, ``<Difference>`` can be used to combine other zones.
*   **`id`**: Each zone is given a unique integer ``id`` so it can be referenced later.

.. code-block:: xml

    <Zones>
        <Plane id="1" point="0.0 0.0 0.0" normal="1. 0. 0."/>
        <Sphere id="2" center="1. 0. 0." radius="0.5"/>
        <Union id="3" z="1 2" />
    </Zones>

Grids
----------------------------------------------------
The ``<Grids>`` block defines the computational meshes.

*   **`id`**: A unique integer ID for the grid.
*   **``<Grid>``**: Defines a structured Cartesian grid.
    *   ``n``: Number of nodes in each direction (e.g., "41 41 41").
    *   ``origin``: Coordinates of the grid's origin.
    *   ``length``: Dimensions of the grid.
    *   ``ofTetras`` / ``ofTriangles``: Boolean to create a simplex-based mesh.
*   **``<Mesh>``**: Loads an unstructured mesh from an external file.
    *   ``filename``: Path to the mesh file (e.g., `.med`).

.. code-block:: xml

    <Grids>
        <Grid id="1" n="41 41 41" origin="0. 0. 0." length="2.0 0.25 1.0" ofTetras="True"/>
        <Mesh id="2" filename="path/to/my_mesh.med"/>
    </Grids>

LevelSets
----------------------------------------------------
The ``<LevelSets>`` block defines the level-set functions used for shape representation.

*   **`id`**: A unique integer ID.
*   **``<LevelSet>``**:
    *   ``support``: The ``id`` of the ``<Grid>`` it is defined on.
    *   ``conform``: Boolean indicating if the mesh should be conformed to the level-set (Lagrangian/Shape Tracking approach).
    *   ``<MesherInfo>``: Contains parameters for the remeshing process when ``conform="True"``.

.. code-block:: xml

    <LevelSets>
        <LevelSet id="1" support="1" conform="True">
              <MesherInfo hmax=".5" hmin="0.01" />
        </LevelSet>
    </LevelSets>

PhysicalProblems
----------------------------------------------------
This block defines the physical simulations to be solved (e.g., static elasticity, thermal analysis).

*   **`id`**: A unique integer ID.
*   **``<GeneralAster>``, ``<StaticFreeFemSolverMeca>``, etc.**: The tag name corresponds to the solver type.
    *   ``type``: The specific analysis type (e.g., "static_elastic").
    *   **Inner Tags**: Define material properties (``<Material>``), boundary conditions (``<Dirichlet>``), and loads (``<Force>``).

.. code-block:: xml

    <PhysicalProblems>
        <GeneralAster type="static_elastic" id="1" >
            <Material young="210.e9" poisson="0.3" />
            <Dirichlet eTag="X0" dofs="0 1 2" value="0.0"/>
            <Force nTag="nTag4" value="0. 0. -10000" />
        </GeneralAster>
    </PhysicalProblems>

OptimProblems
----------------------------------------------------
This block defines the optimization problem itself, linking together the geometry, level-set, and criteria.

*   **`id`**: A unique integer ID.
*   **``<OptimProblem>``**:
    *   ``type``: The problem type, usually ``TopoGeneric``.
    *   ``useLevelset``: The ``id`` of the level-set to be optimized.
    *   ``OnZone``: The ``id`` of the zone defining the active design space.
    *   ``OffZone``: The ``id`` of the zone excluded from the design space.
    *   ``<Objective>``: Defines the objective function.
        *   ``type``: The criterion to use (e.g., "Volume", "Compliance").
    *   ``<Constraint>``: Defines a constraint.
        *   ``type``: The criterion to use.
        *   ``UpperBound`` / ``LowerBound``: The constraint value.
        *   ``useProblem``: The ``id`` of the ``<PhysicalProblem>`` to use for criteria that need it.

.. code-block:: xml

    <OptimProblems>
        <OptimProblem type="TopoGeneric" id="1" useLevelset="1" OnZone="8">
          <Objective type="Volume" name="Volume"/>
          <Constraint type="Compliance" name="Compliance" UpperBound="1.2" useProblem="1" />
       </OptimProblem>
    </OptimProblems>

TopologicalOptimizations
----------------------------------------------------
This block defines the optimization algorithm to be used.

*   **`id`**: A unique integer ID.
*   **``<OptimAlgoNullSpace>``, etc.**: The tag name corresponds to the algorithm type.
    *   ``useOptimProblem``: The ``id`` of the ``<OptimProblem>`` to solve.

.. code-block:: xml

    <TopologicalOptimizations>
        <OptimAlgoNullSpace id="1" useOptimProblem="1" />
    </TopologicalOptimizations>

Actions
----------------------------------------------------
The ``<Actions>`` block defines a sequence of operations to be executed. This is where the simulation is actually driven.

*   **``<Action>``**:
    *   ``type``: The name of the action to perform (e.g., ``InitLevelset``, ``RunTopoOp``, ``SaveShapeToFile``).
    *   **Attributes**: Action-specific parameters. For example, ``RunTopoOp`` needs the ``TopoOp`` attribute set to the ``id`` of a ``<TopologicalOptimization>`` block.

.. code-block:: xml

    <Actions>
        <Action type="InitLevelset" ls="1" zone="6"/>
        <Action type="RunTopoOp" TopoOp="1" />
        <Action type="SaveShapeToFile" ls="1" filename="final_shape.stl"/>
    </Actions>

Outputs
----------------------------------------------------
This block defines how and where to save results.

*   **`id`**: A unique integer ID.
*   **``<Output>``**:
    *   ``name``: The output filename.
    *   ``inTempDirectory``: Boolean to save in the temporary working directory.
*   **``<Proxy>``**: Can be used to combine multiple outputs.

.. code-block:: xml

    <Outputs>
       <Output id="1" name="History.xmf" inTempDirectory="True" />
       <Output id="2" name="History.csv" inTempDirectory="True" />
       <Output id="3" name="PROXY" outputs="1 2" />
    </Outputs>