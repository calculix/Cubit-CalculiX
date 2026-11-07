Resource monitoring architecture
================================

Ownership and interfaces
------------------------

``FrameworkContext`` owns the Framework preferences and GUI attachment. Its
``ResourceStatusWidget`` is appended to the existing ``QStatusBar`` using
``addPermanentWidget(widget, 0)``. Its compact horizontal layout has no expanding
centering spacers: working-directory/status text remains on the left, native
progress occupies the middle, and resource indicators occupy the right in the
same row. No separate toolbar, native progress replacement or reparenting is
introduced. The widget
owns a modeless read-only details window, an options page and the sampling monitor.
Qt pointers guard host-owned widgets against destruction. The options manager
unregisters pages before the status widget is deleted. The monitor joins its
outstanding worker before any worker state is destroyed. GUI detachment precedes
module shutdown; headless sessions do not construct the monitor or widgets.

``UserOptionsManager`` accepts Framework-owned pages as well as workspace pages.
The Cubit-Sim page uses the existing options Apply/Refresh signals. Its changes
submit readable ``sim useroptions monitor ...`` commands through ``CommandDispatcher`` only for edited values. The plugin reserves
``CubitSimUserOptionsCommand``; application modules cannot register that key.
``FrameworkUserOptions`` owns the defaults and validators, merges typed changes
with saved preferences, saves workspace ``framework`` through
``UserOptionsInterface``, then publishes the trivial candidate without throwing.
A GUI refresh failure after saving is reported as a warning, not a failed save.

``ModuleInterface::job_monitor_provider()`` is optional. ``ModuleManager`` exposes
borrowed providers only while initialized. ``JobMonitorProviderInterface::snapshot``
is an owner-thread, read-only query returning copied typed entries. Its contract
forbids checking/completing jobs, loading results or passing native handles and
callbacks into the worker. The worker knows only process identities and labels.
Provider failures are represented in the details window without a repeated log
message on every sample. Job states remain application-reported states.

CalculiX mapping
----------------

``CalculiXModule`` owns ``CalculiXJobMonitorProvider``. It calls
``CalculiXJobsInterface::get_monitor_snapshot`` which delegates to
``CoreJobs::get_monitor_snapshot`` in ``CoreJobsMonitoring.cpp``. Framework never
reads ``jobs_data`` or calls CalculiX GUI code. Existing status codes map to
NotStarted (-1), Running (1), Finished (2), Killed (3), Failed (4), Waiting (5),
and Unknown otherwise.

Each successful Windows/Linux launch calls ``capture_monitor_execution`` to
assign a UUID and capture the actual process creation identity. This observation
must not fail or stop a successfully launched solver. Only running/waiting entries
with the captured PID expose the process identity. Other states expose no live
process association. The last execution record remains available for its label
until rerun, deletion, reset or successful model loading. It is never serialized.
Saved process IDs alone cannot create monitoring associations. Resource sampling
does not change completion handling, result conversion or child-process behavior.

Sampling and mathematical contracts
----------------------------------

``ResourceMonitor::request`` starts a single ``std::async`` worker. It rejects
further requests until the result is consumed. The GUI timer checks completion
without waiting and requests the next sample at the configured interval.
No Cubit, Qt widget, provider or mutable solver Core access occurs in that worker.
The native reader and CPU history are used serially. Destruction waits before
releasing them; disabling monitoring clears both history and pending display data.

``FrameworkContext`` tracks the resource widget with ``QPointer``. Teardown
unregisters options, deletes the resource widget, then releases the workspace
host. Qt removes the destroyed widget from the status layout; the native status
bar and its progress control remain owned by Cubit. Monitoring visibility affects
only the resource widget. Workspace toolbar visibility no longer needs special
handling for monitoring because no resource toolbar is registered.

``ResourceAttribution`` derives both CPU and RAM segments in Core. Every width is
a fraction of total machine capacity. Visible Cubit and unique running/waiting
solver processes receive attributed widths; the remaining used portion is green.
Duplicate process identities are counted once within the job sum. Disabled
categories remain green. Missing process data marks attribution incomplete;
missing machine data clears all fills. The used fraction is authoritative:

.. math::

   s = \begin{cases}U/(C+J), & C+J > U \\ 1, & C+J \leq U\end{cases}

   (w_C,w_J,w_O) = (sC/T,\ sJ/T,\ \max(0,U-s(C+J))/T)

Here U is measured machine usage, T is capacity, and C/J are visible process
measurements. CPU uses percentage points with T=100; RAM uses bytes. Original
measurements are retained separately from adjusted widths. Memory attribution
is approximate because of shared pages, and CPU attribution because sampling
intervals differ. Tooltips explicitly flag adjustment and missing attribution.

``ResourceUsageButton`` paints cumulative segment boundaries, clipped within the
normal styled button, with translucent colors behind palette text. It preserves
keyboard focus/click behavior and elides labels in narrow windows. CPU and RAM
use the same widget, with no change to native Windows/Linux readers. Empty,
stale and failed sample states clear previous fills and details.

Three Core-owned ``#RRGGBB`` defaults configure system green, Cubit blue and job
orange; runtime snapshots hold packed numeric RGB values so post-save publication
remains non-throwing. The options page uses QColorDialog selectors and independent
attribution/button visibility flags. Typed partial changes preserve unrelated
saved settings. Unknown keys, wrong types and invalid colors are rejected before
persistence. The options GUI emits no commands for unchanged fields.

Process identity is PID plus creation FILETIME on Windows, or PID plus native
``/proc/PID/stat`` starttime on Linux. These are opaque runtime tokens for one
machine/boot, not portable timestamps. Native reads must match the recorded token
before returning metrics. Missing/exited processes have no metrics. Successful
samples retain baselines keyed by the complete identity; absent identities are
removed. The initial sample or decreasing counters yields unavailable CPU.

For positive elapsed steady-clock time and a known positive online processor
count, process utilization is:

.. math::

   U = 100\,\frac{\Delta(t_\mathrm{user}+t_\mathrm{kernel})}
                      {\Delta t_\mathrm{wall}\,N_\mathrm{logical}}

The display clamps the result to [0, 100]. It reports total host capacity, not
one-core utilization or affinity/container quotas. Values represent the interval
between valid samples; GUI blocking can lengthen that interval. Memory is current
resident/working-set memory, not committed virtual memory or solver peak memory.

Windows uses ``GlobalMemoryStatusEx``, ``GetProcessTimes``,
``GetProcessMemoryInfo`` with limited-query access, and
``GetActiveProcessorCount(ALL_PROCESSOR_GROUPS)``. Machine CPU uses PDH's
language-neutral ``Processor Information(_Total)\\% Processor Time`` counter,
including machines with multiple processor groups. Failed/invalid counters remain
unavailable. Handles and queries are closed through local ownership.
See Microsoft's `process timing contract
<https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes>`_
and `language-neutral PDH contract
<https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhaddenglishcounterw>`_.

Linux reads the first eight aggregate CPU fields, excluding the duplicate guest
fields. Idle includes idle and iowait. Decreasing/inconsistent totals invalidate
that interval. Used RAM is ``MemTotal - MemAvailable``; if availability is missing,
used RAM is unavailable. Process stat parsing starts after the last closing
parenthesis, so command names containing spaces/parentheses do not shift fields.
CPU fields are utime/stime; child-time fields are excluded. Starttime supplies
identity, RSS pages are multiplied by the native page size, and clock ticks use
``sysconf(_SC_CLK_TCK)``. Zombie/dead records are unavailable.
See the `Linux proc documentation <https://docs.kernel.org/filesystems/proc.html>`_.

Build layout
------------

``Framework/System`` is a position-independent static library, declared before
CalculiX. Both ``calculix_core`` and ``cubitsim_framework`` link it, avoiding a
module-to-Framework DLL dependency cycle. It has no Qt or Cubit dependency.
CMake selects the Windows or Linux source explicitly. Windows links PSAPI/PDH;
Linux uses procfs and libc, without Windows headers/libraries. Both use C++17.
The Framework worker links ``Threads::Threads``. No QRC or generated resources
are introduced. New builds must configure CMake to discover these source files.

Verification
------------

No build, compilation, C++ test execution or native GUI test has been performed
by the agent under the user's no-build restriction. A read-only Python ctypes
probe on Windows verified limited-query process timing/working-set access and the
language-neutral PDH counter. This validates the chosen OS calls, not the compiled
C++ integration. Linux runtime validation remains pending.

``tests/monitoring`` is a standalone CMake test project, not automatically built
with the plugin. It compiles the production native reader/monitor and Framework
options code. Tests cover initial/second process CPU samples, bounded memory,
invalid PIDs, mismatched creation tokens, no sampling backlog, worker teardown,
default options, failed persistence and invalid typed settings/intervals.
Additional tests compile the production attribution functions and cover CPU/RAM
segment totals, proportional fitting, preserved raw values, missing/zero machine
capacity, missing process samples, provider failures, duplicate identities and
hidden categories. Options tests cover RGB defaults, invalid colors, failed color
saves and original six-key journal replay preserving newer preferences.
They require a C++17 compiler, threads and Qt6 Core for the options test, but no
Cubit installation. Configure/build/run only after authorization.

After a successful Windows and Linux build, verify:

* Single bottom-row placement in every workspace: native status/directory left,
  progress middle and CPU/RAM/Jobs right, with no extra toolbar. Check native messages,
  narrow windows, both themes, keyboard focus/clicks and button text contrast over
  each color. Check GUI detach/reattach and closing Cubit during a sample.
* Verify blue/orange/green fills for both CPU and RAM, color edits, independent
  attribution/Jobs visibility, missing-sample clearing and explicit tooltip notices
  for adjusted/incomplete attribution. Restored workspace layouts must leave
  resource visibility controlled by Framework preferences.
* Options Apply/Cancel/reopen, immediate enable/disable, intervals 1 and 60 seconds,
  journal replay, headless Apply, locked/corrupt files and other workspace settings.
* Two concurrent CalculiX jobs, zero jobs, rerun under the same ID, quick exit,
  killed/failed jobs, reset/delete/load, and a saved model with stale process IDs.
* Job measurements in Cubit/Postprocessor as well as CalculiX. Resource monitoring
  must never trigger job completion, conversion, termination or result loading.
* First-sample dashes, inaccessible/exited processes, missing PDH counters/procfs,
  processor groups above 64 CPUs, and Linux command names with spaces/parentheses.
* Long GUI-blocking conversion: no sampling backlog and stale display recovery.
  Verify child processes are excluded and process memory is not summed as machine
  RAM. GPU monitoring is deliberately outside this patch.
