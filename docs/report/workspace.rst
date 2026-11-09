Automatic reports
=================

The Report workspace follows Postprocessor. It assembles workspace-owned model
content and, when available, content supplied by Postprocessor. Batch A supplies
CalculiX model/step reporting. Result evaluation and screenshots are Batch B
provider capabilities; missing contributions are explicitly listed as diagnostics.

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

The source command selects pending configuration without replacing the displayed
report. Generate creates the selected source's document. Update is disabled while
a different pending source is selected. Existing reports remain available in the
report selector. Model-only reports have no associated solver results.

Generation updates the central preview. Open preview / Export opens the separate
preview window. Both viewers display continuously stacked pages with vertical
scrolling, horizontal scrolling when needed, Ctrl+wheel zoom, page navigation,
Fit width and Fit page. Changing zoom does not open another window. Ordinary
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

Select saved reports using the report selector or their stable IDs::

   report select "<report-id>"
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
For job reports, Update rejects changed input files/fingerprints; generate a new
report explicitly to accept current files. For current-model reports, Update
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
