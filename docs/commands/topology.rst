Topology configuration and run control
======================================

Create a topology case
----------------------

.. code-block:: text

   ccx create topology <id>
     [name "<name>"]
     [method {density|level_set}]
     [objective {compliance|mass|displacement}]
     [volume_fraction <value>]
     [void_ratio <value>]
     [iterations <count>]
     [material_cap <count>]
     [filter_radius <length>]
     [move_limit <value>]
     [{adaptive_moves_on|adaptive_moves_off}]
     [minimum_move_limit <value>]
     [density_tolerance <value>]
     [compliance_tolerance <value>]
     [displacement_nodeset <id>]
     [displacement_component {x|y|z|magnitude}]

This is a wrapped presentation of **one command**, not a multiline journal
command. Square brackets indicate optional syntax; braces indicate choices.
Do not type the brackets or braces. Use a positive, unused topology ID.
All options may be omitted; creation applies the defaults in
:doc:`../user/topology-settings`. Creating a case does not run it.

Modify a topology case
----------------------

.. code-block:: text

   ccx modify topology <id> [options]

Modify accepts the same optional arguments as create, listed above. Supply
an existing case ID. Omitted options retain their current values. Stored
iterations and preservation selections remain; earlier evaluations are not
recomputed. Modification while running is rejected.

Shared configuration options
----------------------------

The :doc:`settings table <../user/topology-settings>` defines every default,
valid numeric range, unit, and parameter meaning for both commands.

* ``method`` defaults to ``density``. ``level_set`` is accepted as a
  configuration value but cannot currently run.
* ``objective`` defaults to ``compliance``. ``displacement`` is implemented;
  ``mass`` is accepted as a configuration value but cannot currently run.
* ``material_cap`` defaults to 1024 and must be an integer of at least 2.
  It limits generated topology materials across base materials. It applies to
  new iterations; original section settings remain separate.
* ``filter_radius 0`` selects the automatic radius; it does not disable filtering.
* ``adaptive_moves_on`` and ``adaptive_moves_off`` are individual tokens.
  There is no ``adaptive_moves on`` command form. The default is enabled.
* ``minimum_move_limit 0`` uses 10 percent of the maximum move limit.
* ``compliance_tolerance`` is the compatibility keyword for the relative
  objective tolerance, including displacement optimization. For example,
  ``0.001`` means 0.1 percent. The GUI labels it Relative objective tolerance.
* ``displacement_nodeset 0`` disables optional monitoring for compliance.
  Displacement optimization requires an existing, nonempty nodeset at run time.
* ``displacement_component`` uses global coordinates. X/Y/Z optimize or measure
  the maximum absolute component, and ``magnitude`` the maximum vector norm.

The fixed density floor and penalization are not additional command arguments.
The command/configuration parser accepting a setting is distinct from the
run-time checks for a supported objective, analysis, mesh, and output selection.

Run
---

.. code-block:: text

   ccx run topology <id> [wait]

Start a fresh run for an existing case. Without ``wait``, return control while
jobs progress. With ``wait``, wait until the run finishes. Starting again
resets that case's previous run data and iteration job records; it does not
resume a selected design. See :doc:`../user/topology-running` before rerunning.

The current run supports density optimization with compliance or displacement.
It validates the analysis and displacement selection before resetting existing
run data, then performs run preparation. Later preparation or solver failures
can therefore occur after the previous run data has been cleared.

Wait
----

.. code-block:: text

   ccx wait topology <id>

Wait for a current topology run context. Convergence or reaching the iteration
limit is successful completion; failed and killed runs return failure. A nested
wait is rejected. This does not restart or resume a run loaded from a model file.

Kill
----

.. code-block:: text

   ccx kill topology <id>

Request termination of an active run and its active physical or adjoint job.
The request may be accepted before the process has finished terminating.
Completed iteration results are retained. An inactive case has no run to kill.

Examples
--------

These snippets require a prepared analysis model. Nodeset 3 must exist for the
displacement example. Each snippet is an independent example, not a sequence
to paste together with duplicate case IDs.

Create with defaults and change selected settings:

.. code-block:: text

   ccx create topology 1
   ccx modify topology 1 name "beam" volume_fraction 0.7 iterations 50
   ccx run topology 1 wait

Start a displacement run asynchronously, then wait:

.. code-block:: text

   ccx create topology 1 name "beam_displacement" objective displacement displacement_nodeset 3 displacement_component y volume_fraction 0.7
   ccx run topology 1
   ccx wait topology 1

Stop an active run instead of waiting:

.. code-block:: text

   ccx kill topology 1

See :doc:`topology-preservation` for inclusion, exclusion, and reset commands.
See :doc:`topology-results` for preview, deletion, GIF export, and wrapping.
See :doc:`../user/topology` for the overview.
