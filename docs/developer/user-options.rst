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

The GUI owns the options widgets. It creates a complete JSON candidate and
hex-encodes it into the ccx useroptions apply command so quoting and Unicode
cannot alter command parsing. Core validates every key and value and prepares
both the saved configuration and active candidate before saving the file.
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

Verification pending build authorization
----------------------------------------

Verify APPSETTINGS in GUI and headless Cubit, first startup, every supported
type, missing/new options, invalid types and values, file locking, atomic-save
failure, Unicode paths, component relocation, and preservation of unrelated
workspace groups. Verify panel Apply/Cancel, command replay, startup failure
cleanup and GUI detach/reattach. No compilation is authorized at proposal time.
