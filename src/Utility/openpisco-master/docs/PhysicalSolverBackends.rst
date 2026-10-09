.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

************************
Physical Solver Backends
************************

Overview
########

OpenPisco can interface with several external and internal simulation tools.

All solver backends expose a common interface through
:py:class:`OpenPisco.PhysicalSolvers.SolverBase`, allowing optimization
algorithms and criteria to remain independent of the underlying simulation
software.

External Solvers
################

Code_Aster
==========

Code_Aster [1]_ is an open-source finite-element solver developed by EDF. It supports
a wide range of structural, thermal and multiphysics analyses.

Main features:

* Linear and nonlinear mechanics
* Thermal analyses
* Modal analyses
* Harmonic analyses
* Native field post-processing

The interface relies on:

* :py:class:`OpenPisco.ExternalTools.Aster.AsterInterface.AsterInterface`
* Classes derived from
  :py:class:`OpenPisco.ExternalTools.Aster.AsterSolverBase.AsterSolverBase`

FreeFem++
=========

FreeFem++ [2]_ is a high-level finite-element software package based on the
variational formulation of PDEs.

The OpenPisco interface provides:

* Automatic generation of FreeFem scripts
* Execution of simulations
* Retrieval of generated fields

Main classes:

* :py:class:`OpenPisco.ExternalTools.FreeFem.FreeFemInterface.FreeFemInterface`
* :py:class:`OpenPisco.ExternalTools.FreeFem.FreeFemProblemWriter.FreeFemProblemWriter`

Zset
====

Zset [3]_ is a finite-element software suite used for (non) linear mechanical
analyses.

OpenPisco provides dedicated interfaces for supported mechanical analyses.

OpenFOAM
========

OpenFOAM [4]_ is an open-source finite-volume framework primarily dedicated to
computational fluid dynamics (CFD).

It is currently used within OpenPisco for fluid flow analyses.

Internal Solver
###############

Muscat
======

Muscat [5]_ is the native finite-element solver available within OpenPisco.

Unlike external solvers:

* No intermediate files need to be generated.
* No executable needs to be launched.
* Meshes are directly accessible through OpenPisco and Muscat structures.

The implementation is based on:

* :py:mod:`OpenPisco.PhysicalSolvers.UnstructuredFEAGenericProblem`

Supported Analyses
##################

.. csv-table:: Physical analysis supported w.r.t. tools
   :header: "Analysis", "Code_Aster", "FreeFem++", "Zset", "Muscat", "OpenFOAM"
   :widths: 30, 10, 10, 10, 10, 10

   "Linear elasticity", "True", "True", "True", "True", "False"
   "Thermoelasticity", "True", "False", "False", "False", "False"
   "Static buckling", "True", "False", "False", "False", "False"
   "Modal analysis", "True", "False", "False", "False", "False"
   "Harmonic analysis", "True", "False", "False", "False", "False"
   "Thermal analysis", "True", "False", "False", "True", "False"
   "Thermal eigenvalue analysis", "True", "False", "False", "False", "False"
   "Incompressible flow", "False", "False", "False", "False", "True"



.. [1] https://www.code-aster.org/
.. [2] https://freefem.org/
.. [3] http://www.zset-software.com/
.. [4] https://www.openfoam.com/
.. [5] https://gitlab.com/drti/muscat