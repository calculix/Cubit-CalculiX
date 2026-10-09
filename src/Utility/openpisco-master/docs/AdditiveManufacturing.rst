##############################
Additive manufacturing
##############################

The related xml files are also available in the module  :py:mod:`OpenPisco.Demos`.

.. _Overhang surfaces detection and export:

Overhang surfaces detection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block::

    <Action type="OverHangDetection"
    ls="int"
    angleDetection="45*|float"
    direction="[0 0 1]*|floats"
    surfaceTag="everyelement*|str"
    name="overhangSurface*|str"
    invertOrientation="False*|bool"
    />


The angleDetection parameter must be comprised between 0 et 90. The routine fails if the list of surface elements to inspect is empty.The invertOrientation flag allows to change the surface orientation if needed.


Backing surfaces detection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block::

    <Action type="BackingSurfaceDetection"
    ls="int"
    valueDetection="0.2*|float"
    direction="[0 0 1]*|floats"
    surfaceTag="everyelement*|str"
    name="backingSurface*|str"
    invertOrientation="False*|bool"
    />


Surfaces export to Stl
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block::

    <Action type="WriteSurfaceToStl"
    ls="int"
    eTag="str"
    filename="eTag.stl*|str"
    />

Contact surface penalization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Optimization criterion aiming at penalizing the contact area between the fabrication supports and the overhang-free surfaces of the structure to be manufactured.


.. code-block::

    <Objective type="ContactSurfacePenalization"
    eTags="elementTags:str"
    overhangTag="overhangTag:str" />

The input eTags defines the tags identifying the structure to be manufactured.
The input overhangTag defines the overhang surfaces.


Filter simply connected bodies in tag
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Modify tag in a mesh to keep only simply connected bodies such that, for each simply connected body b, we have:
minRelativeMeasure*size(mesh) < size(b) <  maxRelativeMeasure*size(mesh)

.. code-block::

    <Action type="FilterSimplyConnectedBodiesInTagAction"
    ls="int"
    eTag="str"
    minRelativeMeasure="*0.0|float"
    maxRelativeMeasure="*1.0|float"/>


The input eTag defines the tag to be filtered.

Generate general equivalent force field
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Generate thermoelastic equivalent load based on the resolution of a thermo-mechanical problem

.. code-block::

    <Action type="GenerateEquivalentForceField"
    ls="int"
    useProblem="int"
    forceFieldName="str"
    domainToTreat="str"
    eTag="str"/>

The input useProblem refers to the thermo-mechanical problem id to be used.
The input forceFieldName is the equivalent force field name.
The input domainToTreat defines the 3D tag to be used to compute the surfacic equivalent force field from
The input eTag defines the tag where the surfacic equivalent force field is evaluated.

Push Force Field To Physical Problem
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Retrieve a field from a thermo-mechanical problem and transfer it to another physical problem

.. code-block::

    <Action type="PushForceFieldToPhysicalProblem"
    useAMProcess="int"
    usePhyProblem="int"
    forceFieldName="str" />

The input useAMProcess refers to the thermo-mechanical problem id to retrieve the force field from.
The input usePhyProblem defines the problem id to transfer the force field to.
The input forceFieldName is the equivalent force field name.


Simulation of additive manufacturing process using inherent strain method
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Simulation of additive manufacturing process is based on layer-by-layer simulation.

Using inherent strain method, one needs to calibrate the input residual strain as initial strain for layer-by-layer simulation.

.. code-block::

    <PhysicalProblems>
        <AMProcess id="1*|int" type="IStrain*|str" deltaZ="0.055*|float" IStrains="-0.0001 -0.0001 0.*|floats" debugFilename="test*|str"  >
         <Material  young="110000000000*|float" poisson="0.33*|float" density="1.*|float" />
         <Block  on="Z0*|str" block="0 1 2*|ints"  />
         <Block  on="X0*|str" block="0*|ints"  />
         <Block  on="Y0*|str" block="1*|ints"  />
        </AMProcess>
    </PhysicalProblems>

The input type IStrains stands for inherent strain method.
The input IStrain is the residual strain calibrated by the user.

