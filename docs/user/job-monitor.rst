Job monitor
===========

The monitor retains the live plots and solver console, and presents the solver's
STA and CVG records as read-only tables. These tables inspect solver progress;
opening converted result datasets remains the purpose of the Results button.

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
Follow latest starts enabled. Scrolling upward, dragging the scrollbar or
clicking a row pauses following; recheck the control to follow new rows again.

New complete rows are appended without rebuilding the table. A final line
without its newline stays pending until the writer completes it. Restarting a
managed job or detecting file truncation/replacement clears the previous table
generation and the corresponding plot data. Header-only files display an empty
state rather than an error, including analyses that do not report these records.

Unrecognized or invalid complete rows are retained in the expandable Unparsed
lines section with their source line number. Reader errors appear above the
table while already cached rows remain inspectable. Raw completed lines remain
part of saved job data; typed monitoring caches are reconstructed after reload.
