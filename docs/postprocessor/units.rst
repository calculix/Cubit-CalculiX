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
