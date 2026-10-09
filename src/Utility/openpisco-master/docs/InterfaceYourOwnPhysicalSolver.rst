###################################
Interface your own physical solver
###################################

.. _InterfaceSolver:

This section aims to address concerns for a developer and does therefore require a deeper understanding of the underlying implementation details.
The aim of this section is to explain how to run a physical analysis using your own solver. Some aspects related to this question are already partially covered in the Physical Solvers documentation, it is strongly recommended to read it conjointly.

Principles
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As of now, there are five interfaces with physical solvers available.
All the current interfaces inherit from the same base class and involve the modules :py:mod:`OpenPisco.PhysicalSolvers` and :py:mod:`OpenPisco.ExternalTools`. In particular, they provide support for preprocessing and methods to handle the concept of auxiliary fields. In order to set these ideas down, here is an implementation example

.. code-block::

    from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
    import OpenPisco.PhysicalSolvers.FieldsNames as FN

    class MyAwesomeSolver(SolverBase):
        def __init__(self):
            super(MyAwesomeSolver,self).__init__()

            #Optional
            self.auxiliaryScalarGeneration[FN.MyScalar] = False
            self.auxiliaryFieldGeneration[FN.MyAuxiliaryField1][FN.Nodes] = False
            self.auxiliaryFieldGeneration[FN.MyAuxiliaryField2][FN.Nodes] = False

        def SolveByLevelSet(self, levelset):
            #Solve physical problem
            return RETURN_SUCCESS

        def GetNodalSolution(self):
            #Retrieve solution at nodes
            raise nodalSolution

        def GetAuxiliaryField(self,name,on):
            #Retrieve auxiliary field
            return auxiliaryFieldOn

        def GetAuxiliaryScalar(self,name):
            #Retrieve auxiliary scalar
            return auxiliaryScalar

In order to interface a new physical solver, a developer should redefine the behavior of few methods.

.. csv-table:: Methods to be redefined to interface a new physical solver
   :header: "Method", "Notes"
   :widths: 40, 40

   ":py:meth:`OpenPisco.PhysicalSolvers.SolverBase.SolveByLevelSet`", "run the analysis"
   ":py:meth:`OpenPisco.PhysicalSolvers.SolverBase.GetNodalSolution`", "retrieve the PDE solution at the nodes"
   ":py:meth:`OpenPisco.PhysicalSolvers.SolverBase.GetAuxiliaryField`", "(Optional) Allows a computed auxiliary field to be retrieved"
   ":py:meth:`OpenPisco.PhysicalSolvers.SolverBase.GetAuxiliaryScalar`", "(Optional) Allows a computed auxiliary scalar to be retrieved "


.. note::
    The GetAuxiliary methods implementation depends on your business logic. For instance, for the Code_Aster interface, auxiliary fields are written in the output file produced by Code_Aster. In that case, you have to fulfill accordingly the dictionary structure in the derived class constructor to specify which auxiliary quantities your solver can compute. The auxiliary fields are defined in the :py:mod:`OpenPisco.PhysicalSolvers.FieldsNames`.

In practice, the solver interface implementation offer therefore some leeway to the user, depending on whether there is a python API:

* Existing python API: Such is the case, for example, for the solver provided within the Muscat library. This is the most straightforward case to interface as there is no need to build a wrapper nor writing files on disk to use this solver. Not that the last point is not necessarily always true for all solver with a Python API, it turns out the mesh object used through the platform is also a Muscat object.

* No existing python API: the tool is available by means of an executable. The general procedure in this case is the following:

   a. Writing the input FE model: export the mesh and fields to be used as an input in a suitable format for the solver
   b. Business logic definition: Scripts relying on the solver Domain Specific Language (DSL) are generated, either from scratch or based on preexisting template available on the platform. If some auxiliary quantities were defined in the problem and need to be computed, it is to be handled.
   c. Run the problem: for instance, building the command line and execute it
   d. Post processing: retrieve the physical problem solution and auxiliary fields, if any

It is recommanded to have a look at the current solver implementations in the dedicated module :py:mod:`OpenPisco.PhysicalSolvers`:

* :py:class:`OpenPisco.PhysicalSolvers.AsterSolverBase`, and its derived classes and :py:class:`OpenPisco.ExternalTools.Aster.AsterInterface.AsterInterface`
* :py:class:`OpenPisco.PhysicalSolvers.StaticFreeFemSolverMeca.StaticFreeFemSolverMeca` and :py:class:`OpenPisco.ExternalTools.FreeFem.FreeFemInterface.FreeFemInterface`
* :py:class:`OpenPisco.PhysicalSolvers.OpenFoamSteadyBase`, and its derived classes and :py:class:`OpenPisco.ExternalTools.OpenFoam.OpenFoamInterface.OpenFoamInterface`
* :py:class:`OpenPisco.PhysicalSolvers.GeneralZSetSolverMeca.GeneralZSetPhysicalProblemMeca`
* :py:class:`OpenPisco.PhysicalSolvers.UnstructuredFEAGenericSolver`

Compatibility with OpenPisco and OpenPiscoCL applications
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The only action you have left is to define how the keywords used within the input file for the applications, relying on OpenPisco DSL, match your own implementation. We shall consider Code_Aster interfacing to illustrate that.
Consider the following code from the tutorial :ref:`Topology optimization of a cantilever beam using unstructured conformal level sets`

.. code-block::

    <PhysicalProblems>
        <GeneralAster type="static_elastic" id="1" >
            <Material  young="210.e9" poisson="0.3" />
            <Dirichlet eTag="eTag1" dofs="0 1 2" value="0.0"/>
            <Dirichlet eTag="eTag3" dofs="1" value="0.0"/>
            <Force nTag="nTag4" value="0. 0. -10000" />
        </GeneralAster>
    </PhysicalProblems>

Internally, this code becomes a mere dictionary given to a specific constructor function to initialize properly the physical problem (type of analysis, material properties, boundary conditions...). The function responsible to ensure that for Code_Aster is :py:func:`OpenPisco.PhysicalSolvers.GeneralAsterPhysicalSolver.CreateAsterPhysicalProblemMeca` and return an instance of the suitable physical problem, properly initialized.
As this part is completely decoupled from the rest of the input block of code above, with the exception of the id, there is no restriction whatsoever regarding the keywords you use for your own solver.

Now the last step is simply to add to the physical factory the mapping between the solver name, the associated class and associated constructor function. For instance

.. code-block::

    from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import RegisterClass
    RegisterClass("GeneralAster", AsterStatic,CreateAsterPhysicalProblemMeca)

where:

* "GeneralAster" is the name used in the input block of code above, for the applications
* AsterStatic is the actual class :py:class:`OpenPisco.PhysicalSolvers.AsterStatic` associated to the "static_elastic" analysis
* CreateAsterPhysicalProblemMeca, mentioned above, is the general constructor-like function for all physical analysis using Code_Aster. Note that its implementation depends on the underlying solver interface implementation.


