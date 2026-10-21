Topology export and result contract
===================================

This chapter maps topology preparation, input generation, and result consumption
to the current implementation. See :doc:`topology-algorithms` for derivatives
and integration conventions, :doc:`topology-results` for stored records, and
:doc:`../user/topology-solver` for user-facing restrictions.

Implementation map
------------------

.. list-table:: Entry points
   :header-rows: 1
   :widths: 30 70

   * - Responsibility
     - Source and operation
   * - Shared model validation
     - ``CalculiXCore.cpp``: ``validate_topology_compliance_model``. Despite its name, both objectives use it.
   * - Required output preparation
     - ``CalculiXCore.cpp``: ``prepare_topology_history_outputs`` and the ``ensure_topology_*_output`` helpers.
   * - Iteration material export
     - ``CoreMaterials.cpp``: ``get_topology_material_export`` and ``get_topology_material_name``.
   * - Element section export
     - ``CoreSections.cpp``: ``get_topology_solid_section_export``.
   * - Export context validation
     - ``CalculiXCore.cpp``: ``check_topology_export`` and ``is_topology_adjoint_export``.
   * - Physical and adjoint step export
     - ``CalculiXCore.cpp``: ``get_step_export_data``; the adjoint branch builds one step from the final physical step definition.
   * - Scalar and tensor result reading
     - ``CalculiXCore.cpp``: ``read_topology_element_energies``, ``read_topology_displacement``, and ``read_topology_job_element_tensors``.
   * - Objective evaluation
     - ``CoreTopologyOptimization.cpp`` dispatches compliance or displacement evaluation and checks integration coverage.

Cross-core access remains through ``CalculiXCoreInterface``. The export helpers
return empty strings on invalid required data, and their callers must treat
that as failure rather than continue with an incomplete input deck.

Validation scope and timing
---------------------------

The shared validator requires static steps, small-displacement settings,
standard supported solid element types, supported mechanical loads,
displacement BCs, and no contact pairs or initial conditions. Material
initialization separately checks the supported isotropic elastic properties.
See :doc:`../user/topology-solver` for the complete current list.

Physical preparation uses the default zero-prescribed-displacement requirement.
Adjoint export calls the shared validator with ``require_zero_displacements``
false because it exports homogeneous constrained DOFs itself. This does not
add physical support for nonzero imposed displacements. The adjoint branch
also rejects custom input lines and rejects displacement ``OP=NEW`` during
boundary collection.

These checks are distributed among preparation, iteration creation, and export.
They do not form a transaction rolling back all earlier run changes. The
mathematical assumptions of fixed loads and symmetric linear elasticity still
apply even where no dedicated validator tests every possible model feature.

Materials and sections
----------------------

An iteration stores stiffness factors independently of the base material
definitions. For element :math:`e`, export uses

.. math::

   E_e=s_eE_{\mathrm{base}},\qquad
   \nu_e=\nu_{\mathrm{base}},\qquad
   s_e=v+(1-v)\rho_e^p.

The material name is ``CCX_TOPO_<case>_E<element>``. Its set name appends
``_SET``. The material exporter emits ``*MATERIAL`` and ``*ELASTIC``; the section
exporter emits the single-element ``*ELSET`` and its ``*SOLID SECTION`` using
that name. Existing applicable section information is carried by the section
exporter. Export validates factor coverage and material assignments before
using them. Floating-point elastic properties use scientific notation with
six digits after the decimal point.

The element-specific definitions are generated solver input. The material core
holds a topology collection of base assignments and iteration factors rather
than registering a normal GUI material for every element. Participating sections
are exported through their topology path rather than also emitting their normal
block-wide assignment.

Output preparation contract
---------------------------

For every analysis step and participating block, prepare ``ELSE``. Also prepare
``S`` and ``ME`` when the objective is displacement or the case has a positive
displacement nodeset. With such a nodeset, prepare nodal history ``U`` in all
steps and nodal field ``U`` in the final step.

The helpers first consider assigned matching outputs, then other matching
definitions, before creating a new definition. They add the needed key and
assignment through ``silent_cmd`` and verify the resulting configuration.
They preserve individual output, repairing ``TOTALS=ONLY`` when needed.
An existing totals-only definition can become ``TOTALS=YES``; newly configured
definitions need not use that same totals setting.

Tensor histories use ``GLOBAL=YES`` and ``FREQUENCY=1``. Nodal displacement
history and the required displacement field receive those settings as well.
The ``ELSE`` helper does not impose the tensor frequency/global settings on an
energy-only request. Do not document all output requests as universally
frequency one. Solver-level interactions between frequency/time-point options
must also be considered when inspecting the complete exported deck.

Request configuration and evaluation consumption are distinct. Compliance reads
physical ``ELSE``; displacement measurement reads physical nodal ``U`` from
DAT; displacement derivatives combine physical ``S`` with adjoint ``ME`` from
DAT. Extra requested fields may be present without being used by that objective.

Physical and adjoint steps
--------------------------

The ordinary physical step export supplies its assigned loads, boundary
conditions, and history/field outputs. Multiple physical steps remain possible;
the readers select the final available solver result step/increment.

For a nonzero displacement objective, the adjoint branch of
``get_step_export_data``:

#. Rebuilds the objective-derived global nodal load at the controlling node.
#. Collects model-level and step-assigned constrained DOFs under the supported
   accumulated ``OP=MOD`` behavior.
#. Gets the final physical step definition from ``CoreSteps::get_step_export``.
#. Writes those constrained DOFs with zero values under ``*BOUNDARY``.
#. Writes the objective-derived load under ``*CLOAD``.
#. Exports the final physical step's assigned history and field output definitions.
#. Closes the single adjoint step with ``*END STEP``.

It uses the same iteration stiffness factors as the physical solve. The
physical loading is not the adjoint right-hand side. For component objectives
the active load is the displacement sign; for magnitude it is the normalized
three-component displacement. At zero objective the controller uses zero
integrals and skips the adjoint job.

The adjoint has its own result step numbering, so physical and adjoint step IDs
need not coincide. Select the final result frame independently in each file;
match element and integration-point identities for the contraction. Complete
coverage and finite global-coordinate tensors are required. Nodal field stress
cannot substitute for the required integration-point stress.

Annotated beam excerpts
-----------------------

The following files contain selected blocks from existing local beam inputs:
``topo_1_simple_beam_displacement_iter_0002.inp`` and
``topo_1_simple_beam_displacement_iter_0002_adjoint.inp``. Source filenames are
recorded in their comments. No solver run or fresh export was performed to
produce these excerpts. They illustrate a recorded export, not a guaranteed
byte-for-byte deck from a future run.

Only one material/section pair and selected constraint/step blocks are retained.
Node definitions, connectivity, referenced sets, and other materials are omitted.
The excerpts are incomplete and cannot be solved by themselves. Their IDs and
set names belong to that recorded model, not every execution of the tutorial.

Physical input
~~~~~~~~~~~~~~

.. literalinclude:: ../examples/topology-physical.inp
   :language: text

The elastic modulus shown is already scaled. The physical ``*CLOAD`` carries
the beam load, and the output block contains both required keys and existing
additional selections. ``E`` and ``RF`` here do not become new topology
requirements just because they appear in the recorded deck.

Adjoint input
~~~~~~~~~~~~~

.. literalinclude:: ../examples/topology-adjoint.inp
   :language: text

Compare the homogeneous boundary values and the objective-derived ``*CLOAD``
with the physical excerpt. Matching material names/factors show that the
adjoint evaluates the same density design. Its extra output keys come from
the assigned output definitions; the displacement contraction consumes ``ME``
from this solve and ``S`` from the physical solve.

Solver references and version scope
-----------------------------------

The reference for keyword meanings is the
`official CalculiX CrunchiX 2.23 manual <https://dhondt.de/ccx_2.23.pdf>`_:
``*ELASTIC`` (7.47), ``*EL FILE`` (7.52), ``*EL PRINT`` (7.53), ``*NODE FILE``
(7.97), ``*NODE PRINT`` (7.99), and ``*SOLID SECTION`` (7.119).
The manual distinguishes integration-point print results from extrapolated
nodal element fields, and whole-element energy from energy density.

Local CalculiX 2.23 parser sources ``elprints.f`` and ``nodeprints.f`` provide
additional implementation checks; the integration routines are described in
:doc:`topology-algorithms`. This reference version does not identify the solver
executable that produced an older local result. Record that executable's version
separately when collecting verification evidence. The external manual is not
bundled with the offline documentation.
