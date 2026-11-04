User options
============

Open Cubit's options dialog. Each registered workspace contributes its own page;
CalculiX also exposes **CalculiX > Options**. Apply saves that workspace's
settings. Cancel does not apply edits that have not already been applied.

Preferences live in one ``cubitfea_user_options.h5`` file in Cubit's standard
settings directory. If the SDK supplies no usable directory, the startup working
directory is used with a warning. The selected filename remains fixed for the
session. Preferences are separate from the saved finite-element model.

Each workspace defines explicit defaults. Missing values receive those defaults;
invalid saved values are reported and replaced with defaults. An unreadable file
or unsupported schema causes an error. A busy file also reports an error; retry
after the other process finishes saving. Complete saves from different processes
are serialized, but edits within the same workspace are not merged per field.

Relative resource paths refer to the Framework component directory. Paths inside
that directory are saved relative to it, so the installation can be relocated.
External absolute paths remain absolute. Resource availability is checked when
used. The former CalculiX ``ccx.cfg`` is not imported.

CalculiX settings
-----------------

The CalculiX page configures solver and postprocessor paths, icon and Python
resources, the material-library path, solver/converter thread counts, loaded
result persistence and drawing defaults. Drawing size and visibility defaults
apply to new or reset models; hover and selection colors update the current view.
Changing Python or material-library paths does not reinitialize existing handles.
These are application preferences, not topology case settings.

For the storage and module contracts, see :doc:`../developer/user-options`.
