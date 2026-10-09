##############################
Optimization algorithms
##############################

.. _OptimizationAlgorithms:


The numerical optimization algorithm handles the balancing between the minimization of the objective and the non-violation of the constraints.

The module :py:meth:`OpenPisco.Optim.Algorithms` collects the optimization algorithms provided by OpenPisco.


Null space optimization algorithm
==================================

The class :py:class:`OpenPisco.Optim.Algorithms.OptimAlgoNullSpace` proposes an implementation of a gradient-flow algorithm designed to decrease both the value of the objective function and the violation of the constraints.
See [1]_ for the theoretical background.


Unconstrained optimization algorithms
===================================================

Popular penalization methods such as the Augmented Lagrangian reformulate a constrained optimization problem into a sequence of unconstrained optimization problems by incorporating the constraints as penalizations of the objective function.

The class :py:class:`OpenPisco.Optim.Algorithms.OptimAlgoUnconstrained` proposes an implementation of an unconstrained algorithm combining a gradient descent routine with a linear line search.



.. [1] F. Feppon, G. Allaire, C. Dapogny, Null space gradient flows for constrained optimization with applications to shape optimization, COCV, 26, (2020).