FEA framework: Stage 1 foundation
=================================

Status and scope
----------------

This stage adds opt-in framework entry points and shared module infrastructure.
No CalculiX code is moved, no existing plugin is replaced, and no Postprocessor
or Report module is introduced. The application-switch UI is a later stage,
immediately after CalculiX is migrated into the framework.

The current CalculiX plugin/component remain the supported entry points until
Stage 2. The Stage 1 framework registers no modules and exposes no commands.
Do not deploy its plugin as a replacement for the current CalculiX plugin yet.

Build integration
-----------------

``CCX_BUILD_FRAMEWORK`` defaults to ``OFF``. Enabling it adds:

* ``fea_framework``: shared context and module manager.
* ``fea_plugin``: Cubit command-plugin entry point.
* ``fea_comp``: Cubit GUI component and SWIG binding.

Outputs are isolated under ``<build>/framework-stage1/<configuration>``.
The component module is ``fea_comp`` and its class is ``FEAComp``. There is no
new install rule, plugin-path change, startup configuration or automatic loading.
Existing SDK/Python development paths are reused. The only Python-related
addition is the loader binding; application logic remains C++.

Ownership and lifetime
----------------------

``framework_context()`` is defined in the shared framework library. Both entry
points link that same library instead of compiling their own context instance.

Each successful entry-point startup acquires a context reference. The first
reference initializes modules. The last release shuts them down. The component
stops the GUI providers it owns before releasing its reference. Thus unloading
the component does not shut down command services while the plugin is loaded.
Repeated component startup/cleanup is guarded. Only one GUI owner is allowed.

``ModuleManager`` owns module objects. Providers are borrowed from their module.
The Cubit main window is borrowed. A returned command is owned by Cubit.
Cubit must destroy module command handlers before unloading their code/library.
No dynamic library unloading or live module replacement is implemented here.

Registration and interfaces
---------------------------

``register_framework_modules()`` is the single explicit registration point.
It is empty in Stage 1; Stage 2 adds the CalculiX module there. Registration
occurs before startup. The module set is frozen on first initialization.

``ModuleInterface`` provides stable identity, initialization/cleanup and optional
command/GUI providers. It does not expose CalculiX types or result schemas.
``CommandProviderInterface`` supplies stable command keys and command factories.
``GuiProviderInterface`` supplies GUI attachment and cleanup without owning the host.

Modules initialize in explicit registration order and shut down in reverse.
Registration rejects empty/duplicate module IDs. Startup rejects empty/duplicate
command keys. Initialization/GUI failure unwinds already-started providers in
reverse order, including the provider whose startup was incomplete. Cleanup must
be idempotent and must not throw. Normal command-handler execution errors remain
the responsibility of each command implementation.

Framework lifecycle and registration are serialized by the host thread. This
stage does not make module services thread-safe. Background result loading and
GUI-thread dispatch need explicit contracts when those features are introduced.

Migration boundaries
--------------------

No existing ccx syntax, Python API, options handling, core observer, result
reader, converter or saved HDF5 group is changed in this stage. Stage 2 must
transfer existing entry-point responsibilities without creating a second
``ccx_core`` or registering the same observer/commands twice. It must preserve
existing journals and persistence formats. Solver-neutral data interfaces are
designed after that migration, not guessed into the Stage 1 foundation.

Review and verification status
------------------------------

The proposal is code for review, not a compiled implementation. Do not configure,
build, load the component or launch Cubit without the user's separate consent.
Later checks should cover both entry-point load orders, repeated cleanup, partial
startup failure, duplicate IDs/keys, GUI startup without a host, command-only mode,
and the existing CalculiX baseline with the option disabled.
