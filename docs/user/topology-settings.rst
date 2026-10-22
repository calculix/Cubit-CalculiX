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
   * - Adaptive compliance moves
     - Enabled
     - ``adaptive_moves_on`` enables adaptive restriction of the compliance move limit; ``adaptive_moves_off`` disables it. Displacement updates do not use this adaptation.
   * - Minimum move limit / ``minimum_move_limit``
     - ``0``
     - Nonnegative and no greater than ``move_limit``. Zero automatically uses 10 percent of the configured maximum. Used by adaptive compliance moves.
   * - Density tolerance / ``density_tolerance``
     - ``0.01``
     - Maximum density change allowed for convergence; also the ordinary OC proposal-residual limit for compliance. Greater than 0 and at most 1; dimensionless.
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

The stored iteration factors are the factors actually assigned to the solver.
Continuous densities are retained for volume and design updates; their smooth
SIMP sensitivities approximate the grouped solve. A single-iteration solver
comparison does not validate full optimization convergence. Existing saved
iterations are never quantized again when loaded or exported.

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
applying a displacement objective. Mass and level set remain disabled in the GUI.

The volume target must be feasible with the preserved region and free-element
density floor. The solver model and preservation selections are checked when
preparing the run; valid numeric settings alone do not guarantee a runnable case.

See :doc:`../commands/topology` for accepted syntax and
:doc:`topology-running` for starting, waiting, stopping, and rerunning.
