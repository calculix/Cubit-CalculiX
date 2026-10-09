.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

Topology optimization of a 2D cantilever beam using body-fitted meshes
========================================================================================

The goal of this tutorial is to run a topology optimization of a bidimensional cantilever beam using using body-fitted meshes.

.. note::
   For running this tutorial Code_Aster [2]_ needs to be installed and available from the command line.


Test case
~~~~~~~~~~~~~~~~

Consider the following specifications

* domain of size :math:`2` m :math:`\times 1` m
* elastic material with Young modulus equal to :math:`210.e9` Pa and Poisson ratio equal to  :math:`0.3`
* beam clamped on the plane :math:`x = 0` m and submitted to a vertical load g = :math:`(0, -10000)` N on the point :math:`(2, 0.5)`
* minimization of the volume under a compliance constraint

Data setting : header
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We start by editing a Cantilever2D.xml file.

The data file starts with the following header

.. code-block::

    <data dim="2" >

Data setting : implicit zones
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Define implicit zones

.. code-block::

    <Zones>
        <Plane id="1" point="0.0001 0.25 0.5" normal="1. 0. 0." desc="x0 plane "/>
        <Plane id="2" point="0.05 0.25 0.5" normal="1. 0. 0." desc="dirichlet"/>
        <Sphere id="4" center="2. 0.5 0.0"  radius="0.0009" desc="nodal force"/>
        <Cylinder id="5" center1="1.9999 0.5 0." center2="2.1 0.5 0." radius="0.01" desc="force"/>
        <Holes id="6" n="6 4 2" r="0.2" boundingMin="0. 0. 0." boundingMax="2.0 1.0 1.0" type="Original" desc="Holes"/>
        <Union id="7" z="2 4 6" desc="initialization"/>
        <Union id="8" z="2 5" desc="onzone" />
    </Zones>


Data setting : grids
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Define the background mesh which is a uniform grid

.. code-block::

    <Grids>
        <Grid id="1" n="61 41" origin="0. 0." length="2.0 1.0" ofSimplex="True"/>
    </Grids>

Data setting : levelsets
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Define the level set together with a set of options. The option conform="True" means that the level set is conformal. A set of meshing options are defined to drive the remeshing process.


.. code-block::

    <LevelSets>
        <LevelSet id="1" support="1" conform="True" gradientMethod="gradPhi">
              <MesherInfo hmax=".03" hmin="0.008" iso="0.0" rmc="1e-5"
      nr="True" computeDistanceWith="meshdist"/>
        </LevelSet>
    </LevelSets>


Data setting : physical problems
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Define the physical problem, which is a linear elastic static problem.

We define material properties, one dirichlet condition and one neumann condition.

By default we use linear elements.


.. code-block::

    <PhysicalProblems>
        <GeneralAster type="static_elastic" id="1" >
            <Material  young="210.e9" poisson="0.3" />
            <Dirichlet eTag="eTag1" dofs="0 1" value="0.0"/>
            <Force nTag="nTag4" value="0. -1000" />
         </GeneralAster>
    </PhysicalProblems>


Data setting : outputs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Define the desired output

.. code-block::

    <Outputs>
       <Output id="1" name="Cantilever2DHistory.xdmf" inTempDirectory="True" />
       <Output id="2" name="Cantilever2DHistory.csv" inTempDirectory="True" />
       <Output id="3" name="PROXY" outputs="1 2" />
    </Outputs>


Data setting : optimization problems
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then, we define the optimization problem by entering the objective, the constraints, the initial levelset, the implicit zones, the output and a set of options.

Note that we have to define an upper bound value for the constraint.

Note that, since the constraint is a physical criterion, we precise which physical problem should be used with the keyword useProblem="1".

.. code-block::

    <OptimProblems>
        <OptimProblem type="TopoGeneric" id="1"
            OnZone="8"
            useLevelset="1"
            printMeshQualityInfo = "True"
            output="3"
            outputEveryLevelsetUpdate = "False">
          <Objective  type="Volume"  name="Volume"/>
          <Constraint type="Compliance"  name="Compliance" UpperBound="2*1.90489800e-04" useProblem="1" />
       </OptimProblem>
    </OptimProblems>


Data setting : optimization algorithms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then, we define the optimization algorithm which is linked to the optimization problem


.. code-block::

    <TopologicalOptimizations>
        <OptimAlgoNullSpace id="1"
           useOptimProblem="1"/>
    </TopologicalOptimizations>


Actions setting
~~~~~~~~~~~~~~~~

Here we declare actions which must be runned. At first the level set "1" is initialized with the implicit zone "7"

.. code-block::

    <Actions>
        <Action type="InitLevelset" ls="1" zone="7"/>

We create a nodal tag "nTag4" from implicit zone "4" for the nodal force and we set it as required to preserve it during the remeshing

.. code-block::

        <Action type="CreateTagsFromZones" ls="1" nodalTags="4" nodalprefix="nTag"/>
        <Action type= "SetAsRequired" ls="1" nTags="nTag4" />

We perform a remeshing to generate a conformal mesh

.. code-block::

        <Action type="Remesh" ls="1" hmax=".03" hmin="0.008" iso="0.0" rmc="1e-5"
      nr="False" computeDistanceWith="meshdist" />

We create an element tag to support the Dirichlet boundary condition

.. code-block::

       <Action type="CreateTagsFromZones" ls="1" elementTags="1" dim="1" elemenprefix="eTag"/>

Then, we run the topology optimization

.. code-block::

        <Action type="RunTopoOp" TopoOp="1"/>

Eventually mesh quality indicators are printed

.. code-block::

        <Action type="LsQualityInfo" ls="1" />
    </Actions>
 </data>

.. figure:: ../images/tutoConform2DInitiMesh.png
    :width: 500
    :align: center
    :alt: Optimal Shape
    :figclass: align-center

    Visualisation of the initial material distribution in Paraview


Run
~~~~~~~~~~~~~~~~

To run the script using the command line application execute in a terminal

.. code-block::

    OpenPiscoCL Cantilever2D.xml

To run the script using the GUI application execute in a terminal

.. code-block::

    OpenPisco Cantilever2D.xml

Output
~~~~~~~~~~~~~~~~

After running the tutorial script the following output files have been created in the working directory

* Cantilever2D.xml.log     : run summary
* Cantilever2DHistory.xdmf        : (heavy) file containing the mesh and fields at each iteration in xdmf format
* Cantilever2DHistory.csv         : convergence history in csv format


.. figure:: ../images/tutoConform2DOptimalShape.png
    :width: 500
    :align: center
    :alt: Optimal Shape
    :figclass: align-center

    Visualisation of the optimal design in Paraview

.. figure:: ../images/tutoConform2DOptimalDep.png
    :width: 500
    :align: center
    :alt: Optimal Shape Displacement
    :figclass: align-center

    Visualisation of the displacement field in Paraview


The demo xml file created is available in the module  :py:meth:`OpenPisco.Demos` of the OpenPisco Python library [1]_ .

.. [1] https://gitlab.com/openpisco/openpisco/-/tree/master/src/OpenPisco/Demos/
.. [2] https://www.code-aster.org/