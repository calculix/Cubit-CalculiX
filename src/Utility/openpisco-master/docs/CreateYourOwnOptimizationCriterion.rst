####################################################
Create your own optimization criterion 
####################################################

.. _Create your own optimization criterion:

This section aims to address concerns for a developer and does therefore require a deeper understanding of the underlying implementation details.
The aim of this section is to explain how to implement your own optimization criterion.
We strongly recommend to read the documentation related to the criteria conjointly.

Principles
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All the criteria are available in the module :py:mod:`OpenPisco.Optim.Criteria`. For practical purposes, there are two main types of criteria considered:

* Geometrical: inherit from base class :py:class:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase`
* Physical: inherit from base class :py:class:`OpenPisco.Optim.Criteria.Criteria.PhysicalCriteriaBase`, which also inherit from :py:class:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase`

The main difference between them is the endowed physical problem(s) :py:attr:`OpenPisco.Optim.Criteria.Criteria.PhysicalCriteriaBase.problem` within a physical criterion, which must be run in order to compute the value of the criteria. Typically, its value arises from a postprocessing of the underlying solution to a PDE systems, meaning that such an attribute is an instance of a physical solver supported by the platform.
For the sake of completeness, note that a geometrical criterion could also involve the resolution of a PDE problem, as long as it is related to the actual shape of the structure to be optimized.

In order to illustrate that, here is an implementation example for a typical physical criterion

.. code-block::

    from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass

    class MyAwesomeCriteria(PhysicalCriteriaBase):
        def __init__(self, other=None):
            super(MyAwesomeCriteria).__init__(other)

            if other is None:
                self.SetName("MyAwesomeTopoCriteria")
                self.instanceParameter = 0
            else:
                self.instanceParameter = other.instanceParameter

        def UpdateValues(self,levelSet):
            #Compute criteria value
            self.f_val=criteriaValue
            #Compute criteria sensitivity
            self.fSensitivity_val=sensitivityValue
            return RETURN_SUCCESS

        def GetNumberOfSolutions(self):
            #Provide numberOfSolution
            return numberOfSolution

        def GetSolution(self,i):
            if i >= self.GetNumberOfSolutions():
                raise(Exception("i out of bounds "))

            #Retrieve solution i
            return ithSolution

        def GetSolutionName(self,i):
            if i >= self.GetNumberOfSolutions():
                raise(Exception("i out of bounds "))

            #Retrieve ith solution name
            return ithSolutionName

Before proceeding, the concept of solutions within a criterion should be explained. A given criterion can involve several fields of interest, including the underlying solutions to the PDE system hidden in the :py:attr:`OpenPisco.Optim.Criteria.Criteria.PhysicalCriteriaBase.problem` which should be the first field of interest to consider in the general case.
Throught the optimization process, it can be interesting to export such fields in order to observe how the evolution on the shape impacts them; this is why we provide this possibility for each criteria.
Reading the other mecanism present in the constructor, its purpose is to enable the copy of an instance-defined constant parameter throught the topology optimization process.

The constraints and responsibilities for the developer are as follows:

* Redefinition of the behavior of :py:meth:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase.UpdateValues`: it is the main method. This method is the one actually called by the optimization problem (:py:meth:`OpenPisco.Optim.Problems.OptimProblemConcrete.UpdateValues`) to update the criteria values/sensitivities. Note that it must return the criterion state at the end of the computation, here RETURN_SUCCESS, if it suceeds.

* Redefine the behavior of :py:meth:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase.GetNumberOfSolutions`: return the number of field of interest you would like to export

* Redefine the behavior of :py:meth:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase.GetSolution`: return a field of interest (array) you would like to export for a given index

* Redefine the behavior of :py:meth:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase.GetSolutionName`: return the name of a field of interest (array) you would like to export for a given index

In order to add a new optimization criterion, a developer should redefine the behavior of few methods.

.. csv-table:: Methods to be redefined to add a new optimization criterion
   :header: "Method", "Notes"
   :widths: 40, 40

   ":py:meth:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase.UpdateValues`", "Update the criteria value/sensitivities "
   ":py:meth:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase.GetNumberOfSolutions`", "return the number of field (int) of interest to export "
   ":py:meth:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase.GetSolution`", "return a field of interest (array) to export "
   ":py:meth:`OpenPisco.Optim.Criteria.Criteria.CriteriaBase.GetSolutionName`", "return the name (str) of a field of interest (array) to export"


.. note::
    The number of solutions provided should be consistent between these three methods (same number of names, same number of fields and correct number of solution provided).


Compatibility with OpenPisco and OpenPiscoCL applications
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let us consider an example from the tutorial :ref:`Topology optimization of a cantilever beam using unstructured conformal level sets` with the compliance criterion.

.. code-block::

    <OptimProblems>
        <OptimProblem type="TopoGeneric" id="1"
            OnZone="8"
            useLevelset="1"
        printMeshQualityInfo = "True"
            output="3"
            outputEveryLevelsetUpdate = "False">
          <Objective  type="Volume"  name="Volume"/>
          <Constraint type="Compliance"  name="Compliance" UpperBound="2.5*4.69434000e-01"   useProblem="1" />
       </OptimProblem>
    </OptimProblems>

In order to enable the use of a criterion in the application, in the :py:mod:`OpenPisco.Optim.Criteria.PhyCriteria` module, we add the mapping between the criterion name and the associated class to the criteria factory.

.. code-block::

    from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
    RegisterCriteriaClass("Compliance",TopoCriteriaCompliance)

where:

* "Compliance" is the name used in the input block of code above, for the applications
* TopoCriteriaCompliance is the class :py:class:`OpenPisco.Optim.Criteria.PhyMecaCriteria.TopoCriteriaCompliance` associated to the compliance criteria

Note only the simplest configuration is covered above; it is implicitly assumed that the name of the criterion is enough to build the associated criterion instance. Let us consider another criterion in order to see how to handle more complex cases

.. code-block::

    <OptimProblems>
        <OptimProblem type="TopoGeneric" id="1"
            OnZone="8"
            useLevelset="1"
        printMeshQualityInfo = "True"
            output="3"
            outputEveryLevelsetUpdate = "False">
          <Objective  type="Volume"  name="Volume"/>
          <Constraint type="NodalTargetDisp"  name="NodalTargetDisp" TargetValue="1.e-3" useProblem="1" dir="0 0 1" u0="0.5" nTag="NT3"/>
       </OptimProblem>
    </OptimProblems>

Just like the physical solver, such a block of code becomes internally a mere dictionary. The values within it have to be passed to the criterion instance for its proper initialization. In order to do that, unlike the previous case, we need the associated constructor-like function. For instance

.. code-block::

    from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
    RegisterCriteriaClass("NodalTargetDisp", TopoCriteriaNodalTargetDisp,CreateTopoCriteriaNodalTargetDisp)

where:

* "NodalTargetDisp" is the name used in the input block of code above, for the applications
* TopoCriteriaNodalTargetDisp is the class :py:mod:`OpenPisco.Optim.Criteria.PhyMecaCriteria.TopoCriteriaNodalTargetDisp` associated to the nodal target displacement criteria
* CreateTopoCriteriaNodalTargetDisp, mentioned above, is the general constructor-like function for the criterion considered. Note that its implementation (:py:func:`OpenPisco.Optim.Criteria.PhyMecaCriteria.CreateTopoCriteriaNodalTargetDisp`) depends on the underlying solver interface implementation.

Finally, make sure your new file is imported in :py:mod:`OpenPisco/Optim/Criteria/CriteriaFactory.py` within the :py:func:`InitAllCriteria` function, so the factory is aware of it at startup.