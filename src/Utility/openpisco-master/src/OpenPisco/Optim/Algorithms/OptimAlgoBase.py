# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module provide a base class for the optimization algorithm. In particular, in the context of topology optimization, it drives the whole process
"""
from __future__ import annotations

import abc
from typing import Optional,Dict,Tuple

import numpy as np

from Muscat.Helpers.TextFormatHelper import TFormat
from Muscat.Helpers.Logger import Info, Debug,Error

from OpenPisco.Actions.ActionBase import ActionBase
from OpenPisco.Optim.Problems.OptimProblemBase import OptimProblemBase
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL

class OptimAlgoBase(metaclass=abc.ABCMeta):
    def __init__(self):
        super(OptimAlgoBase,self).__init__()
        self.run = -1
        self.output_number = 0
        self.step = 0
        self.iter = 0
        self.iterOk = True
        self.stepSizeMax = 1.
        self.stepSize = self.stepSizeMax
        self.lastGoodStepSize = self.stepSizeMax
        self.stepUp = 1.1
        self.stepDown = 0.8
        self.direction = None
        self.optimProblem = None
        self.outputEveryIter = False
        self.beforeWritingCallBack = None
        self.outputStatus = 'Unknown' # Options are: "Converged", "MaxIterReached", "Diverged", "Broken"

    def PreStart(self)->int:
        """Initial verifications before running optimization algorithm

        Returns
        -------
        int
            if success, RETURN_SUCCESS (0) else RETURN_FAIL (1)
        """
        self.run  += 1
        if not self.optimProblem.PreStartCheck():
            Error("Error in PreStartCheck")
            return RETURN_FAIL
        return RETURN_SUCCESS

    def RunOptimization(self)->any:
        """Run optimization algorithm, generic structure followed by optimization algorithms available 

        Returns
        -------
        any
            status of the first step that failed
        """
        Debug("steps")
        self.PrintStateHeader()
        self.step = 0
        while self.step < self.numberOfDesignStepsMax:
            status = self.DoOneStep()
   
            if status==RETURN_SUCCESS:
                self.step+=1
            else:
                status=self.StatusFromResult(status)
                return status

    def UpdateOptimProblem(self,optimProblem:Optional[OptimProblemBase]=None):
        """Update optimization problem

        Parameters
        ----------
        optimProblem : Optional[OptimProblemBase]
            if the optimization problem is not provided, update internal optimization problem
            
        Returns
        -------
        bool 
            indicate if the optim problem update is successfull 
        """
        optimProblem = self.optimProblem if optimProblem is None else optimProblem
        status=optimProblem.UpdateValues()
        return status 

    def CopyCurrentOptimProblem(self)->OptimProblemBase:
        """Copy internal current optimization problem

        Returns
        -------
        OptimProblemBase
            copy optimization problem
        """
        return type(self.optimProblem)(self.optimProblem)

    def ComputeConstraintsSlacks(self,optimProblem:Optional[OptimProblemBase]=None)->Dict:
        """Compute all inequality constraints slacks

        Parameters
        ----------
        optimProblem : Optional[OptimProblemBase], optional
            optimization problem, by default None

        Returns
        -------
        Dict
            inequality constraints slacks
        """
        optimProblem = self.optimProblem if optimProblem is None else optimProblem
        return optimProblem.GetInequalityConstraintSlacks()

    def ComputeSensitivityInterfaceProjection(self,
                                              DJ:np.ndarray,
                                              Dg:Optional[np.ndarray]=None,
                                              Dh:Optional[np.ndarray]=None)->Tuple[np.ndarray,np.ndarray,np.ndarray]:
        """Compute sensitivity interface projection for each component of the optimization problem

        Parameters
        ----------
        DJ : np.ndarray
            objective sensitivity
        Dg : Optional[np.ndarray], optional
            equality constraints sensitivity, by default None
        Dh : Optional[np.ndarray], optional
            inequality constraints sensitivity, by default None

        Returns
        -------
        Tuple[np.ndarray,np.ndarray,np.ndarray]
            objective, equality constraint and inequality constraint interface projection
        """
        if hasattr(self.optimProblem.GetCurrentPoint(), "ComputeInterfaceProjection"):
            DJ = self.optimProblem.GetCurrentPoint().ComputeInterfaceProjection(DJ)
            for i in range(len(Dg)):
                Dg[i,:] = self.optimProblem.GetCurrentPoint().ComputeInterfaceProjection(Dg[i,:])
            for i in range(len(Dh)):
                Dh[i,:] = self.optimProblem.GetCurrentPoint().ComputeInterfaceProjection(Dh[i,:])
        return DJ,Dg,Dh

    def GetValuesFromOptimProblem(self,
                                  optimProblem:Optional[OptimProblemBase]=None)->Tuple[float,np.ndarray,np.ndarray]:
        """Get values from optimization problems from each of its components (objective and active constraints value)

        Parameters
        ----------
        optimProblem : Optional[OptimProblemBase], optional
            optimization problem, by default None

        Returns
        -------
        Tuple[float,np.ndarray,np.ndarray]
            Objective, active equality constraint and active inequality constraints
        """
        optimProblem = self.optimProblem if optimProblem is None else optimProblem
        J = optimProblem.GetObjectiveFunctionVal()
        g = np.array([x-b for x,b in zip(optimProblem.GetActiveEqualityConstraintsVals(),optimProblem.GetActiveEqualityConstraintsTargetValues())])
        h = np.array([x-b for x,b in zip(optimProblem.GetActiveInequalityConstraintsVals(),optimProblem.GetActiveInequalityConstraintsUpperBound())])

        return (J, g, h)

    def GetSensitivitiesFromOptimProblem(self,
                                         optimProblem:Optional[OptimProblemBase]=None)->Tuple[np.ndarray,np.ndarray,np.ndarray]:
        """Get sensitivities from optimization problem from each of its components (objective and active constraints)

        Parameters
        ----------
        optimProblem : Optional[OptimProblemBase], optional
            optimization problem, by default None

        Returns
        -------
        Tuple[np.ndarray,np.ndarray,np.ndarray]
            sensitivities of objective and active constraints
        """
        optimProblem = self.optimProblem if optimProblem is None else optimProblem
        DJ = optimProblem.GetObjectiveFunctionSensitivity()
        Dg = np.array(optimProblem.GetActiveEqualityConstraintsSensitivityVals())
        Dh = np.array(optimProblem.GetActiveInequalityConstraintsSensitivityVals())

        if not len(Dg):
           Dg = np.zeros((0, len(DJ)))
        if not len(Dh):
           Dh = np.zeros((0, len(DJ)))

        return (DJ,Dg, Dh)

    def GetDirectionsFromGradients(self,
                                   DJ:np.ndarray,
                                   Dg:np.ndarray,
                                   Dh:np.ndarray,
                                   optimProblem:Optional[OptimProblemBase]=None)->Tuple[np.ndarray,np.ndarray,np.ndarray]:
        """Compute the directions from the respective gradient of each component of the optimization problem (objective and active constraints)

        Parameters
        ----------
        DJ : np.ndarray
            objective sensitivity
        Dg : Optional[np.ndarray], optional
            equality constraints sensitivity, by default None
        Dh : Optional[np.ndarray], optional
            inequality constraints sensitivity, by default None

        Returns
        -------
        Tuple[np.ndarray,np.ndarray,np.ndarray]
            directions of objective and active constraints
        """
        DgExt = np.zeros_like(Dg)
        DhExt = np.zeros_like(Dh)

        DJExt =  -self.ComputeDirectionFromGradient(gradient=DJ,optimProblem=optimProblem)
        for i in range(len(Dg)):
            DgExt[i,:] =  -self.ComputeDirectionFromGradient(gradient=Dg[i,:],optimProblem=optimProblem)
        for i in range(len(Dh)):
            DhExt[i,:] =  -self.ComputeDirectionFromGradient(gradient=Dh[i,:],optimProblem=optimProblem)

        return (DJExt,DgExt,DhExt)

    def ComputeDirectionFromGradient(self,gradient:np.ndarray,optimProblem:Optional[OptimProblemBase]=None)->np.ndarray:
        """Compute the direction from the gradient

        Parameters
        ----------
        gradient : np.ndarray
            gradient of the optimization problem underlying criteria
        optimProblem : Optional[OptimProblemBase], optional
            optimization problem, by default None

        Returns
        -------
        np.ndarray
            direction associated to the gradient
        """
        optimProblem = self.optimProblem if optimProblem is None else optimProblem
        return optimProblem.GetDirectionFromGradient(gradient)

    @abc.abstractmethod
    def StatusFromResult(self,resultVal:int):
        """Status associated to result, 

        Parameters
        ----------
        resultVal : int
            result value from algorithm run
        """
        pass

    def NeedToInterrupt(self)->bool:
        """wWether there is a need to interrupt

        Returns
        -------
        bool
            True if there is a need to interrupt, else False
        """
        return ActionBase.NeedToInterrupt()

    @abc.abstractmethod
    def DoOneStep(self):
        """To be redefined in the derived class, the actual core logic of an optimization algorithm in OpenPisco
        This is where the direction and stepSize attributes are expected to be computed for advancing in a suitable way.
        """
        pass

    def Advance(self,optimProblem:OptimProblemBase):
        """Make optimization problem advance in the computed direction for a given stepsize"""
        return optimProblem.Advance(self.direction, self.stepSize)

    def ComputeGain(self, newObjective):
        self.gain = ComputeObjectiveGain(oldObjective=self.oldObjective, newObjective=newObjective)

    def DecreaseStepSize(self):
        """Decrease step size (multiplication of the step size by a factor expected to be lower than 1)"""
        self.stepSize *= self.stepDown
        Debug("stepSize divided by "+str(1./self.stepDown)+" :" + str(self.stepSize))

    def IncreaseStepSize(self):
        """Increase step size (multiplication of the step size by a factor expected to be grater than 1)"""
        self.stepSize *= self.stepUp
        Debug("stepSize multiplied by "+str(self.stepUp)+" :" + str(self.stepSize))

        if self.stepSize > self.stepSizeMax:
            self.stepSize = self.stepSizeMax
            Debug("Saturated stepSize :" + str(self.stepSize))

    def AcceptOptimProblemIteration(self,optiProblem:OptimProblemBase):
        """Accept optimization problem and update internal optimization problem accordingly

        Parameters
        ----------
        optiProblem : OptimProblemBase
            Optimization problem
        """
        optiProblem.UpdateProblemWhenIterationAccepted()
        self.optimProblem.TakeValuesFrom(optiProblem)

    def ResetState(self):
        """Reset optimization algorithm state"""
        self.stepSize = self.stepSizeMax
        self.step = 0
        self.iter = 0
        self.iterOk = True
        self.outputStatus = "Unknown"

    def PrintStateHeader(self):
        """Print state header, should be used once per optimization algo run
        
        For a problem with 2 criteria names criterion_1 and criterion_2, this is what one could expect
        |cpt| step | iter |      criterion_1    |    criterion_2  |    Objective   |  Gain % |StepSize|

        """
        self.optimProblem.PrintHeader()
        print(TFormat.Center('run : %i' % (self.run),width= 80 ) )
        res = '|cpt| step | iter '
        res += self.optimProblem.PrintStateHeader()
        res += '|'+ TFormat.Center('Objective',fill=" ",width=16)  + '|  Gain % |StepSize|'
        print(res)

    def PrintState(self):
        output_number_str =  TFormat.Center(str(self.output_number), fill=' ',width=3)
        res = ('|%s| %s|      ') % (output_number_str, format(self.step, " 5") )
        res += self.optimProblem.PrintState()
        res += '|' + TFormat.Center( '%.8e'  % self.oldObjective,fill=" ",width=16)
        res += '|         |%s|' %  TFormat.Center("{0:1.4f}".format(self.stepSize),fill=" ",width=8 )
        print(res)

    def PrintCurrentState(self, NOF:OptimProblemBase,newObjective:float):
        """Print current state"""
        output_number_str =  TFormat.Center(str(self.output_number), fill=' ',width=3)
        res = "|%s|      |%s |" % (output_number_str, format(self.iter, "5"))

        res += self.optimProblem.PrintCurrentState(NOF)

        gain = self.gain

        if gain < 0:
            gain = TFormat.InGreen(TFormat.Center("{:+6.3f}%".format(gain),fill=" ",width=9))
        else:
            if gain >= 100:
                gain = TFormat.InRed(TFormat.Center("{:+6.0f}%".format(gain),fill=" ",width=9))
            else:
                gain = TFormat.InRed(TFormat.Center("{:+6.3f}%".format(gain),fill=" ",width=9))

        if newObjective < self.oldObjective:
            comp_vol_obj = TFormat.InGreen(TFormat.Center( "%.8e"% newObjective,fill=" ",width=16) )+"|"
        else:
            comp_vol_obj = TFormat.InRed(TFormat.Center("%.8e"% newObjective,fill=" ",width=16) )+"|"

        res +=  ('%s%s|%s|' % (comp_vol_obj, gain, TFormat.Center("{:1.4f}".format(self.stepSize),fill=" ",width=8) ) )
        print(res)

    def SaveData(self, OF:Optional[OptimProblemBase] = None):
        """SaveData

        Parameters
        ----------
        OF : Optional[OptimProblemBase], optional
            Optimization problem, by default None
        """
        if self.beforeWritingCallBack is not None:
            self.beforeWritingCallBack()

        data = {}
        data["run"]  = self.run
        data["step"] = self.step
        data["iter"] = self.iter
        data["cpt"]  = self.output_number
        data["OK" ]  = 1 if self.iterOk else 0

        try:
            gradient = self.gradient
        except AttributeError:
            gradient = None

        try:
            direction = self.direction
        except AttributeError:
            direction = None

        optimProblem=self.optimProblem if OF is None else OF
        optimProblem.SaveData(data, gradient = gradient, direction=direction)
        self.output_number = self.output_number + 1
        Info("Saving Data Done")
        
def ComputeObjectiveGain(oldObjective, newObjective):
    if np.sign(oldObjective) == 1:
        gain = (newObjective / oldObjective - 1) * 100
    else:
        gain = -(newObjective / oldObjective - 1) * 100
    return gain 

def CheckIntegrity():
    valuesToTest = [(-3.2,-6),(3,-2.8),(8.2,1/3),(-0.5,12)]
    expectations = ["negative","negative","negative","positive"]
    for valueToTest,expectation in zip(valuesToTest,expectations):
        oldObjective,newObjective=valueToTest
        gain = ComputeObjectiveGain(oldObjective=oldObjective,newObjective=newObjective)
        positiveGain = expectation=="positive"
        assert bool(gain > 0)==positiveGain, "The gain should be "+expectation+", gain = " + str(gain)

    return "OK"
    
if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
