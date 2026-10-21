Preservation resolution
=======================

Data and identifiers
--------------------

``TopologyCase`` keeps ``added_preserved_sets``, ``removed_preserved_sets``,
and ``element_preserved`` separately. Set maps use preservation-kind IDs:

* 1: block
* 2: nodeset
* 3: sideset

These are preservation API identifiers, not Cubit element-type IDs. Set IDs
are scoped by kind. Resolved flags use global volume-element IDs and contain
one boolean entry for each element in the prepared analysis domain.

Automatic sources
-----------------

``CalculiXCore::collect_topology_default_sets()`` builds automatic set maps and
a separate direct-element set. It collects constraint/contact selections and
only those load/BC IDs referenced by existing analysis steps. Equation nodes
are mapped through model connectivity; direct body heat flux and trajectory
targets also enter the direct-element set. Output arguments are cleared on
entry, with locally collected results published only after success.

This collection describes preservation sources, not support for running every
analysis containing them. Run validation remains a separate responsibility.

Rules and overlap
-----------------

Let :math:`S_a` be automatic sets, :math:`S_+` explicit inclusions, and
:math:`S_-` explicit exclusions. A set is a pair of kind and ID:

.. math::

   S_{\mathrm{effective}} = (S_a \cup S_+) \setminus S_-.

Let :math:`E(s)` resolve a set to volume elements, :math:`D` contain direct
automatic selections, and :math:`A` be the prepared analysis domain:

.. math::

   P = \left(D \cup \bigcup_{s\in S_{\mathrm{effective}}}E(s)\right)\cap A.

Set exclusion is applied before element union. It does not subtract its
elements from the final union, and it does not subtract from :math:`D`.
Overlaps and repeated IDs are deduplicated through set containers/boolean flags.

Add erases the corresponding exclusion and inserts the inclusion. Remove
erases the inclusion and inserts an exclusion. Each kind's input ID list is
validated before changing its stored rules. Reset clears both override maps.
These operations reject running cases and leave existing resolved flags and
iteration records untouched. The command layer additionally checks all three
set-ID lists before forwarding its per-kind operations through the interface.

Set-to-element resolution
-------------------------

``CalculiXCore::get_topology_set_element_ids()`` requires the prepared topology
material collection, which identifies the participating elements. Blocks are
intersected with that collection. Nodesets are tested against expanded element
connectivity, including quadratic midside nodes, using any-node intersection.
Sidesets resolve their owning volume elements through the mesh export interface.
The output vector contains sorted, unique global IDs in the analysis domain;
it is cleared on entry and remains empty on failure.

``CoreTopology::resolve_preserved_elements()`` starts a local map with false
for every unique analysis element, applies effective sets, and then marks
direct selections that belong to the domain. Nonpositive direct IDs are errors;
positive direct IDs outside the domain are ignored. Resolved set IDs outside
the domain are rejected if unexpectedly returned by a query. The existing
flag map is replaced only after all resolution succeeds.

Lifecycle and consumers
-----------------------

The coordinating core prepares the analysis/material collection and invokes
preservation resolution during run initialization. Override edits do not rerun
that initialization. Restarting the case resets its old run data, then resolves
the current rules against the current model. See :doc:`topology`.

Density construction/updates keep preserved densities and stiffness factors at
1. Objective evaluations store zero sensitivities for fixed variables. Volume
evaluation includes preserved volume in its attainable minimum. Preview and
projection read the resolved flags and exclude preserved elements from removal.

``get_topology_preserved_sets()`` is a rule query for the GUI: it recollects
current defaults and reports automatic/included/excluded/effective status, plus
direct selections. It is not the historical element-flag map used by an earlier
design, and its direct-selection count is not a final union restricted to the domain.

All calls between domain cores pass through ``CalculiXCoreInterface``.
``CalculiXCore`` owns coordination and Cubit queries; ``CoreTopology`` applies
the rules and stores flags without including another domain core's header.

See :doc:`../user/topology-preservation`,
:doc:`../commands/topology-preservation`, and :doc:`api` for public behavior
and method contracts.
