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
