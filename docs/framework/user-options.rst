User options
============

Open Cubit's options dialog. Each registered workspace contributes its own page;
CalculiX also exposes **CalculiX > Options**. Apply saves that workspace's
settings. Cancel does not apply edits that have not already been applied.

Preferences live in one ``cubitcubitsim_user_options.h5`` file in Cubit's standard
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

Framework settings
------------------

The **Cubit-Sim** page configures the resource indicators at the right of Cubit's
existing bottom status row: enabled
state, refresh interval, CPU/RAM bars, Jobs button, Cubit/job attribution and three
colors. Defaults enable both bars and Jobs, with blue Cubit, orange jobs and green
remaining usage at a two-second interval. Apply submits a readable command for
each changed setting and takes effect immediately in every workspace.
See :doc:`resource-monitoring` for metric definitions.

CalculiX settings
-----------------

The CalculiX page configures solver and postprocessor paths, icon and Python
resources, the material-library path, solver/converter thread counts, loaded
result persistence and drawing defaults. Drawing size and visibility defaults
apply to new or reset models; hover and selection colors update the current view.
Changing Python or material-library paths does not reinitialize existing handles.
These are application preferences, not topology case settings.

Postprocessor settings
----------------------

The Postprocessor page has its own icons-directory preference, defaulting to
``Icons/`` relative to the component directory. Browse selects an external
directory. Apply records ``post useroptions icons "<path>"`` only when edited and
saves the preference in the shared options file under the Postprocessor workspace.
Icon-path changes take effect after restarting Cubit; the page marks pending
changes. Cancel discards unapplied edits when the page is refreshed on reopening.
Missing SVG files produce empty icons and do not prevent startup.

Additional display defaults, such as background colors, will belong to the same
Postprocessor options schema. They are not implemented yet. Application defaults
and settings of individual existing views are separate concepts; each future
option must specify when it takes effect.

For the storage and module contracts, see :doc:`../developer/user-options`.

Readable commands
-----------------

Each command changes only its named option. Boolean values accept ``on`` or
``off``. Paths accept quoted UTF-8 text, including spaces, and can be relative to
the component directory or absolute. Embedded quotes, NUL and line breaks cannot
be represented by these commands. Windows paths in generated commands use forward
slashes. The old encoded ``apply`` syntax has been removed.

Framework settings (example values)::

   sim useroptions monitor on
   sim useroptions monitor interval 2
   sim useroptions monitor cpu on
   sim useroptions monitor memory on
   sim useroptions monitor cubit on
   sim useroptions monitor jobs on
   sim useroptions monitor job_attribution on
   sim useroptions monitor system_color "#5CB85C"
   sim useroptions monitor cubit_color "#5B9BD5"
   sim useroptions monitor jobs_color "#E6A23C"

The interval is an integer from 1 to 60 seconds. Monitor colors use ``#RRGGBB``.

CalculiX settings (Windows example paths)::

   ccx useroptions solver "Solver/ccx_2.23.exe"
   ccx useroptions cgx "Postprocessor/CGX/cgx_STATIC.exe"
   ccx useroptions paraview "Postprocessor/ParaView/bin/paraview.exe"
   ccx useroptions icons "Icons/"
   ccx useroptions python_interface "."
   ccx useroptions material_library "libmaterial.hdf5"
   ccx useroptions solver_threads 8
   ccx useroptions converter_threads 8
   ccx useroptions save_loaded_results on
   ccx useroptions drawing size_factor 1.0
   ccx useroptions drawing overlay on
   ccx useroptions drawing boundary_conditions off
   ccx useroptions drawing loads off
   ccx useroptions drawing constraints off
   ccx useroptions drawing orientations off
   ccx useroptions drawing hover_color red
   ccx useroptions drawing selection_color yellow

Thread counts are positive integers. The size factor must be finite and positive.
Drawing colors accept ``red``, ``yellow``, ``green``, ``blue``, ``cyan``,
``magenta``, ``white``, ``black`` and ``orange``.

Postprocessor settings::

   post useroptions icons "Icons/"

List saved values as replayable commands::

   sim useroptions list
   ccx useroptions list
   post useroptions list

Pending restart requirements are printed separately. Listing does not apply the
settings. The GUI validates all edited values before submitting the first
command, submits one command per changed setting, and submits no commands for
unchanged pages. Each successful command persists separately; if a later command
fails, earlier successful changes remain saved. Defaults remain in workspace Core.
