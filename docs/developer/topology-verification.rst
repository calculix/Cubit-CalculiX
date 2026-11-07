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
* Both physical-to-next-iteration and physical-to-adjoint transitions, with
  automatic file export enabled and disabled. Disabled export still builds VTK.

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
     - Native test sources supplied; not built or executed. See
       :doc:`numerical-verification-plan` for coverage and remaining cases.
   * - Level-set nodal assembly, proposal/acceptance and controller
     - Numerical verification remains pending; public execution stays disabled.
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
