Automatic reports
=================

The Report workspace follows Postprocessor. It assembles workspace-owned model
content and, when available, content supplied by Postprocessor. CalculiX supplies
model and step reporting. Postprocessor supplies result tables and component
extrema for an opened source matching the exact recorded solver launch; missing
contributions are explicitly listed as diagnostics. Material and result numeric
tables use independent Report units. See :doc:`../postprocessor/units` for source
provenance, shared unit-editor commands and the current verification status.

Commands
--------

Generate a report using the CalculiX template and its default profile::

   sim workspace report
   report ccx job 1

The Source dropdown lists current models and jobs published by registered
workspace providers. Entries show names rather than requiring command prefixes
or manually entered IDs. Running jobs and missing job decks are disabled with
an explanation. Selection is preserved by source identity when the list refreshes.

CalculiX provides the current model and its existing jobs. These are distinct
sources: a current-model report captures read-only Core properties/settings;
a job report reads the job's existing input deck. Neither starts a solver or
regenerates an input file::

   report source ccx model
   report ccx model
   report source ccx job 1

The source command opens that source's stored report and restores its settings,
including pending edits. There is one report per workspace and model/job source.
If none exists, the preview is empty until Generate succeeds. Generate replaces
the report for that source; a title or profile change does not create another
instance. Failure preserves the previous report. Notes retain their stable
section associations. Different jobs and the current model keep separate reports.
Model-only reports have no associated solver results.

The Source dropdown is the only report navigator. Stored reports remain listed
and selectable when their input files/provider sources are unavailable; Generate
and Update are disabled for those entries. Refresh does not submit commands.

Generation updates the central preview. Open preview / Export opens the separate
preview window. Both viewers display continuously stacked pages with vertical
scrolling, horizontal scrolling when needed, Ctrl+wheel zoom, page navigation,
Fit width and Fit page. The initial default is Fit page; manual fit/zoom choices
remain available. Changing zoom does not open another window. Ordinary
refreshes preserve scroll position; only visible pages are painted.

The right raw-properties dock was removed. Content checkboxes remain in the left
tree, including cover/contents. Report settings opens General, Steps and Page
layout tabs with labeled controls. Apply validates the complete settings patch
in Core and submits one command; Cancel leaves settings unchanged. The dialog
checks committed state because Framework command submission alone does not
confirm successful execution.

Configuration commands use quoted keys and values::

   report configure "ccx.model.materials" "false"
   report configure "ccx.steps.parameters" "true"
   report configure "steps" "1,2"
   report configure "author" "Example author"
   report configure "precision" "6"
   report configure "page_size" "A4"
   report update
   report preview
   report preview page 2 zoom 125
   report preview fit width
   report preview fit page

Other supported common settings are title, project, revision, cover and contents.
Boolean values are true/false. Steps are all or comma-separated positive ordinals.

The settings dialog journals a readable named-argument command containing only
edited fields. Multiple edits remain one atomic Core change. Named boolean
arguments use on/off; omitted arguments remain unchanged::

   report configure settings company_name "Company Name Test" title "Engineering report Test"
   report configure settings footer_line off header_line on
   report configure settings footer_right "Page {page} of {pages}"
   report configure settings logo_file "C:/Company/logo.svg"
   report configure settings logo_remove

Text arguments are quoted. Numeric arguments include precision and logo_width_mm;
page_size accepts A4/Letter and logo_placement accepts header_left, header_center,
header_right, footer_left, footer_center or footer_right. Applying unchanged
settings submits no command. The encoded settings-data form is no longer used.

Preview page numbers are 1-based; zoom is 1 to 300 percent. Preview navigation
controls submit these commands through silent_cmd rather than adding journal entries.
Changed settings require Update before export. The preview continues to display
the last generated content until Update succeeds.

Framework profiles
------------------

Profiles are reusable workspace-specific settings in the existing
``cubitsim_user_options.h5``, under ``framework/reporting/workspaces/<workspace>``.
They contain no generated results or pages::

   report profile save "Standard"
   report profile load "Standard"
   report profile default "Standard"
   report profile list
   report ccx job 1 profile "Standard"

Save replaces a same-named workspace profile and increments its revision. Load
changes configuration; it does not regenerate. Default selects the profile for
future generation. An existing report retains its resolved settings regardless
of later profile edits. A template revision mismatch fails explicitly.

Project persistence and export
------------------------------

Normal explicit Cubit project saves include Report state under ``cubit-sim/report``
inside the .cub file. Project opening restores saved reports without regenerating
them or requiring original input/result files. New/reset clears project reports,
but retains Framework profiles. There is no separate report save/open file format.
Temporary native save notifications are currently skipped, matching the existing
workspace integration; report recovery from native autosaves is not yet verified.

Choose stored reports by their readable source, then add notes or export::

   report source ccx job 1
   report note "ccx.model.materials" "Material data pending review"
   report export pdf "connection.pdf"
   report export odt "connection.odt"
   report export both "connection"

For both, .pdf and .odt are appended to the base name. Each file is replaced only
after its writer succeeds; the two-file operation is not a single transaction.
PDF and preview share one page painter. ODT contains editable headings, tables,
headers, footers and embedded images; its editor controls pagination and index
regeneration. Export does not save the .cub project.

Source and numerical limits
---------------------------

CalculiX reporting reads the existing job .inp, following INCLUDE files relative
to their parent deck and recording SHA-256 fingerprints. It never regenerates
input or starts a solver. Historical input/result-run linkage remains unverified.
For job reports, Update rejects changed input files/fingerprints; Generate again
explicitly to replace the same source report with current files. For current-model reports, Update
explicitly captures current Core data again, preserving report notes. The
reporting-snapshot hash identifies the captured report content, not a solver run. Original input units are not inferred.

Mesh tables summarize explicit NODE/ELEMENT records. Material, section, set,
interaction, load and output definitions retain raw keyword/data records. Step
boundary summaries accumulate literal target/DOF records and OP=NEW; overlapping
sets, aliases, transformations and unsupported options are not fully resolved.
Raw input remains authoritative. Unsupported definitions remain visible in the
additional-input section. Full mesh connectivity and solver results are not
printed by the Batch A model provider.

Build and runtime status
------------------------

This implementation has not been built or compiled under the user's restriction.
SDK integration, preview/PDF rendering, ODT opening and .cub lifecycle behavior
require runtime verification after that restriction is lifted.

Current-model reporting
-----------------------

The model snapshot uses typed CalculiX interfaces for mesh/block summaries,
material properties, sections/orientations, constraints/contact, initial
conditions, amplitudes, load/BC definitions, outputs and step assignments.
It does not invoke solver export preparation or create a temporary job. Array
settings are retained as readable arrays; unsupported result evaluation and image
rendering remain provider capabilities for Batch B. A snapshot hash is computed
from all captured report content before display selection is applied.

For current models, selected steps refer to Core step IDs; job reports use the
ordinals of steps in their exported input. Original unit conventions are retained.
The Report workspace icon is B04 (Report assembly), deployed as Icons/Report.svg.
Framework supplies the component directory; ReportIconLoader performs only SVG
lookup, following the existing workspace resource-loader pattern.

Selecting a named profile in the dropdown loads its settings through a command.
Choosing Workspace default applies the configured default profile, or factory
defaults if none exists (``report profile reset``). This changes configuration,
not the existing generated pages; Update or Generate remains explicit.

Page navigation and company branding
-----------------------------------

The Report file toolbar appears below the menu and reuses Cubit's New, Open,
Save and journal actions. Its PDF and ODT buttons use the P02 and O02 export-arrow
icons. Export buttons require a generated report with no pending source or
configuration changes. Toolbar and preview-window exports share the same file
dialog and confirmation behavior.

An existing destination requires explicit replacement approval. Interactive
exports ask before overwriting; command exports require the overwrite keyword::

  report export pdf "C:/Reports/connection.pdf" overwrite
  report export odt "C:/Reports/connection.odt" overwrite
  report export both "C:/Reports/connection" overwrite

Without overwrite, existing files are preserved. Paired export checks both
destinations before writing either file. PDF and ODT are committed separately;
if ODT writing fails after PDF succeeds, the error reports that partial result.
Cancelling file selection or declining replacement submits no export command.

Both previews provide a page slider and a page number with the total page count.
Dragging previews the target number in the slider tooltip; releasing navigates.
Scrolling updates the slider without submitting another command. Fit page fits
and aligns the current page; repeating it realigns that page after scrolling.

Report settings includes a Header & footer tab with company name/details, left,
centre and right text positions, separator lines and a cover-page toggle. The
factory template uses Company Name and example contact details. No default
company logo is assigned during development.

Text positions support ``{company_name}``, ``{company_details}``, ``{title}``,
``{project}``, ``{author}``, ``{revision}``, ``{page}`` and ``{pages}``.
Unknown or malformed placeholders reject the settings change. Empty text omits
that position. Header/footer bands reserve space outside the report body.

Choose an SVG, PNG or JPEG logo, select a header/footer position and set its
maximum width (5–60 mm). Aspect ratio is preserved within the selected column
and a maximum height of 12.7 mm. Import creates an embedded PNG snapshot, up to
1024 by 512 pixels and 512 KiB. Original files may be at most 5 MiB and 4096 by
4096 pixels; SVG must be self-contained. The original file is unnecessary after
import. SVG logos are rasterized for consistent preview, PDF and ODT output.

Company settings and the embedded logo are saved by ``report profile save`` in
the Framework HDF5 store. Generated reports and configuration drafts retain their
embedded logo inside the project's .cub data. Update or Generate applies changed
branding to the generated pages. PDF shares the preview painter; ODT contains
editable header/footer text, an embedded image and native page-number fields.
Office-editor pagination can differ from the preview.

Examples::

  report configure "company_name" "Example Engineering"
  report configure "header_left" "{company_name}"
  report configure logo file "C:/Company/logo.svg"
  report configure logo placement header right width 30
  report profile save "Company Standard"
  report profile default "Company Standard"
  report update

Remove the image with ``report configure logo remove``. Logo changes, text and
layout settings made in the settings dialog are submitted as one atomic command.

Model presentation units
------------------------

Current-model physical tables use the Report workspace unit preferences,
independently of CalculiX. This includes sections, orientations, interactions,
steps, amplitudes, trajectories, initial conditions, damping and constants, as
well as materials, static loads and boundary conditions. A quantity/unit legend
accompanies model sections. Curve ordinates and normalized offsets remain
dimensionless. Historical jobs and VTK fields retain source-unit metadata;
changing a model's units does not alter their interpretation. Custom solver text
is shown literally and is not dimensionally converted.

Generation progress
-------------------

Generate and Update use one Cubit native progress scope for the entire operation,
including preview content, pagination and contents. The description remains
"Generating report and preparing preview"; detailed stages appear in the command
output. The bar advances through workflow stages rather than estimating elapsed
or remaining time. Known counts advance within a phase; local model file/table
counts are not treated as an aggregate model total. Progress remains below 100%
until both the replacement report and its prepared preview have been published.

Cancellation is checked between parsing batches, model tables and images, and
before publishing the report. Cancelling Generate/Update during preview work
preserves the previous report and prepared layout. A single native rendering,
Postprocessor collection or Qt pagination call can still run until it returns to
a checkpoint. Cancelling a separate preview request leaves the saved report
intact so its preview can be retried. A completed command now includes preview
preparation when the Report GUI is available; headless generation stays possible.

Content and Outline
-------------------

The left dock has two tabs. Content groups the template's existing checkboxes
under Document, Model, Analysis steps, Model and setup images, and Postprocessor.
Header/footer options, requested outputs and per-step images have nested groups.
Group checkboxes apply one atomic settings command; partial selections are shown
explicitly. Expansion, selection and scroll position survive refresh. A disabled
section gate preserves its child choices until re-enabled or changed as a group.

Outline describes the actual generated document, including nested sections,
tables, row counts and images. Counts do not require pagination; the new outline
is displayed with the committed report after preparation. Pre-pagination totals
are printed in the command output. Page locations and total pages are displayed. Activate a
section to navigate to it. Table/image entries navigate to the containing section;
individual table/image page anchors are not recorded. Selecting content does not
automatically discard rows or change the template defaults.

Preview preparation uses bounded text documents rather than one report-sized
Qt layout. Core limits each batch to 128 rows, 2048 cells and 32768 text characters.
Long textual cells continue in additional display rows; the persisted report and
ODT retain their original semantic rows. Headers repeat on table continuations.
Batches start on a new page, which can introduce whitespace. The native progress
bar advances after each prepared batch and remains active through global contents
resolution and publication. Each individual Qt pagination call is synchronous;
progress does not claim page-by-page completion inside that call. Busy/nested
refreshes keep the last preview intact.

GUI refresh only reads the prepared cache. Generate/Update prepare it inside the
command before completion. Source selection, explicit preview/export, note/image
commands and project activation prepare a missing matching layout in application
operations. A cache miss in a passive refresh never starts pagination; it displays
"Report generated. Use Open preview / Export to prepare its preview."


Result selection and detailed listings
-------------------------------------

The workspace template defaults ``post.results.frames`` to ``last_per_step``:
only the last retained result frame of each selected solver step is reported.
The content tree offers one mutually exclusive frame choice:

* ``last_per_step``: canonical result order determines the final frame, not time
  or frequency. Missing fields are not substituted from earlier frames.
* ``envelope``: minima/maxima are compared across selected frames in Report
  units. Governing frame identities are retained; different governing frames
  produce separate minimum/maximum images.
* ``all_frames``: report each selected frame independently.

Block extrema and component images are enabled by default. Only verified
``Blocks`` metadata with an originating model block ID is labeled a model block;
arbitrary DAT element sets are not silently substituted. Positions use original
undeformed source coordinates. Element result positions are parametric centers;
integration-point identities are included where available.

The following detailed listings default to false and are saved with profiles:
``ccx.model.sets.nodes``, ``ccx.model.sets.elements``, ``ccx.model.sets.faces``,
``ccx.model.mesh.nodes``, ``ccx.model.mesh.elements``,
``post.results.nodal_values``, ``post.results.element_values`` and
``post.results.tables``. Summary tables remain independent of detailed source
tables. Historical set summaries show definition-record counts explicitly;
generated ranges and named references are not mislabeled as expanded member counts.

For example::

   report configure settings option "post.results.frames" value "last_per_step"
   report configure settings option "post.results.tables" value "false"

Result reduction uses at most four workers with 65536 samples per immutable
input chunk. VTK extraction, unit conversion, native progress and rendering stay
on the host thread. Worker reduction skips invalid/nonfinite samples and resolves
ties by original sample order. All workers join before cancellation returns.
Automatic report images render a detached source/frame/block/component snapshot
at the existing Full HD capture default without changing live view settings.

Inherited result-image presentation
-----------------------------------

Result image collection freezes the Postprocessor presentation once. By default,
it inherits the active view's camera and scene coordinate basis, background,
per-item styles and colors, scalar palette, vectors, deformation and upstream
clip/section/threshold filters. Existing explosion settings are retained too.
The report selects the block, scalar component and frame; those selections do
not modify the live Postprocessor. Output remains Full HD by default.

The active matching representation is preferred. Otherwise a unique visible
representation of the exact result source in the capture view is accepted.
Ambiguous presentations produce a diagnostic. When none exists, a default
presentation is used and identified in the caption. Unrelated sources and
filters are excluded from rendering.

The Results / Result images content tree offers Presentation, Camera and
Background choices. Report settings / Images also accepts a Postprocessor view
name; empty uses the active view. These settings are saved with profiles::

   report configure settings option "post.images.presentation" value "inherited"
   report configure settings option "post.images.camera" value "preserve"
   report configure settings option "post.images.background" value "current"
   report configure settings option "post.images.view" value "3D View 1"

Alternative values are ``default`` presentation, ``fit`` camera, and ``white``
or ``transparent`` background. Preserved cameras retain their projection and
vertical framing; changing viewport aspect ratio can change horizontal coverage.

Compatible fixed scalar bounds are converted to frozen Report display units.
Bounds for a different or unresolved physical quantity are replaced with an
automatic range and disclosed in the caption. Threshold fields, native plane
units and threshold bound units retain their original meaning.

Filtered images are identified explicitly: numerical minima and maxima still
describe the complete original block, not only its filtered or deformed display.
Image provenance retains the original presentation separately from the adapted
report frame/component, appearance, vector and filter records.


Export progress and cancellation
--------------------------------

PDF and ODT exports use one Cubit native progress bar until their destination
files have been committed. PDF reports completed pages. ODT reports content work,
images and bounded checksum/package copying. ODT-only export does not need to
prepare a preview first. Preview-only batch boundaries do not become forced ODT
page breaks or alter the original semantic table rows.

Cancel is checked between PDF pages, ODT cell blocks, image tasks and 64-KiB
package I/O blocks. A single Qt painting/image-decoding call cannot be interrupted
internally. Uncommitted temporary files are discarded, preserving existing
exports. For ``both``, both files are encoded before publication. The final short
commit phase is non-cancellable; two file commits are not jointly atomic. If the
second commit fails, the error identifies the PDF that was already exported.

ODT contains nested contents links, styled tables with repeated headers and
figure/caption styles. Image dimensions preserve aspect ratio within the report
body. ODT page numbers are calculated by the office editor, independently of PDF
pagination. Large files requiring ZIP64 are rejected rather than truncated.
