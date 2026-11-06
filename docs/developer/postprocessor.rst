Postprocessor architecture and source lifecycle
===============================================

Implementation status
---------------------

The Postprocessor currently provides Core data definitions, a source lifecycle
owner and shared result-provider contracts. Framework registers the CalculiX
provider under command prefix ``ccx`` and application identity ``calculix``.
The Postprocessor module and initial ``post`` job/source command handlers are
registered in source. Workspace panels and an independent empty viewport are
described in :doc:`postprocessor-workspace`. See :doc:`../postprocessor/commands`
for syntax and limitations. The user reported a successful rebuild after the
member-wise swap fix; the subsequent workspace patch has not been compiled or
verified in a native Cubit session.

The default session defines one 3D view and one layout pane. Additional sources
do not create views. The Postprocessor viewport is separate from Cubit's viewport.
Result actors, features, geometry loading, persistence,
independent VTK file readers and independent FRD/DAT loading are not implemented.

.. toctree::
   :maxdepth: 1

   postprocessor-verification
   postprocessor-workspace

Implementation map
------------------

Paths below are relative to ``src``. Downloads preserve source navigation in
the offline manual, following the topology implementation chapters.

.. list-table:: Files and methods
   :header-rows: 1
   :widths: 30 70

   * - Responsibility
     - Source and entry points
   * - Core ownership and queries
     - :download:`Postprocessor/Core/application/PostprocessorCore.hpp <../../src/Postprocessor/Core/application/PostprocessorCore.hpp>` and :download:`implementation <../../src/Postprocessor/Core/application/PostprocessorCore.cpp>`: ``session``, ``source_id_from_name``, ``result_data``.
   * - Application module and boundary
     - :download:`PostprocessorModule.hpp <../../src/Postprocessor/PostprocessorModule.hpp>` and :download:`implementation <../../src/Postprocessor/PostprocessorModule.cpp>` own the application and command factory. :download:`PostprocessorApplicationInterface.hpp <../../src/Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp>` and :download:`implementation <../../src/Postprocessor/Interfaces/PostprocessorApplicationInterface.cpp>` resolve prefixes and names before Core operations.
   * - Command registration and handlers
     - :download:`PostprocessorCommands.cpp <../../src/Postprocessor/Commands/PostprocessorCommands.cpp>` registers :download:`open <../../src/Postprocessor/Commands/sources/PostOpenCommand.cpp>`, :download:`list <../../src/Postprocessor/Commands/sources/PostListCommand.cpp>`, :download:`select <../../src/Postprocessor/Commands/sources/PostSelectCommand.cpp>` and :download:`close <../../src/Postprocessor/Commands/sources/PostCloseCommand.cpp>`. Cubit owns each command; handlers keep weak application references.
   * - Source lifecycle
     - :download:`Postprocessor/Core/sources/PostprocessorCoreSources.cpp <../../src/Postprocessor/Core/sources/PostprocessorCoreSources.cpp>`: ``open_result``, ``refresh_result``, ``select_source``, ``close_source``, ``close_all_sources``.
   * - Persistent definitions
     - :download:`session <../../src/Postprocessor/Core/session/PostprocessorSession.hpp>`, :download:`source <../../src/Postprocessor/Core/sources/DataSource.hpp>`, :download:`source references <../../src/Postprocessor/Core/sources/SourceReference.hpp>`, :download:`representation <../../src/Postprocessor/Core/representations/Representation.hpp>`, :download:`view <../../src/Postprocessor/Core/views/ViewDefinition.hpp>` and :download:`layout <../../src/Postprocessor/Core/layouts/LayoutDefinition.hpp>`. These are definitions intended for future persistence; no serializer exists yet.
   * - Runtime result ownership
     - :download:`SourceRuntime.hpp <../../src/Postprocessor/Core/sources/SourceRuntime.hpp>`: immutable data handle and monotonically increasing refresh revision.
   * - Shared provider contracts
     - :download:`ResultSourceProviderInterface.hpp <../../src/Framework/Interfaces/ResultSourceProviderInterface.hpp>`, :download:`PublishedResultSource.hpp <../../src/Framework/Results/PublishedResultSource.hpp>`, :download:`ResultSourceReference.hpp <../../src/Framework/Results/ResultSourceReference.hpp>` and :download:`VtkResultsData.hpp <../../src/Framework/Results/VtkResultsData.hpp>`.
   * - Provider discovery and lifetime
     - :download:`ModuleInterface.hpp <../../src/Framework/Interfaces/ModuleInterface.hpp>`, :download:`ResultSourceRegistryInterface.hpp <../../src/Framework/Interfaces/ResultSourceRegistryInterface.hpp>` and :download:`ModuleManager.cpp <../../src/Framework/Core/ModuleManager.cpp>`: optional provider accessor, prefix/application registration and combined resolution, cleanup before module shutdown.
   * - CalculiX job loading
     - :download:`CalculiXResultSourceProvider.cpp <../../src/CalculiX/Core/results/CalculiXResultSourceProvider.cpp>` and :download:`declaration <../../src/CalculiX/Core/results/CalculiXResultSourceProvider.hpp>`: ``open``, ``refresh`` and ``load_job`` through existing job/results interfaces. :download:`CalculiXModule.cpp <../../src/CalculiX/CalculiXModule.cpp>` owns the provider.
   * - Build integration
     - :download:`Postprocessor/CMakeLists.txt <../../src/Postprocessor/CMakeLists.txt>`: ``postprocessor_core`` static library, linked only to the shared result-contract target; ``postprocessor_module`` shared library adds application/commands and links the Cubit command/message SDK.

Files and call flow
-------------------

.. code-block:: text

   Cubit command handlers / PostprocessorApplicationInterface
       |
       +--> Framework ModuleManager
       |      result_source_provider("ccx")
       |      resolve_result_provider("ccx") --> provider + "calculix"
       |
       +--> PostprocessorCore
              open_result(provider, request, optional_name, error)
                 |
                 +--> provider.open() --> readers / converter
                 |                         |
                 |                  PublishedResultSource
                 |
                 +--> validate and stage
                 +--> commit source definition + runtime data + selection

   PostprocessorCore
       +-- PostprocessorSession
       |     +-- sources: stable ID, unique name, resolved provenance
       |     +-- views / representations / layout
       |     +-- active source / representation / view
       |
       +-- runtime[source ID]
             +-- shared_ptr<const SimResults::VtkResultsData>
             +-- revision

Ownership, identifiers and naming
---------------------------------

The Core owns its session and runtime registry. Its default constructor creates
``view1`` and ``pane1`` through the session/layout definitions. The Core retains
no provider, reader, GUI or live Cubit entity pointers. Providers are borrowed for
one synchronous call. Framework lookup returns borrowed pointers valid only
during initialized module lifetime. Published data handles own their data and
can outlive provider shutdown, refresh, source closure and Core destruction.

``FrameworkModules`` registers CalculiX before Postprocessor. The Postprocessor
module receives the provider registry through a shared interface; it does not call
``framework_context()`` or link back to the Framework implementation. ModuleManager
resolves prefix and application identity from its existing registries, with no
duplicate application mapping in command code. Lookup is available only after
Framework initialization. Postprocessor's GUI component/workspace provider starts
only in GUI mode; job/source commands remain independent of workspace activation.

Module initialization creates one shared application; the factory and handlers
hold weak references. Each execution locks its handle for the duration of the
command and reports an unavailable application after shutdown. New Framework
initialization creates a new application; old commands cannot target it.

Source IDs are generated internally as ``source1``, ``source2``, and so on.
They are not reused after closure during this Core lifetime. Failed operations
do not consume IDs. The maximum unsigned 64-bit counter value is reserved as
the exhausted state; wraparound is rejected. Restoring an allocator/session is
not implemented. Names are nonempty/nonblank, unique and case-sensitive. An
explicit optional name overrides the job name or filename stem without trimming
or changing case. Omitted names use the provider suggestion. Explicit duplicates
are rejected before provider loading; duplicates in a suggested name are detected
after loading. A duplicate never replaces an existing source.

``session()`` exposes const state. Its nested references and iterators must not
be retained across successful mutations. ``result_data(id)`` returns an owning
handle or an empty handle for an unknown/unloaded source. ``source_id_from_name``
returns an empty string when exact lookup fails. Listing reads ``session().sources``;
there is no duplicate listing store or mutable GUI access path.

Operation contracts
-------------------

``open_result`` validates the request, invokes the provider into temporary output,
validates publication, resolves the name and stages definitions/runtime data.
The commit changes both registries and active source together, clears active
representation context, and advances the ID allocator. It creates no display.

Job publications must match requested application, project and job identity,
carry a nonempty snapshot token, and agree with the data payload's job/token.
An explicit opening token must match exactly. Refresh may resolve a new token.
File requests must name their provider and contain nonempty paths. Published
file references must retain that provider and contain resolved absolute paths.
Actual file resolution/companion selection and array validation are provider
responsibilities; the Core does not independently reread or numerically validate
the supplied arrays. A publication needs a non-null handle with result states
or global tables; individual state contents remain under the provider contract.

``refresh_result`` preserves source ID, name, selection, representations and view
layout. It replaces provenance/data and increments the runtime revision after
validation. Overflow is rejected before provider work. Future feature/rendering
consumers must invalidate derived data when a source revision changes. No such
consumer or invalidation pipeline is implemented in this stage.

``select_source`` selects an existing source by ID and clears active representation
context. ``close_source`` removes the source, runtime handle and direct source
representations. It clears active selection when the selected source/display is
removed. ``close_all_sources`` also clears all representations, retaining views,
cameras and layout. Both closure paths leave solver jobs and source files intact.
Feature dependency checks must be introduced with feature execution; the current
Core cannot create feature definitions or feature representations.

Threading and failure boundary
------------------------------

These methods are host-thread APIs, not concurrent containers. Provider callbacks
may process host events: an RAII flag rejects nested mutations while permitting
read access. The flag is released on success, failure and exceptions. Querying
state during loading sees the previously committed snapshot.

Open/refresh and single-source closure stage metadata and shared handles before
nonthrowing swaps. Result arrays are not copied by these transactions. This costs
metadata copying proportional to the current session/registry size. Failures
return ``false`` with an error string and preserve Postprocessor state, handles,
selection and counters. Provider exceptions are converted to errors. The
command handlers report failures through Cubit's error mechanism and return false.

Publication uses ``PostprocessorSession::swap``, which directly exchanges each
map/string member, including the layout root and nodes. Its exception specification
is conditional on every exchanged member. The compile-time assertion checks that
exact member operation. Generic ``std::swap`` of the session is intentionally
avoided: MSVC's ``std::map`` move constructor can allocate a sentinel, so a
move-based session swap is not guaranteed nonthrowing on that implementation.
New session/layout members must be included in both the exchange and its guarantee.

This transaction excludes provider-side changes: CalculiX loading can clear its
own reader/cache state before a subsequent failure. Existing caller-owned data
handles remain valid. No new ``runtime_error`` exception is introduced.

CalculiX limits and future geometry
-----------------------------------

The CalculiX provider supports job requests in the current model, rejects running
jobs/active conversions, and uses existing readers and ``build_vtk_result``.
Independent FRD/DAT file requests and project-qualified requests are rejected.
Its ``run_identity`` is currently a conversion snapshot token, not a durable
solver-run identity. Explicit tokens can only match retained converted data;
an unavailable historical token must not be silently rebuilt from current files.

Cubit geometry is outside the result-provider implementation. The installed
2026.11 SDK was inspected: ``CubitInterface::cgm_iface()`` exposes
``CubitGeometryInterface`` with entity lookup, topology relationships, geometry
queries and face/edge graphics. Any future geometry adapter should start from
these SDK interfaces. The existing geometry reference is a preliminary definition;
model revision policy, entity lifetime/mapping and geometry publication remain
to be designed and approved. No geometry SDK call is added by this Core patch.

Verification status
-------------------

Source and documentation checks are separate from native execution. The user
reported an MSVC build failure at the original generic-swap assertion and then
confirmed a successful rebuild after the fix. No build, compilation, documentation
generation or native test run was performed by the agent; the workspace patch
remains unverified by compilation. See
:doc:`postprocessor-verification` for the evidence status and acceptance checklist.
