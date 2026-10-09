# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module is dedicated to the definition of a class providing the coupling between objective/contraints and their underlying physical problems
"""
from __future__ import annotations

from typing import Optional, Iterator, Iterable, List, Dict
import itertools
import os

import numpy as np

from Muscat.Helpers.CPU import GetNumberOfAvailableCores
from Muscat.Helpers.Logger import Info,Debug
from Muscat.MeshContainers.Mesh import Mesh

from OpenPisco import RETURN_SUCCESS
from OpenPisco.Optim.Problems.OptimProblemBase import OptimProblemBase
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
from OpenPisco.Optim.Criteria.Criteria import CriteriaBase
from OpenPisco.PhysicalSolvers.SolverBase import SolverBase

class OptimProblemConcrete(OptimProblemBase):
    def __init__(self, other:Optional[OptimProblemConcrete]=None, dataDeepCopy:bool=True):
        """OptimProblemConcrete constructor

        Parameters
        ----------
        other : Optional[OptimProblemConcrete], optional
            another instance of this very problem to copy attributes from, by default None.
            The copy concerns the objective criterion, the constraints criteria and the extra field and scalars
        dataDeepCopy : bool, optional
            whether to use deepcopy, by default True
        """
        super(OptimProblemConcrete, self).__init__(other)

        self.objectiveCriterion = None
        self.inequalityconstraintsCriteria = []
        self.equalityConstraintsCriteria = []
        self.inequalityconstraintsStatus = []
        self.equalityConstraintsStatus = []
        self.nbCpu = GetNumberOfAvailableCores()
        self.problemFields = []
        self.problemScalars = []

        if other is not None:
            self.SetObjectiveCriterion( type(other.objectiveCriterion)(other.objectiveCriterion) )

            for crit,status in zip(other.inequalityconstraintsCriteria,other.inequalityconstraintsStatus):
                self.AddInequalityConstraint(type(crit)(crit),status)
            for crit,status in zip(other.equalityConstraintsCriteria,other.equalityConstraintsStatus):
                self.AddEqualityConstraint(type(crit)(crit),status)

            for problem,fieldName,location in other.problemFields:
                self.AddProblemFields(problem=problem,fieldNames=[fieldName],locations=[location])
            for problem,scalarName in other.problemScalars:
                self.AddProblemScalars(problem=problem,scalarNames=[scalarName])
        else:
            self.CleanConstraints()

    def TakeValuesFrom(self,other:OptimProblemConcrete):
        """Retrieve values from another optimization problem

        Parameters
        ----------
        other : OptimProblemBase
            Another instance of OptimProblemBase
        """
        OptimProblemConcrete.__init__(self,other,True)

    def UpdateValues(self)->int:
        """Update the optimization problem. There are 3 steps
        
        1) Collect every unique physical problem instance amongst all the criteria
        2) Solve each unique physical problem once
        3) Update all criteria

        By doing so, we avoid to solve several times a physical problem shared by at least 2 criteria but requiring different postprocessing for the same solution 
        to a PDE. Then, it is the responsability of each criteria to compute their own values.

        Note that it implies that it is always possible to decouple the resolution of direct problems, associated to the physical problems, 
        and resolution of the adjoint problems/computation of the sensitivity, responsability of the criteria.
        
        Returns
        -------
        int
            parameter indicating if all the criteria update was successfull (typically 0)
        """
        uniquePhysicalProblems = self.RetrieveUniquePhysicalProblems(forSolving=True)
        self.SolvePhysicalProblems(uniquePhysicalProblems)
        res = self.UpdateAllCriteria()
        return res

    def VerifyOptimDomainValidity(self,support:Mesh,phi:np.ndarray)->bool:
        """Verify the domain is valid for each physical solvers. 
        The optimization problem neither does nor should know whether the considered level set function define a proper domain for the PDE of interest.
        For instance, in linear elasticity, it is not acceptable for the Dirichlet region and Neumann region to be disconnected. 
        
        Thus, it is the responsability of each solver to assess whether each domain is suitable. If it is not the case for at least one of them, the verification fails.

        Parameters
        ----------
        support : Mesh
            mesh support
        phi : np.ndarray
            levelset value

        Returns
        -------
        bool
            whether the domain is valid for all physical solvers
        """
        uniquePhysicalProblems = self.RetrieveUniquePhysicalProblems()
        for physicalProblem in uniquePhysicalProblems:
            physicalProblemValidityState = physicalProblem.VerifyPhysicalDomainValidity(support,phi)
            if not physicalProblemValidityState:
                Info("Physical problem "+str(physicalProblem)+" domain not valid")
                return False
        return True

    def RetrievePhysicalBoundaryFilters(self)->Iterator:
        """Retrieve physical boundary regions for all underlying physical problems

        Returns
        -------
        Iterator
            physical boundary regions for all underlying physical problems

        """
        uniquePhysicalProblems = self.RetrieveUniquePhysicalProblems()
        physicalBoundaryFilters = itertools.chain.from_iterable([ itertools.chain.from_iterable(physicalProblem.GetBoundaryConditionsFilters())
                                                 for physicalProblem in uniquePhysicalProblems])
        return physicalBoundaryFilters

    def RetrieveUniquePhysicalProblems(self,forSolving:bool=False)->set:
        """Retrieve unique physical problems. 
        
        Parameters
        ----------
        forSolving : bool
            As a side effect, the auxiliary quantities (fields/scalars) required by each criteria are set if forSolving is True, default is False        

        Returns
        -------
        set
            unique physical problems
        """
        optimProblemComponents=[self.objectiveCriterion]+self.GetConstraintsCriteria()

        uniquePhysicalProblems=set()
        def AddProblems(optimProblemComponent):
            if isinstance(optimProblemComponent, PhysicalCriteriaBase):
                if forSolving:
                    optimProblemComponent.SetAuxiliaryQuantities(self.point)
                uniquePhysicalProblems.add(optimProblemComponent.problem)
            if hasattr( optimProblemComponent, "internalCriteria"):
                for c in optimProblemComponent.internalCriteria:
                    AddProblems(c)                   
        for optimProblemComponent in optimProblemComponents:
            AddProblems(optimProblemComponent)
        return uniquePhysicalProblems

    def SolvePhysicalProblems(self,physicalProblems:set):
        """Solve physical problems
        Now, we solve each unique physical problems once in parallel for the moment kokkos on windows is configured using treads,
        this means we cant call muscat-kokkos routines inside a thread. We need to be in the main thread
        
        Parameters
        ----------
        physicalProblems : set
            Physical problems to be solved (problem are assumed to be distinct, thus a set)
        """
        Debug("In solve physical problems")
        self.PrepareToGenerateExtraQuantities()
        self.SolveAllProblems(physicalProblems)
        Debug("In solve physical problems done")

    def PrepareToGenerateExtraQuantities(self):
        for problem,field,location in self.problemFields:
            problem.SetAuxiliaryFieldGeneration(field,on=location)

        for problem,scalar in self.problemScalars:
            problem.SetAuxiliaryScalarGeneration(scalar)

    def SolveAllProblems(self,physicalProblems):
        #Now, we solve each unique physical problems once in parallel
        # for the moment kokkos on windows is configured using treads,
        # this means we cant call muscat-kokkos routines inside a thread .
        # We need to be in the main thread
        import os
        if self.nbCpu == 1 or len(physicalProblems) == 1 or os.name == 'nt':
            for physicalProblem in physicalProblems:
                result = physicalProblem.SolveByLevelSet(self.point)
                assert result== RETURN_SUCCESS,"Error solving " + str(physicalProblem)
        else:
            import concurrent.futures
            with concurrent.futures.ThreadPoolExecutor(max_workers=self.nbCpu) as executor:
                allDirectProblems = [
                                     executor.submit(physicalProblem.SolveByLevelSet, self.point)
                                     for physicalProblem in physicalProblems
                                     ]
                for problem in allDirectProblems:
                    assert problem.result() == RETURN_SUCCESS,"Error solving " + str(problem)

    def UpdateAllCriteria(self)->int:
        """Update all criteria, one by one

        Returns
        -------
        int
            parameter indicating if all the criteria update was successfull (typically 0)
        """
        Debug("In update all criteria")
        res = self.objectiveCriterion.UpdateValues(self.point)
        if res == RETURN_SUCCESS:
            for Criterion in self.GetConstraintsCriteria():
                res = Criterion.UpdateValues(self.point)
                if res != RETURN_SUCCESS:
                    print("Error on the criterion " + str(Criterion.GetName())  )
                    print(str(Criterion)  )
                    break
        else:
            print("Error on the Objective Criterion" + str(self.objectiveCriterion.GetName())  )
            print(str(self.objectiveCriterion)  )
        Debug("In update all criteria done")
        return res

    def UpdateProblemWhenIterationAccepted(self):
        """Update optimization problem criteria when iteration accepted (only required for very specific criteria)"""
        self.objectiveCriterion.UpdateCriteriaWhenIterationAccepted()
        for criterion in self.GetConstraintsCriteria():
           criterion.UpdateCriteriaWhenIterationAccepted()

    def GetConstraintsCriteria(self)->List[CriteriaBase]:
        """Retrieve contraints criteria

        Returns
        -------
        List[CriteriaBase]
            contraints criteria
        """
        return self.inequalityconstraintsCriteria+self.equalityConstraintsCriteria

    def GetNumberOfSolutions(self)->int:
        """Compute number of solutions, the sum of the number of solution for each criteria

        Returns
        -------
        int
            number of solution
        """
        n = self.objectiveCriterion.GetNumberOfSolutions()
        for crit in self.GetConstraintsCriteria():
            n += crit.GetNumberOfSolutions()
        n +=len(self.problemFields)
        return n

    def GetSolutionName(self,idx:int)->str:
        """Retrieve solution name

        Let ni, ne and nf be respectively the number of inequality constraints, equality constraints and extra fields
        Let sij, sek, sl be respectively the number of solutions for the inequality constraint criterion j, for the equality constraint criterion k 
        and for the objective criterion.

        If we were to represent the whole collection of solutions within the problem is the correct order with pseudo-code, we would get

        solutions = [obj_crit_1 ...  obj_crit_sl] 
                    + [ineq_crit_1_1 ... ineq_crit_1_si1]
                    + [ineq_crit_2_1 ... ineq_crit_2_si2]
                    + ...
                    + [ineq_crit_ni_1 ... ineq_crit_ni_sini]
                    + [eq_crit_1_1 ... eq_crit_1_se1]
                    + [eq_crit_2_1 ... eq_crit_2_se2]
                    + ...
                    + [eq_crit_ne_1 ... eq_crit_ne_sene]
                    + [xtra_1 ... xtra_nf]

        Parameters
        ----------
        idx : int
            id of the solution to be retrieved within the array solutions described above
        
        Returns
        -------
        str
            solution name, depending on the type of entit.
            A prefix is added to the name to be able to associated the solution with the corresponding internal variables:
            -"O_": objective
            -"C_": inequality constraint
            -"EC_": equality constraint
            -"XF_": extra field

        Raises
        ------
        ValueError
            if idx >=len(solutions)
        """
        internalcpt = 0
        for i in range(self.objectiveCriterion.GetNumberOfSolutions()):
            if internalcpt == idx:
                return "O_" + self.objectiveCriterion.GetSolutionName(i)
            internalcpt += 1

        for crit,_ in zip(self.inequalityconstraintsCriteria,range(len(self.inequalityconstraintsCriteria))):
            for i in range(crit.GetNumberOfSolutions()):
                if internalcpt == idx:
                    return f"C_{crit.GetName()}_{crit.GetSolutionName(i)}"
                internalcpt += 1

        for crit,_ in zip(self.equalityConstraintsCriteria,range(len(self.equalityConstraintsCriteria))):
            for i in range(crit.GetNumberOfSolutions()):
                if internalcpt == idx:
                     return f"EC_{crit.GetName()}_{crit.GetSolutionName(i)}"
                internalcpt += 1

        for _,fieldName,location in self.problemFields:
            if internalcpt == idx:
                return f"XF_{fieldName}_{location}"
            internalcpt += 1

        raise ValueError("SolutionName "+str(idx)+" not avilable ")

    def GetSolution(self,idx:int)->np.ndarray:
        """Retrieve solution value

        Let ni, ne and nf be respectively the number of inequality constraints, equality constraints and extra fields
        Let sij, sek, sl be respectively the number of solutions for the inequality constraint criterion j, for the equality constraint criterion k 
        and for the objective criterion.

        If we were to represent the whole collection of solutions within the problem is the correct order with pseudo-code, we would get

        solutions = [obj_crit_1 ...  obj_crit_sl] 
                    + [ineq_crit_1_1 ... ineq_crit_1_si1]
                    + [ineq_crit_2_1 ... ineq_crit_2_si2]
                    + ...
                    + [ineq_crit_ni_1 ... ineq_crit_ni_sini]
                    + [eq_crit_1_1 ... eq_crit_1_se1]
                    + [eq_crit_2_1 ... eq_crit_2_se2]
                    + ...
                    + [eq_crit_ne_1 ... eq_crit_ne_sene]
                    + [xtra_1 ... xtra_nf]

        Parameters
        ----------
        idx : int
            id of the solution to be retrieved within the array solutions described above

        Returns
        -------
        np.ndarray
            solution corresponding to idx

        Raises
        ------
        ValueError
            if idx >=len(solutions)
        """
        internalcpt = 0
        for i in range(self.objectiveCriterion.GetNumberOfSolutions()):
            if internalcpt == idx:
                return self.objectiveCriterion.GetSolution(i)
            internalcpt += 1

        for crit in self.GetConstraintsCriteria():
            for i in range(crit.GetNumberOfSolutions()):
               if internalcpt == idx:
                   return crit.GetSolution(i)
               internalcpt += 1

        for problem,fieldName,location in self.problemFields:
            if internalcpt == idx:
                return problem.GetAuxiliaryField(fieldName,on=location)
            internalcpt += 1

        raise ValueError("Solution "+str(idx)+" not available ")

    def GetScalarsOutputs(self)->Dict:
        """Retrieve scalar outputs

        A prefix is added to the name to be able to associate the scalar with the corresponding internal variables:
        -"O_": objective
        -"IC_": inequality constraint
        -"EC_": equality constraint
        -"XS_": extra scalar
        
        Returns
        -------
        Dict
            scalar output value by scalar name
        """
        res = dict()
        for name,value in self.objectiveCriterion.GetScalarsOutputs().items():
            res["O_"+ name] = value

        for crit,ccpt in zip(self.inequalityconstraintsCriteria,range(len(self.inequalityconstraintsCriteria))):
            for name,value in crit.GetScalarsOutputs().items():
                res["IC"+ str(ccpt)+"_"+name] = value

        for crit,ccpt in zip(self.equalityConstraintsCriteria,range(len(self.equalityConstraintsCriteria))):
            for name,value in crit.GetScalarsOutputs().items():
                res["EC"+ str(ccpt)+"_"+name] = value

        for problem,scalarName,ccpt in zip(self.problemScalars,range(len(self.problemScalars))):
            res["XS"+ str(ccpt)+"_"+scalarName] = problem.GetAuxiliaryScalar(scalarName)

        return res

    def SetObjectiveCriterion(self,objCrit:CriteriaBase):
        """Set objective criterion

        Parameters
        ----------
        objCrit : CriteriaBase
            objective criterion
        """
        self.objectiveCriterion = objCrit

    def GetObjectiveFunctionName(self)->str:
        """Retrieve objective function name

        Returns
        -------
        str
            objective function name
        """
        return self.objectiveCriterion.GetName()

    def GetObjectiveFunctionVal(self)->float:
        """Get objective function value

        Returns
        -------
        float
            objective function value
        """
        return self.objectiveCriterion.GetValue()

    def GetObjectiveFunctionSensitivity(self)->np.ndarray:
        """Get objective function sensitivity

        Returns
        -------
        np.ndarray
            objective function sensitivity
        """
        return self.objectiveCriterion.GetSensitivity()

    def CleanConstraints(self):
        """Clean constraints"""
        self.inequalityconstraintsCriteria = []
        self.inequalityconstraintsStatus = []

    def AddInequalityConstraint(self,objCrit:CriteriaBase,status:bool=True):
        """Add inequality constraint

        Parameters
        ----------
        objCrit : CriteriaBase
            inequality constraint criterion
        status : bool, optional
            whether it is considered active, by default True
        """
        self.inequalityconstraintsCriteria.append(objCrit)
        self.inequalityconstraintsStatus.append(status)

    def AddEqualityConstraint(self,objCrit:CriteriaBase,status:bool=True):
        """Add equality constraint

        Parameters
        ----------
        objCrit : CriteriaBase
            equality constraint criterion
        status : bool, optional
            whether it is considered active, by default True
        """
        self.equalityConstraintsCriteria.append(objCrit)
        self.equalityConstraintsStatus.append(status)

    def AddProblemFields(self,problem:SolverBase,fieldNames:Iterable,locations:Iterable[str]):
        """Add fields to be computed for a physical problem

        Parameters
        ----------
        problem : SolverBase
            physical problem
        fieldNames : Iterable
            collection of fields to be computed
        locations : Iterable[str]
            location where field are computed, size should be consistant with size of fieldNames
        """
        for fieldName,location in zip(fieldNames,locations):
            self.problemFields.append((problem,fieldName,location))

    def AddProblemScalars(self,problem:SolverBase,scalarNames:Iterable):
        """Add scalars to be computed for a physical problem

        Parameters
        ----------
        problem : SolverBase
            physical problem
        scalarNames : Iterable
            collection of scalar to be computed
        """
        for scalarName in scalarNames:
            self.problemScalars.append((problem,scalarName))

    def GetNumberOfConstraints(self)->int:
        """Compute total number of constraints, sum of number of inequality/equality constraints

        Returns
        -------
        int
            total number of constraints
        """
        return self.GetNumberOfInequalityConstraints()+self.GetNumberOfEqualityConstraints()

    def GetNumberOfInequalityConstraints(self)->int:
        """Compute number of inequality constraints

        Returns
        -------
        int
            number of inequality constraints
        """
        return len(self.inequalityconstraintsCriteria)

    def GetNumberOfEqualityConstraints(self)->int:
        """Compute number of equality constraints

        Returns
        -------
        int
            number of equality constraints
        """
        return len(self.equalityConstraintsCriteria)

    def SetInequalityConstraintStatus(self,idx:int,status:bool):
        """Set inequality constraint status

        Parameters
        ----------
        idx : int
            inequality constraint id
        status : bool
            whether constraint is active
        """
        self.inequalityconstraintsStatus[idx] = status

    def GetInequalityConstraintStatus(self,idx:int)->bool:
        """Get inequality constraint status

        Parameters
        ----------
        idx : int
            inequality constraint id

        Returns
        -------
        status : bool
            whether constraint is active
        """
        return self.inequalityconstraintsStatus[idx]

    def SetEqualityConstraintStatus(self,idx:int,status:bool):
        """Set equality constraint status

        Parameters
        ----------
        idx : int
            equality constraint id
        status : bool
            whether constraint is active
        """
        self.equalityConstraintsStatus[idx] = status

    def GetEqualityConstraintStatus(self,idx:int)->bool:
        """Get equality constraint status

        Parameters
        ----------
        idx : int
            equality constraint id

        Returns
        -------
        status : bool
            whether constraint is active
        """
        return self.equalityConstraintsStatus[idx]

    def GetInequalityConstraintName(self,idx:int)->str:
        """Get inequality constraint name

        Parameters
        ----------
        idx : int
            inequality constraint id

        Returns
        -------
        str
            inequality constraint name
        """
        return self.inequalityconstraintsCriteria[idx].GetName()

    def GetEqualityConstraintName(self,idx:int)->str:
        """Get equality constraint name

        Parameters
        ----------
        idx : int
            equality constraint id

        Returns
        -------
        str
            equality constraint name
        """
        return self.equalityConstraintsCriteria[idx].GetName()

    def GetInequalityConstraintVal(self,idx:int)->float:
        """Get inequality constraint value

        Parameters
        ----------
        idx : int
            inequality constraint id

        Returns
        -------
        float
            inequality constraint value
        """
        return self.inequalityconstraintsCriteria[idx].GetValue()

    def GetEqualityConstraintVal(self,idx:int)->float:
        """Get equality constraint value

        Parameters
        ----------
        idx : int
            equality constraint id

        Returns
        -------
        float
            equality constraint value
        """
        return self.equalityConstraintsCriteria[idx].GetValue()

    def GetInequalityConstraintValOriginal(self, idx:int)->float:
        """Get inequality constraint value

        Parameters
        ----------
        idx : int
            inequality constraint id

        Returns
        -------
        float
            inequality constraint value
        """
        return self.inequalityconstraintsCriteria[idx].GetValue()

    def GetInequalityConstraintUpperBoundOriginal(self,i:int)->float:
        """Get inequality constraint upperbound value

        Parameters
        ----------
        i : int
            inequality constraint id

        Returns
        -------
        float
            inequality constraint upperbound value
        """
        return self.inequalityconstraintsCriteria[i].GetUpperBound()

    def GetInequalityConstraintSensitivityVal(self,idx:int)->np.ndarray:
        """Get inequality constraint sensitivity value

        Parameters
        ----------
        idx : int
            inequality constraint id

        Returns
        -------
        np.ndarray
            inequality constraint sensitivity value
        """
        return self.inequalityconstraintsCriteria[idx].GetSensitivity()

    def GetEqualityConstraintSensitivityVal(self,idx:int)->np.ndarray:
        """Get equality constraint sensitivity value

        Parameters
        ----------
        idx : int
            equality constraint id

        Returns
        -------
        np.ndarray
            equality constraint sensitivity value
        """
        return self.equalityConstraintsCriteria[idx].GetSensitivity()

    def GetInequalityConstraintSlacks(self)->Dict:
        """Get inequality constraint slacks (difference between constraint value and upperbound)

        Returns
        -------
        Dict
            inequality constraint slacks
        """
        return {self.GetInequalityConstraintName(i):(self.GetInequalityConstraintValOriginal(i) - self.GetInequalityConstraintUpperBoundOriginal(i)) for i in range(self.GetNumberOfInequalityConstraints())}

    def GetInequalityConstraintUpperBound(self,i:int)->float:
        """Get inequality constraint upperbound value

        Parameters
        ----------
        i : int
            inequality constraint id

        Returns
        -------
        float
            inequality constraint upperbound value
        """
        return self.inequalityconstraintsCriteria[i].GetUpperBound()

    def GetEqualityConstraintTargetValue(self,i:int)->float:
        """Get equality constraint target value

        Parameters
        ----------
        i : int
            inequality constraint id

        Returns
        -------
        float
            equality constraint target value
        """
        return self.equalityConstraintsCriteria[i].GetTargetValue()

    def __str__(self)->str:
        """Retrieve string representation of class (objective/constraint, status, target value/upperbound)

        Returns
        -------
        str
            string representation of class
        """
        sInfo="Instance of "+type(self).__name__+"\n"
        if self.objectiveCriterion is not None:
            sInfo+="Objective criterion: "+self.GetObjectiveFunctionName()+"\n"
        sInfo+="Number of inequality constraint(s): "+str(self.GetNumberOfInequalityConstraints())+"\n"
        for inequalityConstraintCriteria,inequalityConstraintStatus in zip(self.inequalityconstraintsCriteria,self.inequalityconstraintsStatus):
            sInfo+="\t Name: "+str(inequalityConstraintCriteria.GetName())
            sInfo+=", Upperbound: "+str(inequalityConstraintCriteria.GetUpperBound())
            sInfo+=", Status: "+str(inequalityConstraintStatus)+"\n"
        sInfo+="Number of equality constraint(s): "+str(self.GetNumberOfEqualityConstraints())+"\n"
        for equalityConstraintCriteria,equalityConstraintStatus in zip(self.equalityConstraintsCriteria,self.equalityConstraintsStatus):
            sInfo+="\t Name: "+str(equalityConstraintCriteria.GetName())
            sInfo+=", TargetValue: "+str(equalityConstraintCriteria.GetTargetValue())
            sInfo+=", Status: "+str(equalityConstraintStatus)+"\n"
        return sInfo

def CheckIntegrity():
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
