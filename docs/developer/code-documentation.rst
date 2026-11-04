Project code documentation
==========================

Scope and reading paths
-----------------------

This reference documents the project-owned Framework and CalculiX code. It
describes current source behavior, ownership and integration points; it is not
a claim that the current source has compiled or passed native verification.
No production source was changed to add these chapters. Existing Doxygen
comments remain unchanged; the external chapters supply the additional contracts.

.. toctree::
   :maxdepth: 1

   code-framework
   code-application
   code-mesh-materials-sections
   code-loads-bcs
   code-steps-interactions
   code-jobs-export
   code-results
   code-gui
   code-utilities-python
   code-api-index
   code-catalog/index
   numerical-verification-plan
   review-findings

.. list-table:: Choose an entry point
   :header-rows: 1
   :widths: 28 72

   * - Work
     - Read first
   * - Change startup or shared services
     - :doc:`code-framework`, then :doc:`framework-stage1` and :doc:`user-options`.
   * - Add or modify a model feature
     - :doc:`code-application`, the corresponding domain chapter and :doc:`../adding-a-feature`.
   * - Change a command or panel
     - :doc:`code-gui`, then :doc:`code-catalog/index` for the matching command and widget.
   * - Change input writing or job behavior
     - :doc:`code-jobs-export`, including error, cleanup and identifier contracts.
   * - Change result reading or conversion
     - :doc:`code-results` and :doc:`vtk-conversion`.
   * - Change topology mathematics
     - :doc:`topology-algorithms`, :doc:`topology-level-set` and :doc:`topology-verification`.
   * - Locate any implementation file
     - :doc:`code-catalog/index`; extracted declarations are in :doc:`code-api-index`.

Included and excluded code
--------------------------

``src/Framework`` includes entry points, providers, lifecycle, workspace hosting,
options/model storage adapters, shared result ownership and project utilities.
``src/CalculiX`` includes its module, domains, commands, GUI, observers, Python
interface and project-owned adapters in its Utility directory.

The internal implementation of bundled libraries is excluded: CGAL, Eigen,
HDF5, OpenPisco and ``gif.h`` beneath ``src/Utility``; binary libraries beneath
``src/Library``; and bundled executables beneath ``src/Solvers``. Their project
adapters are documented only at the boundary: accepted inputs, ownership,
outputs, errors and deployment requirements. SDK/Qt/VTK implementation code is
also outside this reference. External licenses and the attribution already in
the repository remain applicable.

The source catalogue lists owned implementation/declaration/build files. Icon
and other resource assets are explained through their owning loader rather than
treated as C++ APIs. A filename in the catalogue establishes navigation coverage,
not that every branch in that file has been audited. Doxygen declaration
extraction is not a substitute for semantic contracts or numerical evidence.

Cross-cutting contracts
-----------------------

* ``CalculiXCore`` owns domain state. Commands, GUI and Python use the named
  interfaces; an interface instance is not another model. Cross-domain changes
  go through those interfaces and existing coordination methods.
* Cubit owns geometry and mesh identities. Distinguish entity kind/local ID,
  global node/element ID, block/set ID, feature ID, row index, job ID and topology
  iteration ID. Equal integer values do not make these domains interchangeable.
* Defaults belong to the relevant Core or declared workspace option definition.
  Presence flags distinguish omitted edits from explicit zero values. Displayed
  placeholders do not initialize model values.
* GUI actions submit commands. A successful GUI submission means that a command
  was delivered, not necessarily that Core accepted it. Editors that confirm
  stored values must retain edits after an unconfirmed operation. Previews use
  the existing silent command path.
* Return/error behavior is operation-specific: false, a negative ID or an empty
  collection/string can signal failure with a Cubit error. An empty optional
  export can also be valid. Read the particular function and caller; do not
  impose an exception-based convention on existing APIs.
* Model saving, solver input export, process completion, result loading and VTK
  publication are different operations. Success of one does not certify the next.
  A failed operation is not automatically a rollback of earlier mutations.
* Host/Qt/mesh API work stays on its supported host thread. Worker code must use
  detached numeric/file data and join before owners or code modules disappear.
  Published converted results must not borrow mutable reader storage.

Units and mathematical documentation
------------------------------------

The plugin writes values in the model's consistent unit system. It does not
infer a complete force/length/time/temperature system from a number entered in a
panel. Describe the quantity and required consistency rather than attaching an
unconditional SI unit. For example, density has mass/length cubed, stress has
force/length squared, and whole-element strain energy has force times length.

Document a numerical method with its objective, independent variables, units,
assumptions, discretization, derivative convention, bounds, stopping criteria
and failure behavior. Separate exact derivatives from filtered, grouped or
rounded surrogates. A visual result or a small update is not an optimality proof.
The topology chapters provide the detailed worked example of this format.

CalculiX keyword semantics and plugin support are separate. A workspace exporter
implements a subset and specific combinations. Cite the versioned solver manual
through the existing :doc:`../user/topology-solver` and :doc:`topology-export`
reference pages; do not describe a whole solver capability as implemented merely
because a similarly named parameter is stored.

Maintenance and evidence
-------------------------

Review :doc:`review-findings` before changing affected paths. Findings distinguish
source-confirmed issues, API hazards and coverage blockers. They were not fixed
as part of this documentation pass. Update source links, catalogue entries and
manual contracts when code moves. Record an actual test outcome only after the
corresponding built plugin, platform and model have been exercised.
