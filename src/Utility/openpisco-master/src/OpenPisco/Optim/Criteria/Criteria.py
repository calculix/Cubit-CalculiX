# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module provides the basic structure of a topology optimization criterion
"""
from __future__ import annotations

import abc
from functools import partial
import copy
from typing import Optional,Dict,Union

import numpy as np

import Muscat.Helpers.ParserHelper as PH

from OpenPisco.LevelSetBase import LevelSetBase
from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
from OpenPisco import RETURN_SUCCESS

class CriteriaBase(metaclass=abc.ABCMeta):
    """Base class for criteria, to be redefined by derived class
    
    Parameters
    ----------
    upperBound : float
        upperbound value for inequality constraints
    targetValue : float
        target value for equality constraints

    """
    def __init__(self, other:Optional[CriteriaBase]=None):
        """Criteria base constructor

        Parameters
        ----------
        other : Optional[CriteriaBase], optional
            another instance of this very criteria to copy attributes from, by default None
        """
        super(CriteriaBase, self).__init__()

        self.__name__ = None
        self.upperBound = np.inf
        self.targetValue = np.inf

        if other is not None:
            self.__name__ = other.__name__
            self.upperBound = other.upperBound
            self.targetValue = other.targetValue
            self.SetName(other.GetName())

    def SetName(self,name:str):
        """Set criterion name

        Parameters
        ----------
        name : str
            criterion name
        """
        self.__name__ = name

    def SetUpperBound(self,bound:float):
        """Set upper bound for inequality constraint

        Parameters
        ----------
        bound : float
            upper bound value
        """
        self.upperBound = PH.ReadFloat(bound)

    def GetUpperBound(self)->float:
        """Get upper bound value

        Returns
        -------
        float
            upper bound value
        """
        return self.upperBound

    def SetTargetValue(self,value:float):
        """Set target value (equality constraint)

        Parameters
        ----------
        value : float
            target value
        """
        self.targetValue = PH.ReadFloat(value)

    def GetTargetValue(self)->float:
        """Get target value (equality constraint)

        Returns
        -------
        float
            target value
        """
        return self.targetValue

    def GetName(self)->str:
        """Get criterion name

        Returns
        -------
        str
            criterion name
        """
        return self.__name__

    def GetValue(self)->float:
        """Get the current value of the criterion

        Returns
        -------
        float
            current value of the criterion
        """
        return self.f_val

    def GetSensitivity(self)->np.ndarray:
        """Get the current value of the sensitivity

        Returns
        -------
        np.ndarray
            current value of the sensitivity
        """
        return self.fSensitivity_val

    @abc.abstractmethod
    def UpdateValues(self,point:LevelSetBase)->int:
        """Update value and sensitivity of the criterion for the current levelset

        Parameters
        ----------
        point : LevelSetBase
            current level set

        Returns
        -------
        int
            parameter indicating if the criterion update was successfull (typically 0)
        """
        return 

    def GetNumberOfSolutions(self)->int:
        """Retrieve the number of solutions of the current criterion

        Returns
        -------
        int
            number of solutions
        """
        return 0
     
    def AssertSuitableSolutionId(self,i:int):
        """Assert solution id is suitable (lower than the actual number of solutions)

        Parameters
        ----------
        i : int
            solution id to check
        """
        assert i < self.GetNumberOfSolutions(),"i out of bounds "

    def GetScalarsOutputs(self)->Dict:
        """Get scalar outputs

        Returns
        -------
        Dict
            scalar outputs
        """
        return dict()

    def UpdateCriteriaWhenIterationAccepted(self):
        """Update a criteria when an iteration is accepted (in most cases, nothing to do in derived classes)"""
        pass

class PhysicalCriteriaBase(CriteriaBase):
    """Physical criteria base class.
    Compared to base class usually endowed with a physical problem instance and an adjoint solver, also a physical problem instance if any.
    The underlying assumption behind such a choice is that the implementation is most of the time tailored for a specific physics, 
    even though it may work with several different solver, providing they solve the same problem.

    Parameters
    ----------
    problem : SolverBase
        physical problem instance
    adjointProblem: SolverBase
        adjoint problem instance 
    """
    def __init__(self, other:Optional[PhysicalCriteriaBase]=None):
        """PhysicalCriteria base constructor

        Parameters
        ----------
        other : Optional[PhysicalCriteriaBase], optional
            another instance of this very criteria to copy attributes from, by default None
        """
        super(PhysicalCriteriaBase, self).__init__(other)
        self.problem = None
        self.adjointProblem = None

        if other is not None:
            self.problem = other.problem
            self.adjointProblem = other.adjointProblem

    def GetSolution(self,i:int)->np.ndarray:
        """Get criteria solution

        Parameters
        ----------
        i : int
            solution to retrieve index

        Returns
        -------
        np.ndarray
            solution value
        """
        self.AssertSuitableSolutionId(i)
        return self.GetCriteriaSolution(i)

    @abc.abstractmethod
    def GetCriteriaSolution(self,i:int)->np.ndarray:
        """Retrieve the solution of index i. The index i should be smaller than the total number of solutions

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        np.ndarray
            solution value
        """
        return

    def GetSolutionName(self,i:int)->str:
        """Get criteria solution name

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        str
            solution name
        """
        self.AssertSuitableSolutionId(i)
        return self.GetCriteriaSolutionName(i)

    @abc.abstractmethod
    def GetCriteriaSolutionName(self,i:int)->str:
        """Retrieve the name of the solution of index i. The index i should be smaller than the total number of solutions

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        str
            solution name
        """
        return 

    def SetProblem(self,problem:SolverBase):
        """Associate specific physical problem to criteria

        Parameters
        ----------
        problem : SolverBase
            physical problem object
        """
        self.problem=problem

    def SetAdjointProblemLikeDirect(self,problemArg:Optional[Dict]=None):
        """Define an adjoint problem under the assumption that it is of the same kind as the original physical problem

        Parameters
        ----------
        problemArg : Optional[Dict], optional
            arguments to define adjoint problem, by default None
        """
        if problemArg is None:
            self.adjointProblem=type(self.problem)()
        else:
            self.adjointProblem=type(self.problem)(**problemArg)

    def SetAuxiliaryQuantities(self,point:LevelSetBase):
        """Set auxiliary quantities, must be redefined in the derived class if required.
        As a general basis, the criteria is supposed to ask the problem to provide auxiliary quantities prior to its resolution.
        The quantities asked are either used for the computation of criteria value/sensitivity or are of interest even if they are not explicitly used.

        Parameters
        ----------
        point : LevelSetBase
            Levelset instance
        """
        pass

    def UpdateCriteriaValues(self,point:LevelSetBase)->int:
        """Update criteria values:
        - Define auxiliary quantities, if any
        - Solve the physical problem
        - Update the criteria value/sensitivity

        Parameters
        ----------
        point : LevelSetBase
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        self.SetAuxiliaryQuantities(point)
        if self.problem.SolveByLevelSet(point) != RETURN_SUCCESS:
            print("Error solving " + str(self.problem) )
        self.UpdateValues(point)
        return RETURN_SUCCESS

def UnimplementedFunction(name:str):
    """Default function for initialization of variables

    Parameters
    ----------
    name : str
        function name
    """
    def func(_name):
        raise ValueError("Please set the function '"+_name+ "' !!!")
    return partial(func,name)

class CriteriaFunctionPointer(CriteriaBase):
    def __init__(self, other:Optional[CriteriaFunctionPointer]=None):
        """CriteriaFunctionPointer constructor

        Parameters
        ----------
        other : Optional[CriteriaFunctionPointer], optional
            another instance of this very criteria to copy attributes from, by default None
        """
        super(CriteriaFunctionPointer, self).__init__(other)
        if other is None:
            self.f = UnimplementedFunction('f')
            self.f_val = None
            self.fSensitivity = UnimplementedFunction('fSensitivity')
            self.fSensitivity_val = []
        else:
            self.f = other.f
            self.f_val = copy.deepcopy(other.f_val)
            self.fSensitivity = other.fSensitivity
            self.fSensitivity_val = copy.deepcopy(other.fSensitivity_val)

    def UpdateValues(self,point:LevelSetBase)->int:
        """Update criteria value/sensitivity

        Parameters
        ----------
        point : LevelSetBase
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        self.f_val = self.f(point)
        self.fSensitivity_val = self.fSensitivity(point)
        return RETURN_SUCCESS

    def GetNumberOfSolutions(self)->int:
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions
        """
        return 0

    def GetSolution(self,i:int)->Union[np.ndarray,None]:
        """Get criteria solution

        Parameters
        ----------
        i : int
            solution to retrieve index

        Returns
        -------
        Union[np.ndarray,None]
            solution value
        """
        return None

def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
