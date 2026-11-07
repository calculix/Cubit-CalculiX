Postprocessor commands
======================

Status and scope
----------------

The following command handlers are implemented in source. Native parser behavior,
job loading and journal replay remain unverified until building and testing are
authorized. These commands manage loaded source data. Use
``sim workspace postprocessor`` to enter the :doc:`workspace`; its viewport is
independent and displays supported mesh sources with a neutral surface and edges.

Open job results
----------------

.. code-block:: text

   post open <provider> job <job_id> [name "<source name>"]

Example with an existing CalculiX job:

.. code-block:: text

   post open ccx job 12
   post open ccx job 13 name "Variant B"

The provider prefix is an unquoted string, resolved through Framework's provider
registry. ``ccx`` currently resolves to the CalculiX provider; additional providers
can register their own prefixes without changing this command's syntax. The job
ID must be positive and identify a job in that provider's current application
context. Unknown providers, missing/running jobs and conversion failures fail
the command and are reported through Cubit's error mechanism.

The job name becomes the source name unless a quoted override is supplied. Names
must contain a non-whitespace character and be unique, with case-sensitive matching.
Use the override to open a second independently retained snapshot of the same job.
The new source becomes active; existing sources remain loaded. An explicit duplicate
name is rejected before loading; a duplicate suggested job name can be detected
after loading but cannot replace existing Postprocessor data.

The provider returns an immutable result snapshot. Opening a job without an explicit
snapshot request loads its current result files; the command does not select a
historical solver run. The current CalculiX snapshot token identifies a conversion,
not a durable solver run. Running jobs and active result conversions are rejected.

Show and hide sources
---------------------

.. code-block:: text

   post show "<source name>"
   post hide "<source name>"

These commands affect only the active view. Showing creates its source
representation if needed; repeated show/hide requests are idempotent. Hiding
retains the input data and does not change source selection, an established
camera, or another view's visibility. Unknown names fail. Table-only sources
cannot be shown as meshes. Tree checkboxes submit these same commands and restore
authoritative Core state after failure.

Opening a mesh source automatically creates a visible representation in the
active view, using its first increment containing a supported nonempty mesh.
Each representation stores its own increment index for future time comparison
between views. There is no time-navigation command yet.

Fit visible meshes
------------------

.. code-block:: text

   post view fit

Requests fitting all visible mesh bounds in the active view. It fails without
visible mesh data. The first mesh in an initially unfitted view requests fitting
automatically; later openings retain an established camera. The request preserves
the current orientation/projection as its seed. The GUI completes it when the
viewport is visible and has a valid OpenGL context, using the current viewport
aspect ratio. Its resulting camera is validated and retained in Core.

The command records the fit intent, not exact camera coordinates across different
window sizes. A headless or hidden view retains the request until GUI completion.
Mouse orbit/pan/zoom remain blocked pending their own journalable commands.

List sources
------------

.. code-block:: text

   post list

Lists exact source names and their origin. ``*`` marks the active source. The empty
case reports ``No Postprocessor sources are open.`` and succeeds. Names, rather
than internal IDs, are used in subsequent commands. Listing order follows the
internal source-ID map; it is not a promised creation-time or alphabetical order.

Select a source
---------------

.. code-block:: text

   post select "<source name>"

Selects an existing source by its exact, case-sensitive name and clears stale
active display context. It changes no view, camera or source data. Unknown names
fail without changing selection. Quotes are required, including for single words.

Close sources
-------------

.. code-block:: text

   post close "<source name>"
   post close all

Closing removes the named source and its direct displays from the session. Closing
the active source clears selection. ``post close all`` clears every source and
display, retaining the view/layout definitions. Source files and solver jobs are
not deleted. Caller-owned result handles can remain valid after closure.

A source literally named ``all`` is addressed with quotes:

.. code-block:: text

   post close "all"

The unquoted ``all`` is the bulk-close keyword. Quoted names, spaces and this
keyword distinction are explicitly included in the native verification checklist.

Journaling and replay
---------------------

These are native ``CubitCommand`` handlers registered through the existing
cubit-sim command plugin. Commands entered through Cubit's normal command path
participate in its normal history/journal behavior. No separate journal writer
or silent GUI mutation path is introduced. Source-tree and toolbar/menu actions
submit the same commands through the established command dispatcher.

Replay requires the referenced application job IDs and result files to exist.
The current syntax opens current files, so replay is not a promise of historical
numerical reproducibility after files change. Explicit names make subsequent
selection/closure independent of changed job display names. A duplicate name
still fails; commands never silently replace a loaded source.

Apply user options
------------------

.. code-block:: text

   post useroptions icons "Icons/"
   post useroptions list

The options dialog submits a quoted readable icons command only when the field
changes. Core validates and persists it under workspace ``postprocessor`` in the
shared preferences file. Icon-directory changes take effect after restarting
Cubit. Invalid values and failed saves leave the preference and active icons
path unchanged. ``list`` prints the saved setting and any pending restart notice.
Preferences are independent of model data.

Lifecycle and deferred commands
-------------------------------

The module owns one application instance per initialized Framework lifetime.
Application state is released at module shutdown. Command objects retain weak
application handles; stale commands fail rather than access destroyed state.
No ``post`` refresh/file/draw command is registered in this stage. Independent
VTU/VTPC/VTT files, FRD/DAT file opening, Cubit geometry,
field coloring, time navigation and mouse view controls require their later implementations.

See :doc:`../developer/postprocessor` for source files and method contracts.
