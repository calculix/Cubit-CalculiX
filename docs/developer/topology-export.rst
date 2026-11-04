Topology export and result contract
===================================

This chapter maps topology preparation, input generation, and result consumption
to the current implementation. See :doc:`topology-algorithms` for derivatives
and integration conventions, :doc:`topology-results` for stored records, and
:doc:`../user/topology-solver` for user-facing restrictions.

Implementation map
------------------

.. list-table:: Current export and result entry points
   :header-rows: 1
   :widths: 25 40 35

   * - Responsibility
     - Source
     - Entry points
   * - Shared model validation
     - :download:`Core/topology/CalculiXCoreTopologyValidation.cpp <../../src/CalculiX/Core/topology/CalculiXCoreTopologyValidation.cpp>`; :download:`Core/topology/CoreTopologyModelValidation.cpp <../../src/CalculiX/Core/topology/CoreTopologyModelValidation.cpp>`
     - ``validate_topology_compliance_model`` delegates to ``CoreTopologyModelValidation::validate_model`` for both objectives.
   * - Required output preparation
     - :download:`Core/topology/CalculiXCoreTopologyOutputs.cpp <../../src/CalculiX/Core/topology/CalculiXCoreTopologyOutputs.cpp>`; :download:`Core/topology/CoreTopologyOutputPreparation.cpp <../../src/CalculiX/Core/topology/CoreTopologyOutputPreparation.cpp>`
     - ``prepare_topology_history_outputs`` delegates to ``CoreTopologyOutputPreparation::prepare``; ``ensure_topology_*_output`` wrappers call the element-history, node-history and field-output helpers.
   * - Iteration material export
     - :download:`Core/materials/CoreMaterials.cpp <../../src/CalculiX/Core/materials/CoreMaterials.cpp>`
     - ``get_topology_material_export``, ``build_topology_material_groups``, ``get_topology_group_material_name``.
   * - Element section export
     - :download:`Core/sections/CoreSections.cpp <../../src/CalculiX/Core/sections/CoreSections.cpp>`
     - ``get_topology_solid_section_export``.
   * - Export context validation
     - :download:`Core/topology/CalculiXCoreTopologyValidation.cpp <../../src/CalculiX/Core/topology/CalculiXCoreTopologyValidation.cpp>`; :download:`Core/topology/CoreTopologyExportValidation.cpp <../../src/CalculiX/Core/topology/CoreTopologyExportValidation.cpp>`; :download:`Core/jobs/CalculiXCoreJobs.cpp <../../src/CalculiX/Core/jobs/CalculiXCoreJobs.cpp>`
     - ``check_topology_export`` delegates to ``CoreTopologyExportValidation::check``; ``is_topology_adjoint_export`` reads the active job export context.
   * - Physical step export
     - :download:`Core/steps/CalculiXCoreStepExport.cpp <../../src/CalculiX/Core/steps/CalculiXCoreStepExport.cpp>`; :download:`Core/steps/CoreStepExport.cpp <../../src/CalculiX/Core/steps/CoreStepExport.cpp>`
     - ``CalculiXCore::get_step_export_data`` selects the physical or adjoint path. ``CoreStepExport::get_export`` assembles physical steps.
   * - Adjoint step export
     - :download:`Core/topology/CoreTopologyAdjointStepExport.cpp <../../src/CalculiX/Core/topology/CoreTopologyAdjointStepExport.cpp>`; :download:`Core/topology/CoreTopologyAdjointBoundaries.cpp <../../src/CalculiX/Core/topology/CoreTopologyAdjointBoundaries.cpp>`; :download:`Core/topology/CoreTopologyAdjointOutputs.cpp <../../src/CalculiX/Core/topology/CoreTopologyAdjointOutputs.cpp>`
     - ``get_export`` constructs one adjoint step from the final physical step; ``collect`` resolves constrained DOFs and ``append_history``/``append_field`` write assigned outputs.
   * - Scalar result reading
     - :download:`Core/topology/CalculiXCoreTopologyResults.cpp <../../src/CalculiX/Core/topology/CalculiXCoreTopologyResults.cpp>`; :download:`Core/topology/CoreTopologyResults.cpp <../../src/CalculiX/Core/topology/CoreTopologyResults.cpp>`; :download:`Core/topology/CoreTopologyDisplacementResults.cpp <../../src/CalculiX/Core/topology/CoreTopologyDisplacementResults.cpp>`
     - ``read_topology_element_energies`` and ``read_topology_displacement`` delegate to the result helpers.
   * - Tensor result reading
     - :download:`Core/topology/CalculiXCoreTopologyResults.cpp <../../src/CalculiX/Core/topology/CalculiXCoreTopologyResults.cpp>`; :download:`Core/topology/CoreTopologyTensorResults.cpp <../../src/CalculiX/Core/topology/CoreTopologyTensorResults.cpp>`
     - ``read_topology_job_element_tensors`` delegates to ``CoreTopologyTensorResults::read_job`` and ``read_final``.
   * - Geometry and quadrature
     - :download:`Core/topology/CalculiXCoreTopologyGeometry.cpp <../../src/CalculiX/Core/topology/CalculiXCoreTopologyGeometry.cpp>`; :download:`Core/topology/CoreTopologyGeometry.cpp <../../src/CalculiX/Core/topology/CoreTopologyGeometry.cpp>`; :download:`Utility/ElementIntegration.cpp <../../src/CalculiX/Utility/ElementIntegration.cpp>`; :download:`Utility/ElementNodeOrdering.hpp <../../src/CalculiX/Utility/ElementNodeOrdering.hpp>`
     - ``collect_topology_integration_weights`` delegates to ``collect_integration_weights``; ``ElementIntegration::calculate_weights`` uses the shared node-order conversion.
   * - Objective evaluation
     - :download:`Core/topology/CoreTopologyOptimization.cpp <../../src/CalculiX/Core/topology/CoreTopologyOptimization.cpp>`; :download:`Core/topology/CalculiXCoreTopologyEvaluation.cpp <../../src/CalculiX/Core/topology/CalculiXCoreTopologyEvaluation.cpp>`; :download:`Core/topology/CoreTopologyEvaluation.cpp <../../src/CalculiX/Core/topology/CoreTopologyEvaluation.cpp>`; :download:`Core/topology/CoreTopology.cpp <../../src/CalculiX/Core/topology/CoreTopology.cpp>`
     - The controller selects the objective and contracts matching integration-point data; coordination helpers supply applied factors to the numerical evaluation.
   * - Input-deck assembly
     - :download:`Commands/export/ccxExportCommand.cpp <../../src/CalculiX/Commands/export/ccxExportCommand.cpp>`
     - ``write_file``, ``write_materials``, ``write_sections``, and ``write_steps`` consume interface exports in deck order.

Cross-core access remains through the domain-specific interfaces (see :doc:`interface-layout`). The export helpers
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

An iteration stores applied stiffness factors independently of the base
material definitions. Continuous SIMP factors are requested from densities;
the grouping operation produces the factors actually assigned to the solver.
For element :math:`e`, export uses

.. math::

   E_e=\widehat{s}_eE_{\mathrm{base}},\qquad
   \nu_e=\nu_{\mathrm{base}},\qquad
   s_e=v+(1-v)\rho_e^p.

Here :math:`\widehat{s}_e` is the applied factor after exact sharing or
logarithmic grouping. New material names are
``CCX_TOPO_<case>_G<fixed-width-group>``. Element-set names also include the
original section ID. The material exporter emits one ``*MATERIAL`` and
``*ELASTIC`` per group; the section exporter emits one ``*ELSET`` and
``*SOLID SECTION`` per occupied group within an original section. Orientation
and section data remain separate. Elastic properties use scientific notation
with six digits after the decimal point. This formatting introduces rounding
relative to stored double-precision values; it does not guarantee round-trip
precision. Six decimals is the plugin's formatting policy, not a universal
CalculiX decimal limit.

Grouping and text rounding are separate approximations. The material core
stores the grouped factor before text formatting; the sensitivity evaluator
normalizes by that stored factor. It does not recover a new factor from the
rounded exported modulus. Even exact factor sharing therefore leaves numerical
input/result precision to consider when comparing derivatives.

Historical iterations without grouping metadata retain the legacy
``CCX_TOPO_<case>_E<element>`` material names and six-decimal scientific
material export. Applied factors are never quantized again on reload.

These definitions are generated solver input. The material core stores base
assignments and iteration factors rather than registering a GUI material for
every element. Participating sections use their topology export path rather
than also emitting their normal block-wide assignment. See
`Grouped material export`_ for budgets, caching and persistence.

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
conditions, and history/field outputs. Multiple physical steps remain possible.
The readers require the requested quantities at the final DAT step/increment
recorded by the parser. Missing data at that final frame is an error; they do
not silently fall back to an earlier frame containing the requested quantity.

For a nonzero displacement objective, ``CalculiXCore::get_step_export_data``
in ``Core/steps/CalculiXCoreStepExport.cpp`` rebuilds the adjoint load and
delegates to ``CoreTopologyAdjointStepExport::get_export``. Together these:

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

Grouped material export
-----------------------

New iterations capture ``material_cap`` (default 1024) and store the applied
solver factors in ``TopologyMaterialData::iteration_factors``. The grouping
budget is shared across base materials; one group is reserved for constant
factors and two for a varying range, followed by round-robin distribution of
remaining capacity. Exact factors are shared when they fit. Otherwise nearest
logarithmic bins preserve each base material's minimum and maximum factors.

For one base material, let :math:`B\ge2` be its allocated budget and let
:math:`s_{\min}<s_{\max}` be the extrema of its requested factors. When the
number of distinct factors exceeds that budget, the implementation uses

.. math::

   t_e=\operatorname{clamp}\!\left(
   \frac{\log s_e-\log s_{\min}}{\log s_{\max}-\log s_{\min}},0,1
   \right),\qquad
   b_e=\operatorname{clamp}\!\left(
   \left\lfloor(B-1)t_e+\tfrac12\right\rfloor,0,B-1\right),\\
   \widehat{s}_e=
   \exp\!\left(\log s_{\min}
   +\frac{b_e}{B-1}(\log s_{\max}-\log s_{\min})\right).

The nonnegative bin-position rounding corresponds to ``std::llround`` in the
code. The first and last bins use the stored minimum and maximum directly,
rather than recomputing those endpoints through logarithms/exponentials;
interior results are clamped to that range. Constant ranges and factor sets
that fit their budget use exact sharing without this quantization. Here
:math:`B` is the budget of one base material, not the global ``material_cap``.
The extrema are the factors present in this iteration, not universal SIMP
bounds such as :math:`v` and one.

The reported maximum relative factor error is

.. math::

   \delta_{\max}=\max_e\left|\frac{\widehat{s}_e}{s_e}-1\right|.

The informational percentage is :math:`100\delta_{\max}`. It measures the
change from requested to stored applied factors; it does not include modulus
text rounding, solver precision or result-file precision. The log-bin index
:math:`b_e` in this section is unrelated to the normalized OC benefit using
the same local symbol in :doc:`topology-algorithms`.

``iteration_material_caps`` identifies grouped iterations and is persisted.
``iteration_material_groups`` is a transient cache for one iteration per
case. Reloading rebuilds this cache with exact sharing of the saved factors,
without applying a new cap or quantizing them again. The current save layout requires a cap marker for every stored factor map.

The grouped section exporter retains original orientations and section data.
It generates one element set and solid section per occupied material within
an original section. The original export helpers remain available with the
``_legacy`` suffix. Group names have fixed-width numeric suffixes.

Compliance and displacement normalization use applied solver factors. Density
updates retain continuous SIMP derivatives as surrogate sensitivities for the
quantized solve; these are not exact derivatives through the binning operation.
The current unversioned topology layout stores the material cap and historical
applied factors; older topology layouts are not migrated.
