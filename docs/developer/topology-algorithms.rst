Density optimization algorithms
===============================

This chapter describes the implemented density method in ``CoreTopology`` and
the evaluation sequence in ``CoreTopologyOptimization``. See :doc:`topology`
for the implementation map, :doc:`api` for contracts, and
:doc:`../user/topology-objectives` for interpreting the controls.

Notation and assumptions
------------------------

An element has density :math:`\rho_e`, physical volume :math:`V_e`, and
stiffness factor :math:`s_e`. The stored design supplies the penalization
:math:`p`, void stiffness ratio :math:`v`, and minimum density
:math:`\rho_{\min}`. The analysis assumes fixed loads, homogeneous prescribed
displacements, and symmetric linear-elastic stiffness:

.. math::

   K(\rho)u=f,\qquad K_e=s_eK_e^{\mathrm{base}},\qquad
   s_e=v+(1-v)\rho_e^p.

Thus :math:`s'_e=(1-v)p\rho_e^{p-1}`. Preserved densities and stiffness factors
are exactly one. Their stored sensitivities are zero because they are fixed
variables. These assumptions are required for the derivatives below.

Volume and its derivative
-------------------------

Let :math:`F` contain free elements and :math:`P` preserved elements. Define

.. math::

   V_T=\sum_e V_e,\qquad w_e=V_e/V_T,\qquad
   f_P=\sum_{e\in P}w_e,\qquad
   f(\rho)=f_P+\sum_{e\in F}w_e\rho_e.

The stored volume derivative is :math:`w_e` for free variables and zero for
preserved variables. The stored minimum volume fraction is :math:`f_P`;
the actual minimum attainable design fraction is
:math:`f_P+\rho_{\min}(1-f_P)`. Cubit provides the element volumes directly;
an ``EVOL`` history output is unnecessary.

Compliance derivative
---------------------

For fixed :math:`f`, differentiating equilibrium gives
:math:`K\,du=-dK\,u`. Therefore:

.. math::

   C=f^Tu=u^TKu=2\sum_e U_e,\qquad
   dC=-u^T(dK)u,\\
   U_e=\tfrac12u_e^TK_eu_e,\qquad
   \frac{\partial C}{\partial s_e}=-\frac{2U_e}{s_e},\qquad
   g_e=\frac{\partial C}{\partial\rho_e}
   =-\frac{2U_e}{s_e}(1-v)p\rho_e^{p-1}.

``evaluate_compliance`` uses element internal energies from the physical solve.
It validates complete element coverage and consistency with the stored design
before committing an evaluation. Re-evaluating an existing iteration is rejected.

Displacement adjoint derivative
-------------------------------

For component :math:`c`, :math:`J=\max_{a\in\mathcal N}|u_{a,c}|`.
For magnitude, :math:`J=\max_{a\in\mathcal N}\|u_a\|`.
At a nonzero, fixed controlling node :math:`a`, the adjoint right-hand side
:math:`q=\partial J/\partial u` is zero elsewhere. Its active entry is
:math:`\operatorname{sign}(u_{a,c})` for a component, or its three active entries
are :math:`u_a/\|u_a\|` for magnitude. Solve

.. math::

   K^T\lambda=q,\qquad
   dJ=q^Tdu=-\lambda^T(dK)u.

The implementation contracts physical stress with adjoint mechanical strain:

.. math::

   I_e=\int_{\Omega_e}\sigma^{\mathrm{physical}}:
   \varepsilon^{\mathrm{adjoint}}\,dV
   =\lambda_e^TK_eu_e,\\
   \frac{\partial J}{\partial s_e}=-I_e/s_e,\qquad
   g_e=-\frac{I_e}{s_e}(1-v)p\rho_e^{p-1}.

There is no factor of one half: this is a bilinear contraction of two different
solutions, not the strain energy of one solution. Physical stress already
contains the current stiffness scaling. Multiplying by another :math:`s_e`
would count that scaling twice; division by :math:`s_e` converts the contraction
to the stiffness-factor derivative.

The final available solver step/increment supplies the results. The controlling
node is selected anew each iteration. Ties and changes of active node are
nonsmooth; the formula is a local branch derivative. At zero objective the
implementation chooses a zero subgradient, skips the adjoint solve, and uses
zero element integrals. This is not a differentiability claim at zero.

Integration and tensor conventions
----------------------------------

Physical ``S`` and adjoint ``ME`` use global coordinates, with six components
ordered xx, yy, zz, xy, xz, yz. Mechanical strain uses tensor shear components:

.. math::

   \sigma:\varepsilon=
   \sigma_{xx}\varepsilon_{xx}+\sigma_{yy}\varepsilon_{yy}
   +\sigma_{zz}\varepsilon_{zz}
   +2(\sigma_{xy}\varepsilon_{xy}+\sigma_{xz}\varepsilon_{xz}
   +\sigma_{yz}\varepsilon_{yz}).

Complete matching element and integration-point coverage is required.
``ElementIntegration`` computes reference weights times :math:`\det J` using
the element coordinates, with strictly positive, finite determinants and
weights. Rules and shape gradients follow the locally inspected CalculiX 2.23
sources (``gauss.f`` and the corresponding tetrahedron, wedge, and hexahedron
shape routines).

.. list-table:: Supported displacement integration
   :header-rows: 1

   * - Element
     - Integration points
   * - C3D4
     - 1
   * - C3D6
     - 2
   * - C3D8
     - 8
   * - C3D10
     - 4
   * - C3D15
     - 9
   * - C3D20
     - 27

The shared model validator rejects C3D8R for both compliance and displacement.
See :doc:`topology-export` for validation and output preparation. Point IDs are
one-based and
follow CalculiX order. Input coordinates use Cubit/Exodus connectivity;
``ElementNodeOrdering`` converts the source indices. For WEDGE15 (Cubit type
52), positions 10--12 and 13--15 are exchanged. For HEX20 (type 44), positions
13--16 and 17--20 are exchanged. Positions here are one-based. Other supported
volume types use identity ordering. These permutations must remain shared with
export and result conversion.

Sensitivity filter
------------------

Let :math:`g_e` denote the raw density derivative. For free neighbors within
radius :math:`r`, define

.. math::

   a_{ij}=\left(1-\frac{\|x_i-x_j\|}{r}\right)\frac{V_j}{V_{\max}},\qquad
   \widetilde g_i=\frac{V_i}{\rho_i}
   \frac{\sum_{j\in F,\ d_{ij}<r}a_{ij}\rho_j(g_j/V_j)}
        {\sum_{j\in F,\ d_{ij}<r}a_{ij}}.

:math:`V_{\max}` is the largest volume across all participating elements.
Each free element includes itself. Preserved elements neither receive a
filtered derivative nor enter free neighborhoods. With configured radius zero,
:math:`r=1.5(V_T/N)^{1/3}` using all participating elements.

The implementation buckets free centers into cells of side length :math:`r`
and searches the current cell and its 26 neighbors. This avoids an unconditional
all-pairs search, but dense neighborhoods can still have quadratic cost.
Filtering commits only after validation and invalidates any previously stored
ordinary OC residual. Raw derivatives remain available separately. This is a
sensitivity filter, not a density filter, and is not generally the derivative
of a separately defined filtered objective.

Compliance OC update
--------------------

For move limit :math:`m`, let

.. math::

   l_e=\max(\rho_{\min},\rho_e-m),\qquad
   h_e=\min(1,\rho_e+m),\\
   f_* = \max\left(f_{\mathrm{target}}, f_P+\sum_{e\in F}w_el_e\right).

The staged target :math:`f_*` permits an initial reduction toward the configured
target without violating move bounds. Compliance requires nonpositive filtered
derivatives. Define :math:`b_e=-\widetilde g_e/w_e` and normalize all benefits
by their largest value when positive. A common scale changes only the multiplier.

.. math::

   \rho_e^+(\lambda)=\operatorname{clamp}
   \left(\rho_e\sqrt{b_e/\lambda},l_e,h_e\right).

Zero benefit selects the lower bound. Positive benefit with zero multiplier
selects the upper bound. If the zero-multiplier trial is feasible, accept it;
otherwise bracket the multiplier by doubling from one (up to 100 times), then
bisect for up to 100 steps. Retain a feasible trial and stop when the remaining
volume gap is at most :math:`10^{-10}`. Volume is an upper bound; an update
may use less than the target. Preserved densities stay one.

Adaptive compliance moves
-------------------------

Adaptation needs three ordered stored designs ending at the source iteration.
If any of their volume fractions exceeds the target by more than :math:`10^{-6}`,
use the configured maximum move. Otherwise start with the current design's
recorded incoming move, bounded by the permitted minimum and maximum. The
automatic minimum is ten percent of the maximum.

For free elements, use consecutive changes :math:`a_e=\rho_e^k-\rho_e^{k-1}`
and :math:`b_e=\rho_e^{k-1}-\rho_e^{k-2}`:

.. math::

   c=\frac{\sum w_ea_eb_e}
   {\sqrt{\sum w_ea_e^2}\sqrt{\sum w_eb_e^2}}.

If either weighted squared norm is at most :math:`10^{-24}`, correlation is
unavailable and the selected move is retained. Otherwise clamp correlation
to [-1, 1]. For :math:`c<-0.25`, halve the move down to the minimum. For
:math:`c>0.5`, multiply by 1.1 up to the maximum. Between these thresholds,
retain it. Diagnostics are attached to the next design. Disabling adaptation
or optimizing displacement uses the configured maximum move.

Displacement projected-gradient update
--------------------------------------

Signed displacement derivatives need a different update. With the same bounds
and staged volume target, define

.. math::

   q_e=\widetilde g_e/w_e,\qquad Q=\max_{e\in F}|q_e|,\qquad
   t_e=\rho_e-mq_e/Q.

For :math:`Q=0`, use zero direction, so :math:`t_e=\rho_e`. Project onto the
bounds and volume inequality in a volume-weighted metric:

.. math::

   \min_z\ \tfrac12\sum_{e\in F}w_e(z_e-t_e)^2,\qquad
   l_e\le z_e\le h_e,\qquad f_P+\sum_{e\in F}w_ez_e\le f_*,\\
   z_e=\operatorname{clamp}(t_e-\lambda,l_e,h_e),\qquad\lambda\ge0.

Accept the zero-multiplier trial when feasible within :math:`10^{-10}`.
Otherwise :math:`\max(0,\max_e(t_e-l_e))` bounds the multiplier above;
bisect up to 100 times, retaining a feasible trial. Even a zero-gradient design
can change when volume projection is needed. There is no additional physical
solve for a line search and no monotonic objective guarantee.

Convergence and ordinary OC residual
------------------------------------

From the second evaluation, compute

.. math::

   D_k=\max_e|\rho_e^k-\rho_e^{k-1}|,\qquad
   r_k=\frac{|J_k-J_{k-1}|}{\max(J_k,J_{k-1})}.

For both objective values zero, use :math:`r_k=0`. The implementation evaluates
the relative change through normalized objective values to avoid unnecessary
overflow. Both objectives require :math:`D_k\le\epsilon_\rho`,
:math:`r_k\le\epsilon_J`, and :math:`f_k\le f_{\mathrm{target}}+10^{-6}`.

Compliance additionally evaluates a fresh OC proposal from the current filtered
sensitivities with the configured maximum move, independently of adaptation:

.. math::

   R_k=\max_e|\rho_e^{\mathrm{OC}}-\rho_e^k|\le\epsilon_\rho.

This diagnostic does not solve or store another physical design. Missing residual
availability prevents convergence; invalid available data causes an error.
The diagnostic is recomputed after filtering. ``check_density_convergence``
returns operation success separately from its ``converged`` output.

Evaluation order and diagnostics
--------------------------------

After physical and, when needed, adjoint results are available:

#. Evaluate the objective and raw sensitivities.
#. Store optional displacement monitoring values.
#. Collect Cubit element volumes and centers.
#. Filter density sensitivities.
#. Evaluate the ordinary OC residual for compliance.
#. Mark the iteration completed.
#. Check convergence from iteration 2; otherwise enforce the iteration limit
   or prepare and store the next density design and its update diagnostics.

Element coverage, finite values, consistent factors, and positive geometric
quantities are checked before their respective commits. These are separate
operations; a failure is not a transaction rolling back all earlier operations.
The initial solid design counts toward the iteration limit. Completion at that
limit is distinct from convergence.

Verification scope
------------------

:doc:`../tutorials/index` records the derivative matrix and its limitations.
Central finite differences validate raw objective derivatives on the tested
branches. They do not validate filtered sensitivities as exact objective
derivatives, optimizer convergence, or a global optimum. The straight-sided
integration checks do not establish coverage for curved higher-order elements.
