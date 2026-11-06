Source review findings and verification blockers
=================================================

Review status
--------------

Recorded during the documentation-only pass on 4 November 2026. No production
code, build, solver execution or GUI automation was changed/performed in this
pass. Source findings below are not runtime reproductions. Line numbers identify
the inspected working tree and can move after later edits.

Findings are for later review and approved fixes. They do not authorize changes
or diagnose earlier Cubit/solver crashes. API hazards are distinguished from
confirmed reachable application failures.

R1: Topology Actions still enables unsupported level-set Run
------------------------------------------------------------

Priority: P2, source-confirmed GUI inconsistency.

In :download:`TopologyActionsPanel.cpp <../../src/CalculiX/GUI/topology/TopologyActionsPanel.cpp>`
line 203, Run is enabled solely from ``!information.active``. The Execute path
does not check the configured method before submitting the command. Configured
level-set cases are now selectable, while Management and the case context menu
disable Run for that method. Core still rejects the unsupported run, so this
does not establish an execution-gate bypass.

Later fix: align Actions availability/tooltips with the supported method and
objective checks, retain Core validation, and verify all three entry points with
a configured level-set case. No fix was applied here.

R2: Topology Actions convergence remains density-specific
----------------------------------------------------------

Priority: P2, source-confirmed method-dispatch omission.

The same file hard-codes ``Maximum density change`` around line 277, reads
``settings.density_tolerance`` around line 404, and reports an ordinary density
proposal message around line 422. The shared iteration query can supply nodal
field change for a historical level-set design. Management already queries the
captured method and field tolerance; Actions does not. This is a dormant display
defect for such histories, not evidence that public level-set runs are enabled.

Later fix: use the iteration's captured method/settings and retain the Core
convergence flag as authoritative. Test a historical iteration after editing the
case method. Do not relabel existing history from current settings alone.

R3: ThreadPool destructor relies on callers joining workers
------------------------------------------------------------

Priority: P2 API/lifetime hazard; no crash reproduced.

:download:`ThreadPool.cpp <../../src/CalculiX/Utility/ThreadPool.cpp>` has an empty
destructor at line 6. Its vector can still contain joinable ``std::thread``
objects if destruction occurs after ``start()`` without completing ``stop()``.
Destroying a joinable thread terminates the process. Explicit stop calls exist
in reader/legacy-writer paths; this finding does not assert that those normal
paths currently omit their join.

Later review: establish the intended owner/exception-unwind contract, make
shutdown safe for partial startup and add a lifecycle test before changing it.
Review worker exceptions separately; do not infer that this explains a solver
or FRD/DAT error reported in another run.

R4: ThreadPool worker-count and restart contracts are unenforced
----------------------------------------------------------------

Priority: P3 API hardening gap; normal caller reachability unverified.

``start()`` at line 9 does not reject zero/negative worker counts and uses an
unsigned loop variable. Zero workers with queued jobs leave those jobs unserved;
a negative signed count can convert to a large unsigned bound. ``stop()`` sets
``should_terminate`` but ``start()`` does not reset it, so stop/start reuse is not
a working restart contract. ``busy()`` reports queued jobs only, not in-flight
work. Existing callers that follow queue-drain with ``stop()`` still join their
active workers; do not misreport that pattern as proven premature completion.

Later review: inspect options validation and all call paths, decide whether the
class is explicitly single-use, and test zero/negative counts, startup failure,
queue drain, in-flight work and repeated shutdown. No behavior was changed here.

R5: Reported topology DAT conversion lacks exact formulation
------------------------------------------------------------

Priority: P2 reported runtime integration failure; root cause unresolved.

The log reviewed on 6 November 2026 shows a HEX8 topology solve finishing with
DAT energy available, then VTK planning rejecting the formulation of element 1.
The next iteration cannot start because conversion remains pending. In
:download:`CoreResultsVtkLink.cpp <../../src/CalculiX/Core/results/vtk/CoreResultsVtkLink.cpp>`
line 422, the error covers empty/unsupported/non-integrable formulations.
Link selection at lines 542--589 can retain FRD-only geometry after unsuccessful
model/INP matches. Exact integration formulation must not be guessed from FRD
topology or generic Cubit element shape.

Later review: obtain the matching deck/includes, FRD/DAT and link diagnostics;
identify the failed provenance/formulation path. Test DAT integration-point
conversion with no FRD fields, matching run input, full/reduced formulations and
topology-generated element sets. Distinguish absent fields from absent geometry.
No converter fix or runtime reproduction was performed here.

R6: Topology completion does not establish VTK readiness
--------------------------------------------------------

Priority: P2 source-confirmed coordination gap, also visible in the supplied log.

:download:`CoreTopologyOptimization.cpp <../../src/CalculiX/Core/topology/CoreTopologyOptimization.cpp>`
checks solver status at line 980 and prints ``results are loaded`` at line 1071
after numerical evaluation. In contrast,
:download:`CoreJobs.cpp <../../src/CalculiX/Core/jobs/CoreJobs.cpp>` lines 893--911
gate later starts on pending VTK publication, and lines 914--944 retain pending
state on conversion failure. Thus an evaluated iteration can be reported complete
and a new design/job prepared before the next start is rejected. Solver status,
topology evaluation and converted-result availability are distinct contracts.

Later review: agree the required conversion policy, coordinate it through the
existing interfaces before progressing to another physical/adjoint job, and
report the failing result phase accurately. Test conversion failure, cancellation,
successful retry, optional export failure and reset/deletion. A successful retry
must not be described as resuming an already failed topology run. No production
change was made.

B1: Numerical journal cannot reach native level-set helpers
-----------------------------------------------------------

Type: verification blocker, not a confirmed algorithm defect.

The current :download:`Python interface header <../../src/CalculiX/PythonInterface/CalculiXPythonInterface.hpp>`
exposes settings and stored-result queries, but not the pure interpolation,
element fraction/derivative or proposal/acceptance functions. There is no current
built plugin available for native checks, and production code is frozen. A
Python-only copy of the mathematics would not test those native functions.

The unified journal records numerical stages as BLOCKED and whole verification
as INCOMPLETE. A separately approved native test route and build are required
before :doc:`numerical-verification-plan` can be executed against production code.
No new C++ entry point or public level-set bypass was added.

B2: Documentation extraction and native evidence remain unbuilt
----------------------------------------------------------------

Type: evidence limit.

The Doxygen input now covers the project-owned Framework and CalculiX roots,
excluding external source distributions. The source catalogue has been checked
for local file targets. Doxygen/Sphinx were not run, so generated XML resolution,
HTML navigation/search, formula rendering and file downloads still need an
authorized documentation build and offline viewing. No screenshot is presented
as current native GUI evidence. Source navigation coverage is not a per-function
semantic audit or numerical verification certificate.
