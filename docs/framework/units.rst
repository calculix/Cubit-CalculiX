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

Frequency displays in ``Hz`` by default, independently of the solver time unit.
An explicit frequency display preference takes priority. Resetting that
preference restores ``Hz``; it does not change the coherent solver frequency
unit. Frequency and time have independent display preferences. For example,
``2 Hz`` displays as ``0.002`` when frequency is set to ``1/ms``.

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
   ccx units display quantity "frequency" unit "Hz"
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

Compact labels and headings
---------------------------

CalculiX command-panel labels, property titles, command and material table
headings, and the shared unit editor's read-only unit cells use compact Unicode
notation. Editable unit expressions and commands retain their original syntax.
Tooltips on bound fields, table headings and unit-editor cells retain the input
expression.

.. list-table:: Input expressions and displayed units
   :header-rows: 1

   * - Input expression
     - Display
   * - ``N/mm^2``
     - ``N/mm²``
   * - ``kg/m^3``
     - ``kg/m³``
   * - ``N*m``
     - ``N·m``
   * - ``N^0.5*m^0.5*s^-1``
     - ``√(N·m)/s``
   * - ``N*m^-2*s^-1``
     - ``N/(m²·s)``
   * - ``degC``
     - ``°C``

The formatter supports flat products and quotients with integer or
half-integer powers. Compound denominators are grouped explicitly. Other
expressions retain their original spelling. Material creep headings also
support the explicitly supplied symbolic powers ``-n`` and ``-(m+1)``.
Formatted text is never passed back to the unit parser, used to generate
commands, or persisted as a unit expression. Formatting does not convert
numeric values or change their physical meaning.

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
-------------------------

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

Dimensional values converted to the solver string view use scientific notation
with six digits after the decimal point. For example, a minimum step increment
of ``1e-5`` is written as ``1.000000e-05`` rather than an expanded binary
round-trip representation. Canonical storage and non-solver conversion views
retain full precision; the detached export view receives the output formatting.
Opaque custom solver text remains literal.

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
Report collection lazily converts each requested Core domain once, retaining the
detached view only for that collection pass. A later collection starts with fresh
model values and Report preferences. No converted view is retained globally.

Film export uses a detached solver-unit view for both its header and its data
rows, including the heat-transfer coefficient. Canonical SI storage is unchanged.

All temperature exports use kelvin; an explicitly supplied ``ABSOLUTE ZERO``
must therefore be zero. Beam and shell-edge distributed loads require force per
length and are rejected by the pressure-only command until that separate input
quantity is implemented. Pressure target validation queries live sideset element
types and native side numbers through the mesh interface; it does not depend on export-cache
population. Opaque custom solver text is preserved literally; its
numbers have no quantity declarations and require review when changing solver
units. Export validation reports known dimensional limits before opening the
output file.

Verification status
-------------------

The recorded Windows campaign passed on 16 November 2026. Run
``20261116T132856Z_3f89f3cf`` completed three native regression executables,
export and save/reopen across five presets, rotation-node mapping and expected
rejection checks, and three physical solves with displacement projection.
The user subsequently confirmed manual checks 1--5 and the compact label and
heading presentation on Windows.

**Linux automated and manual verification remains PENDING.**
The recorded automated run precedes the Hz-default and compact-formatter test
extensions; a subsequent execution of those added assertions has not been
recorded. The later presentation checks are user-confirmed manual evidence.
See :doc:`../developer/units-verification` for coverage, provenance and limits.
A complete export still requires dimensional handling at every participating
boundary; opaque custom solver text and unspecified external source units are
not covered by changing the workspace preset.

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

These are reproduction instructions, not an execution record. The Windows
campaign cited above does not establish a result for this separate target.

The separate opt-in ``CUBITSIM_DOMAIN_UNIT_TESTS`` option registers the CalculiX
domain, boundary, result-metadata and GUI presentation regression targets.
The recorded Windows campaign executed ``calculix_domain_unit_tests``,
``calculix_unit_boundary_tests`` and ``calculix_unit_presentation_tests``.
Their fixtures exercise domain conversions, inverse equation coefficients,
independent Report conversion, geometry lengths/angles, failed-conversion and
failed-read retention, SI HDF5 round trips, step backups, amplitude-axis cleanup,
result metadata and GUI draft conversion. Registration alone is not PASS
evidence for additional targets such as ``calculix_result_unit_tests``.

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

The original persistence review was source inspection. The later Windows
campaign adds executable evidence for the listed fixtures; it does not turn
every reviewed contract into a separately verified runtime case. Linux
verification remains pending. See :doc:`../developer/units-verification`.


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

Mechanical components and result axes
-------------------------------------

Force/displacement components 1--3 retain their translational dimensions;
moment/rotation components 4--6 retain moment/radian dimensions. On a rigid-body
``ROT NODE``, export maps components 4--6 onto solver DOFs 1--3. Nonzero
translational components on that node are rejected. Existing zero supports are
accepted, identical zero constraints coalesce, and contradictory prescriptions
fail before export. Equation coefficients use the inverse quantity of the actual
node/solver DOF. Changing a rotation-node role while nonzero equation terms
reference that node requires removing/redefining those equations first.

Displacement projection converts the retained result's solver length unit into
Cubit geometry units before applying its dimensionless scale. Rotation dummy
nodes are excluded from geometric displacement projection. Centrifugal drawing
converts its displayed origin into the same geometry units as element centres.

Generated step exports record ``CUBIT_SIM_STEP_AXIS`` comments keyed by emitted
solver-step ordinal. Harmonic steady-state output uses frequency; transient and
nonharmonic time samples use time. Job records and retained result metadata
preserve this information independently of the current model. Missing or
ambiguous axis provenance remains unknown, including externally supplied decks
without a declaration. FRD mode numbers alone do not establish an axis quantity.
Increment tables use the same quantity as their frame abscissa.

FRD ``FLUX``, ``RFL`` and ``ENER`` carry heat-flux, power and energy-density
quantities. Mechanical/elastic/plastic strains are dimensionless. Integrated
energy tables retain energy rather than energy-density units.

CalculiX command panels display physical input units beside fields and table
columns. Changing preferences converts existing numeric drafts through SI,
including coordinate vectors and affine absolute temperatures. An incomplete
draft prevents partial panel conversion and blocks Apply until resolved in its
still-displayed units. A settings reload adopts the queried display basis without
converting the values a second time. Unit formatting does not execute model
commands or share panel layouts.

Report unit snapshots
---------------------

Report profiles store their presentation system and display overrides. Loading a
profile validates all settings before applying them. Selecting an existing source
does not restore that document's former presentation preferences. Generation and
Update record the actual Report presentation settings in the immutable document
configuration; export renders its stored values without another conversion.

A presentation change marks all existing reports as requiring Update, including
reports for other sources. These flags survive project save/load. The old preview
remains available, but export is disabled until that source is updated. Applying
identical unit settings does not invalidate reports. Explicit/default profile
preferences apply before provider collection; if collection fails, the selected
preferences remain active and the previous document remains unchanged.

Current-model physical properties have individual unit labels and numeric cells.
Mechanical translations/rotations and forces/moments carry distinct quantities.
Step procedure fields use the same solver-owned quantity mapping as conversion.
Constraint coefficients use the reciprocal unit of the referenced DOF; absolute
temperature offsets never enter these homogeneous equation coefficients.

Material reporting reads the CalculiX material store and uses Report preferences
only for presentation. Creep coefficients use the existing Framework material
conversion with row-dependent exponents, stress and time units. No material is
copied into a Report workspace store, and canonical material values are unchanged.

Literal job input tables retain their source units and label them in each table.
Archived run units or explicit input declarations establish this provenance;
current workspace settings never fill missing declarations. Exported coordinate
units are separate from the native geometry unit used to construct setup images.

Postprocessor images retain their captured pixels and presentation-unit
provenance. Preview/PDF and ODT use the same caption formatter to show those
preferences separately from Report table units. Changing Report preferences does
not reinterpret existing images.

Reporting DTOs validate JSON shape, metadata dimensions and finite numeric cells
without depending on Framework Core. Report Core validates physical expressions,
dimensions and temperature roles at generation, project persistence and export
boundaries. Providers supply physical assertions; Report does not infer solver
quantities from table titles. Existing Postprocessor table conversions remain
validated by their Results interfaces, and unknown units remain explicit.

Report project persistence uses schema 3 for unit snapshots, typed cell metadata
and pending unit updates. Development files from earlier schemas are not migrated.
