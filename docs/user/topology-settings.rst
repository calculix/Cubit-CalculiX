Topology case settings
======================

Creating and modifying a case
-----------------------------

A case stores settings and preservation selections separately from its iteration
results. All options on the create command are optional:

.. code-block:: text

   ccx create topology 1

This creates case 1 with the core defaults below. Creation does not mesh the
model, create an analysis step, or start optimization. A positive, unused case
ID is required.

Modify supplies only the values that should change:

.. code-block:: text

   ccx modify topology 1 name "beam" volume_fraction 0.7
   ccx modify topology 1 objective displacement displacement_nodeset 3 displacement_component y

Omitted options keep their current values. Modification preserves stored
iterations and preservation selections; it does not recompute earlier results.
Changing settings while the case is running is rejected. Starting the case
again creates a fresh run and clears its previous run data; see
:doc:`topology-running` before rerunning.

Defaults and parameter meanings
-------------------------------

These defaults apply to a newly created case. Existing saved cases retain their
stored settings when read by the supported configuration loader.

.. list-table:: Case settings
   :header-rows: 1
   :widths: 24 16 60

   * - Setting / command option
     - Default
     - Meaning and valid values
   * - Name / ``name``
     - ``topology_<id>``
     - Nonblank case name. Used to construct iteration job names.
   * - Method / ``method``
     - ``density``
     - Density optimization is implemented. ``level_set`` can be stored but cannot currently run.
   * - Objective / ``objective``
     - ``compliance``
     - Compliance and displacement can run. ``mass`` can be stored but cannot currently run.
   * - Volume fraction / ``volume_fraction``
     - ``0.3``
     - Target solid volume divided by total design volume, including preserved elements. Greater than 0 and at most 1; dimensionless.
   * - Void stiffness ratio / ``void_ratio``
     - ``0.001``
     - SIMP stiffness lower bound relative to the base material. Greater than 0 and less than 1; dimensionless. This is not a density threshold for deletion.
   * - Maximum iterations / ``iterations``
     - ``100``
     - Positive integer limiting the number of evaluated designs, including the initial design.
   * - Material cap / ``material_cap``
     - ``1024``
     - Integer of at least 2 limiting generated topology materials across all base materials. Exact sharing is used when possible; otherwise stiffness factors are grouped logarithmically. Applies to new iterations.
   * - Filter radius / ``filter_radius``
     - ``0``
     - Nonnegative length. Zero selects an automatic radius; a positive value uses the model's length units.
   * - Density move limit / ``move_limit``
     - ``0.2``
     - Maximum absolute change of an element's density per update. Greater than 0 and at most 1; dimensionless.
   * - Adaptive density moves
     - Enabled
     - ``adaptive_moves_on`` enables adaptive restriction of density move limits for compliance and displacement; ``adaptive_moves_off`` disables it.
   * - Minimum move limit / ``minimum_move_limit``
     - ``0``
     - Nonnegative and no greater than ``move_limit``. Zero automatically uses 10 percent of the configured maximum. Used by adaptive density moves.
   * - Density tolerance / ``density_tolerance``
     - ``0.01``
     - Maximum density change allowed for convergence; also the ordinary proposal-residual limit (OC for compliance, projected update for displacement). Greater than 0 and at most 1; dimensionless.
   * - Relative objective tolerance / ``compliance_tolerance``
     - ``0.001``
     - Relative objective-change limit for compliance or displacement; 0.001 is 0.1 percent. Greater than 0 and at most 1.
   * - Displacement nodeset / ``displacement_nodeset``
     - ``0``
     - Nonnegative integer. Zero disables displacement monitoring; a positive ID selects a nodeset. A displacement objective requires an existing, nonempty nodeset at run time.
   * - Displacement component / ``displacement_component``
     - ``magnitude``
     - ``x``, ``y``, or ``z`` measures the maximum absolute global component. ``magnitude`` measures the maximum vector norm in the monitored nodeset.

The automatic filter radius is :math:`1.5(V/N)^{1/3}`, using the stored total
element volume :math:`V` and the number of elements :math:`N` in the design.
Zero selects that automatic radius; it does not turn filtering off.

The density floor is currently 0.001 and the SIMP penalization is 3. They are
implementation parameters, not additional create/modify command options.
The density floor and the void stiffness ratio have different roles even
though their default numeric values coincide.

Material grouping
-----------------

The material cap is available through the create/modify commands. It bounds
new topology material definitions; original materials for unaffected sections
remain available. Different base materials are never mixed. Each varying base
material needs at least two groups to preserve its stiffness endpoints. A cap
too small for those endpoints is rejected when preparing an iteration.

Element sets and solid sections are grouped by material within each original
section. Their count can exceed the material cap when original sections have
different orientation or section data. With one original section their count
is bounded by the cap.

The stored iteration factors are the applied factors after grouping, before
text-export rounding of the elastic properties. Continuous densities are retained
for volume and design updates. Their continuous SIMP sensitivities are surrogates
for a grouped solve; the optimizer does not differentiate bin assignments or
regrouping. Text-export and result precision are separate numerical effects.
A single-iteration solver
comparison does not validate full optimization convergence. Existing saved
iterations are never quantized again when loaded or exported.

Optimization settings versus result threshold
---------------------------------------------

.. list-table:: Different controls, different purposes
   :header-rows: 1
   :widths: 30 70

   * - Control
     - Effect
   * - Volume fraction
     - Bounds density-weighted design volume during optimization, including preserved elements.
   * - Void stiffness ratio
     - Controls the SIMP stiffness interpolation; it is not a removal threshold.
   * - Minimum density
     - Bounds free-element design variables; currently an implementation parameter.
   * - Material cap
     - Limits generated material groups and can approximate requested stiffness factors.
   * - Result threshold
     - Classifies retained/removal elements for preview, wrapping and deletion of a chosen iteration.

Changing the result threshold does not rerun optimization or change the stored
densities and applied stiffness factors. It affects how an existing result is
reviewed or applied. See :doc:`topology-results`.

Objective selection and monitoring
----------------------------------

For compliance, a displacement nodeset optionally records a displacement
measurement alongside the compliance objective. It does not change the
quantity being optimized. Setting its ID to zero disables that measurement.

For displacement, the nodeset and component define the objective itself.
The controlling node is selected from that nodeset at each evaluated design;
it need not remain the same throughout optimization. Component results use
absolute values, so a negative Y displacement can still define a positive
objective.

In the create panel, blank optional fields and ``Default`` selections defer
to core defaults. In the modify panel, the loaded values represent the current
configuration; ``Keep current`` omits that option. Select a nodeset before
applying a displacement objective. Mass remains unavailable. Level-set cases
can be configured; their execution remains blocked pending native verification.

The volume target must be feasible with the preserved region and free-element
density floor. The solver model and preservation selections are checked when
preparing the run; valid numeric settings alone do not guarantee a runnable case.

See :doc:`../commands/topology` for accepted syntax and
:doc:`topology-running` for starting, waiting, stopping, and rerunning.

Method-specific settings forms
------------------------------

In navigation, choose **Topology > Create > Density-based / Level set** or
**Topology > Modify > Density-based / Level set**. Each selection opens a complete
independent settings panel. Applying that form supplies its method when creating
or converting a case; entering a panel alone does not modify the model.

Topology Management has a **Method** dropdown in its Settings tab. Choosing a
method immediately displays that method's complete settings widget in the tab.
The stored method changes only after Apply. Switching methods retains drafts;
opening a case initially selects its stored method. Reset clears all drafts,
and case deletion clears that case's drafts. Historical results retain their
captured method and settings when current case settings change.

Level-set fields are optional on creation; blank fields defer to Core defaults.
Explicit zero requests automatic resolution only for the three length controls.
Modifying an existing case retains omitted parameters. The GUI uses the same
create/modify commands as journals, and confirms submitted values in Core before
clearing edits. Density and level-set forms do not share panel widgets.

.. list-table:: Level-set configuration (execution verification pending)
   :header-rows: 1
   :widths: 38 14 48

   * - Keyword
     - Default
     - Range and meaning
   * - ``level_set_interface_width``
     - 0.1
     - Dimensionless regularized interface width, greater than 0 and at most 1.
   * - ``level_set_regularization_length``
     - 0
     - Nonnegative model length; 0 selects automatic regularization.
   * - ``level_set_move_limit``
     - 0.1
     - Dimensionless coefficient move, greater than 0 and at most 2.
   * - ``level_set_field_tolerance``
     - 0.01
     - Dimensionless field-change and ordinary-proposal tolerance, greater than 0 and at most 2.
   * - ``level_set_seed_spacing``
     - 0
     - Nonnegative model length; 0 selects automatic seed spacing.
   * - ``level_set_seed_radius``
     - 0
     - Nonnegative model length; 0 selects automatic radius. A specified radius must be below half the resolved spacing.
   * - ``level_set_integration_tolerance``
     - 0.0001
     - Fraction and derivative refinement tolerance, greater than 0 and less than 1.
   * - ``level_set_integration_max_refinements``
     - 4
     - Integer 1--5. Higher limits can significantly increase geometry integration cost.

The Run tab labels design change according to the latest completed iteration's
captured method. Level-set field change uses that design's captured field
tolerance. The stored Core convergence flag remains authoritative; passing the
three displayed bars alone does not certify convergence.
