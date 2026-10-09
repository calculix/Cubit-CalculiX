######################################################################
Interface a physical solver with shape optimization capabilities
######################################################################

This section aims to address concerns for a developer and does therefore require a much deeper understanding of the underlying implementation details.
While we have already covered physical solver interfacing in :ref:`Interface your own physical solver <InterfaceSolver>`. documentation, there is a special case not adressed so far.
Some high-performance external physical solvers already provide bricks related to shape optimization. For instance, one can think of solvers with adjoint-computing capabilities able to provide the sensitivity field for a specific criteria/direct problem combination.
While such solver may come with a full shape optimization framework on its own, it can be interesting to leverage on some of their core features to be able to use the level-set approach. 
In other words, being able to do so constitues a form of complementary coupling between OpenPisco and a solver to handle a physical setting not available in OpenPisco.
The aim of this section is to explain how to extend the capabilities of both OpenPisco and a solver. It is still recommanded to read it conjointly with the "Interface your own physical solver" documentation.

Note that the actual implementation is strongly link to the business logic of the underlying physical solver considered. 
Thus, we merely provide pointers/guideline regarding the coupling principles and illustrate it with a fully operational example without diving too much into the solver itself.

Without loss of generality, we assume in what follows the case of a single criterion associated to a direct physical problem handled by a physical solver with shape optimization capabilities.
With these assumptions, the classical scheme would be:

* Solve the direct problem to obtain the direct state
* Computate the criterion value
* Solve the adjoint problem, if any
* Compute the sensitivity by combining the direct and adjoint states

However, several aspects have to be adressed to be compatible with the current software design of OpenPisco. As the solver is likely to require both the direct problem and criterion caracteristics, such a constraint has to be taken into account in the implementation.

Custom criterion implementation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By design, the current architecture of OpenPisco assumes that the direct state and ajoint state can be separately computed in a modular way. In what follows, we describe a strategy for the worst case scenario where it is not possible to do so.
Recall that, when updating the optimization problem, OpenPisco solve once each unique physical problem associated to the criteria subjected to optimization and then update each criteria accordingly. 

Thus, one can redefine the ":py:meth:`OpenPisco.Optim.Criteria.PhysicalCriteriaBase.SetAuxiliaryQuantities`" method to:

* Ask the criteria to be added to the physical problem.
* Consider the sensitivity field as a regular nodal auxiliary field for the solver and ask the solver to compute it. 

Doing so implies that the solver can "see" all the required caracteristics of the criteria (type that can be handled by the solver, parameters). It is convenient to ask the solver to compute the criteria value, by setting it as an auxiliary scalar and relying on the solver capabilities, but it is not mandatory.
Note that the computation of any adjoint state is then hidden as it becomes the responsability of the physical solver. To retrieve the sensitivity value, one can simply retrieve the associated auxiliary field from the solver.

It can also be interesting to retrieve all the states computed by the solver, including adjoint states, by defining properly the ":py:meth:`OpenPisco.Optim.Criteria.PhysicalCriteriaBase.GetCriteriaSolution`" method.

It is recommanded to have a look at the currently available criterion implementation in the dedicated module :py:mod:`OpenPisco.Optim.Criteria.PhyFluidCriteria`:

"Augmented" Physical solvers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Comparing to a regular physical solver than only handle the direct problem, there are several changes:

* Implementation of the method to add a criterion to the solver
* Augment the ":py:meth:`OpenPisco.PhysicalSolvers.SolverBase.SolveByLevelSet`" method with the computation of the sensitivity (and adjoint state computation if any)

In each case, the implementation relies on the business logic of the solver at hand.

It is recommanded to have a look at the currently available solver implementation in the dedicated module :py:mod:`OpenPisco.PhysicalSolvers.OpenFoamSteadyStateComplete`:
