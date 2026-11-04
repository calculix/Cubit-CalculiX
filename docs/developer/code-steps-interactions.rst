Steps, outputs, constraints and contact
=======================================

Steps and analysis-specific storage
-----------------------------------

``CoreSteps`` owns the analysis sequence and assignments. A step's public ID,
its order in the sequence, analysis kind and type-specific record ID are distinct.
Separate parameter and analysis tables hold static, frequency, buckling, heat
transfer, coupled/uncoupled temperature-displacement, dynamic, modal dynamic,
steady-state dynamics and complex-frequency data. Typed ``StepsSettings`` views
provide the editor/property boundary.

The general step parameters include options such as nonlinear geometry, increment
controls and amplitude convention. Analysis tables have their own time,
frequency, solver and procedure parameters. Do not reuse a positional payload
from one procedure for another without its conversion/validation path. A stored
procedure choice is not proof that a topology objective supports that procedure.

Source: :download:`CoreSteps.hpp <../../src/CalculiX/Core/steps/CoreSteps.hpp>`,
:download:`CoreStepsStructs.hpp <../../src/CalculiX/Core/steps/CoreStepsStructs.hpp>` and
:download:`CalculiXCoreStepExport.cpp <../../src/CalculiX/Core/steps/CalculiXCoreStepExport.cpp>`.

Assignments and deletion
-------------------------

Steps reference load, BC and output definitions. Creation alone does not activate
a definition. Use assignment APIs/commands to add or remove references and use
cleanup helpers such as ``remove_bc_references()`` and
``remove_load_references()`` when deleting definitions. Reference IDs must be
checked against the correct family; force ID 1 and pressure ID 1 do not identify
the same object.

Step export is responsible for the configured analysis and assignments. Feature
preparation should create/assign required normal output definitions through
existing commands instead of injecting an unrelated partial copy of step export.
The topology preparation path demonstrates this distinction. Do not silently
append extra analysis steps merely to make an objective helper convenient.

Field, history and initial data
--------------------------------

``CoreFieldOutputs`` and ``CoreHistoryOutputs`` store requested result keys and
selection/frequency settings. A key must be available in the output definition
and that definition must be assigned to the relevant step. File format, entity
association, coordinates, increment/time and component naming determine how a
reader can consume it. A field output is not automatically a substitute for a
history output containing whole-element or integration-point data.

``CoreInitialConditions``, ``CoreDamping`` and ``CorePhysicalConstants`` have
separate state/export responsibilities. Verify whether a record belongs before
the step sequence or inside a particular procedure. Thermal/structural units,
absolute versus relative temperatures and damping conventions must follow the
selected solver keywords, not a GUI label alone.

Constraints
-------------

``CoreConstraints`` uses a main type table and subtype tables for rigid-body,
tie, equation and grouped-equation definitions. Rigid-body references distinguish
target mesh selection, reference node/vertex and rotational node/vertex. Ties
refer to master/slave selections and tolerance data. Equations store node/DOF/
coefficient terms; grouped equations expand relationships into solver terms.

For a homogeneous linear constraint, the intended relation is

.. math::

   \sum_i a_i u_i=0.

This equation explains the coefficient/DOF contract; it is not a claim that
arbitrary redundant or conflicting constraints are detected by the plugin.
Check mesh existence, term DOFs, reference nodes and supported solver semantics.
Constraint cleanup must update dependent references. Importing a Cubit contact
definition is an explicit adapter workflow, not automatic identity of native
and plugin records.

Source: :download:`CoreConstraints.hpp <../../src/CalculiX/Core/interactions/CoreConstraints.hpp>`
and :download:`CoreCubitContactImport.hpp <../../src/CalculiX/Core/interactions/CoreCubitContactImport.hpp>`.

Surface interactions and contact pairs
---------------------------------------

``CoreSurfaceInteractions`` stores contact-property definitions;
``CoreContactPairs`` associates selections and interaction references. Geometry/
mesh surfaces, solver contact surfaces, interaction property IDs and pair IDs
are different entities. Surface orientation, master/slave roles and formulation
parameters affect the exported problem. Exporting a contact pair does not
guarantee an initially compatible or convergent nonlinear contact model.

General CalculiX contact support must not be inferred as supported topology
compliance/adjoint behavior. Follow the explicit topology run validation for
allowed combinations. See :doc:`topology-preservation` for how participating
constraint/contact selections preserve associated volume elements.

Source: :download:`CoreSurfaceInteractions.hpp <../../src/CalculiX/Core/interactions/CoreSurfaceInteractions.hpp>`
and :download:`CoreContactPairs.hpp <../../src/CalculiX/Core/interactions/CoreContactPairs.hpp>`.

Extension and verification
---------------------------

For a new procedure/output/contact option, update parser syntax and presence
handling, Core defaults/ranges, typed settings, editor and properties, cleanup,
persistence and export together. Verify the exact generated keyword placement
against the versioned solver reference. Test empty assignments, deleted targets,
multiple steps and incompatible analyses. An input accepted by Cubit's parser
has not yet been validated by CalculiX.
