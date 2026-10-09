##############################################
Interface your own optimization algorithm
##############################################

.. _Interface your own optimization algorithm:

This section aims to adress concerns for a developer and does therefore require a deeper understanding of the underlying implementation details.

How to add your own optimization algorithm
=======================================================================

Assuming the optimization algorithms available within the platform do not fit your current need, for instance if you have at your disposal a tailored optimization algorithm and feel the urge to use it instead, you can implement your own.

Principles
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As of now, there are two optimization algorithms devoted to solve constrained optimization problems.

All the current interfaces inherit from the same base class :py:class:`OpenPisco.Optim.Algorithms.OptimAlgoBase`. In particular, as the loop over the optimization iterations is already handled in :py:meth:`OpenPisco.Optim.Algorithms.OptimAlgoBase.Start`, the method :py:meth:`OpenPisco.Optim.Algorithms.OptimAlgoBase.DoOneStep` should be redefined as well as the other methods called within :py:meth:`OpenPisco.Optim.Algorithms.OptimAlgoBase.Start` if required.

For a given optimization algorithm, one would expect the following implementation

.. code-block::

    from OpenPisco.Optim.Algorithms.OptimAlgoBase import OptimAlgoBase

    class MyAwesomeOptimAlgo(OptimAlgoBase):
        def __init__(self):
            super(MyAwesomeOptimiAlgo).__init__()

        def Initialize(self):
            #Optional: Computation before the first step of the algorithm

        def StatusFromResult(self,resultVal):
            #Optional: Describe the status of the iteration if the algorithm fails

        def Finalize(self):
            #Optional: Computation after the last step of the algorithm

        def DoOneStep(self):
            # Principal steps:
            #- Update the optimization problem
            #- Retrieve values/gradients from the optimization problem
            #- Aggregate each contributions to compute the descent direction/step size
            #- Call TryToAdvance
            #- Update the step size accordingly

        def TryToAdvance(self):
            # Principal steps:
            #- Call Advance (base) method
            #- Update the optimization problem
            #- Assess shape performances and accept it if the results are satisfactory


Note that, if your intent is to interface an existing algorithm from another library, there is a strong constraint. Its API should allow the computation of a descent direction based on the optimization problem criteria values/gradients and for a single optimization iteration (third step described in DoOneStep); this is mandatory.
In this context, there are several constraints inherent to topology optimization, such as:

* The need to assess whether the "advanced shape" is acceptable at each iteration, which is beyond the scope of the pure optimization problem and can not therefore be done easily "outside"

* The new shape physical evaluation, in order to compare its physical performance with repect to the former shape

If the aforementioned algorithm does not provide enough control to allow that, it is unlikely to be used for topology optimization problems within the platform.

It is recommended to have a look at the other algorithm implementations in the dedicated module :py:mod:`OpenPisco.Optim.Algorithms`:

* :py:class:`OpenPisco.Optim.Algorithms.OptimAlgoNullSpace`
* :py:class:`OpenPisco.Optim.Algorithms.OptimAlgoUnconstrained`


Compatibility with OpenPisco and OpenPiscoCL applications
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The principle is quite similar to the one described for the physical solver and criterion. For instance, in the illustrative case presented above, it would be

.. code-block::

    from OpenPisco.Optim.Algorithms.OptimAlgoFactory import RegisterClass as RegisterOptimAlgoClass
    RegisterOptimAlgoClass("MyAwesomeOptimAlgo", MyAwesomeOptimAlgo,CreateMyAwesomeOptimAlgo)

where:

* "MyAwesomeOptimAlgo" is the name used in the input block for the applications
* MyAwesomeOptimAlgo is your optimization algorithm class :py:class:`OpenPisco.Optim.Algorithms.MyAwesomeOptimAlgo`
* CreateMyAwesomeOptimAlgo is the general constructor-like function for your optimization algorithm.