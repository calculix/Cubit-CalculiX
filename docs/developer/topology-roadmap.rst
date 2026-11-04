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
  their supported quadratic variants. Native execution verification is pending.

* Batch 4a foundation is implemented: direct Cubit queries, resolved lengths,
  deterministic seeded initial fields, fixed preserved nodes, requested stiffness
  factors, and nodal compliance/volume derivative assembly. Helpers return local
  records only; batch 5 adds the internal interface/controller bridge. Native
  execution is pending. See :doc:`topology-level-set` for assumptions/limits.

* Batch 4b foundation is implemented: sparse element-graph regularization,
  bounded convex local updates, signed linearized volume constraints, actual
  volume backtracking/restoration and source-owned ordinary proposal diagnostics.
  Native C++/Cubit verification and performance testing remain pending.

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

* Batch 8a verification sources are supplied: a read-only Python settings query,
  a configuration/save-reset-reopen journal and a separate manual GUI checklist.
  Native checks remain unexecuted; the journal does not run an optimizer or
  verify historical iteration persistence. See :doc:`topology-verification`.
* Batch 8: independent geometry and finite-difference verification, solver
  comparisons, mixed/higher-order meshes, performance checks, lifecycle tests,
  user documentation, journals and screenshots.

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
