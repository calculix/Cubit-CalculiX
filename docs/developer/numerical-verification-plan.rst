Native topology verification specification
==========================================

Status and execution boundary
------------------------------

The November 2026 physical-solve matrix verified selected material-group
stiffness-factor derivatives for all six formulations and three objectives.
See :doc:`topology-verification` for evidence, tolerances and provenance limits.
Those results do not verify the level-set field-to-fraction chain or updates.

The current Python interface exposes historical raw sensitivities but not the
interpolation, level-set geometry, nodal-evaluation or proposal/acceptance helpers.
Native interpolation and geometry tests call the production functions directly
and have passing Windows evidence after the endpoint roundoff fix. The shared
production nodal accumulation/normalization also passed its analytical Windows
suite on 8 November. The extracted numerical QP suite subsequently passed too.
The Cubit-hosted proposal/acceptance library and journal passed the C3D8 batch.
Its six-formulation extension passed all 18 groups after the C3D10 refinement
regression and five-refinement live-test budget. Standalone initialization and
all 18 live initialization/update fixtures subsequently passed (36 phases,
6561 checks). See :doc:`topology-verification` for retained evidence. The developer
helper entry remains stateless; controller verification now has a separate,
default-OFF build capability, not a runtime bypass command.

Local entry points have different scopes:

* ``tests/verify_topology_all.jou``: integration and density lifecycle checks.
* ``tests/verify_topology_sensitivities.jou``: the 18-case physical-solve matrix.
* ``tests/topology/CMakeLists.txt``: native interpolation/geometry/nodal/QP/length checks against
  an already-built matching ``calculix_core`` module. It does not build the plugin.
* The older :download:`configuration entry <../../examples/calculix/topology/verification/verify_topology.jou>`
  remains a configuration manifest, not an invocation of the newer matrices.

The tests directory is Git-ignored and not distributed as manual downloads.
Native binaries require the matching Cubit runtime and plugin dependencies.
Missing binaries, unresolved symbols, SDK initialization failures or loader errors
are BLOCKED evidence, not mathematical PASS results. Windows and Linux require
separate runs. See :doc:`topology-level-set` for method definitions.

Supplied native test scope
-------------------------

``ReferenceElements.hpp`` specifies all six reference elements, independent
CalculiX and Cubit/Exodus edge orderings, and a translated asymmetric affine map
with determinant 1.5. It does not call the production permutation to construct
its oracle. These synthetic fixtures do not replace live Cubit ordering checks.

``InterpolationTests.cpp`` checks nodal identity, partition of unity, gradient
sums, reference/physical coordinate reproduction, affine determinants and
reference-gradient finite differences at steps 1e-3, 1e-4 and 1e-5. Algebraic
identities use absolute tolerances of 2e-12 to 5e-12; gradient comparisons use
2e-8 absolute plus 2e-8 relative. Unsupported types and invalid/singular/inverted
inputs must fail and clear outputs.

``LevelSetGeometryTests.cpp`` checks constant solid/void/zero fields, symmetric
regularized planar cuts, preserved elements, geometric volumes, constraint-volume
scaling and per-node fraction/volume derivatives. The smooth fixtures use
interface width 1, integration tolerance 1e-7 and at most four refinements to
separate derivative errors from narrow-interface resolution. All three steps
1e-3, 1e-4 and 1e-5 are retained and must pass: fraction derivatives use 2e-6
absolute plus 2e-4 relative; volume derivatives scale the absolute tolerance by
the supplied constraint volume. Affine volumes use 2e-10 absolute plus 2e-10
relative. These are declared regression tolerances, not observed accuracy.

The geometry tests record subdivision counts and integration settings. Invalid
requests must clear every output field. A narrow-band fixture with one allowed
refinement and tolerance 1e-13 must report unresolved integration. That rejection
does not establish narrow-band accuracy. Curved/mixed meshes, positive acceptance
refinement studies and full proposal/acceptance integration tests remain to be implemented.
No placeholder test is counted as coverage.

The face-pinned C3D10 regression captures the initial-geometry failure from live
run 20261108T194700Z_d7726c19. Its field is phi(t)=1-3*t+2*t^2, where t is the
opposite vertex's barycentric coordinate; face coefficients are one and the
opposite vertex/three incident edge midpoints zero. At width 1 and tolerance
1e-7 the current production rule must reject at four refinements with cleared
output and converge at five (32 subdivisions). This is a regression for the
current quadrature/error estimator, not a universal claim about all quadrature.

An independent 1D composite Simpson reference uses normalized measure
3*(1-t)^2 dt. Conditional quadratic shape averages are -t*(1-t)/3 for a face
vertex, t*(2*t-1) for the opposite vertex, (1-t)^2/3 for a face-edge midpoint,
and 4*t*(1-t)/3 for an opposite-to-face midpoint. Integrate H(phi) for the
fraction and H'(phi) times each average for the four nodal derivative classes.
Simpson references at 4096/8192 intervals must agree to 1e-11. Production
fraction agreement uses 1e-10 absolute and derivative agreement 2e-9 absolute;
solid-volume tolerances scale by the supplied volume. The native regression
passed on Windows. The other smooth geometry fixtures still use four
refinements; production defaults are not modified.

``LevelSetEvaluationTests.cpp`` supplies 30 analytical cases: all six formulations
with single-element void ratios 0.01, 0.2 and 0.6, plus a two-element shared-face
case and a preserved-neighbor case at ratio 0.2. Shared IDs are assigned from
independently mapped adjacent-element coordinates, including quadratic midsides.
Element constraint volumes and spring load weights differ between neighbors.
Fixed-node derivatives must remain exactly zero, including a fixed shared node
belonging to a non-preserved neighbor. Only free coefficients are perturbed.

Each free coefficient uses central steps 1e-3, 1e-4 and 1e-5. All comparisons
must pass. The declared derivative tolerances are 5e-4 relative plus an absolute
allowance of 2e-6 times max(abs(C),1) for compliance, 2e-6 times total constraint
volume for solid volume, and 2e-6 for volume fraction. Values themselves use
2e-12 absolute/relative checks. These tolerances are declared before execution;
the passing Windows evidence is recorded in :doc:`topology-verification`.

The analytical objective is sum(q_e/s_e) with positive fixed q_e; native
accumulation receives U_e=q_e/(2*s_e). Perturbed objectives use freshly evaluated
fractions and the scalar spring formula, not a copied implementation of the
nodal chain rule. Negative/missing/NaN inputs, mismatched IDs, fixed coefficients,
overflow and invalid finalization must reject and clear partial output.

Production ``evaluate_compliance`` still queries the live Cubit geometry,
validates stored fractions/domain/preservation, and sets factor_approximation.
It now calls the same ``accumulate_compliance`` and ``finalize_compliance``
methods as the native test. The extraction introduces no mesh snapshot or
model owner. The native test cannot certify those outer live-Cubit checks,
grouping semantics, or the complete solver/controller path.

The CTest entries write ``evidence/interpolation.csv``, ``evidence/geometry.csv``,
``evidence/evaluation.csv``, ``evidence/qp.csv`` and ``evidence/initialization.csv``
in their binary directory. Each row includes actual/expected values, explicit
tolerances and its pass flag. Archive each run before rerunning, together with
the tested plugin hash/revision, source revision, platform, compiler/configuration
and matching Cubit version. The fixed filenames are overwritten on another run.

Seed initialization checks
--------------------------

``LevelSetInitializationTests.cpp`` checks production resolve_settings without
querying a live model. Volumes 1 and 15 have mean 8 and h=2; automatic lengths
must be regularization 3, spacing 12 and radius 3. Explicit lengths are retained,
explicit spacing with automatic radius uses spacing/4, and multiplying all
volumes by eight doubles automatic lengths. Empty/zero/NaN volumes, invalid IDs,
overflow, invalid settings and lengths conflicting after automatic resolution
must reject with zeroed output settings and characteristic length. Tolerances
are 1e-12 for derived lengths and exact for unchanged explicit values.

The existing live library adds ``topology_verify_initialization`` with the same
developer-only C ABI as the update entry. It queries the selected mesh and calls
production initialize twice for raw Core defaults, comparing field and metadata
exactly. Default field generation is not a claim that default-width geometry
integration succeeds. The complete node domain, preserved-node union, +1 fixed
coefficients, resolved lengths, iteration 1/source -1, bounds and seed anchor
are checked. The anchor must be a free node nearest the full domain bounding-box
center; controlled symmetric unit hexes also require the smallest-ID exact tie.
General near-ties are checked by distance tolerance, not assumed bitwise ties.

The periodic-distance oracle enumerates 27 neighboring lattice centers using
long-double distances rather than copying std::remainder. It compares
clamp((distance-radius)/radius,-1,1), with preserved values overridden to +1.
Field comparisons use 2e-12 absolute tolerance; fixed values are exact.
Fallback selection is checked independently when no free value lies inside the
regularization band. A deliberately small radius and large spacing force one
free coefficient to zero; the anchor remains -1 unless it is the only free node.
The preserved C3D4 pair must exercise that single-free-node case. Other fields
retain their expected lattice values, and the fallback flag must be set.

A separate complete build_initial_design check uses automatic lengths, width 1,
five refinements and the Core's default integration tolerance (currently 1e-4).
The unchanged 1e-7 live-update and strict geometry tests remain separate. This
controlled broad-band case must produce available volume, a resolved free
interface, the same seed field, complete fractions/factors, and exact preserved
fractions/factors of one. Free factors follow v+(1-v)*alpha. It does not certify
default-width (currently 0.1) integration, controller execution or target feasibility.
An artificially saturated initial field must fail geometry resolution without
changing its input. All-preserved, missing-domain and invalid-void requests must
reject, clearing raw designs and complete-design/factor outputs.

Both native initialization suites passed on Windows. Complete-design checks
use broad-band settings and do not certify default-width integration. No Python
math replica or additional model owner is introduced.

Reference element fixtures
---------------------------

Use deterministic straight-sided reference elements before curved/mixed meshes.
Specify coordinates independently of the implementation being checked. Expected
CalculiX reference-node order is then mapped explicitly to Cubit/Exodus order;
do not use the production permutation to construct both the input and oracle.

.. list-table:: Base reference elements
   :header-rows: 1
   :widths: 20 55 25

   * - Family
     - Corner coordinates
     - Exact reference volume
   * - C3D4 / C3D10
     - (0,0,0), (1,0,0), (0,1,0), (0,0,1)
     - 1/6
   * - C3D6 / C3D15
     - (0,0,-1), (1,0,-1), (0,1,-1), (0,0,1), (1,0,1), (0,1,1)
     - 1
   * - C3D8 / C3D20
     - Standard lower/upper square faces at z=-1/+1 with x,y in {-1,+1}.
     - 8

Quadratic midside nodes are exact edge midpoints. Tetrahedral edge order is
12, 23, 31, 14, 24, 34. CalculiX wedge midsides are lower-triangle edges, upper-
triangle edges and then vertical edges. CalculiX hex midsides are lower-face,
upper-face and then vertical edges. The current Cubit permutations exchange
the upper/vertical midside groups for wedge15 and hex20. Check those positions
against an actual Cubit fixture as well as the independent geometric fixture.

Apply a nontrivial positive-determinant affine map and translation to each family.
Its exact volume is the reference volume times the map determinant. An asymmetric
map catches coordinate/component mistakes hidden by an axis-aligned unit cube.
Reject singular/inverted fixtures separately; a positive determinant at selected
quadrature points does not prove validity of an arbitrary curved element.

Interpolation checks
----------------------

For all six types, evaluate the production reference interpolation at interior
points and every reference node. Check:

.. math::

   \sum_a N_a=1,\qquad \sum_a \nabla_\xi N_a=0,\qquad
   N_a(\xi_b)=\delta_{ab}.

Compare reference gradients against central differences of shape values. Check
affine coordinate reproduction and the physical Jacobian through the production
mapped evaluator, including the Cubit/CalculiX local ordering. Use absolute
residuals near zero rather than a relative error with an arbitrarily tiny
denominator. Record the point, component, step size and both compared values.

Solid fractions and volume derivatives
---------------------------------------

Call ``CoreTopologyLevelSetGeometry::evaluate_element`` directly. Required cases:

* All coefficients +1 and -1: fractions 1 and 0, finite geometric volume and zero
  derivatives outside the regularization band.
* Uniform zero field: fraction 1/2. The sum of nodal fraction derivatives equals
  the regularized Heaviside derivative at zero for a valid partition of unity.
* Symmetric planar field: use x for hexes, z for wedges, and x-y for tetrahedra.
  The symmetric domain and ``H(t)+H(-t)=1`` give fraction 1/2 without claiming
  that a regularized fraction equals a sharp-cut volume in every asymmetric case.
* Preserved element: fraction 1 and feasible derivatives zero after geometry
  validation. Fixed shared nodes also need multi-element assembly checks.
* Cubit-volume scaling: change the supplied positive constraint volume while
  keeping geometry fixed. Fraction and geometric normalization volume stay
  unchanged; solid volume and its derivatives scale with the supplied volume.
* Mild nonuniform coefficients entirely within the smooth band: compare each
  nodal derivative with finite differences of the returned fraction/solid volume.
* Unsupported C3D8R, wrong connectivity size, NaN input, invalid volume and
  inverted geometry: false return with cleared output, not stale prior data.
* An intentionally unresolved narrow interface with a small refinement limit:
  verify the integration-limit failure path. Do not silently relax the tolerance
  until a test passes.

At least three finite-difference step sizes should distinguish truncation error,
roundoff and adaptive-quadrature changes. Begin with smooth broad-band fixtures;
use a separate refinement study for interface crossings and curved quadratic
geometry. Record the integration tolerance, subdivision count and requested
maximum refinements for every comparison.

For quantities A and B, use an explicit combined acceptance rule:

.. math::

   |A-B|\le\tau_{\mathrm{abs}}
       +\tau_{\mathrm{rel}}\max(|A|,|B|).

Set and justify tolerances before running. Do not infer a derivative tolerance
from a volume-only convergence result. Quadratic shape functions can be negative;
individual nodal volume derivatives need not all be positive.

Compliance chain and solver distinction
-----------------------------------------

For the supported fixed-load symmetric linear model,

.. math::

   C=2\sum_e U_e,\qquad
   \frac{\partial C}{\partial s_e}=-\frac{2U_e}{s_e},\qquad
   s_e=v+(1-v)\alpha_e.

An independent analytical unit-load spring per element has
``U_e=1/(2*s_e)`` and ``C=sum(1/s_e)``. Supplying those energies to native nodal
assembly and comparing its derivative against perturbations of the analytical
objective verifies the factor/fraction chain and fixed-node assembly. It does
not verify CalculiX result reading, a continuum FE stiffness matrix, solver
integration rules or the full optimization controller. Label that stage
``analytical compliance chain`` rather than ``solver validation``.

The supplied nodal test generalizes this oracle to unequal fixed q_e, with
C=sum(q_e/s_e) and U_e=q_e/(2*s_e), to detect overwritten shared contributions
and incorrect weighting. Preserved elements have fraction/factor one; all their
nodes are fixed to +1. The normalization uses the fixed total participating
constraint volume, so d(V_s/V)/d(phi)=d(V_s)/d(phi)/V.

The continuum comparison requires actual physical solves, matching applied
factors, outputs, mesh/order and independent perturbations. Grouped material
factors introduce surrogate derivatives; regrouped finite differences test a
different operation. Keep that evidence separate from the analytical chain.

Proposal, acceptance and preservation
---------------------------------------

Numerical QP subproblem
~~~~~~~~~~~~~~~~~~~~~~

QP means quadratic programming: minimizing a quadratic objective under
constraints. ``CoreTopologyLevelSetQP`` contains the existing coordinate-descent
and multiplier-search calculations formerly local to the update implementation.
It owns no mesh or Core store. The production update still queries Cubit and
assembles the graph, positive proximal masses, bounds and right-hand side.
Native tests exercise the same extracted numerical solver directly.

With x_0 the source field and c the signed volume-fraction derivatives, solve

.. math::

   \min_x\;\tfrac12 x^T A x-r^T x,\qquad l\le x\le u,\qquad c^T x\le b,

   b=c^T x_0+\max\left(V_{\mathrm{target}}-V_{\mathrm{source}},
     \min_{l\le x\le u}c^T(x-x_0)\right).

The matrix must be symmetric positive definite. Production assembly supplies
positive proximal masses plus a positive-weight graph penalty; the extracted
solver validates dimensions, finite data, symmetry, positive/consistent diagonal,
bounds and controls without performing a general SPD factorization. This is a
precondition, not an assertion that positive diagonal entries prove convexity.
The bound allows move-limited restoration; satisfying it does not necessarily
reach the target, and it is not the actual nonlinear volume constraint.

``LevelSetQPTests.cpp`` supplies seven analytical cases: coupled unconstrained,
active volume, signed volume derivatives, active box bounds, fixed coefficient,
move-limited restoration and reachable restoration. Each compares the known
solution and objective; cases with a unique analytical multiplier compare it too.
Tests independently compute primal feasibility, multiplier sign, complementarity
and the unscaled Karush-Kuhn-Tucker (KKT) gradient A*x-r+lambda*c. Interior
gradients must vanish, lower-bound gradients be nonnegative, and upper-bound
gradients nonpositive. Fixed coefficients are checked exactly; their gradient
has no such sign restriction. The production projected residual is checked in
addition, not used as the sole correctness oracle.

Declared tolerances are 2e-7 for solution/objective, 2e-6 for multiplier/KKT and
complementarity, and 2e-8 for linear feasibility. These are small-fixture
regression tolerances, not observed accuracy. Fifteen rejection cases cover
malformed inputs, missing restoration directions and coordinate/bracket/multiplier
iteration exhaustion. Ordinary failures must clear outputs. Direct vector aliasing
is rejected before clearing and must preserve the input. Tests do not promise
diagnosis of arbitrary indefinite matrices, whose use violates the SPD contract.

The QP suite passed its declared cases on Windows. It does not establish live
graph assembly, the complete build_proposal path, actual-volume acceptance,
backtracking, preserved mesh domains or controller behavior.

Live proposal/acceptance integration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The optional ``tests/topology/live/LevelSetUpdateVerification.cpp`` library
exports ``topology_verify_update`` through a C boundary. The caller supplies
only existing global element IDs, preserved-element IDs and a UTF-8 report path.
The library queries current block formulation, connectivity, coordinates and
Cubit volumes. It calls production build_geometry, build_factors,
evaluate_compliance, build_proposal and accept_proposal without creating a
stored case or job. Other Core stores are observed through their interfaces.
Expected input-rejection errors are recorded as checks; unexpected exceptions
are caught before returning across the C boundary.

The Python-mode journal ``tests/topology/verify_level_set_update.jou`` first
loads the configured library/symbol, then runs 18 fixture groups: C3D4, C3D6,
C3D8, C3D10, C3D15 and C3D20, each with one selected element, a free face-sharing
pair and a preserved-neighbor pair. Each group resets the model. Single-element
groups check a feasible full step, stale/tampered proposals and aliasing. Shared
free groups require a nonuniform source, an interior optimum and nonzero graph
contributions so bound clipping cannot hide an assembly error. Preserved pairs
check pinned shared nodes and rejection below the fully pinned lower bound.

Hex fixtures retain the one-/two-cell brick. Tet fixtures use tetmesh and wedge
fixtures use a triangular surface swept to its opposite face. For tet/wedge
meshes the selected one-/two-element analysis domain may be a subset of block 1;
the report records the complete model element count and selected IDs. Pair
selection matches canonical corner faces, not neighboring element numbers.
Both Python and C++ validate the entire expanded face, resolving midside IDs
from geometric edge midpoints independently of production permutations.
Linear triangle/quad faces share 3/4 nodes and quadratic faces 6/8. The native
entry rejects mixed formulations and queries each family's actual Cubit volume.

The mandatory deterministic restoration/backtracking cases remain C3D8-only;
quadratic shape functions can have signed nodal derivatives, and non-hex updates
need not be uniform. Reports explicitly record whether those cases are included.
The other formulations are not credited with coverage from an unexecuted case.

For the backtracking fixture, width is 1, the source field is uniformly -0.4,
and the move is 0.2. The target is alpha(source)+0.5*H'(source)*move. On this
negative branch the regularized fraction is convex, so the uniform tangent
proposal overshoots actual volume. The test must observe that overshoot and
at least one halving, not merely a successful call to accept_proposal. The
restoration fixture uses uniform +0.4, move 0.1 and target 0.2; it must reduce
actual volume while correctly reporting that the target is not yet feasible.
Preserved coefficients remain exactly +1 and their factors exactly one.
Live quadrature retains width 1 and tolerance 1e-7. Its budget is now explicitly
five refinements, following the diagnosed face-pinned C3D10 failure at four.
This changes the verification budget, not production defaults or the accuracy
criterion. It is not an automatic retry or tolerance relaxation.

An independent differentiation of the documented clique/proximal energy checks
the actual proposal: mass*(x-origin)+L*x+g/objective_scale+lambda*c. This does
not call the production operator builder or solve the same QP twice as an oracle.
Free interior gradients use 1e-6 absolute tolerance, with corresponding bound
gradient signs. Linear feasibility uses 2e-8 and complementarity uses
2e-6*max(1,lambda). Accepted-field identities and remeasured volumes use 2e-12;
fixed coefficients and the dyadic backtracking scale are checked exactly.

The test independently measures the full-proposal maximum change and the
accepted maximum change. After backtracking, ordinary_proposal_residual must
retain the full source-owned value. It also verifies source/proposal immutability,
next/source iteration IDs, factor interpolation and incoming diagnostics.
Wrong iteration, changed source volume, out-of-bounds field and inconsistent
residual must reject with cleared outputs. Aliasing rejects before clearing
and preserves source data. A lower-bound rejection clears the proposal record.

The journal uses standard-library ctypes.CDLL to release the GIL while calling
the library synchronously on the calling Cubit thread. Python reacquires the GIL
after return; the native boundary does not call the Python C API. This avoids
holding the GIL across Cubit GUI progress-service calls. It checks mesh IDs/connectivity/node
coordinates before and after. The library compares job and topology tree rows
through the existing interfaces, requiring no created/started job or new case.
Analytical energies q/(2*s) are supplied; no solver process is launched. This
is a live-mesh analytical update test, not continuum FE or normal-controller
verification. Seed generation, persistence and curved/mixed meshes remain
outside this batch. The original C3D8 cases passed; the six-formulation extension
first ran with 11 passing groups, one C3D10 preserved-neighbor integration-limit
failure and six unexecuted wedge groups. The five-refinement rerun then passed
all 18 groups (2034 checks). Subsequent seed/update execution passed 36 phases and 6561 checks.
Integration failures must be investigated rather than automatically relaxed.

Build/run procedure for later authorized verification
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Using the existing native-test CMake cache, enable the developer-only target:

.. code-block:: powershell

   cmake -S tests/topology -B build/topology-tests -DCUBITSIM_TEST_TOPOLOGY_LIVE=ON
   cmake --build build/topology-tests --config Release --target topology_update_verification

Set the matching CUBIT_ROOT and CUBITSIM_PLUGIN_DIR when configuring a fresh
cache. This imports the existing production module and SDK query libraries;
it does not build the plugin. No additional Python package is required.
On Linux use a Release single-config build or adjust CONFIG in the journal;
set Cubit's working directory inside the checkout or set ROOT explicitly.
The generated live-config-Release.json supplies the library, production-core
and Cubit paths. Windows ACIS binary directories are included during loading.
Restart Cubit after rebuilding a loaded DLL to avoid stale code; configured
file paths alone do not prove the loaded revision.

Save the current model and play ``tests/topology/verify_level_set_update.jou``
inside Cubit. The library is not a standalone executable/CTest test, is not
installed, and is disabled in CMake by default. Missing configuration/library
or load failures must be resolved before the first reset. Output goes into
a unique ``tests/live_topology_runs/<run>/`` directory: report.json,
commands.csv, and separate per-fixture check/mesh CSVs. A failing group stops
later groups and retains its current mesh. Expected native rejection messages
are distinguished by boolean/CSV checks, not the total Cubit error count.
The tests directory remains Git-ignored and is not a manual download dependency.
The seed-extended live target and journal passed 18 fixtures / 36 phases on Windows
in run ``20261108T202157Z_9bab925c``; see :doc:`topology-verification`.
Both exported entries are checked before reset. Each fixture now runs
initialization first, then update, with separate phase statuses and CSVs named
<formulation>_<fixture>_initialization.csv and <formulation>_<fixture>.csv.
Initialization failure leaves update and later groups NOT EXECUTED. Native
calls release the GIL while remaining synchronous on the calling thread; neither
entry creates a case/job nor changes the live mesh.

Use current Cubit elements with a complete node domain and at least one preserved
and one free region. Query current volumes/connectivity and call the existing
``build_proposal`` / ``accept_proposal`` helpers without registering a normal
topology job or bypassing its public gate. The supplied developer bridge retains
this stateless contract; controller execution remains a later stage.

Check bounds, fixed coefficients exactly +1, zero feasible derivatives on fixed
nodes, finite sparse coefficients, proposal stationarity, signed linearized
volume feasibility, source iteration identity and accepted actual volume.
Compare an unconstrained small convex problem or a symmetric one with a separate
analytical/KKT solution, not with a reimplementation of the same coordinate
descent routine.

Required scenarios include feasible starts, volume restoration, a target below
the fully pinned lower bound, active box bounds, negative nodal volume
derivatives, a backtracked step, a stale/mismatched proposal and failure-output
clearing. An initially infeasible accepted step may still be above target if it
meets the documented restoration reduction; that must not be mislabeled feasible.
The full ordinary residual belongs to the source; backtracking must not turn it
into the smaller accepted change used as an artificial convergence certificate.

Remaining gates
-----------------

Numerical helper checks do not complete controller, history persistence, mixed/
curved-mesh, large-model performance, GUI lifecycle or operating-system checks.
Retain raw per-case evidence and distinguish PASS, FAIL, BLOCKED and NOT EXECUTED.
Keep the public gate until the required native evidence is reviewed. The later
OpenPisco comparison is an additional reference, followed by a separate review
of whether its bundled source is still needed; source removal is not authorized
by completing this specification.

The default-OFF developer controller capability and passing Windows broad-band
controller/persistence evidence are specified in :doc:`topology-verification`.
This capability does not enable production or GUI level-set runs.

Current cache-handoff evidence
------------------------------

Physical run 20261109T110825Z_0fb770d1 passed all 35 Windows controller stages
and retained exact fields, factors and history against its preceding run for
all six formulations. Live run 20261109T121528Z_72c9d6ef passed 18 fixtures /
36 phases / 6781 checks (4527 initialization + 2254 update), zero failures,
including cached-versus-uncached evaluation/proposal/acceptance comparisons.
The loaded plugin revision was not recorded. See :doc:`topology-verification`
for the tracked per-phase CSV and remaining scope. The standalone cache test
has no separately supplied result and is not marked passed here.

The CDLL rerun completed progress start/end; the previous hang did not reproduce.
This supports the GIL/progress-service diagnosis without establishing SDK locking
internals. TRACE_NATIVE defaults to False in the live journal to avoid diagnostic
file I/O in normal verification; True restores hang tracing. The original process
environment is restored on normal return/Python failure. Default-width,
curved/mixed meshes, broader lifecycle, GUI and Linux verification remain separate.

Reference quadrature reuse regression
-------------------------------------

The pass-local rule cache must agree with ordinary integration for values,
fractions, volume-scaled derivatives and refinement choice at existing tolerances.
Its prepared mapping needs independent physical-map finite-difference Jacobian
checks, including perturbed midside nodes, so agreement between two delegating
entry points is not the sole reference. Reference weights must sum to canonical
volume and shapes/gradients satisfy partition identities for all six types.
Zero/exhausted budgets must choose uncached evaluation, preserving numerical
settings. Interrupted construction must leave no published rule or retained
payload; borrowed pointers remain stable until pass destruction.

The user-reported Windows run passed all seven native suites, including the
quadrature suite. Physical controller run ``20261109T133442Z_73ec62b3`` passed
all 35 stages with fields, element results and histories identical to the
pre-quadrature baseline. The full live update regression after quadrature reuse
is still pending; no clean timing comparison or successful longer-run convergence
is established. C3D8 run ``20261109T140032Z_dd5a3f68`` passed execution and
reached volume feasibility at iteration 6 but ended at iteration 60 without
convergence, with persistent field reversal at move limit 0.1. The journal now
prepares a fixed-move 0.05 comparison with all other case settings unchanged
except the generated name. Direction cosine and two-iteration field change are
diagnostics, not convergence criteria. That comparison remains unexecuted;
see :doc:`topology-verification`.
A 32 MiB point/vector payload budget does not
claim a hard bound on total process RSS. Reference data never justifies reusing
physical Jacobians across elements or assuming curved mappings are affine.
