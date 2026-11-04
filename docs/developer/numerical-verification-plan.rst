Native topology verification specification
==========================================

Status and execution boundary
------------------------------

This is a specification for checking the production C++ implementation, not
executed numerical evidence. The current Python interface does not expose the
interpolation, level-set geometry, nodal-evaluation or proposal/acceptance helpers.
The unified journal records those stages as BLOCKED. Production code is frozen
for the documentation work; no bridge, test bypass or public run-gate removal
was added. Implementing duplicate Python formulas would not verify the native
functions and must not be reported as such.

The entry journal is
:download:`verify_topology.jou <../../examples/calculix/topology/verification/verify_topology.jou>`.
It drives the available configuration checks and writes a complete stage manifest.
See :doc:`topology-verification` for reporting conventions and
:doc:`topology-level-set` for the method's definitions and assumptions.

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

The continuum comparison requires actual physical solves, matching applied
factors, outputs, mesh/order and independent perturbations. Grouped material
factors introduce surrogate derivatives; regrouped finite differences test a
different operation. Keep that evidence separate from the analytical chain.

Proposal, acceptance and preservation
---------------------------------------

Use current Cubit elements with a complete node domain and at least one preserved
and one free region. Query current volumes/connectivity and call the existing
``build_proposal`` / ``accept_proposal`` helpers without registering a normal
topology job or bypassing its public gate. A future approved test bridge must
retain this stateless contract.

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
