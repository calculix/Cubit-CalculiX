Postprocessor workspace integration
===================================

Scope and evidence
------------------

This chapter describes the workspace-shell source, not verified GUI behavior.
The user reported a successful rebuild after the member-wise session swap fix.
No build or compilation of the workspace patch was run by the agent. SDK headers,
DLL exports and resource bundles were inspected read-only. Export availability
does not establish VTK C++ ABI compatibility or successful OpenGL embedding.

Implementation map
------------------

Paths are relative to ``src``. The Core/source contracts remain in
:doc:`postprocessor`; user actions are described in :doc:`../postprocessor/workspace`.

.. list-table:: Files and methods
   :header-rows: 1
   :widths: 30 70

   * - Responsibility
     - Source and entry points
   * - Graphics mounting
     - :download:`WorkspaceGraphicsStack.hpp <../../src/Framework/GUI/WorkspaceGraphicsStack.hpp>` and :download:`implementation <../../src/Framework/GUI/WorkspaceGraphicsStack.cpp>`: ``attach``, ``activate``, ``detach``.
   * - Workspace contract and host
     - :download:`WorkspaceProviderInterface.hpp <../../src/Framework/Interfaces/WorkspaceProviderInterface.hpp>`, :download:`CubitWorkspaceHost.cpp <../../src/Framework/GUI/CubitWorkspaceHost.cpp>` and :download:`WorkspaceManager.cpp <../../src/Framework/GUI/WorkspaceManager.cpp>`: optional graphics page, optional native navigation and dock-placement policy.
   * - Module and GUI lifecycle
     - :download:`PostprocessorModule.cpp <../../src/Postprocessor/PostprocessorModule.cpp>` and :download:`PostprocessorGui.cpp <../../src/Postprocessor/GUI/PostprocessorGui.cpp>`: ``start_component``, ``stop_component``, ``workspace_provider``, ``start``, ``stop``, ``refresh`` and ``workspace_view``.
   * - Workspace widgets and commands
     - :download:`PostprocessorWorkspace.cpp <../../src/Postprocessor/GUI/Workspace/PostprocessorWorkspace.cpp>`: menus, toolbar, quoted source commands, committed-state refresh and workspace description.
   * - Independent viewport
     - :download:`PostprocessorViewport.cpp <../../src/Postprocessor/GUI/Viewport/PostprocessorViewport.cpp>`: own generic OpenGL render window, concrete OpenGL renderer, queued initial render and shell-stage input blocking.
   * - Source panels
     - :download:`PostprocessorResultsTree.cpp <../../src/Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.cpp>` and :download:`PostprocessorProperties.cpp <../../src/Postprocessor/GUI/Properties/PostprocessorProperties.cpp>`: source names/selection and read-only source information.
   * - GUI observer
     - :download:`PostprocessorGuiObserver.cpp <../../src/Postprocessor/GUI/Observers/PostprocessorGuiObserver.cpp>`: SDK command-completion observer and coalesced queued refresh.
   * - SDK development adapter
     - :download:`CubitVtkSDK.cmake <../../src/cmake/CubitVtkSDK.cmake>`, :download:`inspect_cubit_vtk.py <../../tools/vtk/inspect_cubit_vtk.py>` and :download:`prepare_cubit_vtk_sdk.py <../../tools/vtk/prepare_cubit_vtk_sdk.py>`: inspect host exports, configure matching headers and generate import libraries without building replacement VTK DLLs.

Graphics ownership and restoration
----------------------------------

.. code-block:: text

   Claro graphics container
     +-- Framework QStackedWidget
           +-- Original Cubit graphics widget
           |     +-- original renderer/camera remain owned by Cubit
           +-- PostprocessorWorkspace
                 +-- PostprocessorViewport
                       +-- own vtkGenericOpenGLRenderWindow
                       +-- own vtkOpenGLRenderer and camera

``WorkspaceGraphicsStack`` replaces the original viewport's direct splitter/layout
slot, preserving the viewport object and ``Claro::graphics_widget()`` identity.
Unsupported parent layouts fail attachment instead of guessing how to rebuild
Cubit's window. A null workspace graphics page selects the original widget, so
Cubit and CalculiX retain their native viewport. A non-null page is borrowed from
its provider and mounted on first activation.

Framework restores the native page before destroying module GUIs. The stack
detaches provider pages back to its original parent, restores the native slot and
removes only its own wrapper. Qt guarded pointers handle prior widget destruction.
WorkspaceManager's activation rollback reapplies the previous page and dock state.
Closing the application follows the existing native-workspace restoration path.

``WorkspaceView::useNativeNavigation`` defaults true, and its dock layout defaults
to the existing stacked-left policy. Postprocessor disables native navigation and
requests left/right docks. Its command dock is forced to the bottom and locked
while active; original area, floating geometry, allowed areas and features are
restored on exit. Dock corners keep the bottom command area between the side docks.
Native toolbar visibility is captured on entering an independent graphics page
and restored on exit. This patch does not change CalculiX toolbar-row policy.

The module's GUI owner uses guarded workspace pointers; stopping its GUI does not
reset application/Core data. Its observer unregisters before widget deletion.
Headless startup creates no widget and exposes no workspace provider.

Command and update flow
-----------------------

.. code-block:: text

   Tree selection / menu / toolbar
        --> injected submit callback
        --> Framework CommandDispatcher
        --> existing native Cubit command processing and journal
        --> Postprocessor application/Core
        --> CubitInterface::ObserverBase::notify_command_complete
        --> queued/coalesced GUI refresh

The Framework supplies the submission callback at module registration. Postprocessor
does not link back to Framework's implementation or call CalculiX GUI internals.
Names are quoted using an available single/double delimiter; control characters
and names containing both delimiters are rejected before submission. Programmatic
tree rebuilding blocks signals and restores selection from committed Core state.
The tree's fallback refresh is queued to avoid deleting items inside its selection
signal. Native docks/layout handling uses the existing workspace host machinery.

Qt/VTK adapter
--------------

Cubit 2026.11 exports ``QVTKOpenGLNativeWidget`` and its render-window setter from
``showviz_qt.dll``. The public VTK 9.4.2 Qt headers are used with that implementation.
The prepared adapter represents this DLL as ``VTK::GUISupportQt``, generates the
rendering/Qt-support configuration headers and links Qt6 Widgets/OpenGLWidgets.
No independent VTK runtime is built, copied or substituted. The concrete renderer
is created directly; no global VTK object-factory override is installed.

The adapter's ``viewport-qt`` feature marker distinguishes older data-only adapters.
On the next user-authorized CMake configuration an older adapter is re-prepared.
Preparation invokes CMake compiler probes and the librarian, so it was not run by
the agent under the no-build restriction. Matching Qt6_DIR is forwarded from the
plugin configuration. Linux still requires an explicitly host-compatible VTK SDK,
now including GUISupportQt and rendering components.

The widget receives its surface format before display. Initial render is queued
until the Qt OpenGL widget is visible and valid. Result actors, camera commands,
frame controls and multi-view layouts are not implemented in this stage.

Native toolbar icon inventory
-----------------------------

Icon reuse is visual only. Postprocessor camera, projection, selection, clipping,
visibility and other interaction features must have their own commands and operate
on the Postprocessor scene. Do not connect reused icons to native Cubit viewport
actions or share native interaction state. Shared hosting and command/journal
infrastructure remain Framework responsibilities. Any future New action must have
an explicitly agreed session/model scope before being wired to a native action.

Read-only inspection of ``light_claro.rcc`` and ``light_images.rcc`` found these
resource paths. Dark-theme bundles use corresponding paths; active resources are
provided by Cubit. Resources are referenced in-process, not copied into this repo.

.. list-table:: Examples
   :header-rows: 1

   * - Action
     - Qt resource path
   * - Open / Save
     - ``:/clarogui/fileopen_cl.svg`` / ``:/clarogui/filesave_cl.svg``
   * - Undo / Redo
     - ``:/clarogui/undo_cl.svg`` / ``:/clarogui/redo_cl.svg``
   * - Zoom / Fit
     - ``:/cubitgui/zoom.svg`` / ``:/cubitgui/zoomreset.svg``
   * - Pan / Rotate
     - ``:/cubitgui/pan.svg`` / ``:/cubitgui/rotate.svg``
   * - Command output / Close
     - ``:/clarogui/cmdwindow_cl.svg`` / ``:/clarogui/dcancel.svg``

Postprocessor uses the last pair for listing/closing sources, with standard Qt
fallback icons if the resources are unavailable. Native visual/theme switching
and toolbar placement remain part of GUI verification.
