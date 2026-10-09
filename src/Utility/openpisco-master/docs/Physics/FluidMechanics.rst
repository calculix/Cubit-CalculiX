.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

#################
Fluid Mechanics
#################

This page presents the fluid mechanics analyses currently supported by
OpenPisco and their corresponding mathematical formulations.

Overview
########

At present, OpenPisco provides support for incompressible steady-state
laminar fluid flow simulations through an interface with OpenFOAM.

Supported Analyses
##################

* Incompressible steady-state laminar flow

Incompressible Steady-State Laminar Flow
########################################

The flow is governed by the incompressible Navier-Stokes equations

.. math::

    \begin{equation}
        \label{eq:steady_state_NS_incompressible}
        \left\{
        \begin{array}{rllll}
            \text{div}(v) &=& 0 & \text{in} & \Omega_f,\\
            -\text{div}(\sigma_f(v,p))
            + \rho \nabla v\,v
            &=& f_f & \text{in} & \Omega_f,\\
            v &=& v_0 & \text{on} & \partial\Omega^D_f,\\
            \sigma_f(v,p)n &=& 0 & \text{on} & \partial\Omega^N_f,\\
            v &=& 0 & \text{on} & \Gamma.
        \end{array}
        \right.
    \end{equation}

where:

* :math:`\rho` is the fluid density;
* :math:`v` is the velocity field;
* :math:`p` is the pressure field;
* :math:`f_f` is the density of volumic forces acting on
  :math:`\Omega_f`;
* :math:`\sigma_f` is the stress tensor of a Newtonian fluid.

The fluid stress tensor is defined as

.. math::

    \begin{equation}
        \sigma_f(v,p)=2\mu e(v)-pI,
    \end{equation}

where

.. math::

    e(v)=\frac{\nabla v + (\nabla v)^T}{2}

is the strain-rate tensor and :math:`\mu` denotes the dynamic viscosity.

Associated Implementation
#########################

The current implementation is provided through:

* :py:class:`OpenPisco.PhysicalSolvers.OpenFoamSteadyState`