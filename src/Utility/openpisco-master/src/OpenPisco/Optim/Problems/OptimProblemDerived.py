# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations

from typing import Optional, Dict

import numpy as np

from OpenPisco.Optim.Problems.OptimProblemBase import OptimProblemBase
from OpenPisco.Optim.Problems.OptimProblemConcrete import OptimProblemConcrete
from OpenPisco.Optim.Problems.ProblemFactory import RegisterClass

from Muscat.Helpers.Logger import Debug

class OptimProblemDerived(OptimProblemBase):
    """
    Decorator base class for `OptimProblemBase`.

    Parameters
    ----------
    other : OptimProblemBase
        The underlying optimisation problem.

    dataDeepCopy : bool, optional
        Whether `other` must be cloned (default is True).

    Attributes
    ----------
    internalOptimProblem : OptimProblemConcrete
        The underlying optimisation problem decorated by this class.

    Notes
    -----
    This class implements a trivial decorator (An instance behaves exactly as
    the original object). Non-trivial decorators can be created subclassing
    `OptimProblemDerived` and overriding its methods.
    """
    def __init__(self, other:Optional[OptimProblemDerived]=None, dataDeepCopy:bool=True):
        """Class constructor

        Parameters
        ----------
        other : Optional[OptimProblemDerived], optional
            another instance of this very problem to copy attributes from, by default None
        dataDeepCopy : bool, optional
            whether to use deepcopy, by default True
        """
        super(OptimProblemDerived, self).__init__(other, dataDeepCopy)

        if other is None:
            self.internalOptimProblem = None
        else:
            self.internalOptimProblem = type(other.internalOptimProblem)(other.internalOptimProblem, dataDeepCopy)

    def SetInternalOptimProblem(self, of:OptimProblemConcrete):
        """Set internal optimization problem (decorated by class)

        Parameters
        ----------
        of : OptimProblemConcrete
            optimisation problem
        """
        self.internalOptimProblem = of

    def GetCurrentPoint(self):
        return self.internalOptimProblem.GetCurrentPoint()

    def GetObjectiveFunctionVal(self):
        return self.internalOptimProblem.GetObjectiveFunctionVal()

    def GetObjectiveFunctionSensitivity(self):
        return self.internalOptimProblem.GetObjectiveFunctionSensitivity()

    def GetNumberOfInequalityConstraints(self):
        return self.internalOptimProblem.GetNumberOfInequalityConstraints()

    def GetNumberOfEqualityConstraints(self):
        return self.internalOptimProblem.GetNumberOfEqualityConstraints()

    def GetInequalityConstraintSlacks(self):
        return self.internalOptimProblem.GetInequalityConstraintSlacks()

    def GetInequalityConstraintName(self,i):
        return self.internalOptimProblem.GetInequalityConstraintName(i)

    def GetInequalityConstraintStatus(self,i):
        return self.internalOptimProblem.GetInequalityConstraintStatus(i)

    def GetInequalityConstraintVal(self, i):
        return self.internalOptimProblem.GetInequalityConstraintVal(i)

    def GetInequalityConstraintValOriginal(self, i):
        return self.internalOptimProblem.GetInequalityConstraintValOriginal(i)

    def GetInequalityConstraintUpperBoundOriginal(self,i):
        return self.internalOptimProblem.GetInequalityConstraintUpperBoundOriginal(i)

    def GetInequalityConstraintUpperBound(self,i):
        return self.internalOptimProblem.GetInequalityConstraintUpperBound(i)

    def UpdateValues(self):
        return self.internalOptimProblem.UpdateValues()

    def PrintHeader(self):
        self.internalOptimProblem.PrintHeader()

    def PrintStateHeader(self):
        return self.internalOptimProblem.PrintStateHeader()

    def PrintState(self):
        return self.internalOptimProblem.PrintState()

    def PrintCurrentState(self, newPoint):
        return self.internalOptimProblem.PrintCurrentState(newPoint.internalOptimProblem)

    def Advance(self, direction, stepSize):
        return self.internalOptimProblem.Advance(direction, stepSize)

    def GetDirectionFromGradient(self,grad):
        return self.internalOptimProblem.GetDirectionFromGradient(grad)

    def UpdateProblemWhenIterationAccepted(self):
        self.internalOptimProblem.UpdateProblemWhenIterationAccepted()

    def PrintCurrentState(self, newPoint:OptimProblemDerived)->str:
        """Print current and former state of the optimization problem.

        Parameters
        ----------
        newPoint : OptimProblemBase
            current problem

        Returns
        -------
        str
            Colored internal values for current/former optimization problems
        """
        return self.internalOptimProblem.PrintCurrentState(newPoint.internalOptimProblem)

    def SaveData(self,data,point:Optional[any]=None, gradient:Optional[np.ndarray]= None, direction:Optional[np.ndarray] = None ):
        """Save data on disk, it is supposed to be called to track the history of the optimization problem

        Parameters
        ----------
        data : data
            data
        point : Optional[any], optional
            point, by default None
        gradient : Optional[np.ndarray], optional
            gradient, by default None
        direction : Optional[np.ndarray], optional
            direction, by default None
        """
        self.internalOptimProblem.SaveData(data, point=point, gradient=gradient, direction=direction)

    def TakeValuesFrom(self,other:OptimProblemDerived):
        """Retrieve values from another optimization problem

        Parameters
        ----------
        other : OptimProblemDerived
            Another instance of OptimProblemDerived
        """
        self.internalOptimProblem.TakeValuesFrom(other.internalOptimProblem)

class OptimProblemPenalized(OptimProblemDerived):
    """Penalize inequality constraints by applying a scalar"""
    def __init__(self,other:Optional[OptimProblemPenalized]=None,dataDeepCopy:bool=True):
        """Class constructor

        Parameters
        ----------
        other : Optional[OptimProblemPenalized], optional
            another instance of this very problem to copy attributes from, by default None
        dataDeepCopy : bool, optional
            bool, by default True
        """
        super(OptimProblemPenalized,self).__init__(other,dataDeepCopy)

        if other is None:
            self.penal = dict()
        else:
            self.penal = other.penal.copy()

    def SetPenals(self, dic:Dict):
        """Set penalisation factors

        Parameters
        ----------
        dic : Dict
            penalization factors
        """
        for k,v in dic.items():
            self.SetPenal(k,v)

    def SetPenal(self, name:str, penal:float):
        """Set the individual value of the penalty factor.

        Parameters
        ----------
        name: str
            Constraint name
        penal : float
            Value of the penalty factor associated to constraint
        """
        self.penal[name] = float(penal)
        Debug("SetPenal called '" + str(name) + "' = " +str(float(penal)))

    def GetPenal(self,name:str)->float:
        """Get penalization factor

        Parameters
        ----------
        name : str
            constraint name

        Returns
        -------
        float
            penalty value
        """
        return self.penal[name]

    def GetInequalityConstraintVal(self, i:int)->float:
        """Get inequality constraint value

        Parameters
        ----------
        i : int
            constraint id

        Returns
        -------
        float
            constrained value multiplied by the penalization factor
        """
        return self.penal[self.internalOptimProblem.GetInequalityConstraintName(i)] * self.internalOptimProblem.GetInequalityConstraintVal(i)

    def GetInequalityConstraintSensitivityVal(self,i:int)->np.ndarray:
        """Get inequality constraint sensitivity value

        Parameters
        ----------
        i : int
            constraint id

        Returns
        -------
        np.ndarray
            constrained sensitivity value multiplied by the penalization factor
        """
        return self.penal[self.internalOptimProblem.GetInequalityConstraintName(i)] * self.internalOptimProblem.GetInequalityConstraintSensitivityVal(i)

    def UpdateValues(self):
        """Update the optimization problem"""
        if self.GetNumberOfInequalityConstraints() and len(self.penal) == 0 :
            raise ValueError('Must call SetPenal first')
        return super(OptimProblemPenalized,self).UpdateValues()

    def TakeValuesFrom(self,other:OptimProblemPenalized):
        """Retrieve values from another optimization problem

        Parameters
        ----------
        other : OptimProblemPenalized
            Another instance of OptimProblemPenalized
        """
        super(OptimProblemPenalized,self).TakeValuesFrom(other)
        self.penal = other.penal

RegisterClass("Penalized", OptimProblemPenalized)


class OptimProblemSquared(OptimProblemDerived):
    """Replaces an optimization problem by one where constraint violation are
    squared.
    """

    def __init__(self, other:Optional[OptimProblemSquared]=None, dataDeepCopy:bool=True):
        """Class constructor

        Parameters
        ----------
        other : Optional[OptimProblemSquared], optional
            another instance of this very problem to copy attributes from, by default None
        dataDeepCopy : bool, optional
            bool, by default True
        """
        super(OptimProblemSquared, self).__init__(other, dataDeepCopy)

    def GetInequalityConstraintVal(self, i:int)->float:
        """Compute new inequality constraint value

        Parameters
        ----------
        i : int
            constraint id

        Returns
        -------
        float
            slack value squared for i-th constraint
        """
        return self._slackValue(i)**2

    def GetInequalityConstraintSensitivityVal(self, i:int)->np.ndarray:
        """Compute new inequality constraint sensitivity value by applying the chain rule

        Parameters
        ----------
        i : int
            constraint id

        Returns
        -------
        np.ndarray
            sensitivity for slack value squared for i-th constraint
        """
        return (2 * self._slackValue(i)) * self.internalOptimProblem.GetInequalityConstraintSensitivityVal(i)

    def _slackValue(self, i:int)->float:
        """Compute slack value: maximal value between an inequality constraint value and its upperbound, and 0

        Parameters
        ----------
        i : int
            constraint id

        Returns
        -------
        float
            slack value
        """
        diff = self.internalOptimProblem.GetInequalityConstraintVal(i) - self.internalOptimProblem.GetInequalityConstraintUpperBound(i)
        return np.maximum(diff, 0.)

    def GetInequalityConstraintUpperBound(self,i:int)->float:
        """Get inequality constraint upperbound

        Parameters
        ----------
        i : int
            constraint id

        Returns
        -------
        float
            new upperbound becomes 0
        """
        return 0.0

RegisterClass("Squared", OptimProblemSquared)


class OptimProblemPenaltyL1(OptimProblemPenalized):
    """L1 penalty function"""

    def __init__(self,other:Optional[OptimProblemPenaltyL1]= None,dataDeepCopy:bool=True):
        """Class constructor

        Parameters
        ----------
        other : Optional[OptimProblemPenaltyL1], optional
            another instance of this very problem to copy attributes from, by default None
        dataDeepCopy : bool, optional
            bool, by default True
        """
        super(OptimProblemPenaltyL1,self).__init__(other,dataDeepCopy)

    def GetInequalityConstraintVal(self,i:int)->float:
        """Compute penalized inequality constraint value.
        Product of penalization coefficient and maximal value between an inequality constraint value and its upperbound, and 0.

        Parameters
        ----------
        i : int
            constraint id

        Returns
        -------
        float
            penalized inequality constraint value
        """
        slack = self.internalOptimProblem.GetInequalityConstraintVal(i) - self.internalOptimProblem.GetInequalityConstraintUpperBound(i)
        slack = max(0,slack)
        return self.penal[self.internalOptimProblem.GetInequalityConstraintName(i)]*slack

    def GetInequalityConstraintSensitivityVal(self,i:int):
        """Compute penalized inequality constraint sensitivity value by applying the chain rule

        Parameters
        ----------
        i : int
            constraint id

        Returns
        -------
        float
            penalized inequality constraint sensitivity value
        """
        slack = self.internalOptimProblem.GetInequalityConstraintVal(i) - self.internalOptimProblem.GetInequalityConstraintUpperBound(i)
        if slack <= 0:
            return 0*self.internalOptimProblem.GetInequalityConstraintSensitivityVal(i)
        else:
            return self.penal[i]*self.internalOptimProblem.GetInequalityConstraintSensitivityVal(i)

    def UpdateValues(self):
        """Update the optimization problem"""
        assert self.penal is not None,'Must call SetPenal first'
        return super(OptimProblemPenaltyL1,self).UpdateValues()


class OptimProblemAddAllContributions(OptimProblemDerived):
    """Class to aggregate objective and inequality constraint"""
    def __init__(self,other:Optional[OptimProblemAddAllContributions]= None,dataDeepCopy:bool=True):
        super(OptimProblemAddAllContributions,self).__init__(other,dataDeepCopy)

    def GetNumberOfInequalityConstraints(self)->int:
        """Get number of inequality constraint. Becomes 0 because of the contraints aggregation.

        Returns
        -------
        int
            No constraint left (0)
        """
        return 0
    
    def GetInequalityConstraintVal(self,i:int):
        """Can not be called, there are no constraint left"""
        raise NotImplementedError()
    
    def GetInequalityConstraintSensitivityVal(self,i:int):
        """Can not be called, there are no constraint left"""
        raise NotImplementedError()

    def GetObjectiveFunctionVal(self)->float:
        """Compute objective function value: sum all contributions from objective and inequality constraints

        Returns
        -------
        float
            aggregated objective value
        """
        objective = self.internalOptimProblem.GetObjectiveFunctionVal()

        for i in range(self.internalOptimProblem.GetNumberOfInequalityConstraints()):
            if self.internalOptimProblem.GetInequalityConstraintStatus(i) == 0 :
                continue
            objective += self.internalOptimProblem.GetInequalityConstraintVal(i)
        return objective

    def GetObjectiveFunctionSensitivity(self)->np.ndarray:
        """Compute objective function sensitivity: sum all contributions from objective and inequality constraints

        Returns
        -------
        float
            aggregated objective sensitivity value
        """
        gradient = self.internalOptimProblem.GetObjectiveFunctionSensitivity().flatten()
        for i in range(self.internalOptimProblem.GetNumberOfInequalityConstraints()):
            if self.internalOptimProblem.GetInequalityConstraintStatus(i) == 0 :
                continue
            gradient += self.internalOptimProblem.GetInequalityConstraintSensitivityVal(i).flatten()

        return gradient

def CheckIntegrity():
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
