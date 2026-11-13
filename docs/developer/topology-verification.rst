Topology verification
=====================

Scope and evidence status
-------------------------

This chapter separates mathematical derivation and source inspection from
numerical verification. See :doc:`topology-algorithms` for the implemented
formulas and :doc:`topology-export` for applied factors and result precision.
A documented equation or method contract is not, by itself, a numerical test.

Current material-group derivative verification
---------------------------------------------

On 7 November 2026, run ``20261107T152047Z_4f21a6a1`` passed all 18 cases:
C3D4, C3D6, C3D8, C3D10, C3D15 and C3D20, each with compliance, absolute Y
displacement and displacement magnitude. The recorded Cubit version was 2026.11.
The loaded plugin revision was **not recorded**; a checkout revision must not be
substituted for the unknown binary revision. These results are evidence for the
tested binary and fixtures, not a certificate for all later source changes.

Each case ran three baseline density iterations and selected one interior,
non-preserved material group from the final physical input. Group membership,
Poisson's ratio, loads and constraints stayed fixed while only that group's
Young's modulus changed. Six additional physical solves supplied three central
finite differences. Displacement baselines used the implemented adjoint solve;
perturbed jobs did not need adjoints. The iteration limit was intentional and
does not mean optimization convergence was established.

For a common applied factor increment in group G, the derivative target is

.. math::

   g_G=\sum_{e\in G}\frac{\partial J}{\partial s_e}.

Native DAT queries recomputed compliance as twice the summed element energies,
or displacement as the absolute Y component or vector norm at the single
monitored node. The baseline objective was checked against the stored value.
The journal rejected Y sign changes and near-zero displacement objectives.
It checked finite raw sensitivities, exact zero sensitivities for preserved
elements, and unchanged topology history after the perturbation jobs.

Requested steps were h, h/2 and h/4, with
``h=min(0.02, 0.2*min(s,1-s))``. The exported modulus used six decimal places in
scientific notation. Actual steps were recovered from those rounded moduli,
using the fixture's base Young's modulus 210000. With positive step lengths a
and b on the lower and upper sides, the journal used

.. math::

   g_{\mathrm{FD}}=
   \frac{b^2(J_0-J_-)+a^2(J_+-J_0)}{ab(a+b)}.

Both finest estimates had to satisfy

.. math::

   |g_{\mathrm{FD}}-g_G|
   \le 10^{-8}\max(|J_0|,1)
      +0.02\max(|g_{\mathrm{FD}}|,|g_G|).

The two finest estimates also had to agree under the same combined rule, with
their magnitudes in the relative term. The absolute allowance is tied to this
fixture and its objective units; it is not a universal tolerance.

.. list-table:: Relative errors at the smallest perturbation (percent)
   :header-rows: 1

   * - Element
     - Compliance
     - Absolute Y
     - Magnitude
   * - C3D4
     - 0.044284
     - 0.016913
     - 0.009910
   * - C3D6
     - 0.043328
     - 0.016969
     - 0.016980
   * - C3D8
     - 0.046429
     - 0.023784
     - 0.029981
   * - C3D10
     - 0.045647
     - 0.007842
     - 0.012861
   * - C3D15
     - 0.044197
     - 0.013481
     - 0.013480
   * - C3D20
     - 0.047419
     - 0.020929
     - 0.022712

The shipped :download:`measurement summary <verification/topology-stiffness-sensitivities-20261107.csv>`
retains all three step sizes, derivatives, errors and comparison outcomes.
The local journal is ``tests/verify_topology_sensitivities.jou``; raw decks,
results and per-case reports are under
``tests/sensitivity_runs/20261107T152047Z_4f21a6a1/``. The tests directory is
Git-ignored, so those local files are not downloads or dependencies of this manual.
Preserve them externally for a full audit. Solver version, thread settings and
binary provenance should be captured explicitly in subsequent runs; Cubit and
wrapper version metadata alone do not establish them.

This verifies selected fixed-group stiffness derivatives. It does not verify
individual element derivatives, differentiation through material regrouping,
the density/filter surrogate, or the level-set fraction, nodal chain and update.
The level-set public execution gate remains closed.

Native interpolation and geometry evidence
------------------------------------------

After rebuilding the plugin with the bounded-Heaviside fix, the user reported
both Windows CTest entries passing: ``topology_interpolation`` in 0.38 seconds
and ``topology_geometry`` in 0.41 seconds (0.80 seconds total CTest wall time).
The retained local CSVs contain 3448 and 10044 check rows respectively, with
zero failed rows. The shipped :download:`native-test summary <verification/topology-native-math-20261107.csv>`
records those counts and the reported execution times. Binary revision/hash
was not recorded. This evidence predates the nodal-assembly extraction and does
not establish execution of its new native test or Linux coverage.

The first geometry run exposed six constant-void failures. Interpolation of a
constant coefficient can round just inside the regularization band, where
cancellation in the sine formula can produce a tiny negative Heaviside value.
Clamping the pointwise value to its mathematical range [0,1] removed that
endpoint issue. The final integrated-fraction validation and the test tolerances
were retained. The subsequent geometry test passed all declared cases, including
expected invalid-input and unresolved-integration rejections. Those expected
Cubit error messages are not unexpected test failures.

This covers the supplied straight-sided reference/affine fixtures, smooth nodal
fraction/volume derivatives and rejection contracts. It does not cover a full
live-Cubit assembly, an optimizer update, curved/mixed meshes or narrow-interface
accuracy. Nodal-assembly evidence from the subsequent run is recorded below.

Native nodal-assembly evidence
-----------------------------

On 8 November 2026 the user reported all three native tests passing on Windows:
interpolation 1.28 seconds, geometry 0.45 seconds and evaluation 11.62 seconds,
with 13.42 seconds total CTest wall time. The retained CSVs contain 3448, 10044
and 6041 check rows with no failures. The shipped
:download:`three-test summary <verification/topology-native-math-20261108.csv>`
records these counts and reported times; tested binary revision/hash remains
unrecorded. These results precede the QP extraction and do not certify that change.

The 30 analytical nodal cases exercise the shared production accumulation and
normalization against finite differences of sum(q_e/s_e). They cover all six
formulations, three void ratios, shared-node pairs, unequal volume/load weights
and preserved-neighbor fixed-node zeros. Invalid inputs and partial-output
clearing also passed. This is an analytical chain check using production geometry;
the complete live-Cubit evaluation, graph assembly, proposal/acceptance and
physical continuum level-set solve remain separate verification targets.

Native QP evidence
------------------

The subsequent Windows run passed all four CTest entries: interpolation in
0.47 seconds, geometry in 0.46 seconds, nodal evaluation in 12.25 seconds and
QP in 0.20 seconds, with 13.48 seconds total wall time. The retained CSVs contain
3448, 10044, 6041 and 131 check rows respectively, all passing. See the shipped
:download:`four-test summary <verification/topology-native-qp-20261108.csv>`.
Binary revision/hash remains unrecorded; these results apply to the tested
binary and do not establish Linux coverage.

The numerical QP cases passed analytical solution, bounds, signed linearized
volume, independent KKT and rejection/limit checks. This does not certify
the graph assembled from a live Cubit mesh or actual-volume backtracking.

Live C3D8 update evidence
------------------------

Run ``20261108T192653Z_fc574ef7`` passed all three live fixture groups inside
Cubit 2026.11 on Windows: single_hex 196 checks, shared_free 87 and
shared_preserved 99, totaling 382 passing checks with no failed CSV rows.
The shipped :download:`live summary <verification/topology-live-c3d8-20261108.csv>`
retains those counts and run identity. Raw evidence remains local under
``tests/live_topology_runs/20261108T192653Z_fc574ef7/``. Loaded binary revision
was not recorded.

The nonlinear backtracking case observed a full-proposal maximum change of
approximately 0.1000000112 and an accepted change of 0.05000000562 at scale 0.5.
The full proposal overshot actual volume; the accepted field was feasible while
the ordinary residual retained the larger source-owned value. Restoration
reduced volume while correctly reporting an unmet target. An interior shared
graph case passed independent KKT checks with a nonzero graph contribution.
Preserved/shared nodes, stale/tampered proposals, pinned lower bounds, aliasing
and failure clearing passed. Mesh and job/case records remained unchanged.
The printed native errors were expected rejection cases.

This exercises live geometry, analytical compliance, proposal construction and
acceptance for straight-sided C3D8 subdomains. It is not a physical continuum
solve or a normal controller run. The journal/library now extend the feasible,
shared-graph and preserved-neighbor checks to all six formulations in 18 fixture
groups; its first run stopped at the integration limit described below. Mandatory
restoration/backtracking fixtures remain C3D8-only. See
:doc:`numerical-verification-plan` for scope and instructions. Public runs remain
gated; controller, seeding, persistence, curved/mixed meshes and Linux execution
remain separate.

Expanded-matrix integration limit
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first expanded run, ``20261108T194700Z_d7726c19``, passed 11 groups and
failed C3D10_shared_preserved; six wedge groups were not executed. The retained
CSV locates the failure in initial live_geometry, before proposal construction.
The two C3D10 elements and their six shared-face nodes passed validation.
The free neighbor had a quadratic face-pinned field at tolerance 1e-7, width 1,
and a maximum of four refinements.

On this affine tetrahedron, with t the opposite vertex's barycentric coordinate,
the field is phi(t)=1-3*t+2*t^2 and the normalized slice measure is
3*(1-t)^2 dt. An independent scalar quadrature diagnostic gave successive
fraction/derivative differences of approximately 1.45e-5 at 8 subdivisions,
1.67e-7 at 16 and 2.73e-9 at 32. Thus the four-refinement budget narrowly misses
the declared 1e-7 criterion. Those scalar calculations are diagnostic evidence,
not a native pass at five refinements.

The live verification budget is now explicitly five refinements. Its tolerance
and all production defaults are unchanged. A new native geometry regression
requires unresolved integration with four refinements and cleared output, then
convergence at 32 subdivisions with five. It compares fractions and all nodal
derivatives against an independent one-dimensional Simpson reference. The new
regression and expanded-matrix rerun subsequently passed, as recorded below.

Expanded-matrix rerun evidence
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The user rebuilt the geometry executable and live library. The face-pinned
geometry regression passed on Windows (0.85 seconds test time, 0.88 seconds
CTest wall time). The retained geometry CSV contains 10093 passing rows, including
49 rows for the new refinement/reference regression. Production defaults and
the 1e-7 live-update tolerance were unchanged.

Run ``20261108T200007Z_7ae44599`` then passed all 18 live fixture groups:
2034 check rows, zero failures. The shipped
:download:`six-formulation live summary <verification/topology-live-solids-20261108.csv>`
records per-formulation/group counts and the explicit restoration/backtracking
coverage flags. All six formulations passed feasible updates, shared graph KKT,
shared-face/midside validation and preserved-neighbor checks. Mandatory restoration
and nonlinear backtracking remain C3D8-only. Loaded binary revision remains
unrecorded; this is Windows evidence, not Linux or curved/mixed-mesh certification.

The fifth standalone initialization suite subsequently passed on Windows
(0.20 seconds test time, 0.23 seconds CTest wall time). Live run
``20261108T202157Z_9bab925c`` passed all 18 fixtures, 36 phases and 6561 checks:
4527 initialization checks and 2034 update checks, with zero failures. Retained
summaries are :download:`standalone initialization <verification/topology-native-initialization-20261108.csv>`
and :download:`live initialization/update <verification/topology-live-initialization-20261108.csv>`.
The loaded binary revision was not recorded. Raw-default seed checks remain
distinct from controlled broad-band complete geometry (width 1, five refinements,
Core tolerance 1e-4). This does not certify default-width optimizer execution.
Controller and persistence verification remain pending.

Current integration evidence
----------------------------

Local run ``20261107T120555Z_ced1ad13`` of ``tests/verify_topology_all.jou``
reported PASS for configuration, the results regression, automatic conversion,
density compliance and Y/magnitude displacement for all six formulations,
preservation overrides, iteration-history persistence, rerun/delete, kill/reset
and projection. Three native reader/converter executables were BLOCKED because
they were unavailable. Manual GUI checks and native level-set mathematics were
not established by that run. Density-history persistence must not be relabeled
as level-set field/proposal persistence. Raw evidence remains local under
``tests/topology_runs/20261107T120555Z_ced1ad13/``.

Current configuration checks (batch 8a)
---------------------------------------

The focused configuration sources remain available. Configuration was exercised
by the integration run above; its broader coverage must be kept separate from
the older configuration-only entry and its stage manifest:

* :download:`Single verification entry journal <../../examples/calculix/topology/verification/verify_topology.jou>`
* :download:`Configuration journal <../../examples/calculix/topology/verification/settings_and_persistence.jou>`
* :download:`Python verification script <../../examples/calculix/topology/verification/settings_and_persistence.py>`
* Local GUI checklist and native-test instructions: ``tests/topology/README.md``.

Use a built plugin and wrapper exposing
``CalculiXPythonInterface::get_topology_settings(id)``. This read-only query returns
``[keyword, value]`` rows containing common, density and level-set settings. It
uses the existing topology interface; doubles use the classic locale and 17-digit
round-trip precision. Missing cases return an empty collection with a Cubit error.
The normal SWIG generation already depends on the interface header. Adding source
files does not establish that the loaded native wrapper has been updated.

Set Cubit's working directory to the repository root or the verification
directory and play **verify_topology.jou**. It invokes the existing configuration
script and records all verification stages in ``verification.csv``. Numerical
helper stages are explicitly BLOCKED because their production C++ functions are
not exposed by the current Python interface. That older entry does not execute
the newer solver/sensitivity matrices. A configuration pass cannot make the
overall result complete.
The native test specification is :doc:`numerical-verification-plan`; the
documentation-only constraint and review items are in :doc:`review-findings`.

The older ``settings_and_persistence.jou`` remains a focused configuration entry.
Record the loaded plugin
revision in its ``PLUGIN_REVISION`` field. **The journal resets the current
model.** It creates one unmeshed brick for native model saving, configures cases,
and checks defaults, optional create/modify parameters, isolated edits, automatic
zeros, method switches, invalid inputs, integer refinement limits, run rejection,
delete/recreate defaults and unrun-case save/reset/reopen. It never starts a
solver. Expected rejection commands intentionally print Cubit errors; their
outcomes are checked separately from unexpected failures. Cubit's SyntaxError
wrapper is handled for expected rejections. Playback pause-on-error is restored
after those commands, including failures.

Evidence goes into a unique directory below the working directory's
``topology_verification`` directory. ``checks.csv`` records actual pass/fail
outcomes, ``commands.csv`` records expected and actual command outcomes and error
counts, and ``settings.csv`` retains every queried comparison snapshot.
``metadata.csv`` records Cubit/Python/platform data, wrapper path and the
user-supplied plugin revision. ``summary.csv`` keeps automated configuration,
manual GUI, historical iteration persistence and numerical coverage separate.
The saved ``settings_and_persistence.cub5`` retains cases for follow-up GUI checks.
Unexpected failures stop the suite and retain the current model, previous CSV
evidence and ``failure.txt``. No failed-state reset or mesh rollback is performed.

A passing configuration journal does not verify derivatives, optimizer behavior,
GUI routing/drafts or persistence of completed/active iteration history. Follow
the separate GUI checklist, record outcomes and screenshots, and retain a model
with historical solver results for the remaining persistence/lifecycle checks.
Windows and Linux need separate native runs. Public level-set execution remains
blocked pending numerical verification.

Historical evidence
--------------------

The historical results below are carried forward from the previous tutorial
index. They were not reproduced against the current revision during this
documentation stage. The named raw CSV evidence was not located in the stated
Cubit working directory during the audit and is not bundled with the manual.
The percentages therefore remain a historical report, not independently
rechecked measurements or a current regression-test pass.

Historical displacement derivative matrix
-----------------------------------------

The previous documentation reported a run on 20 October 2026 with one solver
thread and 20 result-reader threads. It reported passing the criterion below
for all 36 sampled raw density sensitivities: six element types, two objectives
and three free elements per case. Preserved-element sensitivities were reported
as zero. The objectives were absolute Y displacement and displacement magnitude
at a single monitored node. The meshes used straight-sided elements.

The matrix used requested perturbations 0.01, 0.003 and 0.001. Its full default
suite comprised 12 cases, each with three physical/adjoint pairs followed by
18 perturbed physical solves: 288 solves in total. This is historical workload
information, not an instruction to execute those solves as part of a manual
build or documentation check.

.. list-table:: Historically reported best-perturbation relative errors (percent)
   :header-rows: 1

   * - Element
     - Absolute Y objective
     - Magnitude objective
   * - C3D4
     - 0.679080
     - 0.230021
   * - C3D6
     - 0.013755
     - 0.005285
   * - C3D8
     - 0.011261
     - 0.016488
   * - C3D10
     - 0.023172
     - 0.072523
   * - C3D15
     - 0.002176
     - 0.001340
   * - C3D20
     - 0.019265
     - 0.017486


For each sampled element the historical summary selected the perturbation with
the smallest relative error. Each table cell is the largest of those three
sampled elements' best relative errors, expressed as a percentage. It does not
mean every perturbation passed. Some smaller perturbations were reported to
exceed tolerance for weak derivatives, consistent with limited result precision;
without the detailed evidence this explanation has not been independently
rechecked. A positive C3D10 magnitude derivative was also reported to exercise
the signed-sensitivity path.

The reported evidence identifier was
``displacement_matrix_20261020_171632_915539_summary.csv``, with associated
per-case ``_finite_differences.csv`` files. The former working-directory
location is provenance information, not a downloadable artifact included in
this manual. Keep the solver executable version distinct from CalculiX 2.23,
whose manual/source was used to inspect integration and output conventions.
The version used for that historical solve is not established merely by citing
that reference version.

Historical journal availability
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The previous tutorial referenced ``check_displacement_sensitivities.jou`` and
``check_displacement_element_types.jou`` in ``examples/calculix/topology``. Both were
deleted in commit ``023e5604f``. The audit located copies in an older generated
manual and history in Git; this documentation batch does not restore them or
make the generated manual a source dependency.

The historical matrix journal edited legacy per-element material names such
as ``CCX_TOPO_1_E<element>``. Current exports can share one material between
several elements. Those journals must be reviewed and adapted before being
advertised as runnable tests for the current exporter. The beam workflow in
:doc:`../tutorials/topology-beam` remains separate from derivative verification.

Finite-difference procedure
---------------------------

The following is a verification protocol, not a replacement runnable journal.
For one free element, hold all other design variables, base material properties,
loads and constraints fixed. Read the stored raw C++ density sensitivity and
compare it with independent physical solves at two admissible perturbations:

.. math::

   g_{\mathrm{FD},e}=
   \frac{J(\rho_e^+)-J(\rho_e^-)}{\rho_e^+-\rho_e^-}.

With symmetric effective perturbations this reduces to the usual central
finite difference. The denominator uses the effective exported perturbations,
not blindly twice the requested step. In the continuous SIMP test, rounded
exported elastic moduli can be mapped back through the same SIMP relation to
record the effective densities. Do not treat an inverse-SIMP density derived
from a quantized factor as proof of differentiating the original grouped design.

#. Select a finished, evaluated baseline and free elements away from the density
   bounds. Retain its design, raw sensitivities and material assignments.
#. Identify the derivative being checked using the grouping distinctions below.
   Confirm that each perturbation changes only the intended element factor.
#. Construct positive and negative perturbations inside the supported density
   bounds. Record both requested and effective exported values.
#. Solve the two physical analyses with consistent settings. Require successful
   completion and the requested quantities at each parser's final result frame.
#. Recompute the same objective definition and inspect its controlling node and
   component sign. A change of active maximum or a zero crossing requires
   separate nonsmooth analysis; do not report it as an ordinary smooth-branch
   derivative test.
#. Compare the finite difference with the unfiltered C++ sensitivity. Retain
   every perturbation result, including unsuccessful or inaccurate comparisons.

The historical matrix used the existing Python interface for iteration/result
queries and Cubit's bundled ``h5py`` to read stored unfiltered sensitivities
from model snapshots. Filtering, volume projection and adaptive density updates
are not part of the objective derivative being compared. An adjoint is needed
to obtain the baseline displacement sensitivity, not for each finite-difference
physical solve.

Acceptance criteria and summary interpretation
----------------------------------------------

The historically reported criterion was

.. math::

   |g_{\mathrm{FD}}-g_{\mathrm{C++}}|
   \le 10^{-6}+0.01|g_{\mathrm{C++}}|.

The absolute tolerance has the units of the objective derivative; the relative
tolerance is dimensionless. The absolute constant is tied to the historical
model units and is not a universal tolerance for all models. The summary passed
a sampled element if at least one tested perturbation met the criterion.
Preserved-element sensitivities were checked separately for zero values.

Inspect all perturbations, not only the best-step summary. Larger steps can
introduce truncation error; small steps can amplify input/result rounding and
solver noise. A future report must state its complete acceptance rule,
perturbation sizes and handling of failed solves or branch changes. It must
not silently reuse this historical criterion for another model or unit system.

Material grouping and the derivative target
-------------------------------------------

.. list-table:: Distinct verification targets
   :header-rows: 1
   :widths: 30 70

   * - Target
     - Required interpretation
   * - Continuous SIMP density derivative
     - Requested and applied factors must coincide for the compared design;
       verify exact sharing rather than assuming the configured cap guarantees
       it. Account for text-export rounding separately.
   * - Independent applied-factor derivative
     - Perturb only the intended element factor. Editing a shared material
       definition would perturb every assigned element unless that element is
       isolated in the standalone verification input.
   * - Grouped density sensitivity
     - The implementation uses a continuous SIMP surrogate at stored applied
       factors. Finite differences through bin assignment, endpoint changes or
       regrouping do not check the same derivative.

Do not claim that the historical matrix validates differentiation through
material grouping. Merely increasing the material cap does not replace checking
which factors and assignments actually entered the compared input files.

Required evidence and provenance
--------------------------------

A future verification run should retain:

* Plugin revision, Cubit version and the actual solver executable version.
* Solver and result-reader thread counts, model units and solver settings.
* A model snapshot and topology settings, including interpolation, material cap
  and preservation rules/flags.
* Element type, sampled global element IDs, monitored nodeset/component and
  controlling node information for the compared objectives.
* Requested densities, stored applied factors and element-to-material assignments.
* Baseline physical/adjoint input decks and result files, plus every perturbed
  physical deck/result required to reconstruct the comparisons.
* Detailed CSV rows for every requested perturbation: requested/effective
  densities, objective values, C++ and finite-difference sensitivities, absolute
  and relative errors, completion state and comparison outcome.
* A summary identifying the acceptance rule, coverage and any failed, skipped
  or branch-changing cases. Its rows must refer back to the detailed evidence.

This is an evidence-retention checklist, not a claim that these artifacts are
currently supplied. Saving topology records in a model file alone does not
package all external solver inputs/results. See :doc:`topology-results`.

Conversion integration regression coverage
------------------------------------------

The current VTK routine adds a prerequisite between topology solves. Future
verification must record solver completion, reader success, topology evaluation,
VTK publication and optional file export separately. See :doc:`vtk-conversion`
for the reported DAT-only-field failure and :doc:`review-findings` for open items.
The following checks are required; they have not been executed in this pass:

* A supported solid job with DAT integration-point output and FRD mesh but no
  FRD field requests; exact formulations must come from a verified source.
* Matching/mismatching run INP and current-model geometry, including full versus
  reduced integration and topology material-group element sets.
* Conversion failure/cancellation must produce an identifiable blocked phase;
  a stored compliance value must not count as successful VTK publication.
* Successful in-memory retry, failed retry, optional export failure, and pending
  job deletion/reset; record whether a new run is needed after controller failure.
* Both physical-to-next-iteration and physical-to-adjoint transitions. Normal
  completion builds VTK in memory without exporting; explicit export must not
  change readiness for the next solve.

These are native integration tests, not fulfilled by settings/persistence
journals or by a Python reimplementation of the converter.

Limitations and coverage still needed
-------------------------------------

.. list-table:: Verification coverage status
   :header-rows: 1
   :widths: 35 65

   * - Feature
     - Evidence status
   * - C3D4, C3D6, C3D8, C3D10, C3D15 and C3D20; Y and magnitude
     - Fixed-group applied-factor checks passed in the November matrix at a
       single monitored node. Individual element derivatives remain separate.
   * - Preserved-element sensitivities
     - Exact zero raw factor/density entries checked in all 18 November cases;
       level-set fixed-node geometry and assembly still require native checks.
   * - Compliance raw stiffness-factor sensitivities
     - Fixed-group sums passed for all six formulations in the November matrix.
   * - Curved higher-order elements
     - Additional numerical evidence required; straight-sided checks do not
       establish this coverage.
   * - Switching maxima, ties and zero displacement
     - Additional targeted checks required; the smooth-branch matrix does not
       verify the chosen nonsmooth handling.
   * - Current grouped material exports and surrogate sensitivities
     - Fixed-group factor perturbations passed. Grouped density surrogates and
       regrouped finite differences were not verified by that comparison.
   * - Level-set interpolation and fraction/volume derivatives
     - Supplied native cases passed on Windows after the endpoint fix. Linux,
       curved/mixed and positive narrow-band refinement studies remain pending.
   * - Level-set nodal compliance/volume assembly
     - Shared production accumulation/normalization passed the Windows analytical
       suite. Live analytical evaluation passed for all six formulations;
       Linux execution remains pending.
   * - Level-set numerical QP solver
     - Extracted coordinate/multiplier solver passed analytical solution/KKT
       tests on Windows; live graph assembly is not covered by that suite.
   * - Level-set proposal/acceptance and controller
     - All 18 expanded live groups passed after the refinement regression;
       deterministic restoration/backtracking coverage remains C3D8-only.
       Controller and historical persistence remain separate; public runs gated.
   * - Level-set seed initialization
     - Standalone initialization and all 18 live seed fixtures passed on Windows.
       Default-width initial integration is separate.
   * - Filtered sensitivities
     - Not established as exact derivatives of a separate filtered objective.
       Objective finite differences do not verify the filter implementation.
   * - OC/projection updates, adaptive moves and convergence diagnostics
     - Require their own behavioral/numerical evidence; a derivative comparison
       does not validate their implementation or convergence speed.
   * - Optimizer convergence or global optimality
     - Not established by these derivative checks. Reaching an iteration limit
       is not convergence and passing convergence tests is not a global certificate.

The previous documentation also reported inconsistent raw outputs in earlier
multithreaded solver runs; their cause was not investigated. This chapter does
not diagnose that solver behavior. A reproduction of the reported historical
setup would use one solver thread and record the reader thread count separately.

Developer controller and persistence verification
------------------------------------------------

``CUBITSIM_VERIFY_LEVEL_SET_CONTROLLER`` is a default-OFF CMake option on
``calculix_core``. It admits only level-set compliance through the normal
``ccx run topology`` controller. Density behavior and other model validation
remain unchanged. Level-set displacement and mass remain unsupported. The
capability is queried from the loaded binary, never saved in a case and never
inferred from file contents. Ordinary builds reject level-set execution.
GUI run controls remain gated; this stage uses commands.

``tests/topology/verify_level_set_controller.jou`` is the local, Git-ignored
Python-mode entry point. Run it only after rebuilding/restarting the verification
plugin with the option enabled. It performs no build itself and checks the
capability and wrapper methods before any reset. Missing capability reports
BLOCKED and leaves the model untouched. Save current work before running it::

   playback "C:/Users/offic/Desktop/Git/Cubit-CalculiX/tests/topology/verify_level_set_controller.jou"

The journal runs two or three physical compliance iterations for each of the
six supported solid formulations at width 1, integration tolerance 1e-4 and
five refinements. It checks final DAT energy
against C = 2 sum(U), field/factor completeness, fixed coefficients +1, changed
physical designs, consecutive history, and no extra terminal job. Iteration-limit
termination is allowed but is never recorded as convergence. Selection uses the
existing Python selection interface because there is no separate selection CLI;
run, modify, save/open, rerun and deletion use ordinary commands.

It selects iteration 1, saves a native cub5, resets/reopens, compares settings,
selection, evaluations, nodal fields and element fractions/factors, then checks
rerun/deletion isolation from an unrelated case. It does not resume a run on
reopen or require solver files to be deleted. Evidence is retained below
``tests/controller_topology_runs/<token>/``. The first unexpected failure stops
later stages, retains the model/files, and restores playback pause and cwd.

``is_topology_level_set_run_enabled()`` reports the loaded Core capability.
``get_topology_iteration_level_set_field_data(id, iteration=-1)`` returns sorted
``[global node ID, phi, fixed flag]`` rows for a completed historical level-set
design. Omitted iteration resolves the selected iteration; invalid/missing
history reports a Cubit error and returns an empty vector. Coefficients must be
finite in [-1,1], fixed nodes must exist with phi=+1. The query uses the captured
historical method and stored field, without querying current mesh geometry.

Controller/persistence evidence
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Windows run ``20261109T085516Z_1ae3883b`` passed all five original stages:
fixture, physical controller, completed history, native persistence, and
rerun/deletion isolation. Three C3D8 physical iterations completed and terminated
at ``iteration_limit``, not convergence. Field/factor/preservation checks and
compliance against final DAT energy passed. Selection and stored history matched
after save/reset/open. The loaded plugin revision was not recorded. Empty FRD
result messages did not invalidate these DAT-based checks; this evidence does
not verify FRD displacement output or result conversion.

Windows run ``20261109T090529Z_00f2deae`` passed all ten C3D8 stages, including
the lifecycle extension: nonblocking start/separate wait, immediate kill and
restart, infeasible-target preparation failure and recovery, and reset while active.
Expected failed commands are logged and followed by explicit state/job checks.
Unrelated-case isolation is checked before the final reset, which intentionally
clears the whole model. The failure target is a valid positive fraction (0.01)
below this fixture's pinned-element bound; it tests preparation failure, not
solver-process failure. Immediate kill does not establish retention of already
completed iterations during a later kill.

Keep ``CUBITSIM_VERIFY_LEVEL_SET_CONTROLLER=ON`` in the active build cache while
running these and later physical level-set verification journals. It may be
turned OFF whenever returning to ordinary use; reconfigure/rebuild and restart
Cubit for that change to affect the loaded binary. OFF blocks level-set runs,
but does not invalidate saved evidence or enable production support. Removal of
the verification gate is a separate future code change after the agreed evidence
review, not a consequence of turning this option OFF.

Remaining stages include kill after completed iterations, solver-process failure
cleanup, default-width physical runs, six-formulation physical level-set coverage,
GUI lifecycle and Linux evidence. Passing these scoped tests does not establish
optimizer convergence or production readiness.

Six-formulation physical controller matrix
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Status: **PASS on Windows**. The same controller journal runs the five
physical-controller/history/persistence/rerun-deletion stages for C3D8, C3D20,
C3D4, C3D10, C3D6 and C3D15. C3D8 additionally runs its five lifecycle stages.
All cases retain width 1, integration tolerance 1e-4, five refinements, at most
three evaluated physical designs and material cap 8. No tolerance is relaxed
automatically after a failure. This is broad-band controller verification,
not default-width or convergence certification.

Expected nodal domains and preserved-node unions use expanded connectivity,
including midside nodes for quadratic formulations. Each iteration must cover
the complete element/node domain, preserve fixed coefficients and solid factors,
respect the material cap, and agree with compliance from final DAT energy.
The existing wedge fixture checks remain in use. Native persistence compares
settings, evaluations, selected history, nodal fields and element fractions/factors.

Suite report.json and commands.csv stay at the run root. Each formulation has
its own directory containing solver files, connectivity.csv, completed.json,
controller_history.cub5 and reopened.json. The extra C3D8 lifecycle files reside
in C3D8/lifecycle. Case and stage statuses identify the formulation; a failure
stops the matrix and retains the current model/evidence, with later cases marked
NOT EXECUTED. Missing expanded-connectivity support is BLOCKED before reset.
Keep the developer capability ON for this matrix. Production code and GUI gates
are unchanged by this journal extension.

The expanded Windows matrix subsequently passed in run
``20261109T091152Z_850deaec``: all six formulations and all 35 stages, including
C3D8 lifecycle checks. These are broad-band compliance/controller and native
persistence results, not convergence or default-width certification. Loaded
binary revision remains unrecorded.

The reference-quadrature regression run ``20261109T133442Z_73ec62b3`` also
passed all 35 stages. Fields, element fractions/factors and iteration histories
match baseline ``20261109T110825Z_0fb770d1`` exactly for all six formulations;
the only settings difference is the generated case name. All 31 input-export
commands returned successfully, with matching mesh-interface release markers.
The earlier C3D10 iteration-2 export exception did not recur; tracing has not
established its cause. See ``verification/topology-controller-quadrature-20261109.csv``
for the per-formulation evidence summary. This is not a convergence certificate.

Longer level-set convergence study
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``tests/topology/verify_level_set_convergence.jou`` is a separate, Git-ignored
C3D8 compliance study with at most 60 evaluated designs. It resets the model
and runs physical jobs; save existing work before playback. The existing
35-stage regression journal is unchanged. Run inside Cubit with the developer
controller capability enabled::

   playback "C:/Users/offic/Desktop/Git/Cubit-CalculiX/tests/topology/verify_level_set_convergence.jou"

The reviewed beam fixture retains target volume fraction 0.7, material cap 8,
interface width 1, integration tolerance 1e-4 and five refinements. Field and
compliance convergence tolerances come from Core defaults and are recorded from
the created case. The study does not relax tolerances or tune moves after failure.

Each completed iteration is checked for full node/element coverage, preservation,
valid fractions/factors and compliance equal to twice the final DAT internal
energy. Consecutive stored fields and objectives independently determine maximum
field change and relative compliance change. Their values are compared with
the history diagnostics. The ordinary proposal residual remains in command
output and is not independently verified by this journal. Compliance need not
decrease monotonically during volume restoration.

``tests/convergence_topology_runs/<token>/`` retains report.json, commands.csv,
iterations.csv, final_snapshot.json, convergence_history.cub5 and solver files.
The CSV reports volume excess above the target separately from feasibility at
the documented update tolerance of 1e-6. Execution PASS means the run and data
checks succeeded. An iteration-limit exit records convergence NOT_REACHED,
even when volume is feasible. Only a converged exit with consistent diagnostics
counts as REACHED; this is still not a global-optimality certificate.
Tracing is disabled temporarily to avoid diagnostic I/O in timing observations;
the inherited environment value and playback-error setting are restored afterward.
The initial Windows run ``20261109T140032Z_dd5a3f68`` completed all 60
iterations with execution PASS, volume feasible and convergence NOT_REACHED.
Volume first became feasible at iteration 6. Final volume fraction was
0.6990696864, compliance 6307.9029024, maximum field change 0.1 (limit 0.01)
and relative compliance change 0.0137706168 (limit 0.001).
All ten successive update-direction pairs in the final ten iterations reversed;
their mean cosine was -0.9980559798. This indicates persistent field oscillation,
not a convergence certificate. The fixed move limit was 0.1; material grouping
or other algorithm choices are not excluded as contributing causes.

The tracked ``verification/topology-level-set-convergence-20261109.csv``
archives all 60 baseline rows, with oscillation diagnostics calculated from the
stored nodal fields. For d_k = phi_k - phi_(k-1), successive_update_cosine is
the normalized dot product of d_k and d_(k-1). It is empty when either update
has zero norm or fewer than three fields exist. two_iteration_field_change is
max(abs(phi_k - phi_(k-2))). These diagnostics do not modify convergence tests.

The next journal requested level_set_move_limit 0.05 and compared with that
recorded 0.1 baseline. All other case settings except the generated name must
match before starting physical jobs; missing baseline files block before model
reset. Both runs use the same reviewed fixture construction. Settings comparison
does not independently certify identical mesh/model data or binary revisions.
Each run retains its unique directory. report.json compares first feasible
iteration, final compliance/volume/field changes, termination and final-ten
update reversal statistics. A smaller move limit is an experiment, not a promise
of convergence. Windows run ``20261110T072554Z_c4464b21`` passed execution
and the settings comparison but again reached iteration_limit at 60 designs.
Volume became feasible at iteration 11 and ended at 0.6997237353. Final
compliance was 6199.352122, field change 0.05 and relative compliance change
0.0098051904. All ten final direction pairs reversed, with mean cosine
-0.9999999999994. The smaller move reduced amplitude but did not remove the
two-cycle. ``verification/topology-level-set-convergence-20261110.csv`` retains
the complete fixed-0.05 history and direction diagnostics.

The subsequent journal compared controller reversal damping against that fixed-0.05
run, retaining configured move limit 0.05. Only the generated case name may
differ in case settings. Rebuild/restart the plugin before playback; the existing
capability query does not identify whether a loaded binary includes damping.
Retain console scale-reduction messages together with report.json. The comparison
records expected adaptive behavior, not an independent binary-capability claim.

CoreTopologyLevelSetMoves has a new standalone moves suite for reversal streaks,
fixed/zero updates, restoration, scale floor/reset, history validation and alias
errors. The live helper adds damped acceptance and invalid-scale checks, including
unchanged ordinary residual and source/proposal inputs. Rebuild the live helper
after the C++ signature change before running verify_level_set_update.jou.
The user-reported Windows CTest run passed all eight native suites, including
moves, in 16.96 s. Live run ``20261110T120336Z_49229417`` passed all 18 fixtures,
36 initialization/update phases and 6928 numerical checks with zero failures.
All 190 fixture commands returned and completed; the earlier reset interruption
did not recur. Expected rejection messages are checked by the helper and do not
indicate failed CSV checks. The per-phase summary is retained in
``verification/topology-live-damping-20261110.csv``. Loaded binary revisions
remain unrecorded; these Windows results do not establish Linux certification.

Adaptive physical run ``20261110T115248Z_bf1905a1`` completed all 60 iterations
with execution PASS and feasible final volume 0.6999939989, but convergence
NOT_REACHED. Accepted field change fell to 0.005 at adaptive scale 0.1 while
the ordinary proposal residual stayed 0.05. Final compliance was 6155.3758736
and relative compliance change 0.0064655949, above the 0.001 tolerance.
All ten final direction pairs still reversed (mean cosine -0.9533880367).
The unchanged full-proposal criterion prevented damping from falsely certifying
convergence. Its 60 iteration rows and factor diagnostics are archived in
``verification/topology-level-set-adaptive-convergence-20261110.csv``.

Material-cap isolation experiment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Stored iteration-60 data for element 113 show fraction 0.6322428367 ->
0.6369365830 but applied factor 0.5473286076 -> 0.7388361645. For the same
fraction change, the requested continuous factor changes by only 0.0046890525.
The existing grouping uses per-iteration logarithmic minimum/maximum bins and
nearest-bin rounding; moving boundaries and bin crossings can amplify changes.
This is evidence for investigating quantization, not proof that it is the sole
cause of nonconvergence.

The isolation journal compared cap 192 with adaptive cap-8 baseline
``20261110T115248Z_bf1905a1``. The reviewed beam has 192 elements and one base
material. A cap at least as large as the element count can retain every distinct
requested factor using the existing grouping path. The journal checks this
bound before running jobs, verifies the requested cap, and allows only name
and material_cap to differ in case settings. Move limit 0.05, adaptive policy,
convergence tolerances, width/refinement settings and iteration limit stay fixed.
Grouping mathematics and production defaults are unchanged.

New CSV columns report distinct material groups, maximum relative factor error
abs(applied/requested - 1), and maximum absolute requested/applied factor changes
between consecutive iterations. Requested factors use the existing level-set
formula void_ratio + (1-void_ratio)*fraction, with preserved factors exactly 1.
Stored high-cap applied factors must agree within relative tolerance 2e-12.
This does not test six-decimal input-file formatting error. The material export
message now explicitly calls its value quantization error rather than temporal
stiffness change. High-cap run ``20261110T121410Z_a1202968`` passed execution
and converged at iteration 34: field change 0.0049654985, ordinary proposal
residual 0.00538813, relative compliance change 2.3333961e-5 and volume fraction
0.6999486894. Stored quantization error was zero throughout, with 169 groups
in the final design. The run is archived in
``verification/topology-level-set-high-cap-convergence-20261110.csv``.
This establishes convergence only for the reviewed broad-band C3D8 fixture;
it does not authorize removing the production gate or comparing the cap-8
compliance value as if both runs used the same applied material approximation.

Stable capped grouping experiment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CoreMaterialsTopologyGrouping.hpp/.cpp adds fixed linear levels and hysteresis
for level-set runs. Density retains the previous logarithmic grouping path.
Budgets are deterministic from captured base-material element counts and cap,
not per-iteration distinct factors. Levels span the configured void ratio to 1.
A base whose budget covers all its elements remains exact; otherwise endpoint
factors are exact and previous assignments are retained within 0.1 level spacings
past a midpoint. Previous domains, caps and palette membership are validated.

The journal now compares stable cap-8 grouping against adaptive logarithmic
cap-8 run ``20261110T115248Z_bf1905a1``, retaining the converged cap-192 run as
an accuracy reference. Only the generated case name may differ in settings.
Build/restart the new plugin first: the journal checks fixed-palette membership
to detect a loaded old policy. It then saves/reopens the native model and requires
the complete snapshot, including applied factors, to remain identical. Historical
reconstruction uses stored factors directly and never reapplies hysteresis.

The new native material_grouping suite covers fixed levels under changing
extrema/distinct counts, upward/downward retention and switching, multi-level
jumps, exact endpoints, per-base budgets, exact high-cap/singleton paths,
invalid history and aliased outputs. These new tests and the physical stable-cap
comparison are **PENDING EXECUTION**. Eight native suites passed before this
change; that result does not validate the new ninth suite. Quantization error
can remain substantial, particularly near void, and all convergence criteria
stay unchanged. No GUI, default-cap or save-format change is introduced.

Subsequent Windows evidence
~~~~~~~~~~~~~~~~~~~~~~~~~~

The user-reported native run passed all nine suites, including material_grouping,
in 16.96 s. Stable cap-8 run ``20261110T123542Z_23eaf956`` passed execution and
native save/reopen, but ended at iteration 60 without convergence. The last four
applied-factor transitions were identical despite changes in the requested field;
zero compliance change alone was correctly insufficient because the ordinary
proposal residual stayed 0.05. Final quantization error was 26.355344 percent.

Stable cap-32 run ``20261110T124312Z_6c4e8390`` converged at iteration 41 and
passed native save/reopen. Final field change was 0.0053711503, ordinary proposal
residual 0.00556216, relative compliance change zero and volume fraction
0.6999964095. It used 23 material groups with maximum relative factor error
14.615751 percent. Its reported compliance 6305.0011384 is approximately 0.124
percent above the high-cap run, but the optimized fields differ and their applied
material approximations differ. That comparison does not isolate reanalysis
error. The complete cap-32 history is archived in
``verification/topology-level-set-stable-cap32-convergence-20261110.csv``.

Frozen-design exact-factor reanalysis
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``tests/topology/verify_level_set_reanalysis.jou`` prepares two ordinary
file-backed jobs for iteration 41 of ``20261110T124312Z_6c4e8390``. It resets
the model and opens that run's saved convergence_history.cub5; save current work
before playback. It never calls ccx run topology or advances the design::

   playback "C:/Users/offic/Desktop/Git/Cubit-CalculiX/tests/topology/verify_level_set_reanalysis.jou"

The grouped control copies the saved selected input unchanged and must reproduce
archived compliance within relative tolerance 1e-6. The second deck replaces
solid-section assignments with per-element elastic materials using the same
stored fractions and requested factor v + (1-v)*fraction, with preserved factors
exactly 1. Base modulus and Poisson ratio are recovered from an exported preserved
solid and checked against every grouped assignment. Moduli use six-decimal
scientific formatting. The term exact means no grouping; export rounding remains.

The fixture-specific parser rejects includes, non-C3D8 elements, multiple steps,
non-isotropic/temperature-dependent elasticity, unsupported section data and
nonhomogeneous displacement constraints. The original mesh, loads, constraints,
outputs and step are retained. Original sets/material cards remain present but
their old solid-section assignments are replaced. New input files are separately
reread for complete, nonoverlapping per-element section coverage.

``tests/reanalysis_topology_runs/<token>/`` retains factors.csv, report.json,
commands.csv and both jobs' input/results. The report compares exact-factor and
grouped compliance for the same frozen field and verifies that topology settings,
history, factors and fields remain unchanged after both normal jobs. It records
accuracy_acceptance NOT_ASSESSED: no user accuracy threshold or optimization
certificate is inferred. This journal needs no C++ change or new helper build.
The original standalone C3D8 journal passed in Windows run
``20261110T130950Z_484dc3c8``. The unchanged grouped control gave compliance
6305.0011384 and reproduced the archived value; exact-factor reanalysis of the
same frozen field gave 6297.077716. The signed difference is -0.0012566884963
relative to the grouped control (approximately -0.125669 percent). Topology
history remained unchanged. No accuracy threshold was applied. See
``verification/topology-reanalysis-20261110.csv`` for the retained summary.

Six-formulation convergence and reanalysis matrix
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``tests/topology/verify_level_set_matrix.jou`` runs C3D8, C3D20, C3D4, C3D10,
C3D6 and C3D15 sequentially. Each case resets the model, creates the reviewed
beam fixture, runs at cap 32 for at most 60 evaluated designs, validates full
history, saves/reopens its native model, and reanalyses its final evaluated
field with an unchanged grouped control and exact requested factors::

   playback "C:/Users/offic/Desktop/Git/Cubit-CalculiX/tests/topology/verify_level_set_matrix.jou"

Save existing work before playback. Keep the developer controller capability ON.
No additional C++ build or helper-library change is needed for this journal.
Move limit 0.05, current adaptive damping, Core convergence defaults, interface
width 1, integration tolerance 1e-4 and five refinements are retained. Existing
wedge-fixture validation includes expanded midside connectivity. The mesh is
generated per formulation; objectives across those different meshes are not
an independent accuracy or optimality certificate.

An iteration-limit exit records convergence NOT_REACHED while still permitting
final-design reanalysis and later formulations. Unexpected command, solver,
history or control-reproduction failures stop the matrix and retain the current
model and all earlier files; later cases remain NOT EXECUTED. Python interruptions
are recorded separately. Missing APIs or a disabled capability block before reset.
The report exposes the active command phase, with flushed command-enter/return
events for locating a native block that cannot execute Python exception cleanup.

``tests/matrix_topology_runs/<token>/`` contains report.json, summary.csv,
commands.csv and command-events.jsonl. Each formulation has its own directory
with report.json, iterations.csv, factors.csv, final_snapshot.json,
convergence_history.cub5 and all solver inputs/results. The summary separates
execution, convergence, history reopen and signed relative reanalysis difference
(exact/grouped - 1). maximum_factor_error is the maximum across the entire
optimization history, not just the final iteration. Accuracy acceptance remains
NOT_ASSESSED; no threshold is invented from the C3D8 result.

The shared test-only ``level_set_verification.py`` receives APIs/command callbacks
explicitly and performs no Cubit actions on import. Both original standalone
journals retain playback entry points and now delegate reusable validation to it.
The reader supports the six exported formulations and continued quadratic
connectivity, retaining original node order and all unaffected deck lines.
It remains a restricted fixture reader, not a general-purpose CalculiX importer.

Offline tests can be run without Cubit::

   python tests/topology/test_level_set_verification.py

Eleven Python test methods passed, covering all-six synthetic decks, malformed
connectivity/material/section cases, copied-job verification, paired reanalysis,
iteration-limit continuation, unexpected failure/interrupt stopping and environment
restoration, sequential DAT energy parsing, cached history validation, snapshot
mismatches and progress throttling. Additional offline checks passed on actual exported decks for all
six formulations (192 hex, 22600 tet and 2592 wedge elements per corresponding
fixture). Refactored C3D8 standalone reanalysis/history checks passed with mocked
APIs. These are harness checks, separate from the nine passing native suites.
Initial matrix run ``20261110T133641Z_91bf4dfc`` reached C3D4 convergence at
iteration 52 but encountered expensive synchronous history validation. Its
last recorded validation checkpoint was 28/52. This is partial evidence, not
a complete matrix PASS. The updated harness below remains **PENDING CUBIT
EXECUTION**; no production-gate change is implied.

Linear post-run validation
~~~~~~~~~~~~~~~~~~~~~~~~~~

The initial energy oracle called dat_get_element_values_for_component separately
for every element of every iteration. That C++ query scans the matching result
block on every call. Validating 52 iterations of 22600 elements therefore required
roughly 26 billion record examinations, while Python held the synchronous
playback path. The captured history JSON was approximately 140 MB, and repeated
full snapshots increased temporary memory. Solver convergence had already
completed; the expensive phase was the verification harness.

The matrix and standalone journals now use compliance_from_dat, a restricted
single-pass reader of the fixture's internal energy (element, energy) output.
It retains only the latest output time, requires final time 1 in the single
loading step, and validates exact element coverage, duplicates, finite/nonnegative
energies and malformed records. Multiple disjoint output sets at the latest
time are combined without overlap. Earlier increments are discarded. This is
an independent file oracle, not a replacement for the production DAT importer.
The legacy small-fixture API cross-check remains available but is not called
by journal energy validation or paired-job reanalysis.

History validation reuses the captured fields/factors and resolves all iteration
DAT paths once. Save/reopen and post-reanalysis comparisons query one iteration
at a time against that snapshot rather than constructing another full copy.
Snapshot JSON is written with json.dump instead of creating a full JSON string.
Stage labels distinguish capture, writing, validation and comparison. Progress
reporting is throttled to at most one update per 200 ms, with validation iteration
counts retained in report.json. These messages do not pump the GUI event loop
or make Python execution asynchronous. The original snapshot still occupies
memory; this change avoids additional full copies rather than promising a
constant-memory history store.

The new reader agreed with 18 archived DAT files across all six formulations
to maximum relative difference about 4e-15. The local offline check, including
loading the archived snapshots, took approximately 2.2 s; this is not a Cubit GUI
responsiveness benchmark. All eleven Python harness test methods passed without
loading Cubit, a native library or a solver. No C++ code or build changed.
The updated playback must be started in a fresh/restarted Cubit session to load
the new helper module. Existing output directories remain available; a new run
gets a new token. GUI responsiveness and complete matrix execution still need
to be verified. General per-element DAT query indexing is a separate future
optimization; its production implementation is unchanged here.

Targeted export diagnostic selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Run ``20261113T122046Z_82b6f636`` completed C3D8, then failed during C3D20
iteration-12 physical export/startup. The input reached END STEP but the export
command did not complete successfully; no solver start was reported for that
iteration. This does not establish which cleanup/return operation raised the
unexpected exception. Native tracing was disabled in that run.

The isolated C3D20 run 20261113T124243Z_2c79ac2d subsequently passed execution,
all 60 history checks, native reopen and both frozen-design reanalyses. It
reached the iteration limit: convergence remains NOT_REACHED. Grouped compliance
was 6338.3551344; exact-factor compliance was 6342.8470096, a +0.070868 percent
difference. Accuracy acceptance remains NOT_ASSESSED. Its native trace contains
60 matched physical export enter/success pairs and 60 matched enter/leave pairs
for each GUI restoration, material release and mesh release, with no exception
markers. This did not reproduce or fix the earlier intermittent exception.

.. csv-table:: Archived isolated C3D20 diagnostic
   :file: topology-c3d20-export-diagnostic-20261113.csv
   :header-rows: 1

Remaining-formulation batch and checkpoint recovery
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``tests/topology/verify_level_set_matrix.jou`` now selects C3D4, C3D10, C3D6 and
C3D15, with TRACE_NATIVE = True. These four cases keep cap 32, maximum 60
iterations, move limit 0.05, interface width 1, integration tolerance 0.0001 and
maximum refinements 5. Core convergence defaults are unchanged. Run in Cubit::

   playback "C:/Users/offic/Desktop/Git/Cubit-CalculiX/tests/topology/verify_level_set_matrix.jou"

Save current work first: each fresh case resets the model. The developer
controller option must remain ON. No rebuild is required for this journal
change. A fresh/restarted session ensures the updated Python helper is loaded.
``report.json`` records selected formulations, configuration, full_matrix false,
and the trace path. Only selected cases contribute to this attempt's execution
and all_converged fields. Separate prior_coverage records identify C3D8 from
20261113T122046Z_82b6f636 (PASS, REACHED) and C3D20 from
20261113T124243Z_2c79ac2d (PASS, NOT_REACHED). Missing source reports are explicitly
UNAVAILABLE; prior results are never presented as newly executed checks. The
earlier C3D8 case passed although its containing batch later failed on C3D20.

Immediately after successful optimization and a terminal converged/iteration_limit
state check, the journal saves convergence_history.cub5. It records checkpoint
completion only after the save command succeeds and a nonempty native file
exists. Snapshot capture, streaming JSON output, history validation and paired
reanalysis follow. This ordering retains native run state even if postprocessing
fails. A JSON snapshot alone cannot restore a Core topology run.

To recover an attempt produced by this journal, set its explicit configuration::

   RESUME_RUN = "C:/Users/offic/Desktop/Git/Cubit-CalculiX/tests/matrix_topology_runs/<source-run>"

Keep the same plugin build and fixture implementation. Selection and recorded
numerical configuration must match before any SDK call or model reset. There is
no build fingerprint or automatic compatibility conversion. Older reports without
the configuration/checkpoint fields are rejected; they remain useful evidence.
A changed method, default, fixture or plugin build requires RESUME_RUN = None.

Every playback gets a new output directory. Complete PASS cases are retained
with their source provenance and skipped. An incomplete case with a recorded,
nonempty native checkpoint and settings reloads that model and repeats only
postprocessing/reanalysis. Loaded settings must match the checkpoint settings,
and loaded state must be a completed optimization. A missing/empty checkpoint
causes a fresh optimization in the new attempt directory. A checkpoint that fails
to open or fails loaded-state/settings checks stops the batch for review rather
than silently substituting another model.

Recovery reads original physical inputs/DAT files from their original artifact
directory. New snapshots, validation CSVs and reports go into the new attempt;
paired jobs use the new attempt token in filenames. The pre-reanalysis checkpoint
restores the original job IDs, so paired-job IDs can repeat without overwriting
earlier solver files. Source reports and checkpoints remain untouched. Retain
the referenced source directories until recovery is complete. If recovery is
interrupted again, select the newest attempt as RESUME_RUN.

Empty, duplicate and unsupported formulation selections block before SDK access.
The trace environment is restored after success, failure or Python interruption,
both for inherited values and an initially unset variable. Hard native process
termination cannot execute cleanup; flushed trace/report files remain available.
Trace I/O can affect timings. Offline Python harness tests cover checkpoint
selection/recovery, configuration rejection, retained PASS cases, retry filenames,
source-report preservation, trace restoration and the validation/reanalysis path.
Actual native checkpoint recovery and the remaining four-formulation playback
are PENDING EXECUTION. No C++ code, compilation or Cubit execution was performed
for these harness changes. Full-sequence export reproduction may still be needed.

Parallel integration verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The subsequent Boost integration implementation adds LevelSetPipelineTests.cpp,
registered as topology_pipeline in the standalone native project. It compares
one/four workers and a zero-byte quadrature-cache budget for C3D4, C3D10, C3D6,
C3D15, C3D8 and C3D20. Cases include constant solid/void, preservation, a smooth
free field, geometry/derivative comparisons, and exact ordered compliance/nodal
assembly agreement. Additional cases cover source-cache hits/misses, changed
coefficients/settings/iteration, retained entries after rejected insertion,
invalid Jacobians/settings, cancellation, throwing progress callbacks, reuse
after failure, batch bounds and progress callback thread identity.

The native test imports the actual production pipeline; it does not start Cubit
or replace the numerical implementation. BOOST_ROOT defaults to the same
sibling boost_1_92_0 directory as the production source project and can be set
explicitly if that dependency is elsewhere. Rebuild the production plugin first
when authorized, then build the native helpers and run all ten native suites.
No new complete optimization campaign is required just to compare serial and
parallel element math. Existing native/live evidence predates this change.

**PENDING EXECUTION:** compilation, all ten native suites, Windows/Linux runtime
checks and a GUI progress/cancellation/performance smoke check. No build or
native execution was performed while implementing this change. Actual allocation
or scheduling exhaustion is handled in the implementation but is not injected
by this suite. Existing optimization evidence remains evidence for its original
serial implementation and settings, not automatic certification of this update.

The shared-reference-rule update retains this suite and adds borrowed-pointer
identity, stable pointers when a new type is prepared between batches, private
reader counters, zero duplicate rule payload, and subdivision-8 storage within
the former 32 MiB total budget for each formulation. Warm serial/four-worker
batches must retain equal memory/build counts and produce unchanged results.
Mixed-type batches are compared with uncached serial integration. The default
total budget is now 128 MiB, with a persisted positive-integer user option in MiB.
Only coarse levels are prepared initially; finer rules are prepared between
batches after workers actually request them. First-batch fallback counts are
therefore expected. Actual measured speedup is still pending.

After rebuilding, also verify the user-options page: default 128, command-based
save of another positive MiB value, reset-to-default edits, reopening the page,
and persistence across application restart. Verify rejection of zero, negative,
noninteger and address-space-overflowing values, and confirm the next geometry
and sensitivity passes log the selected budget and shared preparation timings.
These GUI/command/persistence checks have not been executed during this change.

Prepared physical mapping regression
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The next source change adds ElementPhysicalMapping.hpp/.cpp and reuses one
prepared mapping across an element's integration points and refinement levels.
LevelSetQuadratureTests.cpp now compares direct prepared evaluation, supplied
reference mapping and existing public wrappers for all six formulations. The
existing physical-map finite-difference Jacobian checks remain an independent
oracle, supplemented by the affine determinant 1.5 fixture. Variants cover
distorted/curved geometry and large translations, owned offsets after input
edits, successful re-preparation, invalid reference sizes/values/gradients,
alias preservation, failed-prepare invalidation, coordinate NaN/offset overflow,
unsupported types, inverted Jacobians and recovery. Existing geometry,
sensitivity, source-cache and parallel suites remain regression requirements.

The completed four-formulation matrix 20261113T143115Z_95e8e00e is the pre-mapping
baseline: C3D4 converged at iteration 52; C3D10, C3D6 and C3D15 reached iteration
60. All four passed execution, history/reopen and paired reanalysis, with
topology history unchanged. None of those results certifies this later source
change. Rebuild when authorized, run the complete native suite and compare
matching fixture/iteration timings before claiming a performance improvement.
No C++ build, compilation, native tests or Cubit playback were performed while
implementing the prepared mapping.

Cubit progress display
~~~~~~~~~~~~~~~~~~~~~~

``CoreTopologyProgress.hpp/.cpp`` owns one calling-thread Cubit ProgressTool
scope. Level-set geometry integration and compliance sensitivity evaluation
report completed elements in the current pass. Proposal/source checks and
backtracking use the same geometry pass automatically. Each pass starts at 0,
updates at most once per 200 ms, reserves 100 percent for successful validation,
and ends its progress scope on every return path. Initial/final display bypasses
the interval limit. No progress bar is required when the service is unavailable.

An optional callback reaches the composite integration subdivision-cell loop,
so a slow individual element can service progress/interrupt polling without
waiting for that element to finish. The percentage can remain unchanged during
such an element; it measures completed elements, not elapsed-time completion.
No extra event-processing loop or worker thread is introduced. Interruption
returns false through the existing calculation/controller failure path; it does
not introduce a separate killed-state policy. Geometry output stays cleared and
complete designs/evaluations are committed only after successful final checks.
Numerical rules and tolerances are unchanged.

Progress repainting, interruption and the regression after this source change
are **PENDING EXECUTION**. The passing matrix above predates the progress change.

Level-set timing diagnostics
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Geometry and compliance sensitivity passes print one two-line summary on scope
exit, including unsuccessful and interrupted passes. They report iteration,
completed/total elements, Cubit mesh-query time, integration time, remaining
wall time, the slowest attempted element, and maximum subdivisions among
successfully integrated elements. An incomplete-element ID identifies the
current element if its processing did not finish; -1 means no element was active
at exit. Status complete means the output was committed; incomplete covers
validation failure, interruption and exception unwinding, with the underlying
error message providing the cause. An empty pass has slowest ID -1 and depth 0.

Proposal summaries separate source-geometry reconstruction, graph/QP assembly,
QP solving and total wall time. Acceptance summaries separate source geometry,
accumulated candidate-geometry time, attempted trials, reductions used for the
last attempted trial, and accepted scale when successful. The first trial counts
as one trial and zero reductions. No accepted scale is reported on failure.

These are calling-thread steady-clock wall times in milliseconds, including
progress handling. Parent measurements include nested child measurements and
must not be added to them. Query time measures query_element; integration time
measures evaluate_element. Remaining work includes coefficient lookup, assembly,
validation and bookkeeping. Normal false-return calls contribute elapsed time;
if a call throws before returning, the outer total still includes that work but
its unfinished subphase counter may not. No timer is sampled per quadrature
point and no per-element message is emitted. Timing counters are local to the
invocation, are not persisted and do not change numerical controls or results.

Timing-output and numerical regression verification after this instrumentation
are **PENDING EXECUTION**. Keep the developer capability ON for comparison runs.

Proposal-owned source geometry reuse
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first performance batch adds CoreTopologyLevelSetGeometryCache.hpp/.cpp.
A fresh transient cache is populated by the source build_geometry pass in
build_proposal. The successful proposal owns it through a shared pointer;
accept_proposal may reuse it only for its source pass. Consuming/clearing the
controller proposal releases ownership, including terminal-state cleanup. No
cache is serialized or retained in historical designs/evaluations. Ordinary
cache-free calls remain valid.

Every pass still queries the current Cubit element type, ordered expanded node
IDs, coordinates and volume. Exact input equality is required for a hit, including
nodal coefficients, preservation, interface width, integration tolerance and
maximum refinements. All settings are validated even on hits. Other valid
settings do not change this geometry operation. A changed valid input causes
normal integration; invalid data still fails. Different positive source
iterations never reuse or overwrite the cache's entries. There is no fingerprint
and no assumed mesh immutability. The stored input vectors are comparison data
for this proposal, not an authoritative mesh model. Additional transient memory
is proportional to the participating source mesh.

Only successfully integrated element geometry is published. Failed/interrupted
misses leave prior entries intact and cannot publish partial geometry. Hits
service the same calling-thread, 200 ms throttled polling callback. Existing
complete-domain, fixed-node, volume, interface and source/proposal consistency
checks remain in place. Candidate/backtracking geometry and physical compliance
sensitivity evaluation remain uncached in this batch; therefore this change
removes one repeated source integration pass, not both repeated passes.

Geometry summaries report per-pass cache hits/misses and lookup/copy/validation
time separately from actual integration. Counter deltas are local to the pass,
while the cache itself counts over its proposal lifetime. Missing caches report
zero hits/misses. Integration depth on a hit describes the stored result's
quadrature depth, not newly performed quadrature. Timings include comparison and
copy overhead; parent and child times still must not be added together.

Local native target topology_geometry_cache_tests and CTest topology_geometry_cache
exercise all six formulations against uncached geometry, input changes,
iteration isolation, invalid requests and interrupt/retry behavior. Evidence is
written to build/topology-tests/evidence/geometry_cache.csv. Like the other local
tests, the sources remain Git-ignored and link the existing production binary.
No build or native execution was performed for this batch. These tests and the
35-stage controller journal are **PENDING EXECUTION** after the cache change.
Keep the developer capability ON; compare source acceptance time with the
pre-cache C3D10 timing evidence before considering further reuse.

Sensitivity-to-proposal cache handoff
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first cache batch passed all 35 controller stages in Windows run
``20261109T102928Z_452b4f2d``. Its saved fields, element fractions/factors and
iteration history matched the pre-cache run ``20261109T095215Z_b6449391`` exactly
for all six formulations. C3D10 source acceptance after iteration 2 fell from
27.913 seconds to 0.202 seconds, with 22600 hits, zero misses and no integration
in that source pass. Total acceptance fell from 70.839 to 41.840 seconds, including
ordinary run-to-run variation in candidate integration. This evidence predates
the next handoff extension; it does not record a native cache-suite result.

The second batch creates one fresh source cache in finish_topology_iteration,
after confirming the physical job and reading case settings. It is created only
for level-set compliance. A non-owning pointer is forwarded through the existing
run interface/coordinator bridge into numerical sensitivity evaluation. The
same shared cache is then moved through the proposal interface into build_proposal,
which transfers ownership to the successful transient proposal. No cache pointer
is stored in the historical evaluation. Early failures release local ownership;
existing proposal consumption/terminal cleanup releases ownership afterward.

The source is still queried and compared exactly at each pass. Sensitivity
assembly retains fraction consistency, complete nodal-domain and fixed-node
checks. Candidate geometry remains uncached. Standalone callers may omit the
optional cache; build_proposal then creates its own as before. No numerical
control, persistence layout, command or GUI changes are introduced.

For an unchanged 22600-element C3D10 source, expected per-pass counters are:
sensitivities 22600 misses, proposal source 22600 hits, acceptance source 22600
hits, and no cache for candidate integration. Sensitivity summaries now separate
lookup/copy/validation overhead from actual integration and other work, using
per-pass counter deltas. Runtime savings for proposal construction must still
be measured; passing counts alone does not verify numerical equivalence.

The live update verification helper now compares uncached, fresh-cache and
cache-hit sensitivity evaluations exactly, including every nodal derivative,
volume and factor-approximation flag. It compares proposals with independently
reintegrated source geometry and accepted designs/factors/diagnostics with a
cache-free source acceptance. Ownership transfer and source-only hit/miss counts
are checked. These comparisons supplement the existing independent analytical
and KKT checks; they are not another mathematical reference. Rebuild/reload the
live helper with the plugin before running verify_level_set_update.jou. Its
exported journal ABI is unchanged, but internal C++ signatures changed.

This handoff extension passed the physical 35-stage Windows controller run
``20261109T110825Z_0fb770d1``. Fields, element fractions/factors and iteration
history matched ``20261109T102928Z_452b4f2d`` exactly for all six formulations.
The subsequent live equivalence run ``20261109T121528Z_72c9d6ef`` passed all
18 fixtures and 36 phases: 4527 initialization checks plus 2254 update checks,
6781 total with zero failures. The compact tracked summary is
:download:`live cache-handoff evidence <verification/topology-live-cache-handoff-20261109.csv>`.
Loaded plugin revision remains unrecorded. This does not establish default-width,
curved/mixed-mesh or Linux behavior, or supply the separate standalone
``topology_geometry_cache`` result. Keep CUBITSIM_VERIFY_LEVEL_SET_CONTROLLER ON
for subsequent physical level-set verification.

Opt-in live initialization hang trace
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CoreTopologyTrace.hpp/.cpp writes timestamped UTC events to the path in
CUBITSIM_TOPOLOGY_TRACE_FILE. An unset/empty value disables tracing. Each event
opens/appends, flushes and closes its file; file-open/write failure does not
change calculation return values. Tracing is diagnostic I/O and can affect
timings; it is not enabled automatically by ordinary topology runs and adds no
CMake option. No worker thread or additional event-processing loop is introduced.

The live update journal defaults to TRACE_NATIVE = False. After successful
library/API preflight it temporarily removes CUBITSIM_TOPOLOGY_TRACE_FILE, even
if inherited, so normal verification does not include diagnostic trace I/O.
Set TRACE_NATIVE = True in the journal to enable OUTPUT/native-trace.log before
the first fixture. report.json records native_trace_enabled and a nullable
native_trace path. Its previous environment value is restored on normal return
or Python exception;
a native call that remains blocked cannot execute that finally block until it
returns. Restart Cubit after a forced termination before another verification run.

Progress-service markers bracket app_util, progress_tool, start, initial/periodic/
final percent, check_interrupt, interrupt-flag access and end. Periodic markers
remain behind the existing 200 ms throttle. Initial-design markers separate seed
initialization, geometry and factor construction without changing short-circuit
failure behavior or output commits. Live seed stages also emit flushed console
and trace markers for default seed, forced fallback, controlled seed and complete
initial design. Compare the final unmatched enter marker with the last CSV row;
a marker alone narrows the blocked call but does not establish its internal cause.

The shared local Checks utility has an optional flush_rows flag, default false.
Only the two live entry points enable it, flushing the CSV header and every
completed check row. Native standalone tests retain their existing buffering.
Rebuild the plugin and live helper, restart Cubit and rerun the same live journal.
The initial reproduction and successful CDLL rerun are recorded below.
No numerical rules or tolerances changed. Disabling automatic tracing is a
journal-only cleanup; it has not itself been run in Cubit.

The first captured hang trace in live run ``20261109T120621Z_de576a25`` ends at
progress.start.enter, without its matching leave. This locates the block in
Cubit ProgressTool::start before integration, but does not establish the
internal locking cause. The live journal now loads its C boundary with
ctypes.CDLL instead of PyDLL, releasing the GIL during the synchronous native
call while keeping the same calling thread. The boundary uses C++/Cubit APIs,
not direct Python C API calls. With the rebuilt live helper and CDLL loader,
run ``20261109T121528Z_72c9d6ef`` completed all fixtures; its trace reaches
progress.end.leave. The progress-start hang did not reproduce. This supports a
GIL/progress-service locking interaction, but does not reveal Cubit's internal
locking implementation. Trace-enabled timings include diagnostic file I/O and
are not a clean performance baseline. Native tracing remains available on demand;
the journal now disables it by default.

Reference quadrature cache verification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The performance batch adds the pass-local reference-rule cache and the
precomputed-reference physical mapping entry point. The user-reported Windows
native run passed all seven suites, including topology_quadrature, in 16.53 s.
Existing numerical accuracy limits are unchanged.
The new local target topology_quadrature_tests / CTest topology_quadrature writes
build/topology-tests/evidence/quadrature.csv. It links the production library and
requires rebuilding the plugin and native tests because helper signatures changed.

Coverage includes all six formulations: prepared/ordinary mapping, independent
finite-difference Jacobians using fixture-based node matching, partition of unity,
zero summed reference gradients, canonical reference volumes, reused rules,
element-specific determinants on perturbed corner/midside geometry, and
cached/ordinary/zero-budget integration comparisons. Inverted elements, malformed
reference gradients, direct output aliasing, unsupported rules, interrupted
construction/hits, partial budget exhaustion and oversized level-32 requests
exercise rejection and fallback behavior. No large table should be allocated
when its estimate already exceeds the remaining payload budget.

Pass summaries distinguish reference-rule hits, successful builds, budget
fallbacks and retained payload bytes from integrated-source cache hits/misses.
Source-cache hits do not use reference quadrature, so those passes may report
zero rule activity. Rule generation is included in integration time; the first
request pays its cost. Pass-local table destruction occurs after its inner timing
summary but is included in surrounding caller wall times. The budget limits
retained point/vector payload, not total process RSS or allocator/map overhead.

The seven native suites and the 35-stage physical controller regression have
passed on Windows. Physical run ``20261109T133442Z_73ec62b3`` retains exact
historical fields, factors and evaluations against ``20261109T110825Z_0fb770d1``.
The complete live update matrix, including damping acceptance checks, passed
in ``20261110T120336Z_49229417`` (36 phases / 6928 checks); its earlier baseline
is ``20261109T121528Z_72c9d6ef``. The quadrature controller regression enabled
diagnostic tracing, so it is not a clean timing
comparison. Keep tracing disabled for performance runs and the controller
verification capability ON. Do not relax tolerances after a failure; record
timings before claiming an improvement. Broader curved/mixed-mesh physical and
default-width verification remain separate from these small-fixture checks.
