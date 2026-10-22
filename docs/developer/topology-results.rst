Topology result storage and processing
======================================

This chapter describes the result consumers, persistence, and mesh checks.
See :doc:`topology-algorithms` for objective/update mathematics,
:doc:`../user/topology-results` for the GUI, and
:doc:`../commands/topology-results` for command syntax.

Stored records and ownership
----------------------------

``CoreTopology`` owns cases and their iteration density designs, objective
evaluations, volume/convergence diagnostics, physical and adjoint job
associations, and resolved element-preservation flags. Preservation flags are
case run data, shared by the designs in that run. Density designs retain their
interpolation parameters and update diagnostics.

The material core owns the topology material collection, including base
material assignments and per-iteration stiffness factors. Job records belong
to the job core. ``CalculiXCore`` coordinates these stores through its existing
interfaces. Consumers must not introduce direct dependencies between domain
core headers; cross-core access goes through the domain-specific interfaces (see :doc:`interface-layout`).

``CalculiXCore::get_topology_iteration_element_data`` checks the iteration job
is finished and an evaluation, density design, and factor map exist. It checks
coverage against preservation and base material assignments and validates
positive IDs, finite densities/factors, and their bounds. Each returned row is:

.. code-block:: text

   [global_element_id, density, stiffness_factor, preserved_flag]

The flag is zero or one. Consumers perform their additional mesh checks before
using these rows. An empty result signals unavailable or invalid data.

Displayed versus selected iteration
-----------------------------------

``TopologyCase::selected_iteration`` is the stored core selection; -1 means
none. Completing an iteration updates it. The core selection API validates a
finished, evaluated iteration before changing it. Element-data requests with
iteration -1 resolve through this field rather than searching for the newest
iteration themselves.

``TopologyResultsPanel`` maintains its own displayed case and iteration.
Opening a case initially chooses the last completed entry when no previous or
explicit selection applies. Browsing the dropdown does not call the core
selection API. Panel preview, deletion, and wrapping pass explicit iteration
IDs; GIF export passes a case ID and processes all eligible iterations.
This distinction must remain clear in UI labels and command documentation.

Tree and panel availability
---------------------------

``TopologyTree`` maps case run states to topology-specific icons. Both
``converged`` and ``iteration_limit`` use the finished icon. Its iteration icons
use physical job status; completion for browsing additionally requires a stored
objective. Each iteration's job child refers to the physical job.

``TopologyResultsPanel`` lists evaluated entries with finished physical jobs.
It validates that the displayed job still matches the iteration association.
Mesh operations additionally require an idle case and available stored
elements. ``meshAvailable`` checks that returned global element IDs are valid
and still exist in Cubit. Job inspection can remain available when mesh actions
are disabled. Reloading refreshes the lists without reconstructing a run.

Save and load
-------------

``CoreTopology::save_configuration`` writes the topology group below
``Cubit-CalculiX/Topology`` with schema version 8. Version 8 stores
``material_cap``; older configurations receive the default for future iterations
without changing their historical applied factors. Case settings, explicit
preservation rules, and runtime records are serialized through the topology
HDF5 archive helpers. Related material and job data are managed by their
own cores; case records refer to them rather than owning duplicate job objects.

Loading validates case settings, iteration keys, job associations, and selected
iteration completeness before committing the collection. Supported older data
is migrated by the reader, including the iteration numbering conversion where
applicable. A case saved as running is restored as failed with an interrupted-run
message; loading does not recreate the transient controller or restart a solver.

GUI timers, the displayed dropdown entry, preview state, and playback are
transient widget state. Persisted core selection is a different field. Stored
iteration records are not a geometry snapshot and do not reconstruct mesh
elements removed after optimization. External solver files are not made
portable merely by saving the topology records; preserve files required by
the job/result workflows separately.

Preview and playback
--------------------

``CoreDraw::draw_topology`` resolves global element IDs to Cubit volume entity
kinds and local IDs before drawing. The classification is
``!preserved && density < threshold`` for removal; all others are retained.
Retained elements are green and removal candidates have red edges.

The Results panel and tree call previews through ``silent_cmd``. Other GUI
actions, including deletion and exports, use the ordinary GUI command path.
The panel guards against reentrant preview calls and defers automatic preview
requests through a short single-shot timer. Playback advances the available
iteration entries, draws each explicitly, then schedules its next timer.
Rendering time adds to the requested interval; this is not a real-time frame
rate guarantee. It stops at the final entry or when availability is lost.

GIF capture and wrapping
------------------------

``CoreDraw::animate_topology`` starts a ``TopologyAnimation`` controller. It
snapshots eligible iteration IDs, validates output settings, and schedules one
silent preview/hardcopy per GUI-thread timer callback. Startup returns before
capture is complete. No Cubit or widget access occurs on the encoding worker.

After capture, the controller moves its temporary directory into a worker using
``std::async``. ``GifAnimation`` receives atomic cancellation/progress values and
returns errors through a string, without calling Cubit logging. The worker
encodes, publishes, and removes temporary PNGs before returning. A GUI timer
polls readiness without waiting and reports progress/completion on the main
thread. Replacement uses ``QSaveFile``; without overwrite, ``QFile::copy``
refuses an existing output, including one created after startup.

Cancellation is cooperative between captures/encoded frames and replacement
copy chunks. A completed publication remains successful even if cancellation
arrives afterwards. Reset/load requests cancellation; shutdown joins any file
worker before destroying plugin-owned code. The Results panel polls active
state, stops competing preview timers, and exposes cancellation. Generic Cubit
commands can still change the graphics view, so users must keep it unchanged
during capture. This uses the current view, not a stored camera snapshot, and
does not restore the previous display. Topology drawing compresses consecutive
IDs into ranges and bounds batches by command length.

``ccxWrapCommand`` collects the threshold-selected volume mesh through
``collect_topology_wrap_mesh``. It validates the entire stored design against
the current mesh before building the retained input. Preserved elements are
included regardless of density. Boundary extraction and corner-node
approximation feed the existing CGAL alpha-wrap workflow. The command supplies
automatic or explicit alpha/offset and optionally imports the resulting STL.

Deletion and partial failure
----------------------------

``CoreResultsProject::project_topology`` rejects a running case, validates all
stored rows, checks element existence and volume kind, and groups removal
candidates by hex, tet, or wedge local IDs. It prepares all deletion commands
before modifying the mesh, batching up to 200 volume elements per command.

With preview enabled, it delegates to drawing. Otherwise it enables developer
commands if necessary, executes deletion through ``silent_cmd``, and restores
the previous developer-command setting on exit. Restoring that setting does
not restore deleted elements.

Failure stops further batches. Earlier batches remain deleted, and the failed
batch may itself have changed the mesh. A subsequent display failure can return
false after successful deletion. Callers must therefore not interpret a false
return as proof that the mesh is unchanged. The Results panel refreshes its
availability checks after deletion without assuming command success.

Mesh identity limits
--------------------

The current checks detect missing IDs and unsupported element kinds. They do
not establish that an existing ID still has its original coordinates,
connectivity, or assignments. Reusing IDs after remeshing can therefore pass
existence checks without representing the optimized model. Preview and wrapping
use current coordinates, while densities and evaluations describe the stored
optimization run. There is no mesh fingerprint or automatic restoration.

Keep historical records for numerical inspection, but use their mesh operations
with the corresponding unchanged mesh. After deliberate deletion, create and
validate a normal analysis of the remaining mesh rather than treating the
historical solve as a verification of that modified model.
