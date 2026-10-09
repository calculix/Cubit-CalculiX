.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

#################
Physical Criteria
#################

This page presents the physical criteria currently supported by OpenPisco and
their corresponding mathematical formulations.

Overview
""""""""

Physical criteria require the solution of a physical problem in order to
evaluate criterion values and sensitivities.

The corresponding classes are collected in:

* :py:meth:`OpenPisco.Optim.Criteria.PhyCriteria`
* :py:meth:`OpenPisco.Optim.Criteria.PhyMecaCriteria`
* :py:meth:`OpenPisco.Optim.Criteria.PhyModalCriteria`
* :py:meth:`OpenPisco.Optim.Criteria.PhyHarmonicCriteria`
* :py:meth:`OpenPisco.Optim.Criteria.PhyBucklingCriteria`
* :py:meth:`OpenPisco.Optim.Criteria.PhyThermalCriteria`
* :py:meth:`OpenPisco.Optim.Criteria.PhyRobustCriteria`

Supported Criteria
""""""""""""""""""

* Elastic compliance
* Von Mises stress
* Target displacement
* k-th eigenfrequency
* Effective radiated power
* Target temperature
* Buckling
* Worst-case elastic compliance
* Fluid related force criterion

Physical Criteria Architecture
""""""""""""""""""""""""""""""

A base class describing physical criteria can be found in

* :py:meth:`OpenPisco.Optim.Criteria.PhysicalCriteriaBase`

Every physical criterion contains a mandatory attribute

* :py:attr:`OpenPisco.Optim.Criteria.problem`

which corresponds to a physical solver.

See :doc:`PhysicalSolvers` for details regarding physical solver capabilities.

Auxiliary Quantities
""""""""""""""""""""

Physical criteria request the quantities they need through the standardized
auxiliary field and scalar mechanism exposed by OpenPisco physical solvers.

Instead of accessing solver-specific result objects, criteria rely on
standardized quantity names.

See :doc:`PhysicalQuantities`.

A Word About Adjoint Problems
""""""""""""""""""""""""""""""

Calculating the sensitivity of a physical criterion efficiently often requires
solving an adjoint problem.

The criterion itself is responsible for defining and computing the adjoint
state, often by relying on a second internal solver instance.

There is no constraint regarding the type of solver used to solve adjoint
problems. It may be identical to the direct solver or a dedicated alternative.

A Word About Sensitivity Direction
"""""""""""""""""""""""""""""""""""

By convention, OpenPisco considers the sign of the sensitivity to be aligned
with an increase of the criterion value.

For instance, for a volume criterion whose sensitivity is equal to one
everywhere, moving the interface outward increases the volume.

Elastic Compliance
==================

This criterion evaluates the elastic compliance of the negative part of the
level set in linear elasticity

.. math::

    J(\Omega) = \int_{\Omega}{\sigma(u):\varepsilon (u)\:dx}

where the quantities :math:`\sigma(u)` and :math:`\varepsilon (u)` are related
to a linear elasticity analysis.

Required auxiliary quantities:

* potential_energy
* int_potential_energy on nodes

This criterion does not require an adjoint solver.

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.PhyCriteria.TopoCriteriaCompliance`

Von Mises Stress
==================

This criterion evaluates the von Mises equivalent stress of the negative part
of the level set in linear elasticity.

.. math::

    J(\Omega) = \big( \frac{ \int_{\Omega}{ \sigma_{vm}^\alpha  \:dx} }{ \int_{\Omega}{ \:dx}  }\big)^{\frac{1}{\alpha}}

where :math:`\alpha` is a positive parameter.

Required auxiliary quantities:

* stress     on integration points
* von_mises  on nodes
* von_mises  on integration points

The adjoint state computation reposes on Code_Aster.

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.PhyMecaCriteria.TopoCriteriaVonMises`

Target Displacement
==================

This criterion evaluates the discrepancy between actual and target
displacements

.. math::

   J(\Omega) = |u_{nTag}*dir -u_0|^2

Required auxiliary quantities:

* stress on integration points

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.PhyMecaCriteria.TopoCriteriaNodalTargetDisp`

k-th Eigenfrequency
==================

.. math::

    J_k(\Omega)= -\omega_k^2.

Required auxiliary quantities:

* FN.EigenFreqSquared_sensitivity
* FN.EigenFreqSquared

This criterion does not require an adjoint solver.

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.PhyModalCriteria.TopoCriteriaModal`

Effective Radiated Power (ERP)
==============================

.. math::

    J(\Omega) = \frac{1}{\omega_{max}-\omega_{min}}\int_{\omega_{min}}^{\omega_{max}}  \frac{1}{2} f \rho_f c_f \int_{S^e_k} |(\dot{u}_\Re +i \dot{u}_\Im)\cdot n|^2 \:ds \:d\omega,

Required auxiliary quantities:

* ERP_sensitivity        on Nodes
* ERP_density            on Nodes
* dominant_SolutionMode  on Nodes
* ERP
* ModalCumulMassFractiondX
* ModalCumulMassFractiondY
* ModalCumulMassFractiondZ
* ModalLastEnergyRatio

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.PhyHarmonicCriteria.TopoCriteriaERP`

Target Temperature
==================

.. math::

    J(\Omega)  = \int_{\Omega}  max(T-T_s,0.)^2

Required auxiliary quantities:

* temperature on Nodes

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.PhyThermalCriteria.TopoCriteriaTargetTemperature`

Buckling
========

.. math::

    J(\Omega)=\gamma^{-1}(\Omega)

Required auxiliary quantities:

* load_crit
* elastic_energy_buckling  on Centroids
* stress                   on Centroids
* strain_green_buckling    on Centroids
* strain_buckling          on Centroids
* mode_buckling            on FN.Nodes
* von_mises                on FN.Nodes

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.PhyBucklingCriteria.TopoCriteriaBuckling`

Worst-Case Elastic Compliance
=============================

This criterion evaluates elastic compliance under uncertain loading.

Required auxiliary quantities:

* elastic_energy on Nodes
* int_elastic_energy
* stress on Centroids

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.PhyRobustCriteria.TopoCriteriaComplianceRobustLoadWorstCase`

Fluid Related Force Criterion
=============================

.. math::

    J(\Omega)  = \frac{int_{S_w} \rho (-\tau_{ij} n_j + p n_i) r_i dS}{0.5 \rho A U_{\infty}}

where

* :math:`tau_{ij}` are the components of the fluid stress tensor
* :math:`\rho` is the constant density
* :math:`p` is the pressure divided by :math:`\rho`
* :math:`n` is the unit normal vector
* :math:`r` is the projection direction
* :math:`S_w` is the force evaluation surface
* :math:`A` is the frontal area
* :math:`U_{\infty}` is the farfield velocity magnitude

Required auxiliary quantities:

* force_sensitivity on Nodes
* force

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.PhyFluidCriteria.TopoCriteriaForce`


.. [1] https://gitlab.com/drti/muscat
.. [2] G. Allaire, Ch. Dapogny, A linearized approach to worst-case design in parametric and geometric shape optimization, M3AS Vol. 24, No. 11 (2014) 2199-2257
.. [4] G. Michailidis, Manufacturing constraints and multi-phase shape and topology optimization via a level-set method. Diss. Ecole Polytechnique X (2014)