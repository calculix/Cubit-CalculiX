# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module is dedicated to the definition of a class for generic optimization problems
"""
from __future__ import annotations

from typing import Union, Literal, Optional, List
from abc import ABC

import numpy as np

from Muscat.Helpers.TextFormatHelper import TFormat

class OptimProblemBase(ABC):
    """Base class for all optimization problems"""
    def __init__(self, other:Optional[any] = None, dataDeepCopy:bool = True):
        super().__init__()

    def TakeValuesFrom(self,other:OptimProblemBase):
        """Retrieve values from another optimization problem

        Parameters
        ----------
        other : OptimProblemBase
            Another instance of OptimProblemBase
        """
        raise NotImplementedError

    def GetCurrentPoint(self)->any:
        """Retrieve current point of the optimization problem
        
        Returns
        -------
        any
            current point of the optimization problem
        """
        raise NotImplementedError
    
## the objective function val and Sensitivity df/dp
    def GetObjectiveFunctionName(self)->str:
        """Retrieve problem objective function name
        
        Returns
        -------
        str
            problem objective function name
        """
        raise NotImplementedError

    def GetObjectiveFunctionVal(self)->float:
        """Retrieve problem objective function value
        
        Returns
        -------
        float
            problem objective function value
        """
        raise NotImplementedError

    def GetObjectiveFunctionSensitivity(self)->np.ndarray:
        """Retrieve problem objective function sensitivity
        
        Returns
        -------
        float
            problem objective function sensitivity
        """
        raise NotImplementedError

## constraints number, val and sensistivity dC/dp
    def GetNumberOfInequalityConstraints(self)->int:
        """Retrieve number of inequality constraints

        Returns
        -------
        int
            number of inequality constraints
        """
        raise NotImplementedError

    def GetNumberOfEqualityConstraints(self)->int:
        """Retrieve number of equality constraints

        Returns
        -------
        int
            number of equality constraints
        """
        raise NotImplementedError

    def GetInequalityConstraintName(self,i:int)->str:
        """Retrieve name of the i-th inequality constraints

        Parameters
        ----------
        i : int
            inequality constraints id

        Returns
        -------
        str
            name of the i-th inequality constraints
        """
        raise NotImplementedError

    def GetEqualityConstraintName(self,i:int)->str:
        """Retrieve name of the i-th equality constraints

        Parameters
        ----------
        i : int
            equality constraints id

        Returns
        -------
        str
            name of the i-th equality constraints
        """
        raise NotImplementedError

    def GetInequalityConstraintVal(self,i:int)->float:
        """Retrieve value of the i-th inequality constraints

        Parameters
        ----------
        i : int
            inequality constraints id

        Returns
        -------
        float
            value of the i-th inequality constraints
        """
        raise NotImplementedError

    def GetEqualityConstraintVal(self,i:int)->float:
        """Retrieve name of the i-th equality constraints

        Parameters
        ----------
        i : int
            equality constraints id

        Returns
        -------
        float
            value of the i-th equality constraints
        """
        raise NotImplementedError

    def GetInequalityConstraintUpperBound(self,i:int)->float:
        """Retrieve upper bound of the i-th inequality constraints

        Parameters
        ----------
        i : int
            inequality constraints id

        Returns
        -------
        float
            upper bound of the i-th inequality constraints
        """
        raise NotImplementedError

    def GetInequalityConstraintSensitivityVal(self,i:int)->np.ndarray:
        """Retrieve sensitivity of the i-th inequality constraints

        Parameters
        ----------
        i : int
            inequality constraints id

        Returns
        -------
        float
            sensitivity of the i-th inequality constraints
        """
        raise NotImplementedError

    def SetInequalityConstraintStatus(self,i:int,status:Union[Literal[0], Literal[1]]):
        """Set inequality constraint status based on the upper bound value

        Inactive = 0,
        Automatic = 1 

        Parameters
        ----------
        i : int
            inequality constraints id
        status : Union[Literal[0], Literal[1]]
            either 0 or 1
        """
        raise NotImplementedError

    def GetNumberOfActiveInequalityConstraints(self)->int:
        """Retrieve number of inequalities constraints

        Returns
        -------
        int
            number of inequality constraints
        """
        return int(np.sum([ self.GetInequalityConstraintStatus(i) for i in range(self.GetNumberOfInequalityConstraints())  ]))

    def GetActiveInequalityConstraintsVals(self)->List[float]:
        """Retrieve all active inequalities constraints values

        Returns
        -------
        List[float]
            all active inequality constraints values
        """
        res = []
        for i in range(self.GetNumberOfInequalityConstraints()):
            if self.GetInequalityConstraintStatus(i):
                res.append(self.GetInequalityConstraintVal(i))
        return res

    def GetActiveInequalityConstraintsUpperBound(self)->List[float]:
        """Retrieve active inequalities constraints upperbound

        Returns
        -------
        List[float]
            active inequalities constraints upperbound
        """
        res = []
        for i in range(self.GetNumberOfInequalityConstraints()):
            if self.GetInequalityConstraintStatus(i):
                res.append(self.GetInequalityConstraintUpperBound(i))
        return res

    def GetActiveInequalityConstraintsSensitivityVals(self)->List[np.ndarray]:
        """Retrieve active inequalities sensitivity values

        Returns
        -------
        List[np.ndarray]
            active inequalities constraints sensitivity values
        """
        res = []
        for i in range(self.GetNumberOfInequalityConstraints()):
            if self.GetInequalityConstraintStatus(i):
                criteriaSensitivity = self.GetInequalityConstraintSensitivityVal(i)
                res.append(criteriaSensitivity)
        return res

    def GetActiveEqualityConstraintsVals(self)->List[float]:
        """Retrieve all active equalities constraints values

        Returns
        -------
        List[float]
            all active equality constraints values
        """
        res = []
        for i in range(self.GetNumberOfEqualityConstraints()):
            if self.GetEqualityConstraintStatus(i):
                res.append(self.GetEqualityConstraintVal(i))
        return res

    def GetActiveEqualityConstraintsTargetValues(self)->List[float]:
        """Retrieve active equalities constraints target values

        Returns
        -------
        List[float]
            active equalities constraints target values
        """
        res = []
        for i in range(self.GetNumberOfEqualityConstraints()):
            if self.GetEqualityConstraintStatus(i):
               res.append(self.GetEqualityConstraintTargetValue(i))
        return res

    def GetActiveEqualityConstraintsSensitivityVals(self)->List[np.ndarray]:
        """Retrieve active equalities sensitivity values

        Returns
        -------
        List[np.ndarray]
            active equalities constraints sensitivity values
        """
        res = []
        for i in range(self.GetNumberOfEqualityConstraints()):
            if self.GetEqualityConstraintStatus(i):
                res.append(self.GetEqualityConstraintSensitivityVal(i))
        return res

    def GetInequalityConstraintStatus(self,i:int)->bool:
        """Retrieve inequality constraints status

        Parameters
        ----------
        i : int
            inequality constraints id

        Returns
        -------
        float
            inequality constraints status
        """
        raise NotImplementedError

    def UpdateProblemWhenIterationAccepted(self):
        """Update optimization problem when iteration accepted (only required for very specific criteria)"""
        raise NotImplementedError

## for pretty  output
    def GetNumberOfInternalVariables(self)->int:
        """Compute number of internal variables (namely objective+ number of constraints)

        Returns
        -------
        int
            number of internal variables
        """
        return 1+self.GetNumberOfInequalityConstraints()+self.GetNumberOfEqualityConstraints()

    def GetVariableName(self,i:int)->str:
        """Retrieve i-th internal variable name

        Parameters
        ----------
        i : int
            optimization problem internal variable id

        Returns
        -------
        str
            i-th internal variable name
        """
        if i == 0:
            return self.GetObjectiveFunctionName()
        numberOfInequalityConstraints=self.GetNumberOfInequalityConstraints()
        if i <= numberOfInequalityConstraints:
           return self.GetInequalityConstraintName(i-1)
        return self.GetEqualityConstraintName(i-1-numberOfInequalityConstraints)

    def GetVariableValue(self,i:int)->float:
        """Retrieve i-th internal variable value

        Parameters
        ----------
        i : int
            optimization problem internal variable id

        Returns
        -------
        str
            i-th internal variable value
        """
        if i == 0: return self.GetObjectiveFunctionVal()
        numberOfInequalityConstraints=self.GetNumberOfInequalityConstraints()
        if i <= numberOfInequalityConstraints:
           return self.GetInequalityConstraintVal(i-1)
        return self.GetEqualityConstraintVal(i-1-numberOfInequalityConstraints)

    def PreStartCheck(self)->bool:
        """Start check, if anything must be verified before running the optimization problem

        Returns
        -------
        bool
            The current default behviour is to simply return True
        """
        return True

    def UpdateValues(self):
        """Update the optimization problem, classically by updating each of its components"""
        raise NotImplementedError

    def Advance(self, direction:np.ndarray, stepSize:float)->bool:
        """
        Parameters
        ----------
        direction : np.ndarray
            Vector-valued direction.

        stepSize : float
            A multiplicative factor for `direction`.

        Returns
        -------
        bool
            Whether the advance succeeded
        """
        self.point += stepSize * direction
        return True

    def GetDirectionFromGradient(self,gradient:np.ndarray)->np.ndarray:
        """Compute the descent direction from the gradient

        Parameters
        ----------
        gradient : np.ndarray
            gradient

        Returns
        -------
        np.ndarray
            direction, simply the opposite of the gradient in the default configuration
        """
        return -gradient

    def SaveData(self,data:any,point:Optional[any]=None, gradient:Optional[np.ndarray]=None, direction:Optional[np.ndarray]=None):
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
        pass
    
    def PrintHeader(self):
        """Print details about optimization problem constraints (inequality/equality, name, upperbound/target value)"""
        for i in range(self.GetNumberOfInequalityConstraints()):
            print(self.GetVariableName(i+1)+" Upper Bound : " + str(self.GetInequalityConstraintUpperBound(i) ))

        for i in range(self.GetNumberOfEqualityConstraints()):
            print(self.GetVariableName(self.GetNumberOfInequalityConstraints()+i+1)+" Target Value: " + str(self.GetEqualityConstraintTargetValue(i) ))

    def PrintStateHeader(self)->str:
        """Retrieve status of each internal variables (objective and constraints), meaning whether they are active or not for constraints, 
        for formatting header (used for printing original state of optimization problem before run)

        Returns
        -------
        str
            formatted header depending of internal variable status
        """
        res = ''
        for i in range(self.GetNumberOfInternalVariables()):
            res += '|'
            if i > 0 and i < self.GetNumberOfInequalityConstraints() :
                if self.GetInequalityConstraintStatus(i-1):
                    res += " "
                else:
                    res += "I"
            elif i > 0 and i > self.GetNumberOfInequalityConstraints():
                if self.GetEqualityConstraintStatus(i-1-self.GetNumberOfInequalityConstraints()):
                    res += " "
                else:
                    res += "I"
            else:
                res += " "
            res += TFormat.Center(self.GetVariableName(i), fill=" ", width=15)

        return res

    def PrintState(self)->str:
        """Retrieve proper format compatible with the number of internal variables (used for formatting current state of optimization problem)

        Returns
        -------
        str
            formatted state
        """
        res = ""
        for i in range(self.GetNumberOfInternalVariables()):
            res += '|' + TFormat.Center('%.8e' % self.GetVariableValue(i), fill=" ", width=16)
        return res

    def PrintCurrentState(self, newPoint:OptimProblemBase)->str:
        """Print current and former state of the optimization problem.
        Allow to see whether there is an improvement between the current problem values and former problem value for any internal variable.
        If there is, the value appears in green. Else, it appears in red.

        Parameters
        ----------
        newPoint : OptimProblemBase
            current problem

        Returns
        -------
        str
            Colored internal values for current/former optimization problems
        """
        def PrintOneState(oldval:float, newval:float, tocompare:float)->str:
            res = ""
            if  tocompare == np.inf:
                if newval < oldval:
                    res += TFormat.InGreen(TFormat.Center( "%.8e"% newval,fill=" ",width=16) )+"|"
                else:
                    res += TFormat.InRed(TFormat.Center("%.8e"% newval,fill=" ",width=16) )+"|"
            else:
                if newval < tocompare:
                    res += TFormat.InGreen(TFormat.Center( "%.8e"% newval,fill=" ",width=16) )+"|"
                else:
                    res += TFormat.InRed(TFormat.Center("%.8e"% newval,fill=" ",width=16) )+"|"
            return res
        
        res = ""
        for i in range(self.GetNumberOfInternalVariables()):
            oldval = self.GetVariableValue(i)
            newval = newPoint.GetVariableValue(i)

            if i == 0:
                res += PrintOneState(oldval,newval, np.inf )
            elif i-1 < self.GetNumberOfInequalityConstraints():
                res += PrintOneState(oldval,newval, newPoint.GetInequalityConstraintUpperBound(i-1) )
            else:
                equalityConstraintId=i-1-self.GetNumberOfInequalityConstraints()
                res += PrintOneState(oldval,newval, newPoint.GetEqualityConstraintTargetValue(equalityConstraintId) )

        return res

def CheckIntegrity():
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
