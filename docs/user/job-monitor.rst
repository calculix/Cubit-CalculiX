Job monitor
===========

The Job Monitor has four tabs: **Live Monitor**, **Console Output**,
**Convergence (CVG)** and **Status (STA)**. Use **Load** to load and convert job
results. **Results** opens the converted job as a Postprocessor source. Switch
to the Postprocessor workspace to inspect fields and DAT section tables or
create history plots. See :doc:`../postprocessor/tables` and
:doc:`../postprocessor/plots`.

The FRD, DAT and DAT Section tabs and their ``ccx result csv job`` and
``ccx result plot job`` command forms have been removed. Update journals that
call those commands. The Postprocessor exports plot samples to CSV. Use the
existing CalculiX Python result queries for scripted access to raw FRD, DAT
and section records.

Status (STA)
------------

Each row describes a reported increment attempt. Columns are Step, Increment,
Attempt, Outcome, Iterations, Total time, Step time and Increment size. Times are
solver time, which may be pseudo-time for static analyses, not elapsed wall time.

CalculiX writes an unsuccessful attempt with a ``U`` suffix, for example ``1U``.
The table separates this into Attempt 1 and Outcome Unconverged, with bold colored
text. The failed attempt remains visible when a later retry converges. An outcome
describes the increment attempt, not whether the entire solver job finished.

Convergence (CVG)
-----------------

Each row describes one iteration. Columns are Step, Increment, Attempt,
Iteration, Contact elements, Force residual [%], Displacement correction [%],
Flux residual [%] and Temperature correction [%]. Values and inactive-channel
zeros are preserved as reported by CalculiX. No generic red/green convergence
threshold is applied. Scientific notation remains available for very large or
small numbers.

Filter by step, increment or attempt using the All/numeric selectors. Selecting
an STA row filters CVG to that step, increment and attempt. Use All to broaden
the selection again. Filters do not change the Live Monitor plots.

Table controls and live updates
------------------------------

Click a column header to sort numerically. Select rows and press Ctrl+C, or use
the context-menu copy action, to copy tab-separated values with column headers.
Rows use compact font-based heights and the longer column titles span two lines.
Columns fit the first populated batch of each run; subsequent manual resizing
is preserved during live updates. Copied headers remain on a single line.
Follow latest starts enabled. Scrolling upward, dragging the scrollbar or
clicking a row pauses following; recheck the control to follow new rows again.

New complete rows are appended without rebuilding the table. A final line
without its newline stays pending until the writer completes it. Restarting a
managed job or detecting file truncation/replacement clears the previous table
generation and the corresponding plot data. Header-only files display an empty
state rather than an error, including analyses that do not report these records.

Unrecognized or invalid complete rows are retained in the expandable Unparsed
lines section with their source line number; this section is hidden when empty.
Reader errors appear above the table while cached rows remain inspectable.
Raw completed lines remain
part of saved job data; typed monitoring caches are reconstructed after reload.
