.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

*****************
Physical Analyses
*****************

OpenPisco currently supports several classes of physical analyses used in
shape and topology optimization workflows.

The sections below provide an overview of the supported analyses and link to
their theoretical formulation.

.. grid:: 3
   :gutter: 3

   .. grid-item-card:: 🏗️ Structural Mechanics
      :link: Physics/StructuralMechanics
      :link-type: doc

      Elasticity, buckling, modal and harmonic analyses for structural
      applications.

   .. grid-item-card:: 🌡️ Thermal Analyses
      :link: Physics/ThermalAnalyses
      :link-type: doc

      Thermal conduction, thermal eigenmodes and thermoelastic analyses.

   .. grid-item-card:: 🌊 Fluid Mechanics
      :link: Physics/FluidMechanics
      :link-type: doc

      Incompressible steady-state laminar flow analyses.

Overview
########

Structural Mechanics
====================

OpenPisco provides several structural analyses commonly encountered in
mechanical engineering applications:

* **Linear Elasticity**
  
  Static analysis of elastic structures subjected to prescribed loads and
  boundary conditions.

* **Static Buckling**
  
  Linearized buckling analysis used to estimate critical loads and instability
  modes.

* **Modal Analysis**
  
  Computation of eigenfrequencies and vibration modes.

* **Harmonic Analysis**
  
  Frequency-domain response of structures subjected to periodic excitations.

See :doc:`Physics/StructuralMechanics` for the mathematical formulations and
governing equations.

Thermal Analyses
================

OpenPisco provides several thermal analyses:

* **Thermal Conduction**
  
  Computation of transient or steady-state temperature fields.

* **Thermal Eigenvalue Analysis**
  
  Spectral decomposition of thermal problems.

* **Thermoelasticity**
  
  Weakly coupled thermo-mechanical analyses combining thermal and structural
  simulations.

See :doc:`Physics/ThermalAnalyses` for details.

Fluid Mechanics
===============

Currently, OpenPisco supports:

* **Incompressible Steady-State Flow**
  
  Laminar Newtonian flows governed by the Navier-Stokes equations.

See :doc:`Physics/FluidMechanics` for details.

Associated Implementations
##########################

The following classes currently implement the available analyses:

* Linear elasticity

  * :py:class:`OpenPisco.PhysicalSolvers.AsterStatic`
  * :py:class:`OpenPisco.PhysicalSolvers.GeneralZSetSolverMeca.GeneralZSetPhysicalProblemMeca`
  * :py:class:`OpenPisco.PhysicalSolvers.StaticFreeFemSolverMeca.StaticFreeFemSolverMeca`
  * :py:class:`OpenPisco.PhysicalSolvers.UnstructuredFEAMecaSolver.UnstructuredFEAMecaProblem`

* Static buckling

  * :py:class:`OpenPisco.PhysicalSolvers.AsterBuckling`

* Modal analysis

  * :py:class:`OpenPisco.PhysicalSolvers.AsterModal`

* Harmonic analysis

  * :py:class:`OpenPisco.PhysicalSolvers.AsterHarmonicForced`

* Thermal conduction

  * :py:class:`OpenPisco.PhysicalSolvers.AsterThermal`
  * :py:class:`OpenPisco.PhysicalSolvers.UnstructuredFEAThermalSolver.UnstructuredFEAThermalProblem`

* Thermal eigenvalue analysis

  * :py:class:`OpenPisco.PhysicalSolvers.AsterThermalEigenValues`

* Thermoelasticity

  * :py:class:`OpenPisco.PhysicalSolvers.AsterThermoElastic`
  * :py:class:`OpenPisco.PhysicalSolvers.UnstructuredFEAMecaSolver.UnstructuredFEAMecaThermalStrainProblem`

* Incompressible steady-state flow

  * :py:class:`OpenPisco.PhysicalSolvers.OpenFoamSteadyState`
