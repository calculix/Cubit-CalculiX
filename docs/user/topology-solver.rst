Topology solver requirements and outputs
========================================

Topology optimization repeatedly exports and solves the prepared analysis.
This chapter describes the current plugin requirements and automatic output
configuration. It does not extend topology support to every analysis that
CalculiX itself can solve. See :doc:`topology-objectives` for the objective
definitions and :doc:`../developer/topology-export` for export details.

Supported analysis
------------------

Both compliance and displacement density runs use the shared model validator.
Its current checks require:

* At least one analysis step, with every step static and ``NLGEOM`` absent or
  ``NO``.
* Volume solid sections using C3D4, C3D6, C3D8, C3D10, C3D15, or C3D20.
  C3D8R is rejected for both objectives.
* Isotropic elasticity with one elastic-property row, a positive finite Young's
  modulus, and a finite Poisson ratio strictly between -1 and 0.5. Orthotropic,
  engineering-constant, and anisotropic elasticity are rejected, as is enabled
  plasticity. Temperature-dependent elastic tables are unsupported.
* Step loads limited to force, pressure, and surface traction.
* Displacement boundary conditions only, including model-level BCs, with zero
  prescribed values in the physical analysis. Fixed-DOF flags use the
  exporter's zero-value override.
* No contact-pair or initial-condition definitions.

Displacement optimization additionally requires an existing, nonempty nodeset
and a valid global component or magnitude selection. Adjoint export rejects
custom input lines and currently does not support displacement ``OP=NEW``.
These checks occur at different preparation and export stages; a rejection can
therefore happen after a run has begun. Read the reported error before changing
the model.

The mathematical derivatives assume a symmetric linear-elastic stiffness
matrix, fixed loads, and homogeneous prescribed displacements. Passing the
implemented checks does not prove that arbitrary combinations of constraints,
loads, or other model features satisfy every assumption. In particular, a
supported load category does not establish support for a nonlinear or
density-dependent loading law.

Required output configuration
-----------------------------

Preparation examines existing output definitions and their step assignments.
It reuses suitable definitions, adds missing keys, creates definitions when
needed, and assigns them to the required steps through silent plugin commands.
Information messages report automatic changes. This modifies the model's
output configuration, rather than inserting untracked text into an export.

.. list-table:: Requests prepared by the plugin
   :header-rows: 1
   :widths: 35 65

   * - Case configuration
     - Output requests
   * - Every topology case
     - Element history ``ELSE`` for each participating block in every analysis step.
   * - Displacement objective or a positive displacement monitoring nodeset
     - Element history ``S`` and ``ME`` for each participating block in every step.
   * - Positive displacement nodeset
     - Nodal history ``U`` for that nodeset in every step.
   * - Positive displacement nodeset
     - Nodal displacement field ``U`` in the final analysis step.

Nodal displacement history and the required tensor histories are configured
for global coordinates and frequency one. The displacement field is also
configured for global coordinates and frequency one. History requests must
provide individual values; a totals-only request is insufficient. Automatic
repair can retain totals alongside individual values rather than removing
totals entirely. An ``ELSE``-only request is not universally rewritten to
frequency one by its preparation helper.

This request policy is broader than the minimum mathematical data for one
objective. For example, compliance with displacement monitoring also prepares
``S`` and ``ME``, although its compliance derivative uses element energy.
Displacement jobs can contain ``ELSE`` even when that objective does not
consume it.

What each evaluator reads
-------------------------

.. list-table:: Numerical data sources
   :header-rows: 1
   :widths: 35 65

   * - Calculation
     - Source
   * - Compliance and its raw derivatives
     - Physical ``.dat`` whole-element internal energy ``ELSE``.
   * - Displacement objective or optional measurement
     - Physical ``.dat`` nodal ``U`` for the configured nodeset.
   * - Displacement sensitivity
     - Physical ``.dat`` integration-point ``S`` contracted with adjoint ``.dat`` integration-point ``ME``.
   * - Volume fraction and volume derivatives
     - Element volumes queried directly from Cubit; no ``EVOL`` request is needed.

The readers use the final available solver step/increment in each relevant
result file. A physical solve can contain multiple loading steps. The adjoint
uses its own exported step; its step index need not equal the final physical
step index. Required elements, nodes, and integration points must still be
covered by the selected result data.

Understanding the solver keywords
---------------------------------

In CalculiX, ``*NODE PRINT`` and ``*EL PRINT`` write ASCII ``.dat`` results.
``ELSE`` is whole-element internal energy; ``S`` is stress and ``ME`` is
mechanical strain at integration points. ``*NODE FILE`` writes nodal field
results to ``.frd``; element fields from ``*EL FILE`` are extrapolated and
averaged at nodes. Such averaged fields cannot replace the integration-point
tensors required by this implementation.

These distinctions are documented in the
`CalculiX CrunchiX 2.23 manual <https://dhondt.de/ccx_2.23.pdf>`_, especially
sections 7.52, 7.53, 7.97, and 7.99. Its output-frequency options can interact
across requests, so inspect the final input deck when troubleshooting missing
increments or conflicting output settings.

Exported stiffness and solver jobs
----------------------------------

For each element, export scales Young's modulus by the iteration stiffness
factor and retains Poisson ratio. It generates an elastic material, a
single-element set, and a solid-section assignment. These generated input
definitions do not create one material item per element in the GUI tree.

Each evaluated design owns a physical job. A nonzero displacement objective
also needs an adjoint job with the same iteration stiffnesses and an
objective-derived nodal load. A zero displacement objective uses the implemented
zero-subgradient branch and skips that extra solve.

See :doc:`../developer/topology-export` for annotated physical/adjoint input
excerpts and the implementation map. The official manual is an external
reference, not a bundled runtime dependency. Match its version to the solver
you actually run; the local 2.23 source check does not identify the executable
version used by an older saved result.
