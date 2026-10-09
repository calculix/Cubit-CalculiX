.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

****************
Physical Solvers
****************

Physical solvers are responsible for computing numerical approximations of the
partial differential equations involved in simulation, shape optimization and
topology optimization studies.

OpenPisco provides a common abstraction through the
:py:class:`OpenPisco.PhysicalSolvers.SolverBase` class, allowing several
finite-element and finite-volume solvers to be used through a unified
interface.

This design provides:

* A common workflow regardless of the underlying solver.
* Easy replacement of one solver by another.
* Consistent access to physical fields and scalar quantities.
* Support for level-set based optimization workflows.

Solver Architecture
###################

All physical solvers inherit from the
:py:class:`OpenPisco.PhysicalSolvers.SolverBase` class.

The common interface allows OpenPisco components to interact with physical
solvers independently of their implementation.

Level Set-based Optimization
############################

The solver infrastructure supports two common approaches used in topology
optimization:

* **Body-fitted (conformal) approach**, where the computational mesh matches
  the geometry represented by the level-set.

* **Fixed-mesh (Ersatz material) approach**, where the geometry is represented
  implicitly on a background mesh.

The actual implementation depends on the capabilities of each solver backend.

Documentation
#############

.. grid:: 3
   :gutter: 3

   .. grid-item-card::
      :link: PhysicalSolverBackends
      :link-type: doc

      ⚙️ Solver Backends

      Description of the simulation tools interfaced with OpenPisco.

   .. grid-item-card::
      :link: PhysicalAnalyses
      :link-type: doc

      📐 Physical Analyses

      Structural, thermal and fluid analyses supported by the platform.

   .. grid-item-card::
      :link: PhysicalQuantities
      :link-type: doc

      📊 Fields and Scalars

      Auxiliary quantities, post-processing fields and integrated values.

.. toctree::
   :hidden:

   PhysicalSolverBackends
   PhysicalAnalyses
   PhysicalQuantities
