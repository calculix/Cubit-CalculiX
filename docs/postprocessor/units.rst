Units and material references
============================

The Framework owns unit definitions and workspace presentation preferences.
Postprocessor and Report have independent preferences; changing either does not
change solver output arrays or another workspace's settings. Open the shared
editor from the workspace's Units action or use::

   post units open
   post units system "N-mm-s"
   post units display quantity "stress" unit "MPa"
   post units display reset quantity "stress"
   post units defaults save
   post units close

The same operations use the ``report units`` prefix for Report. Workspace
preferences are saved once in the Framework group of the native ``.cub`` file.
``defaults save`` changes the defaults for later projects. Closing or switching
a workspace does not discard its project data.

Source values and presentation
------------------------------

CalculiX reader state retains the unit system recorded with the actual solver
input when those results are loaded. The retained context is saved alongside
reader data. Rebuilding retained results never takes units from the current
workspace or a newer solver launch. Older projects and external files without
declared units remain unknown; selecting display units does not declare their
source units.

Known field components are converted for color values, ranges, probes, spatial
tables and history plots. Native result-table columns use their own quantity
metadata. Plot CSV adds explicit X, Y and time-unit columns. Unsupported result
quantities remain unchanged and are labelled with unknown units. Modal analysis
parameters are not assumed to be time or frequency without an established
quantity definition.

Changing Postprocessor presentation units converts existing fixed color limits
and invalidates cached ranges. Manual plot axes return to automatic fitting.
Geometry, clip planes and cameras stay in each source's declared coordinate
units. Displacements are converted into those coordinate units before applying
the dimensionless deformation scale. Numeric probe positions can be presented
in the workspace length unit without changing annotation anchors or geometry.

Materials in results
--------------------

Result metadata carries project identity, workspace identity and local material
IDs. CalculiX solver material ordinals are retained separately. They become
Framework material references only when the recorded input provides the exact
material-name-to-local-ID mapping. Unknown mappings are never resolved using a
current material name or a coincidentally equal ID.

Material IDs are categorical. They are excluded from physical field selection,
interpolation, vector magnitudes and extrema. Elements and integration points
can expose material references; a shared node has no single material. A probe
resolves a material name only in the matching project and workspace. Otherwise
it displays an unresolved scoped reference. VTK metadata contains references,
not material-property copies, revisions or material history.

VTK files and reports
---------------------

Versioned ``CubitSim.ResultsMetadata`` FieldData preserves source units,
component quantities, table-column quantities, physical field roles and scoped
material references on individual mesh leaves and independent tables. Export
writes metadata on private VTK copies. VTU, VTPC and VTT imports validate metadata
against the actual arrays before publishing a source.

The existing PVD history-import gap remains: the file provider opens individual
VTU, VTPC and VTT files, not the complete exported ``results.pvd`` collection.

Report model-material tables use Framework column definitions and Report units.
Job-input material text remains the exact archived solver input in its recorded
source units; it is not regenerated from current material properties.
Result tables/extrema are supplied by Postprocessor only for the exact recorded
solver launch associated with the job report. Open that result source first.
Unavailable or ambiguous provenance produces a diagnostic instead of silently
substituting another run. Changing Report units marks its configuration changed;
Update regenerates values. Already saved document content retains its rendered
values. No historical material properties are reconstructed from current data.
Extrema include entity IDs and undeformed reference coordinates in Report length
units. Element reference positions use the element parametric center; nodal and
integration-point results use their actual source positions. Integer identity
columns retain exact text, including IDs larger than a floating-point integer.

Verification status
-------------------

The implementation includes mesh/table XML roundtrips, partition metadata,
invalid-metadata rejection, independent presentation settings and source-buffer
immutability tests in ``Postprocessor/Tests/UnitsMetadataTests.cpp``. No build or
native-session test has been run for this change because builds were explicitly
excluded from the current implementation task.

Ranges and limits when sources are unavailable
----------------------------------------------

Each representation saves the quantity, presentation unit, physical dimension and
unit role belonging to its fixed range and minimum/maximum limit values. These
settings can be converted without resident result arrays. For example, changing
stress presentation from MPa to Pa converts a stored limit of 200 to 200000000,
even when the external result source is unavailable. Disabled limits retain their
unit metadata and convert too, so enabling them later preserves their meaning.
Source arrays are never rescaled.

New field selections bind bounds to the current presentation unit. Unknown source
units remain explicitly unknown; they are not inferred from names or preferences.
Older projects without saved bounds-unit metadata are supported. A unit change
first searches available frames for the selected field. If the legacy field cannot
be resolved, the operation fails with a diagnostic and preserves both workspace
units and representation settings. Reconnect the source before changing those
legacy settings. Unit changes are staged before publication.

Open table values and unit headers refresh when the Framework unit generation
changes. Automatic columns widen as needed, while manually sized columns, current
selection and scroll position are retained.

Additional regression coverage is defined in ``ColoringUnitsTests.cpp`` for
unavailable sources, missing fields in the active frame, legacy metadata,
persistence validation, temperature-difference conversion, unknown units and Qt
table refresh notifications. ``UnitsMetadataTests.cpp`` uses always-active,
fail-fast checks so fixture writes, reads and checks execute in Release builds.
These native cases have not been built or run for this patch.

Navigation, annotations and plane coordinates
-------------------------------------------

Frame navigation converts the recorded independent variable into Postprocessor
presentation units and labels it as Time, Frequency or Analysis parameter using
its quantity metadata. Unknown units remain explicit. Unit changes update label
text during playback without changing frame indices, slider state or animation
cadence. Ordinary result labels and both extrema paths include their display unit.
Their geometric anchors remain in native source coordinates.

Clip and Section origin fields show their native coordinate unit; normals are
dimensionless. A newly created plane stores its native coordinate-unit basis in
its definition and project persistence. Presentation-unit changes never move the
plane. If refreshing a source changes that basis (for example mm to m), the
filtered output is rejected with a diagnostic. Recreate the filter using the
current native coordinates. Unit aliases with the same length scale are accepted.
Legacy unbound planes bind to the previous known basis during source refresh;
missing units are never inferred from geometry sizes.

Plot axes
---------

Visible curves with valid samples must have compatible physical dimensions and
unit roles on each shared axis. Their values are normalized to the first visible
valid curve's axis units. For example Pa and MPa stress curves can share an axis;
force and stress cannot. Frame indices cannot share an axis with a physical
parameter. Unknown-unit curves may share an axis only when their exact scoped
source/field or table-column identity establishes the same raw basis. A field
name alone does not establish physical equivalence.

Adding or showing an incompatible curve fails before publishing the modified
plot. Existing/restored definitions are also checked during evaluation. Hidden
curves do not establish a shared axis. Separate physical axes remain deferred.
Numeric plot samples are private derived values; published source arrays remain
unchanged.

Capture presentation context
----------------------------

A capture snapshot retains an immutable Postprocessor presentation context.
Color arrays, color ranges, probe values, labels and plot samples are evaluated
against that context, even if live workspace units change afterward. Rendering
does not temporarily replace global unit settings. Unit definitions and
conversion arithmetic remain Framework-owned.

Capture manifest version 2 records the presentation system and quantity/unit
preferences, plus the quantity, unit, dimension and role of representation bounds.
Already saved PNG figures remain fixed image assets in Report. This context is
additional rendering provenance; it does not duplicate result arrays.

The added ``postprocessor_presentation_units`` cases cover frozen ranges, probes,
labels and plots; compatible/incompatible axes; unknown-unit scope; frame axes;
and changed plane coordinate bases. Navigation and capture boundary cases are
also extended. No native build or test execution was performed for this patch.

Scene coordinates and viewport scale
------------------------------------

A view fixes its scene length unit when a source with known coordinate units is
first displayed. Native source arrays remain unchanged. Uniform source-to-scene
scaling is applied to geometry, picking, highlights, annotation anchors, editing
planes and fit bounds. Presentation preferences change displayed labels rather
than camera coordinates. Saved views retain their scene unit and camera.

The ruler samples a horizontal length in the camera's focal plane and uses a
1/2/5 interval in the current presentation length unit. Perspective rulers say
``at focus``: apparent scale varies with depth. The ruler is suppressed for an
empty viewport or visible geometry with an unknown coordinate basis. Hidden
uncalibrated sources do not suppress it. The ruler and presentation context are
also retained by PNG capture snapshots.

Logical result keys remain stable across frames. A group whose component
quantity, dimension, role or known/unknown status changes is unavailable for
coloring; an already-restored selection draws neutral geometry without a
misleading legend. Compatible known units normalize through workspace settings.

New manual plot ranges retain their entered presentation units and raw-data
scope. Rendering converts their numbers into the current common plot units.
Changing to an incompatible quantity or unknown scope automatically fits that
axis without modifying its saved bounds. Range metadata is saved with the plot.
Legacy ranges without metadata retain their original display behavior and are
cleared on a presentation-unit change or change of the first visible curve.

CalculiX result-component units
-------------------------------

CalculiX output metadata is assigned per component before a converted result is
published. CONTACT opening/slip components carry length units, while pressure
and shear components carry stress units. Amplitude/phase outputs, mass/inertia,
turbulence and fracture outputs likewise retain separate component quantities.
The native result arrays are never rescaled by annotation.

Source units come from the solved job's recorded system, independently of current
workspace display preferences. Without a complete source system, dimensional
quantities stay unresolved except for lengths/times with explicit coordinate/time
unit provenance. Strains and other known dimensionless values retain unit ``1``.
Error estimators retain native percent values; phases retain native degrees.
Temperature oscillation amplitudes use difference units and have no affine offset.

The rules follow the ccx 2.23 output documentation and writer component labels.
In particular, ``frdfluidfem.f`` emits primitive ``k`` and ``omega`` in TURB3DF,
whereas DAT turbulence conservative variables contain density-weighted values.
Unknown component names, state variables, sensitivities and undocumented fracture
components remain unresolved. Standard DISP/STRESS mappings describe ordinary
structural output; repurposed network DOFs and beam SECTION FORCES need additional
semantic provenance and must not be treated as a universal displacement/stress
mapping for arbitrary solver DOFs.

Refresh/reconvert existing results to regenerate unit metadata. Selecting a new
display unit does not supply a missing source unit system for an external file.
Regression cases are registered in ``calculix_result_unit_tests``; configuring,
building and running them is a separate verification step.
