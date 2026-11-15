Workspace units
===============

Framework owns unit definitions, conversion and one unit context for each
workspace. Selecting units in CalculiX, Postprocessor or Report does not change
another workspace's selection. Framework material values use canonical SI.
Cubit coordinates and published result arrays retain their declared source
units; display and export conversions use separate values.

Presets and defaults
--------------------

The five supported mechanical presets are:

.. list-table:: Coherent solver units
   :header-rows: 1

   * - Preset
     - Length
     - Force
     - Time
     - Mass
     - Density
     - Stress
   * - SI
     - m
     - N
     - s
     - kg
     - kg/m^3
     - Pa
   * - N-mm-s
     - mm
     - N
     - s
     - tonne
     - tonne/mm^3
     - N/mm^2
   * - kN-m-s
     - m
     - kN
     - s
     - tonne
     - tonne/m^3
     - kN/m^2
   * - N-mm-ms
     - mm
     - N
     - ms
     - g
     - g/mm^3
     - N/mm^2
   * - kN-mm-ms
     - mm
     - kN
     - ms
     - kg
     - kg/mm^3
     - kN/mm^2

New workspace contexts default to ``N-mm-s``. Input/display defaults override
mass to ``kg``, density to ``kg/m^3`` and stress/pressure to ``N/mm^2``. These
preferences do not change the coherent solver basis. Thus density entered as
``7850 kg/m^3`` is stored as 7850 in SI and exported as ``7.85e-9`` in N-mm-s.

Solver temperature is K for all five presets. Input and display accept
``degC`` and ``degF`` for absolute temperature. A quantity explicitly declared
as a temperature difference uses only the scale, never the absolute offset.
Compound units use K or ``delta_degC``/``delta_degF``; affine ``degC`` cannot
appear inside a product or power. Angle is distinct from a dimensionless ratio.
Solver adapters retain keyword-specific degree/radian requirements.

Commands and editor
-------------------

CalculiX exposes its own workspace commands::

   ccx units list
   ccx units system "N-mm-s"
   ccx units display quantity "density" unit "kg/m^3"
   ccx units display quantity "stress" unit "N/mm^2"
   ccx units display reset quantity "stress"
   ccx units geometry unit "mm"
   ccx units defaults save
   ccx units convert 210 from "GPa" to "N/mm^2"
   ccx units open
   ccx units close

``convert`` prints a scalar conversion and changes no model values. ``geometry``
declares what one existing Cubit coordinate unit means; it never rescales mesh
or geometry. An established declaration cannot be changed through that command.
Workspace preset changes likewise do not rescale Cubit geometry.
Geometry starts undeclared. Before export, explicitly declare its source length,
for example ``ccx units geometry unit "mm"``.

The shared editor shows input/display units beside coherent units. It submits
workspace commands through an injected callback and reads the accepted settings
back from Framework. It holds no Cubit host pointer. User-facing failures are
reported through ``CubitMessage`` at the command or GUI boundary.
Window-close and Escape actions submit the corresponding workspace close
command, preserving the command-driven GUI workflow.

Persistence and API
-------------------

``CoreFrameworkUnits`` is the common conversion service, obtained through
``framework_units()``. Operations identify their workspace or source preset
explicitly. ``to_si``/``from_si`` use a unit expression and expected physical
dimension; ``to_solver``/``from_solver`` select a workspace's coherent preset.
``system_to_si``/``system_from_si`` use an explicit source preset. Material-law
coefficients may supply resolved dimensional exponents instead of a named
quantity. Dimension order is kg, m, s, K, A, mol, cd.

``SourceUnitContext`` is unspecified by default. Opening an external file does
not infer units from active workspace preferences, field names or value ranges.
Mixed-unit field/table columns require individual quantity/unit metadata.
Published VTK buffers remain immutable and retain their source meaning.

User defaults reuse ``UserOptionsStore`` in ``cubitsim_user_options.h5`` under
``workspaces/<workspace>/units``. Current project contexts are stored in the
``.cub`` file under ``cubit-sim/framework/units/workspaces/<workspace>``; the
shared geometry length declaration belongs to that same framework units group.
``CoreFrameworkUnitsPersistence`` uses the existing ``HDF5Tool`` and stages only
its owned group. The project coordinator orders this operation with workspace
model persistence. Reading Framework units, materials and project identity uses
detached state; the live state is replaced only after validation and a successful
file close. Module startup failures release partially initialized services and
detach their options references before the options store is destroyed.
The runtime generation counter invalidates display caches
and is not stored as material revision history.

CalculiX model boundaries
------------------------

The nine static load types and displacement/temperature boundary conditions
convert their input preferences to SI before storage. Their settings queries
and solver exports use detached converted copies. Force components use force;
rotational components use moment or angle as appropriate. Gravity uses
acceleration, centrifugal magnitude uses angular speed squared, and its centre
uses length. Heat flux, volume heat flux, film conductance, absolute temperature
and time delays each retain their own physical dimensions. The existing surface
traction command distributes a total force, so its input quantity remains force.
Their HDF5 groups explicitly record ``value_units=SI``. Nonempty older groups
without that marker are rejected; no automatic legacy migration is performed.

The remaining eleven domains also store physical values in SI: sections,
orientations, constraints, contact pairs, surface interactions, steps,
amplitudes, trajectory loads, initial conditions, global damping and physical
constants. Changing the workspace preset updates only its unit context. It does
not clear or rewrite records, IDs, assignments, inactive materials, optional
values, step backups, jobs or results. There is no reset guard.

``CalculiXDomainUnits`` maps dimensional fields at the input, display, solver,
geometry and Report boundaries. Dimensional arithmetic stays in the Framework.
Editing operates on a temporary converted copy and commits only after successful
conversion back to SI. Solver export and settings queries use detached copies.
Equation coefficients convert inversely to their displacement/rotation units;
orientation angles export in degrees. Section offsets and beam directions remain
dimensionless. Step time/frequency fields, modal-force limits, contact penalties,
thermal contact data, stress tensors, damping and physical constants have their
own quantities. Empty optional values remain distinct from zero.

All eleven HDF5 feature groups record ``value_units=SI`` through ``HDF5Tool``.
Loading stages records before replacing live data. A missing marker can only be
normalized when saved ``native_system`` metadata establishes the source basis;
otherwise populated dimensional records are rejected without replacing live
state. That field is read-only migration information, not a preset lock. New
records never capture a native unit system. Reset/deletion uses the existing
domain lifecycle; amplitude axis metadata is cleared with its curves, including
batch deletion of generated trajectory amplitudes. Core shutdown releases both
trajectory subtype stores; detached conversion views do not own those stores.

Amplitudes default to a time axis. Frequency-dependent curves explicitly declare
``axis frequency``. Abscissae and horizontal shifts use that quantity; ordinates
and vertical shifts remain dimensionless multipliers of the associated load or
boundary condition. Absolute-temperature multipliers act on kelvin values.
Changing an axis requires replacement points and an explicit ``shiftx``; export
rejects curves used by a procedure with an incompatible axis. For example::

   ccx create amplitude name "Frequency response" time_amplitude 0 1 100 1 axis frequency

The existing ``time_amplitude`` point-list keyword is retained for both axes.

Geometry-facing orientation and trajectory queries convert lengths to the
separately declared Cubit coordinate unit. Trajectory-generated amplitudes,
loads and step splits use the current input units and return to SI through their
normal creation paths. Geometry therefore need not share the solver's length
unit. Preparation cleanup restores the original canonical step data.

Report model tables use independent Report preferences and include a quantity/unit
legend. Existing VTK and job data retain their immutable source-unit metadata;
changing a model preset does not reinterpret an earlier result.

All temperature exports use kelvin; an explicitly supplied ``ABSOLUTE ZERO``
must therefore be zero. Beam and shell-edge distributed loads require force per
length and are rejected by the pressure-only command until that separate input
quantity is implemented. Opaque custom solver text is preserved literally; its
numbers have no quantity declarations and require review when changing solver
units. Export validation reports known dimensional limits before opening the
output file.

Verification status
-------------------

The implementation is being integrated with the material, export, Postprocessor
and Report paths. No build or native Cubit execution has been performed for
this change. A complete export requires dimension-aware handling at every
participating model boundary; the unit service alone does not convert opaque
custom solver text or establish units for arbitrary legacy numeric data.

The opt-in ``CUBITSIM_BUILD_MODEL_TESTS`` option is off by default. It adds
``cubitsim_model_tests`` and the ``framework_model_services`` CTest case. These
checks cover all five preset scales, workspace isolation, affine temperature
rules, rejected conversions, unrestricted workspace preset changes, failed-read state retention, HDF5 unit/material round
trips, inactive card preservation, missing versus zero optional temperatures,
next-ID persistence, independent material-library copies, and shared library-file
preference persistence with failed-selection rollback. Files are created
only inside a ``QTemporaryDir``; the installed material library is not used.

When native builds are permitted, use the normal project dependencies and a
separate build directory::

   cmake -S src -B build-model-checks -DCUBITSIM_BUILD_MODEL_TESTS=ON
   cmake --build build-model-checks --config Release --target cubitsim_model_tests
   ctest --test-dir build-model-checks/Framework/Core -C Release --output-on-failure

These are instructions for subsequent validation. The regression target has
been added without configuring, compiling or executing it for this change.

The separate opt-in ``CUBITSIM_DOMAIN_UNIT_TESTS`` option adds
``calculix_domain_unit_tests`` / ``calculix_domain_units``. Its source fixtures
cover the eleven domains across all five presets, inverse equation coefficients,
independent Report conversion, geometry lengths/angles, failed-conversion and
failed-read retention, SI HDF5 round trips, step backups and amplitude-axis
cleanup. These tests have been added but have not been built or executed.

Persistence review
------------------

Workspace restoration failures roll back the Framework unit context, CalculiX
material store and project identity. Domain reads validate dimensional values
in detached copies before committing canonical SI records, including files
already marked SI. Constraint equations reject fractional DOFs, nonfinite
coefficients and conversion overflow without changing live records.

Rewriting a material after clearing its optional scalar or expansion reference
temperature removes the corresponding HDF5 dataset. Reload therefore preserves
absence rather than recovering an older value. Existing ``CCX_*`` property names
in the Framework presentation adapter are internal lookup keys; they do not call
the native Cubit material API. Material activation is persisted as typed card
state. Trajectory step backups are temporary export-preparation data; they are
preserved across unit changes and restored during cleanup, rather than stored
as project history.

This review used source inspection and whitespace checks. No build or executable
test run was performed; native validation follows in the next stage.


Workspace toolbar actions
-------------------------

The Framework appends shared editor actions to a toolbar supplied by each
workspace. CalculiX ends with Materials followed by Units. Postprocessor and
Report end with Units only. There is no separate Units/Materials toolbar row.
Actions retain their readable workspace commands and shared editor ownership.

``FrameworkIconLoader`` follows the workspace icon-loader pattern: it loads SVGs
from the deployed component's ``Icons`` directory. The Framework build stages
``Materials.svg`` and ``Units.svg`` alongside the existing workspace icons.
Missing resources report through CubitMessage.
