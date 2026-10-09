# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module provide an implementation of a classical optimization algorithm where constraints are simply penalized to consider an unconstrained problem
"""

from __future__ import annotations

from typing import Dict,Optional,Tuple

import numpy as np

import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.Logger import Debug,ForcePrint

from OpenPisco.Optim.Algorithms.OptimAlgoBase import OptimAlgoBase
from OpenPisco.Optim.Algorithms.OptimAlgoFactory import RegisterClass
from OpenPisco.Optim.Problems.OptimProblemBase import OptimProblemBase
from OpenPisco.Optim.Problems.OptimProblemDerived import OptimProblemPenalized,OptimProblemSquared

from OpenPisco import RETURN_SUCCESS, RETURN_FAIL

def GetOptimProblemWithPenalizedInequalityConstraints(of:OptimProblemBase)->OptimProblemPenalized:
    """Convert an optimization problem in a unconstrained one by penalizing the inequality constraints

    Parameters
    ----------
    of : OptimProblemBase
        optimisation problem

    Returns
    -------
    OptimProblemPenalized
        Unconstrained problem
    """
    OP = OptimProblemPenalized()
    OP.SetInternalOptimProblem(of)
    return OP

def GetOptimProblemWithQuadraticPenalizedConstraints(of:OptimProblemBase)->OptimProblemPenalized:
    """Convert an optimization problem in a unconstrained one by quadratically penalizing the inequality constraints

    Parameters
    ----------
    of : OptimProblemBase
        optimisation problem

    Returns
    -------
    OptimProblemPenalized
        Unconstrained problem
    """
    OPS = OptimProblemSquared()
    OPS.SetInternalOptimProblem(of)
    OP = OptimProblemPenalized()
    OP.SetInternalOptimProblem(OPS)
    return OP

def CreateOptimAlgoUnconstrained(ops:Dict)->OptimAlgoUnconstrained:
    """Convert an optimization problem in a unconstrained one

    Parameters
    ----------
    of : OptimProblemBase
        optimisation problem

    Returns
    -------
    OptimAlgoUnconstrained
        Unconstrained problem
    """
    res = OptimAlgoUnconstrained()
    op = ops["optimProblem"]

    if type(op) != OptimProblemPenalized:
        ops["optimProblem"] =  GetOptimProblemWithQuadraticPenalizedConstraints(op)
        op = ops["optimProblem"]
        for key in list(ops.keys()):
            prefix = "Penal"
            if (len(key) > len(prefix)) and (key[0:len(prefix)] == prefix):
                cname = key[len("Penal"):]
                penal = PH.ReadFloat(ops[key])
                del ops[key]
                op.SetPenal(cname,penal)

    PH.ReadProperties(ops, ops, res)

    return res

class OptimAlgoUnconstrained(OptimAlgoBase):
    def __init__(self):
      super(OptimAlgoUnconstrained, self).__init__()
      self.oldObjective = 0

      self.numberOfDesignStepsMax = 100
      self.numberOfIterationsMax = 10
      self.minimalChange = 0.001

      self.addInequalityConstraintsConstribution = True
      self.firstTime = True
      self.automaticPenalisation = False

      """
      tol_merit : tolerance for acceptance step with the merit function method
                  (`method`==`merit_function`)
      """
      self.tol_merit =  0.0005
      self.numberOfBadStepAccepted_ = 0
      self.numberOfBadStepMaxAccepted = 3
      self.stepDown = 0.5
      self.stepUp = 1.1

    def Start(self)->int:
        """start run

        Returns
        -------
        int
            algorithm status
        """
        self.PreStart()
        self.Initialize()
        self.RunOptimization()
        return RETURN_SUCCESS

    def Initialize(self):
        """Initialize algorithm: Compute objective gradient for the first time, initialize constraint slacks and the descent direction """
        assert self.optimProblem.GetNumberOfEqualityConstraints()==0,"The Unconstrained algorithm can not handle equality constraint"
        (self.oldObjective,self.gradient)  = self.ComputeObjectiveAndGradient()
        self.oldslack = self.ComputeConstraintsSlacks()
        self.direction = self.ComputeDirectionFromGradient(gradient=self.gradient)
        self.SaveData()

    def StatusFromResult(self,resultVal:int)->int:
        """Define proper state depending on status

        Parameters
        ----------
        resultVal : int
            result int value

        Returns
        -------
        int
            either RETURN_FAIL or RETURN_SUCCESS
        """
        if resultVal != RETURN_SUCCESS :
            return RETURN_FAIL
        if self.outputStatus == "Converged":
            return RETURN_SUCCESS
        return RETURN_SUCCESS

    def DoOneStep(self):
        """Perform a single optimization step (core logic of the algorithm)"""
        self.iter = 0
        self.PrintState()
        self.iterOk = False
        self.optimProblem.UpdateValues()

        for self.iter in range(self.numberOfIterationsMax):
            self.iterOk = self.TryToAdvance()
            if self.stepSize <= self.stepSizeMax*0.000001:
                ForcePrint('Termination :  stepSize to small')
                return RETURN_FAIL
            if self.iterOk:
                self.IncreaseStepSize()
                break
            else:
                self.DecreaseStepSize()
        else:
            print("Termination : maximal number of iterations reached")
            return RETURN_FAIL

        self.SaveData()

        if abs(self.gain) < self.minimalChange and self.gain < 0.0 and self.stepSize < 0.001:
            if self.numberOfBadStepAccepted_  >= self.numberOfBadStepMaxAccepted:
                print("Termination : gain less than " + str(self.minimalChange)+ "%")
                self.outputStatus = "Converged"
                return RETURN_SUCCESS
            else:
                self.numberOfBadStepAccepted_ += 1


        self.lastGoodStepSize = self.stepSize
        return RETURN_SUCCESS

    def TryToAdvance(self)->bool:
        """Try to advance. It is done in several steps:
        -Copy the optimization problem and make it advance according to the computed direction/stepsize
        -Evaluate it and compare the new objective function value to the value of the original problem  
        -Check that objective has decreased

        If it does, the advance is accepted, else it is invalid

        Returns
        -------
        bool
            True if optimization advancing allowed, else False
        """
        Debug("in TryToAdvance")
        NOF = self.CopyCurrentOptimProblem()
        while True:
            if self.stepSize <= 0.0001:
                ForcePrint('Step length too small, unable to advance')
                self.stepSize = 0.0
                return False
            ok = self.Advance(optimProblem=NOF)
            if ok:
                break
            self.DecreaseStepSize()

        (newObjective,gradient)  = self.ComputeObjectiveAndGradient(NOF)

        if self.automaticPenalisation:
            penalvariation = 0.1
            Constraint_validation = 0.01
            # try to encrease the penalisation at constant merit function value
            for i in range(NOF.GetNumberOfInequalityConstraints()):
                if NOF.GetInequalityConstraintStatus(i) == 0 :
                   continue

                name = NOF.GetInequalityConstraintName(i)

                slack = (NOF.GetInequalityConstraintValOriginal(i) - NOF.GetInequalityConstraintUpperBoundOriginal(i))
                if slack <= 0:
                    continue
                if NOF.GetInequalityConstraintUpperBoundOriginal(i) == 0:
                    if slack < Constraint_validation:
                        continue

                elif abs(slack/NOF.GetInequalityConstraintUpperBoundOriginal(i)) < Constraint_validation:
                    continue

                objective_striped = 1.1*self.oldObjective-(newObjective  - NOF.GetInequalityConstraintVal(i))
                oldpenal = NOF.GetPenal(name)
                NOF.SetPenal(name,1.)
                cv = NOF.GetInequalityConstraintVal(i)
                newpenal = objective_striped/cv

                if abs(oldpenal*(1+penalvariation)) < newpenal:
                    newpenal = oldpenal*(1+penalvariation)

                newObjective = -oldpenal*cv + newpenal*cv

                if newpenal < oldpenal:
                    NOF.SetPenal(name,oldpenal)
                    continue

                if cv == 0:
                    NOF.SetPenal(name,oldpenal)
                else:

                    NOF.SetPenal(name,newpenal)

            (newObjective,gradient)  = self.ComputeObjectiveAndGradient(NOF,withUpdateValues=False)
            self.oldObjective  = self.ComputeObjective(NOF.penal)


        self.ComputeGain(newObjective)
        self.PrintCurrentState(NOF, newObjective)

        res = newObjective <= self.oldObjective
        if res:
            self.numberOfBadStepAccepted_ = 0
        else:
            res2 = newObjective < (1+np.sign(self.oldObjective)*self.tol_merit)*self.oldObjective
            if res2:
                self.numberOfBadStepAccepted_ += 1
                res = res2

        if self.numberOfBadStepAccepted_ >= self.numberOfBadStepMaxAccepted:
            res = False

        if res:
            self.iterOk = True
            # Keep the new point
            self.AcceptOptimProblemIteration(NOF)
            self.oldObjective = newObjective

            # Update the gradient
            self.gradient = gradient
            self.oldslack = self.ComputeConstraintsSlacks(optimProblem=NOF)

            # Update the velocity field (chosen as a descent direction)
            self.direction = self.ComputeDirectionFromGradient(gradient=self.gradient)
        else:
            self.iterOk = False
            if self.outputEveryIter:
                self.SaveData(OF=NOF)

        return res

    def ComputeObjectiveAndGradient(self, optimProblem:Optional[OptimProblemBase]= None, withUpdateValues:bool = True)->Tuple[float,np.ndarray]:
        """Compute the objective and its gradient

        Parameters
        ----------
        optimProblem : Optional[OptimProblemBase], optional
            optimisation problem, by default None
        withUpdateValues : bool, optional
            whether to update the optimization problem, by default True

        Returns
        -------
        Tuple[float,np.ndarray]
            objective and gradient
        """
        if optimProblem is None:
            optimProblem = self.optimProblem

        if withUpdateValues:
            self.UpdateOptimProblem(optimProblem=optimProblem)

        merit =  optimProblem.GetObjectiveFunctionVal()
        meritSensitivity = np.copy(optimProblem.GetObjectiveFunctionSensitivity().flatten())

        if self.firstTime and self.automaticPenalisation:
            self.firstTime = False
            for i in range(optimProblem.GetNumberOfInequalityConstraints()):
                if optimProblem.GetInequalityConstraintStatus(i) == 0 :
                   continue
                name = optimProblem.GetInequalityConstraintName(i)
                oldpenal = optimProblem.GetPenal(name)
                optimProblem.SetPenal(name,1.)
                cv = optimProblem.GetInequalityConstraintVal(i)

                if cv == 0:
                    print("Warning: Constraint '" + name + "' not active (val = 0): Imposible to do Nondimensionalisation ")
                    optimProblem.SetPenal(name,oldpenal)
                else:
                    val = merit*oldpenal/cv
                    optimProblem.SetPenal(name,val)


        if self.addInequalityConstraintsConstribution:
            for i in range(optimProblem.GetNumberOfInequalityConstraints()):
                if optimProblem.GetInequalityConstraintStatus(i) == 0 :
                    continue
                merit += optimProblem.GetInequalityConstraintVal(i)
                meritSensitivity += optimProblem.GetInequalityConstraintSensitivityVal(i).flatten()

        return (merit,meritSensitivity )

    def ComputeObjective(self,usingPenals:Optional[Dict]=None)->float:
        """Compute objective 

        Parameters
        ----------
        usingPenals : Optional[Dict], optional
            whether to consider the penalisation coefficient for the computation of the objective, by default None

        Returns
        -------
        float
            objective value
        """
        optimProblem=self.optimProblem
        objective =  optimProblem.GetObjectiveFunctionVal()
        if self.addInequalityConstraintsConstribution:
            for i in range(optimProblem.GetNumberOfInequalityConstraints()):
                if optimProblem.GetInequalityConstraintStatus(i) == 0 :
                    continue
                if usingPenals is not None:
                    name = optimProblem.GetInequalityConstraintName(i)
                    oldpenal = optimProblem.GetPenal(name)
                    optimProblem.SetPenal(name,usingPenals[name])
                    cv = optimProblem.GetInequalityConstraintVal(i)
                    optimProblem.SetPenal(name,oldpenal)
                else :
                    cv  = optimProblem.GetInequalityConstraintVal(i)
                objective += cv
        return objective


RegisterClass("OptimAlgoUnconstrained", None,CreateOptimAlgoUnconstrained)

def CheckIntegrityTopo(GUI=False):
    OA = OptimAlgoUnconstrained()
    OA.ResetState()
    from OpenPisco.TestData.TestCases import TestOptimTopoCubeFlexion
    OF = TestOptimTopoCubeFlexion("OptimAlgoUnconstrained",factor=1)

    OriginalPointID = id(OF.point)
    OA.numberOfDesignStepsMax = 5

    OP = GetOptimProblemWithQuadraticPenalizedConstraints(OF)
    OP.SetPenal("Volume",1)
    OA.automaticPenalisation =True
    OA.optimProblem = OP
    OA.outputEveryIter = True
    OA.lastGoodStepSize = 0.1
    OA.stepSizeMax = 0.1
    OA.stepSize = 0.1
    OA.Start()

    print(OF.point)
    print(OriginalPointID)
    print(OA.optimProblem.internalOptimProblem.internalOptimProblem.point)
    print(id(OA.optimProblem.internalOptimProblem.internalOptimProblem.point))
    return "ok"

def CheckIntegrity2D(GUI=False):
    from OpenPisco.TestData.TestCases import TestOptim2D

    OA = OptimAlgoUnconstrained()
    OA.addInequalityConstraintsConstribution = False
    OA.numberOfDesignStepsMax = 30
    originalProblem = TestOptim2D()
    if GUI :
        originalProblem.InitWriter()
    OA.optimProblem = originalProblem
    op = id(originalProblem)
    print(op)
    print(OA.optimProblem)

    OA.outputEveryIter = False

    OA.stepSizeMax = 1
    OA.ResetState()
    OA.stepSize = 0.1

    from OpenPisco.Optim.Problems.OptimProblemDerived import OptimProblemPenalized
    from OpenPisco.Optim.Problems.OptimProblemDerived import OptimProblemSquared

    OPS = OptimProblemSquared()
    OPS.SetInternalOptimProblem(originalProblem)
    OP = OptimProblemPenalized()
    OP.SetInternalOptimProblem(OPS)

    OP.SetPenals({"NOT_Outside":1 ,"Not y<0":1})
    OA.automaticPenalisation =True

    OA.optimProblem = OP
    OA.addInequalityConstraintsConstribution = True
    OA.numberOfDesignStepsMax = 10

    OA.ResetState()
    OA.stepSizeMax = 0.1
    OA.stepSize = 0.001
    OA.Start()
    penalVal =1
    for _ in range(3):
        print(OA.optimProblem.GetCurrentPoint())
        penalVal *= 2
        OA.optimProblem.SetPenals({"NOT_Outside":penalVal ,"Not y<0":penalVal})
        OA.firstTime = True
        OA.ResetState()
        OA.stepSize = 0.001
        OA.Start()
    print(OA.optimProblem.GetCurrentPoint())
    print(op)
    print(id(OA.optimProblem.internalOptimProblem.internalOptimProblem) )
    return "ok"

def CheckIntegrity2DII(GUI=False):
    from OpenPisco.TestData.TestCases import TestOptim2DParabol

    OA = OptimAlgoUnconstrained()
    OA.numberOfDesignStepsMax = 5
    originalProblem = TestOptim2DParabol()
    if GUI :
        originalProblem.InitWriter()
    OA.optimProblem = originalProblem
    op = id(originalProblem)
    print(op)
    print(OA.optimProblem)

    OA.outputEveryIter = False

    OA.stepSizeMax = 1
    OA.ResetState()
    OA.stepSize = 0.1

    from OpenPisco.Optim.Problems.OptimProblemDerived import OptimProblemPenalized
    from OpenPisco.Optim.Problems.OptimProblemDerived import OptimProblemSquared
    from OpenPisco.Optim.Problems.OptimProblemDerived import OptimProblemPenaltyL1
    from OpenPisco.Optim.Problems.OptimProblemDerived import OptimProblemAddAllContributions

    OPS = OptimProblemSquared()
    OPS.SetInternalOptimProblem(originalProblem)
    OP = OptimProblemPenalized()
    OP.SetInternalOptimProblem(OPS)

    OPL1 = OptimProblemPenaltyL1()
    OPL1.SetInternalOptimProblem(originalProblem)
    OPL1.SetPenals({"parabol":10 ,"plane":10})
    OPAAC = OptimProblemAddAllContributions()
    OPAAC.SetInternalOptimProblem(OPL1)

    OP.SetPenals({"parabol":10 ,"plane":10})
    OP.SetPenals({"parabol":5,"plane":5})
    OA.automaticPenalisation = True
    OA.optimProblem = OP

    OA.addInequalityConstraintsConstribution = True
    OA.numberOfIterationsMax = 8
    OA.outputEveryIter = True

    OA.ResetState()
    OA.stepSizeMax = 0.1
    OA.stepSize = 0.001
    OA.Start()
    return "ok"

def CheckIntegrity(GUI=False):
    CheckIntegrityTopo(GUI=GUI)
    CheckIntegrity2D(GUI=GUI)
    CheckIntegrity2DII(GUI=GUI)
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity(GUI=True)) # pragma: no cover
