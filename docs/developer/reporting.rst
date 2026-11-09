Report implementation
=====================

Ownership and boundaries
------------------------

Framework/Reporting owns value contracts and provider interfaces. Each class has
its own header and implementation. CalculiX owns its template/defaults, input
resolution and model/step content. Postprocessor exclusively owns future result
evaluation, result tables and result pictures. Report Core assembles documents;
it does not include solver implementation or viewport headers.

ModuleManager discovers borrowed providers during module initialization. Report
is registered after Postprocessor. FrameworkContext owns the HDF5 profile store
and keeps it alive until modules stop. Commands hold weak application references.
GUI widgets submit commands through Framework callbacks and refresh from committed
snapshots. No Core widget pointers or GUI-side default definitions are required.

Storage
-------

Hdf5FileTransaction is the single lock/stage/atomic-replacement implementation
used by UserOptionsStore and ReportProfileStore for the shared options HDF5 file.
Profiles use stable UUID groups, an index and an optional default-profile ID.
Each profile payload is versioned structured JSON in an HDF5 string dataset;
numeric metadata remains numeric inside the payload. Unknown template options
and incompatible template revisions are rejected before application.

ReportProjectPersistence stores a versioned session payload under cubit-sim/report.
The structured JSON contains all documents, resolved configuration, drafts, notes,
diagnostics and embedded image data (base64). Temporary HDF5 groups stage a new
session and retain the previous group during publication. This is group-level
replacement, not a transaction covering native Cubit and all workspace writers.
The observer reports failures explicitly after the native save notification.

Session reads and report generation build candidates and commit only on success.
Projects without Report data restore an empty report session.
Malformed/unsupported data is rejected. Updates require matching input fingerprints
and preserve notes; they never silently accept a different job input snapshot.

Layout and exports
------------------

ReportDocumentLayout owns the Qt text layout. Preview and PDF use its page painter.
The ODT writer serializes native ODF XML and an uncompressed ZIP package; mimetype
is first and uncompressed. It uses public Qt XML/device APIs and no external office
process. The writer includes master-page headers/footers rather than PDF-only
decoration. ODT pagination is editor-dependent. Export staging uses QSaveFile.

Limits and verification
-----------------------

Input INCLUDE recursion, report nesting, payload size, table width and row counts
have explicit limits. Contact/results support remains a provider capability for
Batch B. Autosave persistence and native project dirty-state integration require
host lifecycle verification. Batch A implementation is source-reviewed only;
no build/configure/compiler invocation was performed.

The optional ``CUBITSIM_REPORT_TESTS`` target supplies tests for transactional
generation, changed-source rejection, profile precedence/revisions, shared-file
locking and preservation, session round trips, malformed tables, PDF/ODT output
and INCLUDE cycles. The test sources have not been built or executed. Read-only
source audits checked class file pairs, local includes, delimiter counts, Allman
control braces, target source registration and whitespace.

Source/preview revision
-----------------------

Project report schema 2 stores explicit model/job source kinds, pending source
configuration and current-model snapshot fingerprints. Only the current report
format is supported during development; no older-format migration is provided.
Pending requests are separate from active
document requests, preventing a source-combo change from changing provenance.

ReportPreviewWidget now derives from QAbstractScrollArea. It calculates document
coordinates in points and viewport coordinates in logical pixels, applying DPI
once for percentage zoom. Scrollbars expose the full page stack; painting visits
visible pages only. Zoom anchors a document position, and repeated layout/state
refreshes do not navigate again. Explicit page commands have a separate revision
from zoom/fit preferences and from opening the preview window.

ReportSettingsDialog uses Core validation and one settings command. It checks
the committed configuration after dispatch before accepting. ReportIconLoader
uses Framework's injected component directory and the standalone deployed SVG.
No shared Framework icon loader or extra generic panel abstraction was added.

Additional test sources cover pending-source isolation, atomic patch rejection,
model-only reports, continuous scrolling,
horizontal scrolling under zoom and view-position preservation. These tests
remain unbuilt/unrun under the active no-build restriction.

Workspace refresh is read-only: source discovery never selects a model/job by
command. Only explicit source activation submits ``report source``. Configuration
controls require an available source matching the committed Core request;
discovery failures clear stale choices. Existing report previews remain separate
from source availability. This prevents observer refreshes from repeatedly
submitting source commands while Cubit is executing a job or journal.

Regression test sources cover construction, queued/repeated refresh, source
appearance/removal, discovery failure/recovery, project reset and one command per
explicit source activation, including a command that does not commit Core state.
These additional tests have not been built or executed.

Company decoration and preview navigation
----------------------------------------

ReportPreviewCommand distinguishes the PAGE token in FIT PAGE from a numeric
page argument. Fit requests have their own revision, allowing repeated Fit page
actions to align the current visible page without ordinary observer refreshes
resetting scrolling. Both preview surfaces own native QSliders; synchronization
blocks signals and slider dragging commits navigation on release through
silent_cmd. No shared toolbar/panel abstraction was introduced.

ReportUserOptions owns company text, header/footer templates and logo layout
defaults. No company logo is assigned by default. ReportPageDecoration owns the
shared placeholder expansion, point-based page bands and decoration rendering.
It also serializes editable ODT decoration tables with native page-number fields.
The body reserves 96 points at the top and bottom; each logo fits within its
column and a 36-point height, preserving its aspect ratio. PDF uses the preview
painter. ODT reserves matching bands but its editor controls final pagination.

ReportLogo imports self-contained SVG/PNG/JPEG using Qt Svg and image readers,
then normalizes a bounded PNG snapshot. ReportConfiguration carries an optional
typed company_logo outside its string options. replace_company_logo expresses
patch intent only: ordinary text patches preserve the logo, while an explicit
null replacement removes it. JSON stores the image or null, not the intent flag.
ReportCore merges patches and validates candidates before committing. Requests
do not perform document validation.

The dialog stages logo imports through ReportApplicationInterface and submits
one settings command. The new settings-data form transports base64-encoded JSON
to avoid command quoting ambiguities; logo imports carry a filename rather than
image bytes in the command. Core imports that file again transactionally. The
dialog verifies committed text and logo bytes before closing. It does not mutate
Core state directly. Profile payloads are limited to 2 MiB, including the logo;
project payloads remain limited to 64 MiB. Neither export nor profile loading
reopens the original image file.

Additional unbuilt/unrun regression tests cover fit alignment/resizing, slider
dispatch without refresh feedback, atomic company settings validation, logo
removal, HDF5 profile/project round trips after deletion of the original file,
PDF export, and ODT image embedding, XML and native page fields. No build,
configure, compiler or test executable was invoked for these changes.
