##############################
Optimization problem
##############################

An optimization problem is specified by an objective criterion to be minimized and a set of constraints criteria.
More precisely, we focus on constrained optimization problems of the following form

.. math::

    \begin{equation}
    \begin{array}{rrclcl}
    \displaystyle \min_{x\in\mathcal{O}_{ad}} & {J(x)}\\
    \textrm{s.t.} & g(x) = 0\\
    &h(x)\geq 0    \\
    \end{array}
    \end{equation}

where :math:`\mathcal{O}_{ad}` is the optimization set, :math:`x` is the state variable, :math:`J` is the objective, :math:`g` and :math:`h` are respectively the equality and inequality constraints.

A base class describing an optimization problem is the following :py:class:`OpenPisco.Optim.Problems.OptimProblemBase`.

The auxiliary methods used to create an optimization problem are defined in the class :py:class:`OpenPisco.Optim.Problems.OptimProblemConcrete`. The key methods are the following

* :py:func:`OpenPisco.Optim.Problems.OptimProblemConcrete.SetObjectiveCriterion()`
* :py:func:`OpenPisco.Optim.Problems.OptimProblemConcrete.AddConstraint()`

The following method updates the values of objective and constraints on the current optimization point

* :py:func:`OpenPisco.Optim.Problems.OptimProblemConcrete.UpdateValues()`

Note that, when defining an optimization problem, the criteria available in OpenPisco can be used indifferently as a constraint or objective.

Topology optimization problem
==================================
In the topology optimization context the space :math:`\mathcal{O}_{ad}` is a set of admissible shapes.
Thus, the current state of a topology optimization problem is a :py:class:`OpenPisco.LevelSetBase` class.

See the :doc:`optimization criteria <Criteria>` documentation for a complete list of optimization criteria provided by OpenPisco.

Design space and implicit zones
--------------------------------------
The search for an optimal design is casted in a given design space. The default design space is the region described by the attribute :py:attr:`OpenPisco.LevelSetBase.support`.

It is possible to modify the default behaviour by indicating the following regions

* OffZone    : subregion included from the design space (subjected to optimization)
* OnZone     : subregion excluded in the design space (not subjected to optimization). 

In other words, any part designated as included in the design space may change through the optimization process while any part excluded from it will remain as it is. 
For instance, unless it is for a very specific purpose, it is not recommanded to apply loads on part of the OffZone region. As a matter of fact, if the shape were to change on this very part, the loads could not be applied properly anymore to the underlying physical problem.

The ImplicitGeometry module of the library Muscat [1]_ is used to define specific zones.

Advance
--------------

The method :py:func:`OpenPisco.Optim.Problems.OptimTopoProblemBase.Advance()` is responsible for updating the state, i.e. the level set, and checking if the new optimization point is acceptable.
The routine tests the following geometric conditions:

*  the relative volume change is not too large : avoid drastic shape variations within one iteration
*  no deconnection between specific zones (such as boundary conditions zones associated to Dirichlet and Neumann) : avoid meaningless physical problem
*  boundary zones in the void phase : avoid meaningless physical problem



.. [1] https://gitlab.com/drti/muscat
