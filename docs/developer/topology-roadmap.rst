Topology implementation roadmap
===============================

The unstructured C++ level-set feature is staged. Existing density compliance
and displacement runs remain the supported production methods.

Current foundation
------------------

* Batch 1: document the representation, mathematical assumptions, integration
  contract, limitations and verification gates in :doc:`topology-level-set`.
* Batch 2: introduce separate settings and in-memory level-set design records;
  initialize defaults in Core. Batch 6 adds serialization; command editing
  is implemented in batch 7. No public level-set optimizer is enabled.
* Batch 3: share element interpolation/Jacobians and introduce regularized
  solid-fraction and derivative evaluation for tet, wedge and hex, including
  their supported quadratic variants. Straight-sided native math tests passed on Windows.

* Batch 4a foundation is implemented: direct Cubit queries, resolved lengths,
  deterministic seeded initial fields, fixed preserved nodes, requested stiffness
  factors, and nodal compliance/volume derivative assembly. Helpers return local
  records only; batch 5 adds the internal interface/controller bridge. Native
  helper tests passed on Windows; physical controller verification remains pending. See :doc:`topology-level-set` for assumptions/limits.

* Batch 4b foundation is implemented: sparse element-graph regularization,
  bounded convex local updates, signed linearized volume constraints, actual
  volume backtracking/restoration and source-owned ordinary proposal diagnostics.
  Native helper verification passed on Windows; controller and performance testing remain pending.

* Batch 5 internal integration is implemented: separate proposal/acceptance,
  interface-based design/factor/evaluation storage, method dispatch, current
  convergence checks, transient proposal ownership and reset cleanup. Existing
  jobs/export/wait/kill paths are reused. The public level-set run gate
  remains disabled pending native verification; batch 7 adds configuration GUIs.

* Batch 6 is implemented at source level: settings, fields, fractions and
  diagnostics use one required unversioned save layout. Old topology layouts are
  not migrated. Queries dispatch by historical method and preserve their row
  layouts; rerun/delete/reset clear level-set history with common run data.
  Native save/load verification remains pending.

* Batch 7 is implemented at source level: optional method settings in commands,
  separate complete navigation panels and management settings widgets, Method
  selection, draft lifecycle and captured level-set field tolerance in progress.
  Public execution remains blocked. Native GUI/parser verification is pending.

Remaining implementation
------------------------

* Configuration, density integration/lifecycle and density-history persistence
  have local November 2026 passing evidence. Manual GUI and level-set history
  checks remain separate. See :doc:`topology-verification` for scope/provenance.
* The physical-solve fixed-material-group derivative matrix passed all 18
  cases: six formulations with compliance, absolute Y and displacement magnitude.
  It does not establish density/filter or level-set nodal derivatives.
* Native interpolation/geometry tests passed on Windows after the bounded
  Heaviside endpoint fix. Linux, curved/mixed and narrow-band accuracy checks
  remain separate. Binary provenance limits are recorded with the evidence.
* Native nodal compliance/volume accumulation and normalization passed the
  Windows analytical suite: six formulations, shared nodes, unequal weights and
  preserved neighbors. The complete live-Cubit evaluation remains separate.
* The existing numerical QP solver is extracted into CoreTopologyLevelSetQP.
  Seven analytical solution/KKT cases and failure/limit tests passed on Windows.
  Graph assembly and actual-volume acceptance remain in the
  live-mesh update code. The local tests directory remains Git-ignored.
* The developer-only live C3D8 batch passed all 382 checks: graph KKT,
  actual-volume acceptance, backtracking, restoration, preservation and source
  residual ownership. No production command or public-gate bypass was introduced.
* The expanded live matrix passed all 18 groups and 2034 checks after the
  C3D10 refinement regression and five-refinement test budget. Tolerance and
  production defaults stayed unchanged. Mandatory restoration/backtracking
  coverage remains explicitly C3D8-only.
* Standalone initialization and the 18 live seed/update fixtures passed on
  Windows: 36 phases, 6561 checks, zero failures. Broad-band complete geometry
  remains distinct from raw-default seeds and default-width certification.
* The six-formulation broad-band physical controller/persistence matrix passed
  all 35 stages on Windows, including C3D8 lifecycle tests. Sensitivity-to-proposal
  cache handoff retained exact historical fields, factors and evaluations.
* Live cache equivalence run 20261109T121528Z_72c9d6ef passed 18 fixtures /
  36 phases / 6781 checks with zero failures. The progress-start hang no longer
  reproduced after switching the live loader to CDLL. Diagnostic tracing is now
  opt-in, default OFF in the journal; no extra CMake option is introduced.
* A pass-local, bounded reference quadrature cache and precomputed-reference
  mapping path passed all seven native suites and the 35-stage Windows physical
  controller regression (20261109T133442Z_73ec62b3), retaining exact fields,
  element results and histories against baseline. All 31 export commands returned
  successfully; the earlier export exception remains unexplained. The full live
  update regression after quadrature reuse and clean timing comparison are pending.
* The 60-iteration C3D8 broad-band run 20261109T140032Z_dd5a3f68 passed execution
  and reached volume feasibility at iteration 6, but did not converge. Fields
  continued reversing at the fixed 0.1 move limit. The journal now prepares a
  controlled 0.05 comparison with unchanged tolerances and explicit oscillation
  diagnostics; that comparison is not yet executed.
* Next: compare the smaller fixed move limit before deciding whether adaptive
  level-set moves are needed. Then extend the study to other element types and
  expand default-width scope using the documented numerical gates.
* Remaining: kill after completed iterations, solver-process failure cleanup,
  mixed/curved meshes, narrow-band refinement, larger-model performance, GUI
  lifecycle and Linux evidence. The standalone cache-suite result is separate.
* Review the required evidence before enabling the supported level-set scope.
  Public runs remain gated; current level-set integration supports compliance
  only. Passing density displacement tests does not add level-set displacement.

OpenPisco comparison and removal reminder
-----------------------------------------

The component must not require users to install OpenPisco, Muscat, SciPy or
another Python library. The implementation does not import OpenPisco.

Keep these explicit follow-up tasks open:

* After independent C++ verification, compare suitable compliance cases against
  OpenPisco in a developer-controlled reference environment. Match mesh/domain,
  loads, supports, preservation, volume definition and material interpolation
  where possible. Record versions, settings and deviations in formulation.
* Compare objective, volume and refinement behavior. Do not require identical
  shapes or iteration histories when the evolution/interpolation differs.
  OpenPisco is an additional comparison, not the only correctness oracle.
* Preserve the comparison inputs, results and interpretation as verification
  evidence before removing a reference implementation.
* **Reminder: review removal of src/Utility/openpisco-master after the comparison.**
  Search build lists, runtime imports, scripts, examples, packaging and
  documentation first. Ask the user before removing the bundled source.
  Remove it only if no required runtime or developer workflow depends on it;
  retain relevant method citations and verification evidence.

This checklist is the durable reminder. It does not schedule an external
notification or authorize source deletion.

The default-OFF developer controller capability and passing Windows broad-band
controller/persistence evidence are specified in :doc:`topology-verification`.
This capability does not enable production or GUI level-set runs.
