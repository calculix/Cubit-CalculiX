cubit-sim project identity
=========================

The application is named cubit-sim. Native entry points are cubitsim_plugin
and cubitsim_comp; its Python class is CubitSimComp. The shared targets are
cubitsim_framework, cubitsim_storage, cubitsim_results and cubitsim_results_io.

Framework commands are sim version, sim workspace and sim documentation.
Version reporting is implemented by CubitSimVersionCommand and has no
CalculiX workspace dependency. The previous workspace version command is
removed. CalculiX remains the solver/workspace identity and retains its other
ccx commands, module libraries and runtime options object.

Preferences use cubitsim_user_options.h5 and the cubit-sim QSettings identity.
There is no old-name preferences migration. CUBIT_PLUGIN_DIR is a Cubit SDK
setting and keeps its standard name. Diagnostic tracing uses
CUBITSIM_STARTUP_TRACE.

New model design
----------------

The framework owns cubit-sim/metadata/schema_version (integer 1) and
cubit-sim/metadata/application_version (string). Each workspace owns its
namespace and its own schema version. CalculiX stores data beneath
cubit-sim/workspaces/calculix; its metadata contains schema_version (integer 1)
and workspace_version (string). Domain dataset suffixes retain their meaning.

No legacy model reader or converter is provided. Older models remain the
responsibility of the older releases. Unsupported new schema versions are
rejected before workspace data is loaded.

Deployment
----------

Use a fresh build/output directory when building is authorized. Register the
cubitsim_comp module and CubitSimComp class in Cubit, replacing the old
component registration. Do not put both old and new plugin entry-point DLLs
in the active plugin directory. Existing built binaries and generated files
are not rewritten by this source proposal.

Renaming the GitHub repository and the active local checkout directory are
separate operations. Historical download URLs and release history retain
their actual destinations until an external repository rename is performed.
No settings file or model is rewritten by this proposal.
