Domain interfaces and source layout
===================================

``CalculiXCore`` owns the model and its domain objects. Stateless domain
interfaces forward operations to the shared ``ccx_core`` instance. Creating an
interface does not initialize a second model or allocate a domain object.

Each directory beneath ``src/Core`` contains its interface, associated classes,
struct headers and a ``CMakeLists.txt``. The parent build creates
``calculix_core`` before adding the Core directory; each child registers its
explicit source list with ``target_sources(calculix_core PRIVATE ...)``.
All domains remain in the existing shared library.

.. list-table:: Domain ownership
   :header-rows: 1
   :widths: 22 35 43

   * - Directory
     - Interface
     - Responsibilities
   * - ``application``
     - ``CalculiXApplicationInterface``
     - Commands, logging, lifecycle, persistence and observer access.
   * - ``mesh``
     - ``CalculiXMeshInterface``
     - Blocks, sets, element types, connectivity and entity queries.
   * - ``materials``
     - ``CalculiXMaterialsInterface``
     - Material queries, library operations and material export.
   * - ``sections``
     - ``CalculiXSectionsInterface``
     - Sections and orientations.
   * - ``interactions``
     - ``CalculiXInteractionsInterface``
     - Constraints, surface interactions and contact pairs.
   * - ``loads``
     - ``CalculiXLoadsInterface``
     - Load definitions and every trajectory implementation and query.
   * - ``bcs``
     - ``CalculiXBCsInterface``
     - Displacement, temperature and homogeneous boundary conditions.
   * - ``amplitudes``
     - ``CalculiXAmplitudesInterface``
     - Amplitude definitions and export.
   * - ``steps``
     - ``CalculiXStepsInterface``
     - Steps, assignments, outputs, initial conditions, damping and constants.
   * - ``jobs``
     - ``CalculiXJobsInterface``
     - Job configuration, input preparation, processes and status.
   * - ``results``
     - ``CalculiXResultsInterface``
     - Result loading, conversion, projection, plotting and derived values.
   * - ``results/frd``
     - ``CalculiXFrdInterface``
     - FRD result queries; reader and reader-specific structs.
   * - ``results/dat``
     - ``CalculiXDatInterface``
     - DAT result queries; reader and reader-specific structs.
   * - ``topology``
     - ``CalculiXTopologyInterface``
     - Topology configuration, run entry points, queries and export context.
   * - ``topology``
     - ``CalculiXTopologyRunInterface``
     - Internal iteration preparation, physical/adjoint evaluation and updates.
   * - ``drawing``
     - ``CalculiXDrawInterface``
     - Drawing data, drawing commands and topology animation.
   * - ``export``
     - ``CalculiXExportInterface``
     - Export preparation/cleanup and custom lines.
   * - ``gui``
     - ``CalculiXGuiResourcesInterface``
     - Icon forwarding, compiled into the core library.

Dependency rules
----------------

Include another domain using its path from the ``src`` include root. Use a
named local interface in the method that needs it, for example::

   #include "Core/loads/CalculiXLoadsInterface.hpp"

   bool update_force(int force_id, std::vector<std::string> options, std::vector<int> markers)
   {
     CalculiXLoadsInterface ccx_loads_iface;
     return ccx_loads_iface.modify_loadsforces(force_id, options, markers);
   }

Interfaces have no owning pointers and need no heap allocation. Caller headers
do not include interfaces used only by implementation methods. Interface headers
include their standard-library dependencies and forward-declare domain records
where sufficient. Callers that construct or inspect records include the domain's
struct header explicitly. Interface implementations include
``Core/loadCalculiXCore.hpp``; public interface headers do not.

Load definitions belong to the loads interface; attaching loads to a step belongs
to the steps interface. Tree-data queries follow the same ownership rule.
Drawing operations belong to the drawing interface regardless of the entity.
The Python API keeps its existing public method signatures and routes methods to
the relevant Jobs, Topology, FRD, DAT and Loads interfaces.

Observer identity
-----------------

``CalculiXApplicationInterface::core_observer()`` defines the single shared
observer in its implementation file. Plugin registration and GUI callback
installation access that same instance. ``CoreObserver`` uses local application
interfaces for model operations and does not own an interface pointer.

Incremental builds
------------------

Implementation changes normally recompile that translation unit and relink the
affected targets. Interface declaration changes recompile consumers of that
domain header. Changes to central core declarations or shared struct headers can
still affect several domains. Directory boundaries organize source ownership;
header dependencies determine recompilation scope.

Command and GUI feature directories
-----------------------------------

``src/Commands`` and ``src/GUI`` use the same feature names as the core.
Each class's header and implementation stay together. Each feature directory
registers its explicit source list through its own ``CMakeLists.txt``.
Command sources belong to ``calculix_plugin``; GUI sources belong to
``calculix_comp``. The parent build adds these directories after creating both
targets. Qt ``AUTOMOC`` remains enabled on ``calculix_comp`` and the source lists
include the moved Qt headers.

.. list-table:: Command and GUI ownership
   :header-rows: 1
   :widths: 20 40 40

   * - Feature
     - Commands
     - GUI
   * - ``application``
     - Version, reset, initialization, logging and application options.
     - Model tree, GUI timer and command submission.
   * - ``common``
     - No command directory.
     - Shared panel table, highlighting and tree context-menu helpers.
   * - ``mesh``
     - Block element types and wrapping.
     - Block/node/sideset trees and the element-type panel.
   * - ``materials``
     - Material-library operations.
     - Material management, cards, tables and material tree.
   * - ``sections``
     - Sections and orientations.
     - Section and orientation panels and trees.
   * - ``interactions``
     - Constraints, contact pairs and surface interactions.
     - Corresponding panels and trees.
   * - ``loads``
     - All load commands, including trajectories.
     - All load panels and trees, including trajectories.
   * - ``bcs``
     - Displacement, temperature and homogeneous BC commands.
     - BC panels/trees and homogeneous BC management.
   * - ``amplitudes``
     - Amplitude operations.
     - Amplitude panels and tree.
   * - ``steps``
     - Steps, assignments, outputs, initial conditions, damping and constants.
     - Corresponding panels, trees and step management.
   * - ``jobs``
     - Job configuration, run, wait and kill.
     - Every ``Jobs*`` class, including all ``JobsMonitor*`` classes.
   * - ``results``
     - Load, conversion, projection and external viewers.
     - No separate GUI directory.
   * - ``results/frd`` and ``results/dat``
     - Result plotting and CSV commands.
     - Monitor widgets remain in ``GUI/jobs``.
   * - ``topology``
     - Configuration, preservation and run control.
     - Topology panels/tree and the displacement-text helper header.
   * - ``drawing``
     - Draw, draw-step and topology animation.
     - No separate GUI directory.
   * - ``export``
     - Export and custom lines.
     - Custom-line panels and tree.

FRD/DAT monitors, their plots and section views, and live-monitor widgets all
remain together in ``src/GUI/jobs``. Their result-query dependencies still use
the core's FRD/DAT interfaces. Step-specific load and BC trees belong to
``GUI/steps``; load and BC definition trees belong to their respective features.

Use path-qualified includes across features, for example
``GUI/jobs/JobsMonitor.hpp``, ``GUI/common/PanelTable.hpp`` and
``Commands/loads/ccxLoadsForcesModifyCommand.hpp``. A class can include an adjacent
header by filename. The shared ``src`` include root resolves these paths; the
build does not add every feature directory to a global include search path.

This organization does not change command syntax, widget class names, icons,
resource locations or saved-model data. Adding a class requires listing its
source/header in the owning feature's CMake file. Header-only helpers are listed
explicitly too.
