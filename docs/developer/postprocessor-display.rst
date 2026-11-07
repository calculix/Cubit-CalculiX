Postprocessor mesh displays
==========================

Scope and ownership
-------------------

This patch implements first mesh visualization, per-view source visibility and
camera fitting. It does not implement field coloring, deformation, timeline
controls, split views, geometry import, picking or mouse navigation.

``PostprocessorCore`` owns representation definitions, selected increment indices
and camera definitions. ``PostprocessorScene`` owns actors and their input handles
for one view. GUI actions submit ``post show``, ``post hide`` and ``post view fit``
through the existing dispatcher; none calls a native Cubit viewport action.

.. code-block:: text

   Commands / tree checkbox / fit toolbar action
          |
          v
   PostprocessorApplicationInterface
          |
          v
   PostprocessorCore
     source runtime handle + representations + views
          |
          v
   PostprocessorWorkspace refresh
          |
          v
   PostprocessorViewport / PostprocessorScene
     per-partition surface filter -> OpenGL mapper -> OpenGL actor

Core files and methods
----------------------

``Core/representations/ResultMesh`` extracts borrowed ``vtkDataSet`` pointers
from a single dataset, partitioned dataset or partitioned dataset collection.
It skips empty partitions, non-mesh objects and datasets without finite ordered
bounds, points or cells. The caller retains the result handle. Published arrays
are never modified. Bounds use input coordinates directly, with no deformation,
transformation or unit conversion. Overlaying unrelated datasets requires a later
explicit unit/coordinate contract.

``first_mesh_increment`` scans increments in order. Opening selects its first
displayable mesh increment and prepares the source's initial representation in
the same staged session transaction as source publication. Table-only sources
still open successfully without a representation. Initial frame discovery is
cached in ``SourceRuntime``; each representation stores its own increment index,
allowing future views to choose different times without duplicating source data.

``PostprocessorCoreRepresentations`` implements ``set_source_visible`` and
initial display preparation. There is at most one source port-zero representation
per view. Show creates it if absent; hide without a representation succeeds.
Repeated requests preserve the display revision. Visibility does not change
selection or an established camera. Unknown source/view IDs and showing table-only
data fail. Core rejects mutations during provider operations. Failed staged
operations leave definitions and revision counters unchanged.

Refresh preserves each existing representation's visibility and valid increment
index. If the old index is no longer displayable, it falls back to the new first
mesh increment. If the refreshed source has no mesh, its direct representations
are removed. A previously table-only source requires explicit show after gaining
mesh data. Closure removes direct representations; close-all retains view/camera
definitions. Actor input changes are detected by immutable result-handle identity
and increment index.

``DisplayDefaults`` owns the neutral surface color, edge color/width and initial
isometric orientation. There is no GUI-local copy of these defaults.

Camera-fit protocol
-------------------

``PostprocessorCoreViews::visible_bounds`` unions finite bounds of visible mesh
partitions belonging to the requested view. No visible mesh is an error for an
explicit fit command. ``request_camera_fit`` increments the view's session-local
fit token and sets fit_pending, preserving the stored camera as an orientation
and projection seed. An initially empty view starts pending its first fit.

The GUI fulfils that request only when its Qt OpenGL widget is visible and valid.
It temporarily fits a separate concrete OpenGL camera on the view renderer, using
the current render-window aspect ratio, then restores the previous active camera.
It submits the resulting CameraDefinition through
``PostprocessorApplicationInterface::complete_camera_fit``. This is completion
of the existing open/fit action, not a separate user command.

Core compares the view token and global display revision, rejecting stale results
after source open/refresh/closure or visibility changes. Camera completion checks
finite coordinates, positive parallel scale, perspective angle strictly between
zero and 180 degrees, distinct position/focal point and a nonzero view-up vector.
Normalized up/direction vectors must have cross-product magnitude greater than
1e-12 to avoid an unstable nearly parallel frame. Rejected completion leaves the
request pending and the previous camera unchanged.

Successful completion retains the numeric camera in Core and clears fit_pending.
The scene applies it and recomputes clipping bounds. Opening subsequent sources,
show/hide and ordinary resizing preserve that camera. A fit request made headlessly
or while hidden remains pending until GUI completion. Journal replay preserves
fit intent; different viewport sizes may yield different fitted coordinates.
Fit tokens are transient bookkeeping, excluded from future serialization.

Rendering and GUI lifetime
--------------------------

``PostprocessorScene`` uses ``vtkDataSetSurfaceFilter``, concrete
``vtkOpenGLPolyDataMapper``, ``vtkOpenGLActor`` and ``vtkOpenGLProperty`` without
changing global object factories. Scalar visibility is off. The surface filter
creates separate display outputs, with no writes to published result buffers.
One actor is created per supported partition. Filter errors are observed locally
during extraction and observer stack-data pointers are removed before retaining
the pipeline. Unchanged hidden actor groups are retained; changed hidden inputs
are discarded and rebuilt when shown.

Scene synchronization removes closed, replaced or failed inputs rather than
leaving stale actors behind. Actor groups retain shared result ownership. Error
recovery clears scene props, including partially applied updates. Scene lifetime
ends before the widget's renderer/render window. Native Cubit graphics objects
are never shared.

The workspace retains its viewport pointer and synchronizes CPU-side scene state
even while hidden, releasing closed sources' actor handles. Actual rendering is
queued and coalesced. Show, GL initialization and resize schedule rendering only
after the widget is visible/valid. Interactive mouse/key/gesture input remains
blocked. Source-tree refresh blocks signals; user checkbox changes submit quoted
commands and defer restoration of Core state until outside the item signal.
The shared fit icon is visual reuse of ``:/cubitgui/zoomreset.svg`` only.

Build and verification
----------------------

Core now links VTK CommonDataModel for mesh inspection, while still containing no
rendering classes. The module additionally links FiltersGeometry. SDK checks
require that target on Windows and Linux; the preparation closure names it
explicitly. The existing prepared Windows SDK already includes it. Static PE
inspection confirmed native New exports for the surface filter and concrete
OpenGL actor/camera/mapper/property classes. This is symbol evidence, not an ABI
or graphics-runtime validation.

``src/Postprocessor/Tests`` is a standalone CMake project compiling production Core,
application interface and scene code with a deterministic provider. It covers
single grids, partition collections, first nonempty increment selection,
table-only opening, immutable input topology/arrays, idempotent visibility,
provider reentry rejection, stale/invalid camera completion, retained cameras,
failed refresh preservation, frame fallback, actor visibility and actor removal.
It deliberately does not call Render or instantiate the Qt viewport. Windows
uses the prepared Cubit SDK; Linux needs the explicit host-compatible VTK_DIR.
Use the same Qt6 installation as the plugin.

No CMake configuration, compilation, C++ tests or actual OpenGL/GUI tests were
run by the agent under the user's no-build restriction. Before accepting the
patch, run the tests after authorization and complete the native checks in
:doc:`postprocessor-verification`, including journal replay, hidden views,
context recreation and source refresh/closure across workspace switches.
