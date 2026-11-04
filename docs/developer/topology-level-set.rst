Unstructured level-set foundation
=================================

Status and scope
----------------

This chapter specifies the C++ implementation stages of a parameterized level-set
method on the existing Cubit volume mesh. Internal controller integration exists,
but the public run validator continues to reject level-set runs. The GUI allows
case configuration through independent method-specific forms.
Compliance is the
first planned objective; displacement integration follows separate verification.

The foundation consists of separate settings and in-memory design records,
shared element interpolation, a seeded initial-design builder, element geometry
and nodal compliance/volume derivatives. The helpers calculate requested stiffness
factors but do not assign materials or modify a Core model store. They do not
create a job or change the Cubit mesh. A local update helper now prepares an
outgoing bounded field. Internal iteration-loop dispatch is wired, but public
execution remains disabled. Case settings and level-set iteration records are
serialized in one required unversioned topology layout. Result queries use the
captured method and expose level-set solid fractions through the existing row
contract. Optional command editing and method-specific settings forms are wired;
native verification remains pending.
Reset clears level-set designs and nested evaluations; transient proposals are
discarded on terminal states and controller removal. Do not enable production
level-set runs until the verification gates pass.

See :doc:`topology-roadmap` for implementation gates and the later OpenPisco
comparison/removal decision. No OpenPisco import, Python package, new binary
library or runtime installation is introduced.

Representation and units
------------------------

Global Cubit node IDs identify design coefficients. Global element IDs identify
fractions. Coordinates and expanded connectivity are queried from Cubit by the
future caller; only the current element arrays are passed to the evaluator.
There is no independently persisted copy of the Cubit mesh.

The nodal coefficients are dimensionless and bounded in [-1,1]. Positive values
represent solid, negative values void; the zero contour represents the boundary.
This parameterized field is not required to be a signed-distance function.
Hamilton--Jacobi evolution and distance reinitialization are not implemented or
implicitly assumed. The spatial regularization/seed lengths below have model
length units, but the field and its interface half-width do not.

The interpolation is

.. math::

   \phi_h(\boldsymbol{x})=\sum_a N_a(\boldsymbol{x})\phi_a.

Quadratic shape functions can be negative and the interpolated field can exceed
its nodal bounds. Therefore a nodal sign test alone cannot establish that an
entire higher-order element is solid or void. The integration evaluates the
field at quadrature points rather than classifying an element by nodal signs.

Regularized fraction and derivatives
------------------------------------

The compact C1 Heaviside approximation is

.. math::

   H_\varepsilon(t)=
   \begin{cases}
     0, & t\le-\varepsilon,\\
     \frac12\left(1+\frac{t}{\varepsilon}
        +\frac{\sin(\pi t/\varepsilon)}{\pi}\right),
        & |t|<\varepsilon,\\
     1, & t\ge\varepsilon.
   \end{cases}

Within the transition band its derivative is

.. math::

   H'_\varepsilon(t)=\frac{1+\cos(\pi t/\varepsilon)}{2\varepsilon};

outside and at both endpoints it is zero. For fixed geometry and a fixed
quadrature rule, define

.. math::

   W_e &= \sum_q w_q,\\
   \alpha_e &= \frac{\sum_q w_q H_\varepsilon(\phi_h(\boldsymbol{x}_q))}{W_e},\\
   \frac{\partial\alpha_e}{\partial\phi_a}
     &=\frac{\sum_q w_q H'_\varepsilon(\phi_h(\boldsymbol{x}_q))
                     N_a(\boldsymbol{x}_q)}{W_e}.

Each physical weight includes the reference weight and the positive geometric
Jacobian determinant exactly once. Fractions lie in [0,1]; higher-order nodal
fraction derivatives need not all be positive. These are derivatives of the
regularized, discretely integrated fraction, not of an exact clipped polyhedron.
Changes of quadrature resolution are not differentiated. Finite-difference
checks must account for resolution switching and require independently refined
reference calculations.

Cubit volume remains authoritative for the constraint:

.. math::

   V_{\mathrm{solid},e} &= V_e^{\mathrm{Cubit}}\alpha_e,\\
   \frac{\partial V_{\mathrm{solid},e}}{\partial\phi_a}
     &=V_e^{\mathrm{Cubit}}\frac{\partial\alpha_e}{\partial\phi_a}.

The geometric normalization volume W_e and Cubit's element volume are distinct
quantities. The former is returned for diagnostics, not substituted for the
latter. This matches the separation used by the density volume constraint and
adjoint integration in :doc:`topology-algorithms`.

Preservation
------------

The element evaluator accepts an explicit preserved flag. Preserved elements
receive fraction one, Cubit solid volume equal to their full Cubit volume, and
zero fraction/volume derivatives. Geometry is still validated. Initialization
pins the union of all expanded-connectivity nodes of preserved
elements at +1, including midside nodes. Nodal derivatives for these fixed
variables are zero. The later field update must retain this same fixed-node set.
Pinning can also force neighboring nominally free elements to remain solid.
The minimum_design_volume_fraction diagnostic includes the full Cubit volume of
all elements whose nodes are fixed; this is a conservative lower bound. Partially
pinned elements can impose further restrictions not quantified by that bound.

Element interpolation and node ordering
---------------------------------------

ElementInterpolation evaluates C3D4, C3D6, C3D8, C3D10, C3D15 and C3D20.
C3D8R and incompatible Cubit node counts/families are rejected.
Reference values and gradients use CalculiX node order. Physical evaluation
accepts expanded Cubit connectivity order and returns shape values in that same
Cubit order, using ElementNodeOrdering for the permutation. The nodal field and
coordinate arrays must refer to the same nodes and ordering.

Tetrahedral reference coordinates are the last three barycentric coordinates.
Wedges use the last two triangle barycentric coordinates and z in [-1,1].
Hexahedral reference coordinates all lie in [-1,1]. The Jacobian is evaluated
with translated coordinates to reduce cancellation; a non-finite or nonpositive
determinant fails the operation. Positive sampled determinants are not a proof
of validity throughout a curved element.

Geometry quadrature
-------------------

Geometry uses composite three-point Gauss rules on a unit parameter cube. Tet
and wedge rules use positive Duffy mappings:

.. math::

   \text{tet: }(u,v,w)&\mapsto(u,(1-u)v,(1-u)(1-v)w),\\
   J_{\mathrm{Duffy}}&=(1-u)^2(1-v),\\
   \text{wedge: }(u,v,w)&\mapsto(u,(1-u)v,2w-1),\\
   J_{\mathrm{Duffy}}&=2(1-u),\\
   \text{hex: }(u,v,w)&\mapsto(2u-1,2v-1,2w-1),\\
   J_{\mathrm{map}}&=8.

These mappings generate integration points; they do not split or remesh the
Cubit element. One, two, four, and subsequent doubled subdivision counts are
compared. Convergence requires the maximum of relative normalization-volume
change, absolute fraction change, and interface-width-scaled absolute nodal
derivative changes to be within integration_tolerance.

This is a successive-refinement error estimate, not a certified bound. An
unresolved narrow feature can be missed by successive sampled rules. Use
independent refined/analytic geometries in verification; do not interpret a
reported convergence as proof of feature resolution. Smaller interface widths
can need more refinement. Failure to meet the estimate before the configured
limit returns false with a Cubit error and cleared outputs. The uniform
refinement cost grows cubically; this foundation must be profiled before full
optimization runs. It does not yet cache reusable quadrature operators.

ElementIntegration's CalculiX solver rules and point ordering are unchanged.
They now call the shared interpolation code for Jacobians. In particular, the
single C3D4 solver point is not reused as the geometry fraction rule.

Core defaults
-------------

CoreTopologyLevelSet::default_settings() is authoritative. Struct value
initializers do not define valid preference defaults. Existing case creation
and loading both initialize the new settings through Core defaults.

.. list-table::
   :header-rows: 1
   :widths: 30 15 55

   * - Field
     - Default
     - Meaning
   * - interface_width
     - 0.1
     - Dimensionless Heaviside half-width, in (0,1].
   * - regularization_length
     - 0
     - Resolves to 1.5h, where h=cbrt(mean participating Cubit volume).
   * - move_limit
     - 0.1
     - Maximum proposed nodal coefficient change, in (0,2].
   * - field_tolerance
     - 0.01
     - Proposed field/proposal convergence tolerance, in (0,2].
   * - seed_spacing / seed_radius
     - 0 / 0
     - Spacing resolves to 6h; radius resolves to one quarter of the resolved spacing.
   * - integration_tolerance
     - 0.0001
     - Dimensionless successive-rule estimate threshold, in (0,1).
   * - integration_max_refinements
     - 4
     - Up to 16 subdivisions per unit-cube axis; permitted range is 1 through 5.

All fields must be finite where applicable. Explicit positive seed radius must
be less than half an explicit positive seed spacing. The initializer resolves
automatic values from the supplied Cubit volumes and
validates the resulting positive lengths.
Defaults are provisional numerical choices, not convergence guarantees.

Planned stiffness and optimization contract
-------------------------------------------

The prepared design returns requested factors for the existing element
stiffness-factor export, without assigning them to the material core:

.. math::

   s_e=v+(1-v)\alpha_e,\qquad
   \frac{\partial C}{\partial\phi_a}
     =\sum_e\frac{\partial C}{\partial s_e}(1-v)
                         \frac{\partial\alpha_e}{\partial\phi_a}.

This is a proposed linear fraction interpolation, separate from SIMP density
penalization. The existing fixed-load linear-elastic compliance derivative
supplies dC/ds. Material grouping and six-decimal input rounding retain their
existing approximation limitations; they are not differentiated here. The helper
computes the chain rule; the internal bridge assigns factors through the
existing topology/material interfaces.

A fully saturated constant field has zero Heaviside derivative. The seeded
initializer described below avoids relying on an all-solid constant field.
An all-solid initialization followed blindly by the density update is not a
valid level-set algorithm. The local update implements regularization, volume
handling, protected-node bounds and ordinary proposal diagnostics. Native
native verification and public enablement are still required before
enabling method level_set. No objective-monotonicity or global-optimality
claim is made.

Seed initialization and preparation
-----------------------------------

CoreTopologyLevelSet::build_initial_design() takes requested settings, the void
ratio, complete element preservation flags and Cubit element volumes. It queries
the current Cubit domain, returns an initial iteration-1 record and requested
factor map only after every stage succeeds, and leaves both outputs empty on
failure. The helper itself does not register an iteration or write a material.
The internal controller bridge stages the record and assigns factors through
the existing topology factor interface.

A temporary map of participating nodal coordinates supports field initialization;
block/connectivity/material metadata are not retained as another mesh model.
The characteristic mesh length is cbrt(mean participating element volume).
Explicit lengths remain unchanged; unresolved zero lengths use the defaults in
the table. A radius conflicting with the resolved spacing is rejected.

The seed lattice is anchored at the free node nearest the complete domain's
bounding-box center, with lower global node ID breaking a tie. For position x
and origin o, each Cartesian component of the nearest periodic offset is
remainder(x_i-o_i, spacing). With radial distance d and seed radius r, the field is

.. math::

   \phi_a=\operatorname{clamp}((d_a-r)/r,-1,1).

The seed origin is a real free node; initialization does not allocate a structured
mesh or enumerate every sphere. Spheres may intersect domain boundaries or lie
outside irregular domain portions. Fixed nodes override the seeded value to +1.
There must be at least one free node. If all free nodal values are outside the
Heaviside band, the non-anchor free node closest to the band is placed at zero;
for a single free node that node is placed at zero. The fallback is recorded in
transition_node_inserted and does not move the Cubit mesh.

The initializer alone does not prove that quadrature resolves a free interface.
The design builder checks for a nonzero integrated fraction derivative on at
least one free node; otherwise preparation fails with an adjustment/refinement
message. This remains subject to the quadrature limitations above and is not a
connectivity, minimum-feature-size or initial volume-feasibility guarantee.
The initial seeded volume need not satisfy the target; enforcing the volume
constraint is handled by the separate outgoing-update helper. Seed construction and length factors are
provisional defaults requiring mesh/refinement validation.

Compliance and volume derivative assembly
------------------------------------------

CoreTopologyLevelSetEvaluation::evaluate_compliance() takes the prepared field,
complete volume/flag maps, physical whole-element energies and actual applied
stiffness factors. The caller must verify physical job completion and the fixed
mesh assumption. The helper re-queries element connectivity/coordinates and
re-evaluates the same geometry rule; it rejects missing domain nodes, incorrect
fixed-node unions and stored fractions that disagree beyond integration tolerance.
This is a consistency check, not a mesh fingerprint.

It computes C=2 sum(U_e) and dC/ds_e=-2 U_e/s_e under the existing homogeneous,
fixed-load linear-elastic assumptions. Shared-node contributions are summed,
not averaged. There is no extra one-half factor. Derivatives of solid volume and
volume fraction are accumulated with supplied Cubit volumes and fixed total
participating volume. Fixed-node entries remain explicitly zero in all three
nodal derivative maps. The volume diagnostic's density_sensitivities map remains
empty; nodal derivatives belong to TopologyLevelSetEvaluation.

The factor_approximation flag reports when supplied applied factors differ from
requested continuous factors by more than 1e-12. It cannot detect additional
text rounding in the exported material cards. Even when the flag is false,
rounding limits can affect a finite-difference comparison. A grouped solve uses
surrogate continuous derivatives; grouping/rounding decisions are not differentiated.
No derivative record is committed to a topology case by this helper.

Regularized update and volume control
--------------------------------------

CoreTopologyLevelSetUpdate::build_update() consumes a solved source record and its
nodal evaluation. Evaluation iteration_id must match the source. The helper
re-evaluates source geometry and checks measured volume against the evaluation.
It prepares a new field, fractions, requested factors and diagnostics only after
success. It does not register jobs/iterations or modify materials, model stores
or selections. The mesh must remain unchanged throughout the calculation.

Graph and local quadratic model
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The regularizer is an element-clique graph, not the FE stiffness/Laplacian of a
published level-set solver. For each element, every pair of expanded-connectivity
nodes contributes a nonnegative edge weight. With P_e=n_e(n_e-1)/2, define

.. math::

   m_a &= \sum_{e\ni a}\frac{V_e}{n_e V_{\mathrm{total}}},\\
   w_{ab} &= \sum_{e\supset\{a,b\}}
       \frac{V_e}{V_{\mathrm{total}}P_e}
       \left(\frac{\ell}{\|\boldsymbol{x}_a-\boldsymbol{x}_b\|}\right)^2,\\
   M&=\operatorname{diag}(m_a),\\
   R_{aa}&=\sum_{b\ne a}w_{ab},\qquad R_{ab}=-w_{ab}.

Cubit volumes define the weights. Distinct graph nodes must have positive finite
separation; degenerate distances fail. All six supported volume elements include
their midside nodes. Assembly scales with the sum of per-element node pairs,
not all pairs of nodes in the global model. The field assumes compatible
shared-node interpolation; graph assembly does not
create hanging-node or mixed-order continuity constraints. Sparse operators are
transient and use the bundled Eigen headers; no extra user installation is introduced.

The reference compliance C_ref is the source's stored objective_scale when
positive, otherwise the first physical compliance (or one for zero compliance).
It is captured in the next design and retained thereafter, so regularization is
not renormalized against each new physical result. Define g=(dC/dphi)/C_ref.
For an outgoing field y, solve

.. math::

   \min_y\quad \frac12(y-\phi)^T M(y-\phi)
           +g^T(y-\phi)+\frac12 y^T R y.

M has positive diagonal entries and R is a positive-semidefinite graph operator,
so the local model is strictly convex in exact arithmetic. The implementation
rejects non-finite weights/operators and scaling that loses the positive proximal
mass when added to the regularization diagonal. This remains a numerical local
model, not a proof about the original nonconvex physical objective.

Bounds and the linearized volume constraint
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Free nodes obey both [-1,1] and the configured maximum move from phi. Fixed
nodes have lower=upper=1. Let a be the source volume-fraction derivative, f_k the
current measured volume fraction, and f_target the requested target. The QP uses

.. math::

   a^T(y-\phi)\le\max(f_{\mathrm{target}}-f_k,d_{\min}),\\
   d_{\min}=\min_{l\le z\le u}a^T(z-\phi).

The reachable linear bound permits several restoration iterations when the target
cannot be reached in one move. A target below the conservative fully pinned
volume bound is rejected. An infeasible source without a resolved free linear
volume-reduction direction is also rejected; this is a failure of the local
model, not proof of global infeasibility.

For multiplier lambda>=0 the bound-constrained quadratic has matrix A=M+R and
right-hand side b=M phi-g-lambda a. Cyclic coordinate minimization projects each
coordinate to its bounds. Its stopping residual is the maximum coordinatewise
projected-gradient displacement, scaled by the corresponding diagonal of A.
For an active constraint, multiplier bracketing/bisection uses the monotone
response of this strictly convex quadratic problem. Signed entries in a are
allowed; no monotonicity of the actual higher-order Heaviside volume is assumed.
Iteration limits fail with a Cubit error rather than accepting an unresolved QP.
The convex quadratic/duality background is described in
`Boyd and Vandenberghe, Convex Optimization <https://web.stanford.edu/~boyd/cvxbook/>`_.
This graph/update formulation is our implementation choice, not an OpenPisco replica.

Actual volume acceptance
~~~~~~~~~~~~~~~~~~~~~~~~

The ordinary full-move proposal is evaluated along halved step lengths, with
bounds/fixed values reapplied against floating-point drift. Each trial calls the
geometry evaluator and recomputes Cubit-volume-weighted solid fractions. There
are no additional physical solves in this acceptance check.

A source already feasible within volume_tolerance must remain feasible within
that tolerance. An infeasible source may either reach feasibility or reduce its
measured volume by at least the smaller of 0.1 times the predicted reduction and
half its current excess. It must also make a resolved decrease of at least the
smaller of 0.1 volume_tolerance and half that excess. Otherwise the step is halved.
An invalid/unresolved geometry calculation is a hard error rather than an
accepted approximate trial. Exhausting the halving budget fails and clears
outputs. The original source record is never changed.

This controls measured volume; it does not line-search the physical objective.
Physical compliance need not decrease, especially during initial volume removal.
Graph regularization is part of the local surrogate and can trade compliance
against field smoothness. It does not guarantee minimum feature size, connected
material, a sharp interface or a global optimum. Compact-Heaviside saturation
can also produce zero derivatives: the controller must distinguish a missing
free interface from a meaningful convergence result.

Proposal residual and source ownership
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ordinary residual is max(abs(y-phi)) for the QP computed with the full
configured move limit, before actual-volume backtracking. It cannot be made
small merely by accepting a heavily reduced step. Diagnostics also contain the
actual maximum field change, scale, effective move bound, measured/linearized
volume fractions, restoration/feasibility flags, QP residual and multiplier.

These diagnostics belong to source_iteration. The next design keeps a copy as
incoming_update, describing how it was produced. That incoming residual must
never certify convergence of the next evaluated design: a fresh proposal must
be computed at the current solved state. The internal controller now applies
that ownership rule. QP tolerances and surrogate factors also limit any stationarity
interpretation; the residual is not a global optimality certificate.

Internal controls
~~~~~~~~~~~~~~~~~

CoreTopologyLevelSet::default_update_controls() owns the internal defaults:
2000 coordinate sweeps per box solve, 60 multiplier bracket iterations,
80 multiplier bisection iterations, 16 step halvings, QP tolerance 1e-8,
linearized volume tolerance 1e-8, measured volume tolerance 1e-6 and restoration
fraction 0.1. These are implementation controls, not new command/GUI parameters.
Defaults are provisional until native numerical/performance verification. Geometry
re-evaluation and repeated sparse solves can be expensive; this implementation
does not yet cache operators or move Cubit queries to worker threads.

Internal controller integration
-------------------------------

The existing Preparing/Starting/Waiting/Evaluating controller phases dispatch by
method. Density follows its existing path. Internal level-set preparation accepts
compliance only; mass and level-set displacement remain unsupported. Public
begin-run validation explicitly rejects level set pending native
verification, before resetting case data or creating a solver job. No bypass
command, additional solver executable or export-only job type was introduced.

After common material/section initialization and preservation resolution, the
level-set bridge collects Cubit volumes, constructs iteration 1 locally, and
stages it in iteration_level_set_designs. Requested factors are assigned through
CalculiXTopologyInterface. The staged design is removed if assignment fails;
completed earlier records are not removed by next-design preparation. Fresh
level-set domain initialization rejects existing iteration/job records. As with
the existing material setter, this is not a new general multi-core transaction.

CalculiXMaterialsInterface::get_topology_iteration_factors() supplies validated
applied factors. No new level-set bridge imports CoreMaterials or CoreSections.
The physical job uses the existing naming, export, result loading and wait/kill
machinery. Compliance reads the existing required element energy output; no
adjoint is launched for level-set compliance.

A common TopologyIterationEvaluation stores the captured method and objective,
physical objective/factor sensitivities, nested level-set nodal evaluation and
current proposal diagnostics. Optional displacement monitoring still updates the
common record. Failed later preparation/proposal work does not discard a
successfully stored physical objective. The completed marker follows successful
required evaluation stages.

Proposal calculation and acceptance are separate:

* build_proposal() validates the solved source and computes the full-move QP
  without constructing, backtracking or registering the next design.
* The controller stores the proposal only in the active run context and records
  its source diagnostics in the current common evaluation. Phase polling copies
  process state without copying large proposal maps.
* Convergence and iteration-limit decisions happen before acceptance. A terminal
  decision discards the transient proposal; there is no unused next job/design.
* Continuing to Preparing consumes the cached proposal by move, checks source
  iteration and run serial, and calls accept_proposal(). Acceptance performs
  geometry-only backtracking and factor construction without solving the QP again.
* The bridge stages the accepted design and factor assignment before normal job
  creation. Completion/failure/kill/reset removes cached proposals. Rerun/reset
  clears level-set designs along with common energies/evaluations and job data.

The original build_update() helper remains a proposal-plus-acceptance convenience
for native verification. Proposal diagnostics describe the source and its QP;
accepted-step diagnostics belong to the next design's incoming_update. A standalone
single-iteration evaluation may discard the returned transient proposal while
retaining its current diagnostic record.

Level-set convergence starts with the second evaluated iteration and requires
maximum nodal field change and the current ordinary proposal residual to be at
most the captured field_tolerance, relative compliance change within the existing
objective tolerance, and actual volume within the requested target plus Core
volume tolerance. Method/iteration IDs, field domains and fixed-node sets must
match. Fixed coefficients must remain +1. A missing resolved free interface is
reported as a stalled/saturated field, not convergence based on zero derivatives.
Incoming diagnostics are never substituted for a current residual.

The stored method tag is independent of later case edits. Batch 6 serializes
settings/designs/evaluations and generalizes result queries. There is no topology
save version, backward-compatibility fallback or migration. Loading validates
stored numeric domains without querying the live mesh, and does not resume a run.
The transient QP proposal payload and sparse operators are never persisted; source
proposal diagnostics are historical data only. See :doc:`topology-results` for
the save/load and shared fraction-query contracts. The public execution gate remains closed until native verification is ready.
GUI Method selection configures cases without enabling execution.

Verification gates
------------------

Before enabling runs, verify nodal interpolation, partition of unity, zero sum
of shape gradients, finite-difference agreement of shape gradients, affine
Jacobian volumes, all six node orderings, regularized planar cuts, fraction and
volume derivatives, preservation, failure-output clearing and refinement limits.
Curved higher-order, mixed-family and narrow-band cases need independent
refinement studies. Native C++ compilation/execution is a separate authorized
verification step; source inspection alone does not establish numerical correctness.

The parameterized fixed-mesh background is described in
`A level-set based topology optimization using the element connectivity
parameterization method <https://link.springer.com/article/10.1007/s00158-010-0485-y>`_.
The equations and contracts above define this implementation proposal; they do
not claim to reproduce that paper's particular connectivity parameterization.
