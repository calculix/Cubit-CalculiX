CalculiX model lifecycle, commands and persistence
==================================================

One model, several entry paths
------------------------------

``loadCalculiXCore`` supplies the shared ``ccx_core``. ``CalculiXCore`` coordinates
the domain objects declared in its header; its member definitions are split
between domain translation units. A ``CalculiXCoreTopology...cpp`` or
``CalculiXCoreStep...cpp`` file is part of that same class, not another model.
Stateless domain interfaces forward requests to it.

Commands, GUI actions and Python queries converge on those interfaces:

.. code-block:: text

   Cubit command -> command parser -> domain interface -> existing Core state
   Qt editor     -> GUI command    -> command parser   -> same Core state
   Python query  -> Python wrapper -> domain interface -> same Core state

``CalculiXModule`` supplies the Framework providers. The Framework owns module
lifetime; the CalculiX module owns its internal GUI and command provider. Native
geometry/mesh remains Cubit's property. Do not duplicate coordinates/connectivity
in persistent feature records merely to communicate between domains.

Lifecycle phases
-----------------

``init()``, ``init2()``, ``update()`` and ``reset()`` have different roles. Check
their current implementations rather than assuming every ``init`` is a pure
idempotent read. The second-stage initialization acquires ``MeshExport`` and
``Material`` SDK interfaces and initializes domains. Mesh export initialization
has host-side effects and must not be invoked casually from a background worker.
Material-library initialization can be deferred until Cubit is fully loaded.

``update()`` synchronizes the domains that need current Cubit entities and
maintains derived relationships. Plugin-owned loads/BCs are not wholesale mirrors
of all native Cubit load definitions. ``reset()`` must coordinate controller and
worker termination with domain reset and GUI invalidation. A cleared model cannot
leave timers or editors referring to old IDs as though they were current data.

Source: :download:`CalculiXCore.hpp <../../src/CalculiX/Core/CalculiXCore.hpp>`,
:download:`initialization <../../src/CalculiX/Core/application/CalculiXCoreInitialization.cpp>`,
:download:`maintenance <../../src/CalculiX/Core/application/CalculiXCoreMaintenance.cpp>`
and :download:`CoreObserver.cpp <../../src/CalculiX/Core/application/CoreObserver.cpp>`.

Feature records and typed settings
----------------------------------

Many older domains store a primary table plus subordinate tables. A public
feature ID identifies a row through an explicit lookup function; it is not the
row offset. Primary records can reference a type-specific ID, name record,
amplitude, time delay or set. When deleting a feature, remove or validate related
assignments through the existing cleanup/coordination path.

Typed ``*Settings`` and ``*Properties`` records provide a read view for editors
and properties. They do not authorize a caller to mutate another domain's raw
tables. Create/modify commands often use presence markers: an omitted value
leaves the default/current value intact, whereas an explicit zero has the
parameter's documented meaning. For topology, historical designs capture the
method and interpolation settings independently of later case edits.

Command registration and execution
----------------------------------

``CalculiXCommands`` lists command keys and creates matching ``ccx*Command``
objects. A key is a registration identity; ``get_syntax()`` defines the text Cubit
parses. Most commands provide ``get_syntax_help()``, ``get_help()`` and
``execute(CubitCommandData&)``. Keep the syntax labels consistent with the keys
read by ``execute``. Generated navigation marker names are a separate domain
from command keys and user-facing command strings.

The parser should validate counts without silently truncating fractional values,
check missing values and then call the appropriate interface. Domain validation
must still protect callers that bypass a GUI. Do not assume a disabled button
is sufficient model validation. A return value must be interpreted at the layer
that produced it: GUI delivery, command acceptance and completed solver work are
different outcomes. See :doc:`code-gui` and :doc:`../adding-a-feature`.

Model persistence
------------------

``CalculiXCore::read_cub()`` and ``save_cub()`` coordinate domain persistence through
the project HDF5 adapter. They handle host progress reporting, cancellation and
busy guards. A scope guard restores a temporary flag; it does not undo model
mutations already committed by a domain. Do not infer a global all-domain
transaction from local staging in one reader.

The ``Core*Persistence`` helpers own domain layouts and validation. File metadata,
model settings, job records, drawing state and topology data have distinct
responsibilities. Current topology saves have one required layout without
backward migrations. That topology policy does not imply that unrelated options
storage formats have no schema handling.

Saved process IDs are not permission to attach to or kill an operating-system
process in a later session. Loading a running topology case does not recreate its
controller or resume a proposal. Job references and native numeric histories may
survive, but external solver files and a matching mesh remain separate evidence.

Source: :download:`CalculiXCorePersistence.cpp <../../src/CalculiX/Core/application/CalculiXCorePersistence.cpp>`
and the persistence helpers listed in :doc:`code-catalog/index`.

Thread and event boundaries
---------------------------

``CoreTimer`` and observers coordinate periodic maintenance, job progress and
host notifications. Their existence does not make every operation asynchronous.
Result loading and current VTK building can perform synchronous host-thread work.
Reentrant command processing during native events requires the existing busy
guards; it is not equivalent to background execution.

GUI observers belong to the GUI lifecycle; model observers belong to model/module
coordination. Unregister callbacks before owners disappear, stop polling during
teardown, and avoid SDK access after Qt/native shutdown starts. Evidence for
Windows and Linux shutdown remains platform-specific.
