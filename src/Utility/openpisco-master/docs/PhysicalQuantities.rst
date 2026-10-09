.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

***************************
Physical Fields and Scalars
***************************

Overview
########

Besides the primary solution of a physical problem, OpenPisco allows physical
solvers to generate additional quantities such as stresses, strains or
integrated energies.

These quantities are collectively referred to as auxiliary fields and
auxiliary scalars.

Auxiliary Fields
################

The capability of a solver to generate a given field is described through the
attribute

* :py:attr:`OpenPisco.PhysicalSolvers.SolverBase.SolverBase.auxiliaryFieldGeneration`

Useful methods include:

* :py:meth:`OpenPisco.PhysicalSolvers.SolverBase.SolverBase.CanSupplyAuxiliaryField`
* :py:meth:`OpenPisco.PhysicalSolvers.SolverBase.SolverBase.SetAuxiliaryFieldGeneration`
* :py:meth:`OpenPisco.PhysicalSolvers.SolverBase.SolverBase.GetAuxiliaryField`

Auxiliary Scalars
#################

Scalar quantities follow a similar workflow through:

* :py:meth:`OpenPisco.PhysicalSolvers.SolverBase.SolverBase.SetAuxiliaryScalar`
* :py:meth:`OpenPisco.PhysicalSolvers.SolverBase.SolverBase.GetAuxiliaryScalar`

Field Naming Convention
#######################

All field and scalar identifiers are defined in

* :py:mod:`OpenPisco.PhysicalSolvers.FieldsNames`

This module provides a common vocabulary shared by all solver interfaces.

Available Fields
################

Stress
=======

Cauchy stress tensor.

Strain
======

Small-strain tensor:

.. math::

   e(u)=\frac{\nabla u + (\nabla u)^T}{2}

Von Mises Stress
================

Equivalent stress defined by

.. math::

   \sigma_v=\sqrt{\frac32 s_{ij}s_{ij}}

Potential Energy
================

Local strain energy density:

.. math::

   \frac12 \sigma(u):e(u)

Available Scalars
#################

Volume
======

.. math::

   \int_\Omega dx

Mass
====

.. math::

   \int_\Omega \rho \, dx

Integrated Potential Energy
===========================

.. math::

   \int_\Omega \frac12 \sigma(u):e(u)\,dx

Field Availability
##################

N, C and IP denote respectively:

* N: nodes
* C: cell centroids
* IP: integration points

.. csv-table:: Fields available w.r.t. tools
   :header: "Field", "Code_Aster", "FreeFem++", "Zset", "Muscat"
   :widths: 30, 10, 10, 10, 10

   "stress", "N,C,IP", "N", "", ""
   "strain", "N,C,IP", "", "", ""
   "von_mises", "N,C,IP", "N", "", "N,C,IP"
   "potential_energy", "N,C,IP", "N", "N", "N,C,IP"

Scalar Availability
###################

.. csv-table:: Scalars available w.r.t. tools
   :header: "Scalar", "Code_Aster", "FreeFem++", "Zset", "Muscat"
   :widths: 30, 10, 10, 10, 10

   "volume", "False", "False", "False", "False"
   "mass", "False", "False", "False", "False"
   "int_potential_energy", "True", "True", "True", "True"