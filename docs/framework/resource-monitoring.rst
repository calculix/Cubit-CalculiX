Resource monitoring
===================

The Framework places resource indicators at the right of Cubit's existing bottom
status row, after the native progress control. Working-directory/status text stays
on the left, native progress occupies the middle, and CPU/RAM/Jobs occupy the
right. No additional toolbar row is created. The indicators remain available in
Cubit, CalculiX and Postprocessor. Click any indicator to open a modeless details
window with machine usage, Cubit process usage and solver jobs.

The compact display contains stacked CPU and RAM bars and a Jobs button. CPU shows
machine utilization; RAM shows physical memory used and total. Jobs shows the
number reported running or waiting by their application. The details window lists job names, state, CPU
and physical memory individually. A running job can have unavailable measurements
when its process has exited before the application updates its state.

CPU is expressed as a percentage of total online-machine processor capacity,
from zero to 100. A process using one processor fully on an eight-processor
machine is approximately 12.5 percent. Process CPU requires two valid samples.
A dash means unavailable or awaiting that second sample, rather than zero usage.
Memory uses GiB (1024 cubed bytes). Process memory includes shared resident pages;
do not sum the job rows and interpret the result as machine memory consumption.

Both bars use blue for Cubit, orange for running/waiting solver processes and
green for remaining/shared usage, with unused capacity unfilled. Colors are
configurable. The entire bar width represents total machine capacity. The sum
of colored widths always matches measured machine usage.

Process attribution is approximate. Shared memory can be counted in multiple
processes, and CPU sampling intervals can differ. When attributed process usage
exceeds machine usage, blue/orange widths are reduced proportionally to fit;
tooltips explicitly mark that adjustment. Original process measurements remain
unchanged in tooltips and details. Missing process measurements are marked
incomplete and remain part of the green, unassigned region. Missing machine usage
clears the entire fill. Disabling an attribution color folds its share into green.

CalculiX reports the solver process launched for each job. Child processes,
external solvers not launched by this Cubit session and result conversion are
outside the initial scope. Loading a model with saved process IDs does not attach
monitoring to those processes. Each launch has a fresh runtime identity.

Windows uses native process/memory APIs and a language-neutral performance counter
for machine CPU. Linux uses ``/proc/stat``, ``/proc/meminfo`` and each process's
``stat`` record. Restricted or unavailable measurements remain unavailable.
GPU sampling is not implemented.

Settings and commands
---------------------

Open Cubit's options dialog and select **Cubit-Sim**. Monitoring is enabled by
default, refreshes every two seconds, and shows CPU, RAM and Jobs. CPU/RAM and the
Jobs button can be hidden individually. Cubit and solver-job attribution can be
toggled separately from indicator visibility. Three color selectors accept opaque
RGB colors. The refresh interval accepts one through 60 seconds.
Disabling monitoring, or hiding every indicator, stops sampling. Apply takes
effect immediately; no restart is required.

The page emits readable journalable setters; for example:

.. code-block:: text

   sim useroptions monitor interval 2
   sim useroptions monitor cpu on
   sim useroptions monitor system_color "#5CB85C"
   sim useroptions list

Only edited settings produce commands; ``list`` prints saved values in the same
readable syntax. Boolean settings accept on/off; colors use quoted ``#RRGGBB``;
intervals accept integers from 1 to 60 seconds. See :doc:`user-options` for all
commands. Each setter changes its named option, preserving other saved values.

Preferences use the shared HDF5 options file under workspace ``framework``.
``monitor cubit`` controls Cubit attribution in both bars; it does not create a
separate memory indicator. The encoded command payload format has been removed.
Validation and save failures leave active preferences unchanged. The command also
works headlessly, saving preferences without creating a timer or sampling worker.
Opening details and automatic measurements are read-only and are not journaled.

Sampling runs outside the GUI thread with at most one request outstanding.
Application job snapshots and widget updates run on the application owner thread.
Result conversion can block the GUI, so visible updates pause during conversion.
No request backlog is accumulated. Old results are discarded after the greater
of ten seconds and three configured refresh intervals.

Implementation and verification status are documented in
:doc:`../developer/resource-monitoring`.
