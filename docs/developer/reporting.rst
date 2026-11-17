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
one readable settings command containing only changed named arguments. Text is
quoted, boolean arguments use on/off, and logo imports carry a filename rather
than image bytes. The encoded settings-data form has been removed. Core imports
that file again transactionally. The
dialog verifies committed text and logo bytes before closing. It does not mutate
Core state directly. Profile payloads are limited to 2 MiB, including the logo;
project payloads remain limited to 64 MiB. Neither export nor profile loading
reopens the original image file.

Additional unbuilt/unrun regression tests cover fit alignment/resizing, slider
dispatch without refresh feedback, atomic company settings validation, logo
removal, HDF5 profile/project round trips after deletion of the original file,
PDF export, and ODT image embedding, XML and native page fields. No build,
configure, compiler or test executable was invoked for these changes.

Report file toolbar and overwrite policy
---------------------------------------

ReportToolbar owns only its PDF/ODT actions and separators. Native New/Open/Save
actions are borrowed from ClaroFileMenu::get_items_for_toolbar and identified by
unique standard shortcut bindings. The toolbar retains original native ownership,
handlers, icons and enabled states. Missing/ambiguous native bindings fail GUI
startup with an explicit error rather than guessing by translated text or list
position. Journal actions come from ClaroToolsMenu::get_items_for_toolbar.
WorkspaceView::toolbars supplies placement and visibility; ReportWorkspace deletes
its toolbar explicitly because QMainWindow reparents registered toolbars.

ReportExportDialog is shared by the toolbar and preview window. A save dialog
sets the single-format default suffix. Both-format export treats its input as a
base name, lists the actual existing outputs in one replacement prompt, and
submits overwrite only after approval. Cancellation is handled without dispatch.
ReportApplicationInterface also checks existing paths for command-only callers:
overwrite defaults to false and every destination is preflighted before any
output. PDF/ODT encoders write to caller-owned temporary devices. Application
export stages and encodes both outputs before committing either. Cancellation
before publication preserves both destinations. Final commits are non-cancellable
and not jointly atomic; a second commit failure reports the already published
PDF explicitly. Standalone filename writers also use QSaveFile staging.

P02/O02 are deployed as Icons/ExportPdf.svg and Icons/ExportOdt.svg. Report's GUI
now links the Claro GUI libraries for native action access. Added regression test
sources cover toolbar format selection, export enablement, single-format
replacement policy, paired-output collision preflight and directory rejection.
These tests have not been built or executed under the active restriction.

Readable settings and project feedback
-------------------------------------

ReportConfigureCommand derives named settings arguments from ReportUserOptions
and the existing steps selector. Omitted arguments preserve their current values;
ReportCore validates and commits the complete patch atomically. The dialog keeps
its opening configuration to identify edits, and ReportSettingsCommandText builds
a deterministic readable command. Applying without edits submits nothing. Mixed
quote delimiters that cannot be represented by the existing command quoting
helper produce an explicit error, with no encoded fallback. Regression sources
cover readable flags, empty strings, placeholders, filenames, quoting errors and
the dialog's changed-field-only/no-change behavior; they remain unbuilt/unrun.

Framework ProjectStateCoordinator now reports successful writes per provider and
successful read commits per provider. This includes the implemented Postprocessor
prepare/resolve/commit handler. Framework metadata is validated before restoration
and written after workspace writers close their files. Framework success is not
reported as a complete workspace save when any writer failed. CalculiX's old
duplicate global start/completion messages are removed. Native File-menu actions
remain available in all workspaces; the existing native Save action owns Ctrl+S,
Save As for unnamed projects, replacement prompts and cancellation.

One report per source
---------------------

ReportCore matches reports by canonical workspace ID, source kind and model ID
or job ID. Title, profile, fingerprints and run identity are excluded from that
key: explicit Generate may replace those properties without adding an instance.
Generate builds and validates a candidate before replacing the stored document,
retaining its private ID and notes. Update still checks job source consistency.
The 1000-report limit applies to new sources, not replacement. Session reads
reject duplicate canonical source identities; no older-format migration is added.

The public ID-based selection API and command and the second report dropdown
have been removed. Source selection calls a private activation helper to restore
the document and its configuration draft. Named profiles are already loaded by
profile commands, so the GUI's Generate action submits only the source command,
preserving edits made after loading a profile. Profile selection is reflected
from committed configuration without dispatch during refresh.

ReportSourceEntry::available describes regeneration capability, while has_report
allows a cached source to remain selectable. Live discovery is merged with saved
reports; missing live entries and failed discovery retain cached-only sources.
The Core rejects generation/update for unavailable sources independently of GUI
button state. Cached preview/export does not need original input files. A source
without a generated report shows an empty preview while its pending settings
remain separate from the prior source's stored document. Initial preview mode
is Fit page; ordinary refresh preserves manual fit/zoom choices.

Unbuilt/unrun regression sources cover repeated generation, title/profile changes,
failed replacement, changed job snapshots, canonical provider aliases, per-source
notes/drafts, missing sources/discovery, duplicate-session rejection and replacement
at the report-count limit. The no-build/no-compile restriction remains active.

Native operation progress
-------------------------

``ReportProgress`` is a synchronous, borrowed provider contract without GUI or
host dependencies. Core accepts an optional sink for Generate/Update and forwards
it only through transient request fields. The sink is not serialized, and Core
clears the pointer before committing the resolved request into session state.

``ReportNativeProgress`` adapts this contract to Cubit's ``ProgressTool`` in the
host-facing Report application. It runs only with the GUI on its owning thread,
throttles repeated checkpoints to 200 ms and closes the native display on every
exit path. The native tool is started once, never restarted for stage/count
changes, and ended only when the operation scope exits. Explicit
``ReportProgressStage`` values drive monotonic overall workflow progress. Only
``finish`` reaches 100%; it does not poll cancellation after publication. The
native description is stable and detailed stage changes use command output.
Report does not introduce a custom progress widget, a worker thread for SDK
captures or an additional Qt event-processing loop. Mutating Report application
operations are guarded while generation or preview preparation is running.

Regression sources exercise stage notifications, cancellation before publication,
retry after cancellation, transient request serialization and atomic input-reader
cancellation. They are registered but have not been compiled or executed.

Content groups and preview construction
--------------------------------------

``ReportContentGroup`` and ``ReportOption::group_id`` are template metadata.
Core validates group IDs, parent cycles, option membership and boolean gates.
The Content tree submits grouped changes through the existing readable atomic
settings command. It never emits commands during passive refresh and defers its
own rebuild until Qt has finished delivering the edited item's signal.

``ReportOutlineTree`` reads committed sections and aggregate counts without
serializing or copying table contents. It retains page locations for the same
immutable document across pre-layout refreshes. The diagnostics page is a
layout-owned anchor, explicitly recognized by both navigation widgets.

``ReportDocumentLayout::prepare`` accepts a borrowed progress sink and plans
bounded ``ReportLayoutBatch`` documents. Core owns row/cell/text limits. Each
batch uses disabled undo/layout while inserting cells, then paginates privately.
Large tables continue with repeated headers; long textual cells can continue in
additional display rows. Original semantic rows remain in the outer report for
ODT and persistence. Planning copies table headers and selected rows only, never
the full source table on each continuation. Page offsets map a global preview/PDF
page to one batch and its local page. TOC markers are integer positions replaced
in equal-width monospaced ranges after global page offsets are known. A page-count
change during contents resolution fails preparation instead of publishing stale
page references. All candidate batches remain private until preparation succeeds.

Workspace refresh is guarded against reentry. The observer keeps its pending
flag set throughout refresh, including native progress callbacks. Layout progress
does not introduce another Qt event loop. Regression sources cover grouped
commands, partial selection, content-group cycles, outline counts/navigation,
preview cancellation/retry, and the synthetic diagnostics page title.

Transactional preview publication
---------------------------------

``ReportDocumentPreparation`` is an optional application callback forwarded
through Core generation/update. Core validates the candidate, invokes preparation
and checks cancellation again before publishing session changes. The GUI
application uses the callback to build a temporary layout with the same borrowed
progress sink. Callback failure/exception leaves the previous report and layout
intact. Successful Core publication is followed by publication of the matching
layout cache, then native progress completion and command return. Headless
generation supplies no Qt layout work.

``prepared_layout`` is a read-only cache accessor, and rejects pending sources or
documents that differ from the cache's immutable source. Workspace refresh uses
this accessor only. Explicit application operations use ``layout`` to prepare a
cache miss; no timer/observer refresh initiates expensive layout. Project restore
prepares a GUI layout from its staged session before activating that session.

Preview-only operations normalize progress to the content/pagination/contents
portion of the workflow. Preview counts describe completed bounded batches;
model callbacks remain coarse because their local counts cannot establish total
model completion. Each individual Qt pagination call remains synchronous, but
native progress and cancellation checkpoints occur between batches. There is no
final whole-report Qt pagination call or additional nested event loop.

Regression sources additionally cover failed preparation and post-preparation
cancellation before publication, successful callback ordering, headless content
generation, read-only GUI refresh, explicit cache preparation and progress ranges.


Postprocessor report collection
------------------------------

Frame policy is resolved from the workspace template and shared by result and
image collection. ``last_per_step`` is the CalculiX default; ``envelope`` and
``all_frames`` are alternatives in the content tree. Choices use stable values
in readable settings commands. Boolean group operations skip choice editors.
Detailed mesh/membership/nodal/element/source-table output is gated before its
report rows are constructed. Historical input stays immutable and is not
re-exported to resolve membership.

Verified block metadata is matched by dataset ID and originating model ID.
Unknown block mappings emit diagnostics. Cross-frame extrema compare Report-unit
values and retain each winner's source frame and original sample location;
incompatible value or coordinate units fail collection. The source buffers are
never changed. Automatic captures use a detached snapshot with explicit
source/frame/block/component selection and frozen Report presentation preferences.
The existing named-view capture path and live Postprocessor state are preserved.

``color_values_range`` resolves a VTK array once per bounded host-thread chunk.
Up to four async workers reduce private 65536-sample buffers, with deterministic
tie handling and no VTK/Cubit/GUI access. Cancellation signals workers and drains
all futures before returning. Rendering and Qt batch layout remain on the host
thread. These boundaries allow progress without unsafe concurrent rendering.


Export progress and ODT packaging
--------------------------------

``ReportProgress`` includes PDF, ODT content, package and commit phases plus a
non-cancellable ``finish`` notification. Native export progress uses an explicit
phase order so omitted formats and cached preview preparation reserve no unused
ranges. Generation retains its original phase denominator through Contents.
Percentages describe work completion, not remaining wall-clock time. Only
successful file publication finishes the native bar at 100 percent.

``export_document`` retains an immutable report and holds the application busy
guard throughout validation, optional PDF layout, encoding and commit. It never
calls public ``layout`` under that guard: a missing PDF layout is prepared through
the private service with the same sink. ODT consumes ``ReportDocument`` directly
and never paginates a Qt preview. A borrowed progress sink supports non-GUI callers
and cancellation regressions. Encoders neither close nor commit output devices.

PDF checkpoints occur before and after each page. ODT checkpoints cover planning,
256-cell content blocks, images, XML finalization and package I/O. Individual Qt
page-paint/image-decode calls and final file commits remain synchronous. There is
no additional Qt event loop or background SDK/render thread.

``ReportOdtStyles`` separates heading, table, figure and caption styles. Cached
contents recurse through section children and link to inline heading bookmarks;
table titles and the contents title are not outline headings. Native ODT page
numbers remain editor-generated. Shared point-based page geometry and the
300-point figure-height limit preserve landscape and portrait aspect ratios in
both preview and ODT. Semantic report rows and stored unit values are unchanged.

``ReportOdtPackage`` writes uncompressed classic ZIP entries with mimetype first.
Content, styles and manifest XML are seekable temporary files. Only one decoded
image payload is held at a time, and only directory metadata accumulates in
memory. Entries are read twice in 64-KiB chunks (CRC then copying), with a table
based CRC-32 and cancellation checks in both passes. Directory copying also has
checkpoints. Classic ZIP sentinel sizes/offsets and 65535 entries are rejected
before narrowing; ZIP64 is unsupported. The final package is never published on
encoding, size-limit or cancellation failure.

``ReportExportTests`` covers cancellation in PDF/ODT/package/publication phases,
paired destination preservation, no-preview ODT export, busy/reentrant guards,
original report immutability, nested contents links, image proportions, numeric
units, XML structure, package limits and the standard CRC-32 check vector. These
regressions require the normal user-run build/test workflow; source checks do not
substitute for native export and office-editor validation.
