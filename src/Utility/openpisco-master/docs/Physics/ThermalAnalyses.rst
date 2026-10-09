.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

##################
Thermal Analyses
##################

This page presents the thermal analyses currently supported by OpenPisco and
their corresponding mathematical formulations.

Overview
########

OpenPisco provides thermal and thermo-mechanical analyses used in
topology optimization workflows.

Supported Analyses
##################

* Thermal conduction
* Thermal conduction based on eigenvalues
* Thermoelasticity

Thermal Conduction
##################

We are looking for the temperature :math:`T`, solution of the following problem

.. math::

    \begin{equation}
        \left\{
        \begin{aligned}
        - \rho C_p \frac{\partial T}{\partial t} - \text {div} (\lambda \nabla T) &= S  \qquad &\text{ in } \Omega \times \mathbb{R}^{+},\\
         \lambda \nabla T \cdot n &= -h (T-T_{ext})  \qquad &\text{ on } \Gamma_N \times \mathbb{R}^{+}, \\
         \lambda \nabla T \cdot n &= 0  \qquad &\text{ on } \Gamma \times \mathbb{R}^{+}, \\
         T &= T_d\qquad &\text{ on  } \Gamma_D \times \mathbb{R}^{+},\\
        T(t=0) &= T_{init}\qquad &\text{ in  } \Omega.\\
        \end{aligned}
        \right.
    \end{equation}

where:

* :math:`S` is the thermal source
* :math:`T_{ext}` is the emperature of the fluid in contact with the structure
* :math:`T_d` is the prescribed temperature
* :math:`h` is the convection coefficient
* :math:`\rho` is the material density
* :math:`C_p` is the thermal capacity
* :math:`\lambda_c` is the thermal conductivity
* :math:`T_{init}` is the initial temperature
* the temperature is prescribed at :math:`T_d` on :math:`\Gamma_D`

Associated Implementations
^^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.PhysicalSolvers.AsterThermal`
* :py:class:`OpenPisco.PhysicalSolvers.UnstructuredFEAThermalSolver.UnstructuredFEAThermalProblem`

Thermal Conduction Based on Eigenvalues
#######################################

The unknown temperature :math:`T` of following thermal problem

.. math::

    \begin{equation}
        \left\{
        \begin{aligned}
         - \rho C_p \frac{\partial T}{\partial t} - \text {div} (\lambda \nabla T) &= S  \qquad &\text{ in } \Omega \times \mathbb{R}^{+},\\
         T &= T_d\qquad &\text{ on  } \partial \Omega \times \mathbb{R}^{+},\\
         T(t=0) &= T_{init}\qquad &\text{ in  } \Omega.\\
        \end{aligned}
        \right.
    \end{equation}

can be computed using a spectral decomposition involving the following eigenvalue problem

.. math::

    \begin{equation}
        \left\{
        \begin{aligned}
         \text {div} (\lambda_c \nabla T_i) &= \mu_i T_i  \qquad &\text{ in } \Omega \times \mathbb{R}^{+},\\
         T &= T_d\qquad &\text{ on  } \partial \Omega \times \mathbb{R}^{+},\\
        \end{aligned}
        \right.
    \end{equation}

where:

* :math:`\mu_i` is the i-th eigenvalue
* :math:`T_i` is the i-th eigenmode
* :math:`T_d` is the fixed temperature
* :math:`\rho` is the material density
* :math:`C_p` is the thermal capacity
* :math:`\lambda_c` is the thermal conductivity

Associated Implementations
^^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.PhysicalSolvers.AsterThermalEigenValues`

Thermoelasticity
################

We investigate a weakly coupled thermoelastic problem. As such, the first step
is to perform a classic thermal analysis to find the temperature :math:`T` (see
thermal analysis for more details). The second step is, for a given temperature
field, to find the displacement governed by the following problem

.. math::

    \begin{equation}
    \left\{
    \begin{aligned}
    \text {div} (\sigma(u,\Delta T)) + f &= 0  \qquad &\text{ in } \Omega,\\
    \sigma(u,\Delta T) &= {\cal A} e(u) + {\cal A} e^{ther}(\Delta T)  \qquad &\text{ in } \Omega,\\
    \sigma(u) \cdot n &= g  \qquad &\text{ on } \Gamma_N, \\
    \sigma(u) \cdot n &= 0  \qquad &\text{ on } \Gamma, \\
     u &= 0\qquad &\text{ on  } \Gamma_D.\\
    \end{aligned}
    \right.
    \end{equation}

where:

* :math:`e(u)` is the strain tensor
* :math:`e^{ther}` is the thermal strain equal to

.. math::

    \begin{equation}
    e^{ther}(\Delta T) = -\alpha {\cal A} (\Delta T I).
    \end{equation}

* :math:`{\cal A}` is the fourth order elasticity tensor equal to

.. math::

    \begin{equation}
    {\cal A}_{ijkh}=\lambda\,\delta_{ij}\delta_{kh}+\mu(\delta_{ik}\delta_{jh}+\delta_{ih}\delta_{jk})
    \end{equation}

with :math:`\delta_{ij}` the kronecker symbol.

* :math:`\lambda` and :math:`\mu` are the Lamé coefficients
* :math:`g` is the density of surfaces forces
* :math:`f` is the density of volumic forces
* the body is clamped on :math:`\Gamma_D`

Associated Implementations
^^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.PhysicalSolvers.AsterThermoElastic`
* :py:class:`OpenPisco.PhysicalSolvers.UnstructuredFEAMecaSolver.UnstructuredFEAMecaThermalStrainProblem`
