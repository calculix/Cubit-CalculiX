Preserved elements
==================

Preserved and free elements
---------------------------

Each element participating in a topology run has a preserved/free flag.
Preserved elements remain at density 1 and retain their base stiffness.
Free elements may change density during optimization. Preserved-element design
sensitivities are stored as zero because their densities are fixed.

Solid sections provide the analysis elements and material assignments. They
are not preservation selections. Use blocks, nodesets, and sidesets to add or
exclude preservation rules for a case.

Automatic preservation
----------------------

The plugin collects automatic selections from the model when preparing a run.
Loads and boundary conditions contribute only when assigned to existing
analysis steps; unused load/BC definitions do not contribute. Constraint and
contact-pair definitions are collected separately as model-level definitions.

.. list-table:: Sources of automatic selections
   :header-rows: 1
   :widths: 35 65

   * - Source
     - Selection collected
   * - Rigid-body constraint
     - Its block or nodeset.
   * - Tie constraint
     - Both sidesets.
   * - Equation constraint or equation group
     - Elements connected to the referenced nodes; a direct element selection.
   * - Contact pair
     - Master and slave sidesets, plus its optional adjustment nodeset.
   * - Force, displacement, temperature assigned to steps
     - Associated nodesets.
   * - Pressure or heat flux assigned to steps
     - Associated sidesets.
   * - Gravity or centrifugal load assigned to steps
     - Referenced blocks.
   * - Film, radiation, or surface traction assigned to steps
     - Referenced sidesets.
   * - Body heat flux assigned to steps
     - Referenced blocks or directly selected elements, according to its selection mode.
   * - Heat-flux trajectories assigned to steps
     - Volume-element owners of selected faces, or the directly selected trajectory elements.

This describes the preservation collector. It does not mean every analysis
using those features is supported by the topology run validator. For example,
the existence of a thermal-load selection path does not establish support for
thermal topology optimization.

A nodeset selected only for displacement monitoring is not automatically
preserved merely because it is monitored. It can still be preserved through
a load, boundary condition, constraint, or an explicit case selection.

How sets select elements
------------------------

All selections are restricted to elements participating in this topology
analysis. A set can exist yet select zero participating elements.

.. list-table:: Selection-to-element resolution
   :header-rows: 1
   :widths: 20 80

   * - Set
     - Participating elements marked preserved
   * - Block
     - Elements belonging to the block.
   * - Nodeset
     - Any element containing at least one selected node. Expanded connectivity includes midside nodes of quadratic elements; not all element nodes need to be selected.
   * - Sideset
     - Volume elements returned as owners of the sideset sides.

For example, fixing the nodes on an end face normally preserves the layer of
elements touching that face. A force applied to a free reference node may
select no volume elements through that node alone; the rigid-body block or
nodeset supplies the preservation of its connected region separately.

Adding and removing selections
------------------------------

For an existing case 1 and existing sets, use commands such as:

.. code-block:: text

   ccx topology 1 add preserved block 2 nodeset 5 6 sideset 7
   ccx topology 1 remove preserved nodeset 5

Add records an explicit inclusion and cancels an exclusion of that same set.
Remove records an exclusion and cancels its explicit inclusion. A set means
the combination of its kind and ID: block 5 and nodeset 5 are different sets.
Repeated inclusion or exclusion is idempotent.

These commands change the preservation rules for subsequent runs. They do not
remove nodes, sides, or elements from the Cubit model, and they do not remove
the associated analysis loads or boundary conditions. Changes while a case is
running are rejected.

Overlapping selections
----------------------

An element is preserved if any effective set selects it, or if it is selected
directly by an automatic source. Duplicate selections do not multiply its
volume or create multiple element entries.

.. code-block:: text

   ccx topology 1 add preserved block 2 nodeset 5
   ccx topology 1 remove preserved nodeset 5

In this example, elements also belonging to block 2 remain preserved. Removing
a nodeset is not an element-wise instruction to make all touching elements
free. Another nodeset or sideset can also keep them preserved.

Direct automatic selections, such as equation-connected elements and direct
load targets, are applied separately. The block/nodeset/sideset remove commands
cannot exclude those direct selections.

Restoring automatic defaults
----------------------------

.. code-block:: text

   ccx topology 1 reset preserved

Reset clears explicit inclusions and exclusions. The next run uses the
automatic selections collected from the model as it exists then. It does not
disable automatic preservation or make every element free. Stored iterations
and results remain available until the case is run again or otherwise reset.

When changes take effect
------------------------

The rules and resolved element flags are separate data. Add, remove, and reset
change the rules without immediately recalculating the existing element flags
or rewriting stored iteration results. Run initialization resolves the rules
against the current model and marks every participating element preserved or
free. Starting again clears the previous run data as described in
:doc:`topology-running`.

The preservation panel displays rule selections collected from the current
model; those can differ from the flags used by the previously prepared run
after edits to the model or overrides.

Volume feasibility
------------------

Preserved elements contribute their full volume, while free elements cannot
go below the density floor. The minimum attainable volume fraction is:

.. math::

   f_{\min} =
   \frac{V_{\mathrm{preserved}} + \rho_{\min}V_{\mathrm{free}}}
        {V_{\mathrm{total}}}.

The volume target must accommodate that minimum. Preserving an entire analysis
block can make a target infeasible; preserving all analysis elements leaves no
free densities to optimize. The example IDs above are illustrative, not
recommended selections for every model.

Preview and deletion
--------------------

Preserved elements retain density 1 during density updates. They are excluded
from threshold-removal candidates in preview and from actual element deletion.
Changing overrides after a run does not retroactively free elements in its
stored design. See :doc:`topology` for the preview command.

Using the preservation panel
----------------------------

Select the topology case and set kind, then move selections between
**Available / excluded** and **Preserved** using **Add >** and **< Remove**.
Rows identify automatic, explicitly included, or excluded rules. **Reload**
refreshes the lists. **Restore automatic defaults** clears the overrides for
subsequent runs, corresponding to ``reset preserved``.

The panel also reports the number of elements collected through direct
automatic selections. That count is not the union of all preserved elements;
the final run flags are restricted to the analysis domain during resolution.
An element can remain preserved after removing a row because another set or
a direct selection still covers it. Editing is disabled while running.

See :doc:`../commands/topology-preservation` for syntax and
:doc:`../developer/topology-preservation` for the resolution algorithm.
