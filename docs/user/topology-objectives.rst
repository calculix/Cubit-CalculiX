Objectives and convergence
==========================

Choosing an objective
---------------------

The density method currently supports compliance and displacement optimization.
Mass and level-set optimization are not available. See
:doc:`topology-settings` for controls and :doc:`../commands/topology` for syntax.

Compliance measures flexibility under the applied loads. For the supported
linear analysis it is twice the total internal strain energy. Minimizing it
seeks a stiffer structure within the permitted volume.

Displacement minimizes the maximum absolute X, Y, or Z displacement, or the
maximum displacement magnitude, among nodes in the chosen nodeset. The
controlling node is selected again at each iteration. A change of controlling
node can make the objective nonsmooth. Monitoring displacement during a
compliance run does not change that run's objective to displacement.

These derivatives assume fixed loads, homogeneous prescribed displacements,
and a symmetric linear-elastic stiffness matrix. Both density objectives
support C3D4, C3D6, C3D8, C3D10, C3D15, and C3D20. Their shared validator
rejects C3D8R. See :doc:`topology-solver` for the other analysis restrictions.
The derivation and integration conventions are in
:doc:`../developer/topology-algorithms`.

Volume and filtering
--------------------

The target volume fraction is an upper bound on density-weighted volume divided
by total participating element volume. Preserved elements remain fully solid
and count toward this bound. Their volume is an unavoidable contribution;
the minimum density of the other elements adds a further lower bound.

The initial design is fully solid. A move limit restricts how much each density
can change in one update, so reaching the target can take several iterations.
During this volume reduction, compliance or displacement can increase as
material is removed. Exceeding the target during this initial phase is expected.

The filter smooths sensitivities using element centers, volumes, and densities.
It does not average the densities themselves. Preserved elements are excluded
from the filter neighborhood. A zero filter radius selects
:math:`1.5(V_{\mathrm{total}}/N)^{1/3}`, where :math:`N` includes all participating
elements. The selected physical length therefore depends on the mesh.

Updates and adaptive moves
--------------------------

Compliance uses an optimality-criteria (OC) update. Adaptive moves are enabled
by default: opposing recent density changes reduce the move limit, while
strongly aligned changes can increase it toward the configured maximum.
Adaptation starts only after sufficient history exists and recent designs
satisfy the volume bound. This avoids treating the initial volume reduction
as oscillation.

Displacement uses a projected-gradient update that accepts signed derivatives.
The same adaptive move selection applies to it. There is no objective line
search or guarantee that every displacement update decreases the objective.

Reading convergence indicators
------------------------------

Starting with the second evaluated iteration, the controller checks:

.. math::

   \Delta\rho &= \max_e |\rho_e^k-\rho_e^{k-1}|,\\
   r_J &= \frac{|J_k-J_{k-1}|}{\max(J_k,J_{k-1})},\\
   f_k &\le f_{\mathrm{target}}+10^{-6}.

When both objective values are zero, :math:`r_J=0`. The density change must
not exceed the density tolerance, and the relative objective change must not
exceed the relative objective tolerance.

Both objectives also require an ordinary proposal residual: OC for compliance,
and projected-gradient for displacement:

.. math::

   R=\max_e |\rho_e^{\mathrm{ordinary}}-\rho_e^k|
   \le \text{density tolerance}.

This proposal uses the configured maximum move limit, even if the actual
adaptive update used a smaller one. It requires no additional solve and
prevents a small adaptive move alone from implying convergence.

The action panel's bars show usage of each displayed limit, rather than a
percentage of the optimization completed. ``WAIT`` means that criterion has
not passed. All three visible indicators can pass while the additional
objective-specific proposal check still prevents convergence.
``iteration_limit`` means the maximum number of evaluated designs was reached;
it does not certify convergence. The initial solid design counts as iteration 1.

Use the stored iterations and preview to judge the result before applying a
deletion threshold. Convergence does not guarantee a globally optimal design.
