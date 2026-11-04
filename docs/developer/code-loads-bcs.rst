Loads, boundary conditions and amplitudes
=========================================

Definition and step assignment
-------------------------------

Load and BC definitions are plugin-owned feature records. A definition's presence
does not mean it is assigned to every analysis step. Step assignment, operation
mode, amplitude and optional time delay determine how it participates in the
export. The topology preservation resolver considers the relevant referenced
definitions, rather than all native Cubit load objects indiscriminately.

Most domains use a primary integer table and subordinate value/name/time-delay
tables. For example, a force primary row links load ID, operation mode, amplitude,
time delay, nodeset, force payload and name. Displacement BCs additionally track
per-DOF modes. Typed settings isolate GUI/property consumers from these row
layouts. Cleanup removes assignments through the existing step/reference helpers.

Source: :download:`CoreLoadsForces.hpp <../../src/CalculiX/Core/loads/CoreLoadsForces.hpp>`,
:download:`CoreBCsDisplacements.hpp <../../src/CalculiX/Core/bcs/CoreBCsDisplacements.hpp>`,
:download:`CoreSteps.hpp <../../src/CalculiX/Core/steps/CoreSteps.hpp>` and
:download:`CoreAmplitudes.hpp <../../src/CalculiX/Core/amplitudes/CoreAmplitudes.hpp>`.

Load families
--------------

.. list-table:: Project load domains
   :header-rows: 1
   :widths: 30 70

   * - Core family
     - Responsibility and interpretation
   * - ``CoreLoadsForces``
     - Concentrated components on a nodeset; six stored force/moment DOFs. Exports the force definition through the step's assignment path.
   * - ``CoreLoadsPressures`` / ``CoreLoadsSurfaceTraction``
     - Distributed surface loading. Face orientation and exported entity selection matter; a scalar pressure is not interchangeable with a traction vector.
   * - ``CoreLoadsGravity`` / ``CoreLoadsCentrifugal``
     - Body loading and its direction/axis parameters. Check material density and the intended consistent unit system.
   * - ``CoreLoadsHeatfluxes`` / ``CoreLoadsBodyHeatflux``
     - Surface versus volumetric heat input. Their magnitudes differ in dimensions and selection semantics.
   * - ``CoreLoadsFilm`` / ``CoreLoadsRadiation``
     - Surface thermal exchange definitions, ambient data and associated parameters.
   * - ``CoreLoadsTrajectory``
     - Dispatches trajectory IDs to heatflux or spherical body-heatflux subloads; coordinates preparation/cleanup and queries for drawing/export.
   * - ``CoreLoadsTrajectoryHeatflux`` / ``CoreLoadsTrajectoryBodyHeatfluxSphere``
     - Spatial/time-dependent prepared selections and magnitudes for a moving heat source.

The source catalogue lists each create/modify/delete command and panel. The
existence of a Core class is not a statement that all combinations of analysis,
material, entity kind and solver option are supported.

Displacement and temperature conditions
----------------------------------------

``CoreBCsDisplacements`` stores prescribed component values separately from
mode flags. The modes distinguish free, prescribed and FIXED behavior; a zero
prescribed displacement must not accidentally be interpreted as an omitted DOF.
Optional amplitude and time-delay references are resolved through the export
path. ``CoreBCsTemperatures`` is a separate thermal condition domain.

``CoreHBCs`` and initial-condition handling have their own assignment/export
roles. Do not infer that the same table or keyword is used for every initial,
homogeneous and step-applied condition. Topology's compliance/adjoint assumptions
place additional restrictions on allowed prescribed displacements; see
:doc:`topology-algorithms` and :doc:`topology-preservation`.

Amplitudes and time
--------------------

``CoreAmplitudes`` owns amplitude records used by loads and BCs. An amplitude
ID is a feature reference, not a raw multiplier embedded independently in every
definition. Preserve its time convention and step assignments when exporting.
A time-delay record changes temporal evaluation rather than the identity of the
target mesh set. Deleting referenced definitions requires relationship cleanup.

Trajectory preparation
------------------------

Trajectory preparation can query the current mesh and create derived node,
face/element, coordinate, radius, magnitude and time data for export and drawing.
It is not just a string formatter. ``prepare_export_if_needed()`` and
``clean_export_if_needed()`` delimit reuse/cleanup. Moving the mesh, editing the
trajectory or changing source parameters can invalidate prepared data. Inspect
the current invalidation path before adding a cache or calling preparation from
a worker. Core owns these definitions; the GUI must submit their commands.

Source: :download:`CoreLoadsTrajectory.hpp <../../src/CalculiX/Core/loads/CoreLoadsTrajectory.hpp>`
and the trajectory helpers in :doc:`code-catalog/index`.

Units and review rules
-----------------------

Concentrated force and moment components have different dimensions. Pressure
and traction are force/area; body acceleration is length/time squared. Surface
and body heatflux use power/area and power/volume respectively. Thermal exchange
parameters must use a temperature convention consistent with the material and
solver model. The exporter does not silently normalize these inputs to SI.

When adding a family, document target entity kind, sign/direction convention,
units, optional parameter presence, step operation semantics, exported keyword
and unsupported combinations. Add definition deletion/reference cleanup,
persistence, tree/property routing and drawing selection. Use the existing
:doc:`../adding-a-feature` guide as the end-to-end implementation example.
