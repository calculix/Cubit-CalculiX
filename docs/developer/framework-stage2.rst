Stage 2: CalculiX module integration
====================================

Stage 2 replaces the old CalculiX loader entry points with one
``CubitSimPlugin`` command plugin and one ``CubitSimComp`` component. The framework
registers an internal ``CalculiXModule``. Module code belongs in
``src/CalculiX``, not in the solver executable directory.

Source layout::

  src/
    CMakeLists.txt
    Framework/
      CubitSimPlugin.hpp/.cpp
      CubitSimComp.hpp/.cpp
      Core/
      Interfaces/
      Utility/
    CalculiX/
      CMakeLists.txt
      CalculiXModule.hpp/.cpp
      Commands/
        CalculiXCommands.hpp/.cpp
        ...existing command domains
      Core/
      GUI/
        CalculiXGui.hpp/.cpp
        MyCmdWidgetFactory.hpp/.cpp
        cmdPanelManager.hpp/.cpp
        CCXDockWindowModelTree.hpp/.cpp
        MenuManager.hpp/.cpp
        ToolbarManager.hpp/.cpp
        ExportManager.hpp/.cpp
        ...existing GUI domains
      EventObservers/
      UserOptions/
        CalculiXSettings.hpp/.cpp
        ...existing options
      PythonInterface/
      Icons/
      Utility/
      libmaterial.hdf5
    Solvers/
      CalculiX/
        Win/
        Linux/
    Utility/     (existing third-party sources)
    Library/
    Postprocessor/
      CGX/

The existing bundled CGX directory is still an external tool, not the new
postprocessor implementation. Shared binary dependencies in ``Library`` and
packaging scripts remain outside the CalculiX source module. Bundled sources
already under ``Utility`` remain in ``src/Utility``, including gif.h. Our own
utility sources move to ``CalculiX/Utility``. This stage does not create a
shared utility framework; small helpers may be copied for future modules.

Loading and ownership
---------------------

The new ``calculix_commands`` and ``calculix_gui`` libraries are internal
providers, not Cubit-discovered entry points. ``calculix_module`` implements
the framework contracts and links those providers. Only ``cubitsim_plugin`` uses
``CUBIT_PLUGIN``; only ``cubitsim_comp`` has component SWIG bindings.

Load ``cubitsim_plugin`` through ``CUBIT_PLUGIN_DIR`` before starting ``CubitSimComp``.
The component reports an error through CubitMessage if that command plugin
has not attached. No automatic scanning of unrelated plugin folders occurs.
Use a fresh deployment directory and restart Cubit: do not mix the old
``calculix_plugin``/``calculix_comp`` binaries with the new deployment.

There remains exactly one shared ``calculix_core`` and one shared
``calculix_useroptions``. GUI detach clears its callbacks and UI registrations;
it does not reset the model or kill solver jobs. Existing command names,
command registration order, Python ``ccx`` facade, model persistence formats
and options panel registration are retained.

The legacy CalculiXPlugin/CalculiXComp sources and component SWIG declaration
are replaced by the internal providers. The already-deprecated
``loadCalculiXComp.cpp`` is removed. The deprecated ExportManager stays with
the CalculiX GUI, excluded from compilation, so this migration does not also
delete that unrelated implementation.

Component startup
-----------------

``CubitSimComp`` starts every component provider with the original ``withGUI``
flag. A null Qt host is valid for headless startup; providers decide which
features require a GUI. CalculiX initializes its material library and Python
interface in both modes, and creates its GUI only when ``withGUI > 0``.

The ``cubitsim_comp`` SWIG binding enables threading so native component calls
release the caller's Python GIL before re-entering Cubit's interpreter.
Python initialization is an explicit component startup step, not a synthetic
``component_loaded`` callback or a GUI-only tracker side effect.

Settings and resources
----------------------

.. note::

   This section describes the Stage 2 implementation. The subsequent
   framework user-options service replaces CalculiXSettings and ccx.cfg
   with required Core declarations and cubitcubitsim_user_options.h5. See
   :doc:`user-options` for the current contract.

CalculiX settings restoration is extracted from the old component into
``CalculiXSettings`` and runs before core initialization, including headless
command use. Missing entries are prefilled with their existing defaults.
The configuration remains ``ccx.cfg`` in the working directory, with the same
keys and standard Cubit Options panel.

Default resource locations are derived from the actual loaded options
library instead of searching the Broker registry for ``calculix_plugin``.
The documentation action uses that same directory. DLLs, icons, material
library, Python proxy and runtime tools retain the existing relative
deployment layout; the source directory rename does not require users to
edit their saved solver path. The source executables move from the current
singular ``src/Solver/CalculiX`` to ``src/Solvers/CalculiX``; deployed solver
files remain in ``Solver/`` for compatibility.

Build changes
-------------

The root CMake file discovers SDK/Qt/HDF5/CGAL/Threads/SWIG and adds the
CalculiX module before Framework. CalculiX-specific source lists, Python API
binding, resources and targets live in ``CalculiX/CMakeLists.txt``. Per-domain
command/GUI lists target the internal provider libraries. The framework is
now part of the normal build rather than an empty opt-in Stage 1 build.
The dependency sources under Utility retain their existing contents.

This proposal requires no changes to the result reader, VTK converter, job
format or .cub5 schema. Workspace switching is the following stage, before
the user's pending CalculiX GUI changes and any postprocessor implementation.
The bottom command/error/journal dock remains untouched.

Validation boundary
-------------------

The source migration preserves all 175 command keys, their registration order
and key-to-command-class mappings. Application include paths, explicit CMake
source paths and diff whitespace are checked without configuring the project.
No CMake configure, compilation, Cubit launch or runtime validation has been
performed for Stage 2. Those checks require separate user consent.
