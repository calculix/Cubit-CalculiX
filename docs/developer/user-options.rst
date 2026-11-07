Framework user options
======================

Ownership and initialization
----------------------------

`FrameworkContext` owns `UserOptionsStore`, implemented in `cubitsim_storage`.
It resolves paths and passes `UserOptionsInterface&` to module initialization.
The service outlives the modules; modules must release borrowed pointers on
shutdown. GUI pages are unregistered before workspace widgets are destroyed.

Every workspace declares each supported key, kind, and explicit default.
A validator may constrain the declared type further. Invalid defaults,
duplicate keys, parent/child option-key collisions, undeclared writes and
wrong value types are errors. Validators are synchronous and must not mutate
workspace state. No function stores the caller's validators.

Storage
-------

The single file is named `cubitcubitsim_user_options.h5`. Cubit's APPSETTINGS API
supplies the preferred directory. Empty or existing non-directory results
fall back to the startup working directory. Directory creation or file access
errors are reported; the store does not silently switch files after startup.
The exact SDK path mapping remains a runtime verification item.

The root contains integer schema_version 1 and the framework group.
Workspace values live below workspaces/<stable-id>, with schema_version 1.
Each option key is a group containing kind and value datasets of rank one.
Boolean values are stored as integer zero or one. Integer, real and UTF-8
string values use the existing HDF5Tool dataset methods.

Missing values use declared defaults. Invalid saved values are reported and
replaced by their declared defaults. Unsupported schema versions or an
unreadable/corrupt file cause failure without replacing the original.
Each transaction locks the file, copies it to staging, changes only declared
workspace items, closes HDF5 handles, then commits with QSaveFile. Unknown
keys and other workspace groups are preserved. A busy file returns an error;
the caller can retry. Concurrent edits to the same workspace use the latest
successful complete save; this is not a per-field merge.

ComponentPath values are absolute or relative to the host component directory.
Workspace defaults specify their own resource subdirectories. Files under
the component directory are saved relative to it, preserving relocation.
External absolute paths stay absolute. Resources are validated when used.

CalculiX
--------

CoreUserOptions.cpp defines all 17 defaults and their validators.
The data DLL retains ccx_uo, initialized with validated effective values
before Python, material-library or GUI resource initialization. Core also
retains the saved configuration returned to the options panel. These snapshots
are identical at startup; ccx_uo always represents the active session values.
The UserOptions and DrawingOptions constructors supply safe empty state;
they do not define user preference defaults.

The GUI owns the options widgets and retains a baseline snapshot from refresh.
It validates edited values and submits readable commands only for differences.
Commands pass typed changes through the application interface. Core reads the
saved snapshot, validates keys, types and values, merges the patch and prepares
both saved and active candidates before saving the file.
The active candidate retains the current icons, Python-interface and
material-library paths. Only after saving succeeds does Core commit both
snapshots using non-throwing member swaps. Validation or save failures leave
both snapshots unchanged. A drawing refresh failure after a successful save
is reported as a warning, rather than presenting the committed save as failed.

The following options take effect at different times:

* Solver, CGX and ParaView paths: next launch of the corresponding program.
* Solver and converter thread counts: next operation.
* Save loaded results: next model save.
* Drawing colors: immediately, through the existing drawing interface.
* Drawing size and visibility: new or reset models.
* Icons, Python-interface and material-library paths: after restarting Cubit.

Restart-dependent paths are saved to HDF5 immediately, but remain unchanged in
ccx_uo for the current session. In particular, material operations continue
opening the active library so existing GUI group references do not switch to
a different file. The panel displays the saved configuration, marks those
fields with tooltips and lists the pending restart changes. Core compares
normalized saved and active paths; restoring an active path and applying it
clears that item's restart notice. Windows comparisons ignore case.

Successful Apply refreshes the panel from the saved configuration. Edits made
since the last successful Apply remain local to widgets until another Apply
succeeds. Cancel discards those unapplied edits when the panel is refreshed
on its next opening; it does not undo a previously successful Apply.

No ccx.cfg import or compatibility reader exists.

Framework monitoring
--------------------

``Framework/Core/Monitoring/FrameworkUserOptions`` defines monitoring defaults
under workspace ``framework``. Its Cubit-Sim options page is registered alongside
workspace pages without creating a new workspace. Apply submits
individual ``sim useroptions monitor ...`` commands for edited settings. Settings take
effect immediately; headless Apply only saves preferences. Validation and save
failure leave the active snapshot unchanged. See :doc:`resource-monitoring` for
ownership, native-platform contracts and deferred verification.

Postprocessor
-------------

``Postprocessor/Core/options/PostprocessorUserOptions`` owns the schema,
validators, saved configuration and active icons path. The initial schema has one
ComponentPath key, ``paths/icons``, defaulting to ``Icons/``. It loads through the
borrowed Framework ``UserOptionsInterface`` under workspace ``postprocessor``
before GUI startup, including headless startup. The store must outlive the
application and is detached during module shutdown.

``PostprocessorApplicationInterface`` exposes initialization, saved preferences,
active path, pending-restart status and Apply. GUI and commands use that boundary.
``GUI/Options/PostprocessorOptionsPanel`` supplies the existing Framework options
page callbacks. Apply emits ``post useroptions icons "<path>"`` only after an edit;
the command uses a weak application handle and passes a typed change to Core. Empty,
unknown-key, wrong-type and invalid-path updates fail before saving.
Absolute paths inside the component directory are saved relative to it. External
paths remain absolute. The resource need not exist until used.

Core prepares the saved candidate before persistence and publishes it with a
non-throwing string swap only after a successful save. The active icons path is
unchanged until restart. Failure leaves both snapshots unchanged. Restart status
compares normalized paths, ignoring case on Windows. The SVG loader accepts the
active directory and contains no preference defaults or storage access.

Future preferences, including background colors and other display defaults,
extend this schema and its typed snapshot, typed validation, panel and tests.
No additional options registry or persistence file is required. Each setting must
declare whether it changes current views, supplies defaults for new views, or
requires restart. Background colors are not implemented by this patch.

Verification pending build authorization
----------------------------------------

Verify APPSETTINGS in GUI and headless Cubit, first startup, every supported
type, missing/new options, invalid types and values, file locking, atomic-save
failure, Unicode paths, component relocation, and preservation of unrelated
workspace groups. Verify panel Apply/Cancel, command replay, startup failure
cleanup and GUI detach/reattach. No compilation is authorized at proposal time.

Readable command transport
--------------------------

``Framework/Commands/UserOptionCommandSupport.hpp`` derives textual names and
argument kinds from Core schemas, formats quoted UTF-8 strings and locale-neutral
round-trip numbers, and converts parser values into typed changes. It defines no
defaults. ``merge_user_option_changes`` rejects every invalid entry before
modifying the candidate snapshot. The existing store owns persistence.

The old hex/JSON command transport is removed. ``useroptions list`` queries saved
values through each established application interface and prints the same setter
syntax plus separate restart notices. Per-command persistence is transactional;
a GUI Apply containing several commands is not one storage transaction.

``tests/monitoring/FrameworkOptionsTests.cpp`` covers typed partial updates,
invalid types/values, failed persistence, string transport and GUI change
filtering. ``tests/verify_user_options.jou`` checks list commands and rejected
setters through Cubit's parser without changing preferences. Native tests and
interactive GUI command replay require the user's build; no build is performed
as part of this change.
