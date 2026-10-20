Topology: mathematics and implementation map
============================================

This first page establishes the notation and links to the implementing code.
Detailed derivations, filter/update algorithms, persistence, and GUI behavior
will be documented in subsequent chapters.

Linear model and density interpolation
--------------------------------------

For fixed loads, homogeneous prescribed displacements, and the supported
symmetric linear-elastic model:

.. math::

   K(\rho)u=f,\qquad
   K_e=s_eK_e^{\mathrm{base}},\qquad
   s_e=v+(1-v)\rho_e^p.

Preserved elements are fixed at :math:`\rho_e=s_e=1`. Their stored design
sensitivities are zero because their densities are not optimization variables.

Compliance
----------

.. math::

   C=f^T u=2\sum_e U_e,\qquad
   U_e=\tfrac12u_e^T K_eu_e,\qquad
   \frac{\partial C}{\partial\rho_e}
   =-\frac{2U_e}{s_e}(1-v)p\rho_e^{p-1}.

Displacement adjoint
--------------------

For a monitored node, the objective is the absolute selected component or the
vector norm. For a nodeset, the implementation selects the node attaining the
maximum. Away from a change in that node or a nondifferentiable zero:

.. math::

   K^T\lambda=\frac{\partial J}{\partial u},\qquad
   \frac{\partial J}{\partial\rho_e}
   =-\lambda_e^T\frac{\partial K_e}{\partial\rho_e}u_e.

The component adjoint load is a signed unit load; the magnitude load is
:math:`u/\lVert u\rVert` at the controlling node. The current implementation
uses the zero subgradient at zero monitored displacement and skips the adjoint.

Using physical stress and adjoint mechanical strain:

.. math::

   I_e=\sum_q
   \left(\sigma_e^{\mathrm{physical}}:
   \varepsilon_e^{\mathrm{adjoint}}\right)_q w_q\det J_q,
   \qquad
   \frac{\partial J}{\partial\rho_e}
   =-\frac{I_e}{s_e}(1-v)p\rho_e^{p-1}.

There is **no factor of one half** in this contraction: it is the bilinear
adjoint product :math:`\lambda_e^T K_eu_e`, not stored strain energy.
For tensor shear components, the contraction uses normal terms once and
the three shear terms twice. Both fields must use matching global coordinates
and CalculiX integration-point IDs.

Implementation entry points
---------------------------

* :cpp:class:`CoreTopologyOptimization` orchestrates physical/adjoint jobs and evaluation.
* :cpp:class:`CoreTopology` stores cases and performs objective/density calculations.
* :cpp:func:`ElementIntegration::calculate_weights` produces :math:`w_q\det J_q`
  using Cubit/Exodus coordinates and CalculiX integration rules.
* The shared node-order helper converts local connectivity positions; its
  element-type IDs must match the installed Cubit SDK. WEDGE15 swaps midside
  positions 10--12 with 13--15 when exported to C3D15.

Cross-core operations pass through ``CalculiXCoreInterface``; geometry and
materials are supplied through the existing core and Cubit interfaces.

See :doc:`../tutorials/index` for verification scope and
:doc:`../user/topology` for the user workflow.
