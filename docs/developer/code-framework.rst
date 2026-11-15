Framework lifecycle and shared services
=======================================

Responsibilities and ownership
------------------------------

The Framework is the loader-facing shell. It owns module registration, common
services and workspace hosting rather than a second CalculiX model. The command
plugin and GUI component acquire the same shared ``framework_context()`` from
the Framework library. Keeping that implementation in one library is important:
separate context statics in entry-point libraries would split ownership.

.. list-table:: Principal types
   :header-rows: 1
   :widths: 28 72

   * - Type
     - Contract and owner
   * - ``CubitSimPlugin``
     - Cubit command-plugin entry point. Acquires/releases command attachment, exposes Framework keys, delegates workspace command construction.
   * - ``CubitSimComp``
     - Loader-facing GUI component. Requires the command plugin, acquires the Framework and attaches a GUI host when requested; cleanup releases its acquisition.
   * - ``FrameworkContext``
     - Owns options storage, module manager, workspace manager and options pages. Reference counts clients; initializes services on first acquisition and shuts modules down after the last release.
   * - ``ModuleManager``
     - Owns registered module objects and borrows their providers. Tracks successfully started modules/components for cleanup; maps command keys to their provider.
   * - ``CalculiXModule``
     - Implements module, command, component and workspace provider contracts for CalculiX. Owns its command provider and internal GUI owner.

Source: :download:`CubitSimPlugin.cpp <../../src/Framework/CubitSimPlugin.cpp>`,
:download:`CubitSimComp.cpp <../../src/Framework/CubitSimComp.cpp>`,
:download:`FrameworkContext.hpp <../../src/Framework/Core/FrameworkContext.hpp>`,
:download:`ModuleManager.hpp <../../src/Framework/Core/ModuleManager.hpp>` and
:download:`CalculiXModule.hpp <../../src/CalculiX/CalculiXModule.hpp>`.

Startup and failure paths
-------------------------

The Framework registers its compiled modules, resolves component paths, creates
the shared options store and initializes modules against that store. GUI startup
then starts components, registers workspace views and selects a workspace through
the existing command/provider path. Headless command use must not require a
viewport or native GUI window. A missing provider is different from a provider
whose initialization returned an error.

``ModuleInterface::shutdown()`` is valid after partial initialization and is
``noexcept``. Provider pointers are borrowed and remain valid until their module
is destroyed. Do not retain those pointers beyond module teardown. A module owns
the resources it creates; removing its UI must not remove Cubit's unrelated UI.
Registration order, duplicate-key validation and unwind behavior are detailed in
:doc:`framework-stage1` and :doc:`framework-stage2`.

Workspace hosting
------------------

``WorkspaceManager`` owns its Cubit host adapter and header, stores registered
``WorkspaceView`` values, and tracks per-workspace layouts and navigation
selections. Views borrow module-owned widgets. Activation switches the existing
host presentation; it does not create another solver model. Reentrancy guards
protect switching and shutdown. QPointer tracks deletion of borrowed Qt objects;
it is not ownership of those objects.

``CubitWorkspaceHost`` is the SDK-specific boundary. Keep assumptions about native
window names, navigation widgets, docks, toolbars and visibility restoration in
that adapter. The intended workspace visibility of a toolbar differs from the
temporary hidden state of an inactive workspace. See :doc:`framework-stage3`.
Module toolbar order comes from WorkspaceView::toolbars. The host reasserts a
single module group after saved-layout restoration, placing it below visible
native top rows. Only module bars may be removed/reinserted for this correction;
native Cubit bars remain under their existing QMainWindow layout ownership.

Options, storage and results
----------------------------

Workspace option declarations contain defaults, conversion and validators. The
shared ``UserOptionsStore`` borrows declarations during an operation; it owns
file access and workspace separation. The GUI options manager requests workspace
pages rather than hard-coding every solver preference. Options storage is not
the same file/schema as a model's CalculiX analysis data. Details and atomic-save
limits are in :doc:`user-options`.

The Framework ``HDF5Tool`` is a project-owned storage adapter over the external
HDF5 library. Document its group/dataset behavior and callers; the external HDF5
implementation is out of scope. Numerical result ownership uses
``SimResults::VtkResultsData``: immutable published data, owning handles and no
borrowed solver-reader storage. Its optional exporter is separate from the
in-memory ownership contract; see :doc:`vtk-conversion`.

Paths and documentation
------------------------

``FrameworkPaths`` resolves locations relative to the loaded component and the
user options directory. Avoid assuming the process working directory is the
plugin installation directory. ``DocumentationCommand`` and the Help action
resolve a local ``documentation/index.html``. Accepted desktop opening means a
request was submitted, not proof that a browser rendered the page. Documentation
generation and staging are separate from plugin compilation; see :doc:`../README`.

Extension checklist
--------------------

Register a module in the existing module-registration path, provide stable IDs
and unique command keys, initialize only required services, report failures via
the existing return/error contracts, and make shutdown safe after every partial
startup stage. Add workspace assets to the host registration lifecycle and test
headless use, repeated activation, failed startup and application teardown.
The full source inventory is in :doc:`code-catalog/index` and provider declarations
are extracted in :doc:`framework-api`.

Component startup is synchronous within CubitSimComp::start_up. Framework
acquisition initializes the shared material library; component providers then
initialize Python and create their GUI before workspace registration. Headless startup
omits GUI creation. See :doc:`framework-stage3` for lifecycle details.


Shared editor toolbar integration
----------------------------------

``WorkspaceView::modelManagementToolbar`` is a borrowed workspace toolbar.
``ModelEditorManager`` owns its actions and moves them onto the active toolbar
after workspace activation, removing them from the previous toolbar first.
Actions are appended last: Materials then Units for CalculiX, Units only for
Postprocessor and Report. Toolbar bindings and actions use QPointer; editor
cleanup deletes the shared actions without deleting workspace-owned toolbars.

``GUI/Resources/FrameworkIconLoader.hpp`` and ``.cpp`` form the Framework sibling
of ReportIconLoader, PostprocessorIconLoader and CalculiX's CoreIconLoader. SVG
files are staged to the shared component Icons directory by CMake, matching the
existing workspace deployment pattern. Framework GUI code does not call solver
icon services.

Postprocessor result navigation uses the existing graphicsBottomDock contract,
with layout revision 2 to migrate saved upper-toolbar layouts. Navigation is
owned by its dock; the workspace deletes the dock once, allowing Qt to delete
its toolbar and controls. The dock is separate from the viewport and command
line, preserving the Qt resize dividers above and below it.
