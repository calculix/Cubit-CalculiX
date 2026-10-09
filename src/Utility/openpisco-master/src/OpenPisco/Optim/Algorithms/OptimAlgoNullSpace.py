# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module provide an implementation of a NullSpace algorithm
https://hal.science/hal-01972915/

"""
from __future__ import annotations

from types import SimpleNamespace
from typing import Dict,Optional,Iterable
import numpy as np
from scipy import sparse
import osqp

import Muscat.Helpers.ParserHelper as PH
from Muscat.MeshTools.MeshInspectionTools import ComputeMeshMinMaxLengthScale
from Muscat.Helpers.Logger import Debug

from OpenPisco.Optim.Algorithms.OptimAlgoBase import OptimAlgoBase
from OpenPisco.Optim.Algorithms.OptimAlgoFactory import RegisterClass
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL

def CreateOptimAlgoNullSpace(ops:Dict)->OptimAlgoNullSpace:
    """Constructor of NullSpace algorithm

    Parameters
    ----------
    ops : Dict
        algorithm options

    Returns
    -------
    OptimAlgoNullSpace
        Null Space algorithm instance
    """
    res = OptimAlgoNullSpace()
    cp = ops["optimProblem"].GetCurrentPoint()
    if "dt" not in ops:
        if cp.support.props.get("IsConstantRectilinear",False):
            ops["dt"] = min(cp.support.props.get("spacing"))
        else:
            ops["dt"] = ComputeMeshMinMaxLengthScale(cp.support)[0]
        print("Setting dt automatically " + str( ops["dt"]))

    if "alphaJ" not in ops:
        ops["alphaJ"] = 1.
    if "alphaC" not in ops:
        ops["alphaC"] = .5

    PH.ReadProperties(ops, ops, res)
    return res


class OptimAlgoNullSpace(OptimAlgoBase):
    """
    Attributes
    ----------
    numberOfIterationsMax : int
        Number of trials in between time steps until the merit function decreases (default 20)
    numberOfDesignStepsMax : int
        Maximal number of design steps (steps) (default : 200)
    alphaJ : float
        Scaling coefficient for the null space direction (default 1)
    alphaC : float
        Scaling coefficient for the Gauss Newton direction (default 1)
    alphas : array_like
        Vector of dimension problem.nconstraints+problem.nineqconstraints of
        proportionality coefficients for the tuning how the Gauss Newton
        direction accounts for each of the constraints (not yet implemented)
    dt : float
        The normalisation of the descent step (default 1) is such that
        ||dxiJ|| = alphaJ * dt
        ||dxiC|| = alphaC * dt
    K : float
        Tunes the distance at which inactive inequality constraints are felt.
        Constraints are felt from a distance K*dt.
    tol_merit : float
        Tolerance for acceptance step with the merit function method
    """

    def __init__(self):
        super(OptimAlgoNullSpace, self).__init__()
        self.numberOfIterationsMax = 10
        self.numberOfDesignStepsMax = 50
        self.alphaJ = 1.
        self.alphaC = 1.
        self.AJ = 0.
        self.AC = 0.
        self.merit = 0.
        self.dt = 1.
        self.K = 0.5
        self.tol_merit =  0.0005
        self.oldObjective = 0
        self.stepDown = 0.5
        self.stepUp = 1.1
        self.tolLag = 1e-8
        self.optimproblemUpdated = False

    def Start(self)->int:
        """start run

        Returns
        -------
        int
            algorithm status
        """
        if self.PreStart() != RETURN_SUCCESS:
           return RETURN_FAIL
        self.RunOptimization()
        self.Finalize()
        return RETURN_SUCCESS

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
        if self.outputStatus == 'MaxIterReached':
            return RETURN_FAIL
        if resultVal != RETURN_SUCCESS:
            return RETURN_FAIL
        return RETURN_SUCCESS

    def Finalize(self):
        """Finalize algorithm"""
        if self.step == self.numberOfDesignStepsMax:
            print("OptimAlgoNullSpace output status : Maximal number of design steps reached")

    def DoOneStep(self):
        """Perform a single optimization step (core logic of the algorithm)"""
        
        if not self.optimproblemUpdated:
           self.UpdateOptimProblem()

        (J, g, h)           = self.GetValuesFromOptimProblem()
        (DJ, Dg, Dh)        = self.GetSensitivitiesFromOptimProblem()
        (DJExt,DgExt,DhExt) = self.GetDirectionsFromGradients(DJ,Dg,Dh)

        DJ,Dg,Dh=self.ComputeSensitivityInterfaceProjection(DJ=DJ,Dg=Dg,Dh=Dh)

        Debug("Step optim algo")
        Eps = self.ComputeEps(Dh)
        tildeIdsIn = self.ComputeTildeIds(h)  # default eps_i = 0
        tildeEpsIdsIn = self.ComputeTildeIds(h,eps=Eps)

        tildeIdsDh = np.full(len(Dh),False)
        tildeIdsDh[tildeIdsIn] = True
        tildeMask = np.asarray(np.concatenate(([True]*len(Dg), tildeIdsDh)), dtype=bool)
        tildeEpsIdsDh = np.full(len(Dh),False)
        tildeEpsIdsDh[tildeEpsIdsIn] = True
        tildeEpsMask = np.asarray(np.concatenate(([True]*len(Dg), tildeEpsIdsDh)), dtype=bool)

        C = np.concatenate((g, h))
        DC = np.concatenate((Dg, Dh),axis=0)
        DCExt = np.concatenate((DgExt, DhExt),axis=0)
        DJExtT = np.transpose(DJExt)
        DCExtT = np.transpose(DCExt)

        hatIdsIn = []
        mu = np.zeros(len(C))
        if len(tildeEpsIdsIn):
            res = self.SolveDualProblem(DJ,DC,DCExtT,tildeEpsMask)
            mu[tildeEpsMask] = res.x
            hatIdsIn = np.nonzero(mu[len(Dg):]>self.tolLag)[0]

        hatIdsDh = np.full(len(Dh),False)
        hatIdsDh[hatIdsIn ] = True
        hatIds= np.asarray(np.concatenate(([True]*len(Dg), hatIdsDh)),dtype=bool)

        starIdsIn = np.union1d(tildeIdsIn,hatIdsIn).astype(int)
        starIdsDh = np.full(len(Dh),False)
        starIdsDh[starIdsIn] = True
        starIds = np.asarray(np.concatenate(([True]*len(Dg), starIdsDh)), dtype=bool)

        S =  np.linalg.inv(np.dot(DC[hatIds,:],DCExtT[:,hatIds]))
        xiJ = DJExtT - DCExtT[:,hatIds].dot(  S.dot ( DC[hatIds,:].dot( DJExtT ))  )
        starS = np.linalg.inv(DC[starIds,:].dot(DCExtT[:,starIds]))
        xiC = DCExtT[:,starIds].dot( starS.dot(C[starIds]) )

        self.AJ = (self.alphaJ * self.dt)
        normxiJ = self.ComputeNorm(xiJ)
        if normxiJ > 1e-9:
           self.AJ /= normxiJ

        self.AC = (self.alphaC * self.dt)
        normxiC = self.ComputeNorm(xiC)
        if normxiC > 1e-9:
           self.AC /= normxiC

        self.AC = min(0.9, self.AC)
        self.direction = -(self.AJ*xiJ + self.AC*xiC)
        self.gradient = DJExtT
        self.tildeMask = tildeMask
        self.LM = mu
        self.S = np.linalg.inv(np.dot(DC[tildeMask,:],DCExtT[:,tildeMask]))
        self.merit = self.ComputeMeritFunction(J,C)
        self.oldObjective  = self.merit
        self.PrintState()
        self.SaveData()
        Debug("Step optim algo done")


        self.iterOk = False
        self.iter = 0
        for self.iter in range(self.numberOfIterationsMax):
            self.iterOk =self.TryToAdvance()

            if self.iterOk:
                self.IncreaseStepSize()
                break
            else:
                self.DecreaseStepSize()
            if self.iter == self.numberOfIterationsMax:
                print("OptimAlgoNullSpace warning : Maximal number of trials reached. Exiting ")
                self.outputStatus = 'MaxIterReached'
                return RETURN_FAIL

        return RETURN_SUCCESS

    def TryToAdvance(self)->bool:
        """Try to advance. It is done in several steps:
        -Copy the optimization problem and make it advance according to the computed direction/stepsize
        -Evaluate it and compare the new merit function value to the value of the original problem  
        -Check that merit has decreased

        If it does, the advance is accepted, else it is invalid

        Returns
        -------
        bool
            True if optimization advancing allowed, else False
        """
        self.oldObjective = self.merit
        OptiProblem = self.CopyCurrentOptimProblem()
        okAdvance = self.Advance(optimProblem=OptiProblem)
        if not okAdvance:
            Debug("Not ok to advance")
            self.iter +=1
            self.iterOk = False
            return self.iterOk
        
        okUpdateValues = self.UpdateOptimProblem(optimProblem=OptiProblem)
        if okUpdateValues != RETURN_SUCCESS:
            Debug("Not ok update values, physical problem evaluation failed ")
            self.iter +=1
            self.iterOk = False
            return self.iterOk
        else:
            self.optimproblemUpdated = True
       
        (newJ, newg, newh)           = self.GetValuesFromOptimProblem(optimProblem=OptiProblem)
        newC = np.concatenate((newg, newh))
        newMerit = self.ComputeMeritFunction(newJ,newC)
        self.ComputeGain(newMerit)
        self.PrintCurrentState(OptiProblem, newMerit)

        res = newMerit < (1+np.sign(self.merit)*self.tol_merit)*self.merit
        if res:
            self.iterOk = True
            self.AcceptOptimProblemIteration(OptiProblem)
            self.optimProblem.point = OptiProblem.point 
            self.merit = newMerit
            Debug("ok merit function decreasing ")
        else:
            Debug("Merit function is not decreasing")
            self.iterOk = False
            self.iter +=1

        return self.iterOk

    def ComputeNorm(self,data:np.ndarray)->float:
        """Compute infinite norm of a quantity

        Parameters
        ----------
        data : np.ndarray
            data

        Returns
        -------
        float
            data infinite norm value
        """
        return np.linalg.norm(data, np.inf)

    def ComputeEps(self,Dh:np.ndarray)->np.ndarray:
        eps = np.zeros(len(Dh))

        for i in range(len(Dh)):
            eps[i] = -self.K*self.dt*np.sum(abs(Dh[i,:]), 0)

        return eps

    def ComputeTildeIds(self,h,eps:Optional[np.ndarray]=None)->np.ndarray:
        if eps is None:
           eps = np.zeros(len(h))

        tildeIds = np.nonzero(h[:]>=eps[:])[0]

        return tildeIds

    def ComputeMeritFunction(self,J:float,C:np.ndarray)->float:
        """Compute merit function

        Parameters
        ----------
        J : float
            objective value
        C : np.ndarray
            constraints value

        Returns
        -------
        float
            merit function value
        """
        AJ = self.AJ
        AC = self.AC
        LM = self.LM
        S = self.S
        indicesEps = self.tildeMask

        return AJ*(J+LM[indicesEps].dot(C[indicesEps]))+0.5*AC*C[indicesEps].dot( S.dot(C[indicesEps] ) )

    def SolveDualProblem(self,DJ:np.ndarray,DC:np.ndarray,DCExtT:np.ndarray,tildeEpsIds:Iterable[int])->SimpleNamespace:
        """Solve dual problem

        Parameters
        ----------
        DJ : np.ndarray
            Objective sensitivity
        DC : np.ndarray
            Constraints sensitivity
        DCExtT : np.ndarray
            _description_
        tildeEpsIds : Iterable[int]
            _description_

        Returns
        -------
        SimpleNamespace
            internal dual problem solution
        """
        qpP = sparse.csc_matrix(DC[tildeEpsIds, :].dot(DCExtT[:, tildeEpsIds]))
        qpA = sparse.csc_matrix(np.concatenate((np.zeros((len(tildeEpsIds), 0)), -np.eye(len(tildeEpsIds))), axis=1))
        qpq = DJ.dot(DCExtT[:, tildeEpsIds])
        qph = np.zeros((len(tildeEpsIds), 1))

        options = {'verbose': False,
        'eps_rel': 1e-14, 'eps_abs': 1e-14,
        'eps_prim_inf': 1e-14, 'eps_dual_inf': 1e-14}

        qp = osqp.OSQP()
        qp.setup(qpP, qpq, qpA, None, qph, **options)
        res = qp.solve()

        return res


RegisterClass("OptimAlgoNullSpace", None, CreateOptimAlgoNullSpace)


def CheckIntegrityTopo(GUI=False):
    OA = OptimAlgoNullSpace()
    OA.ResetState()

    from OpenPisco.TestData.TestCases import TestOptimTopoCubeFlexion
    OF = TestOptimTopoCubeFlexion("OptimAlgoNullSpace",factor=1)
    OA.numberOfDesignStepsMax = 5
    OA.numberOfIterationsMax = 5
    OA.optimProblem = OF
    OA.outputEveryIter = False
    OA.alphaC = 1.
    OA.alphaJ = 1.
    OA.dt = 0.02
    OA.K = 0.1
    OA.L = 0.1
    OA.tol_merit=0.0005
    OA.Start()
    return "ok"


def CheckIntegrity2D(GUI=False):
    from OpenPisco.TestData.TestCases import TestOptim2DParabol
    OA = OptimAlgoNullSpace()
    OA.numberOfDesignStepsMax = 5
    originalProblem = TestOptim2DParabol()
    if GUI :
        originalProblem.InitWriter()
    OA.optimProblem = originalProblem
    op = id(originalProblem)
    print(op)
    print(OA.optimProblem)
    OA.stepSizeMax = 0.5
    OA.ResetState()

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

    OA.optimProblem = originalProblem

    OA.addInequalityConstraintsConstribution = True
    OA.numberOfIterationsMax = 1
    OA.outputEveryIter = True

    OA.ResetState()
    OA.stepSizeMax = 1.0
    OA.numberOfDesignStepsMax = 5
    OA.alphaC = 0.2
    OA.alphaJ = 1
    OA.dt = 0.1
    OA.K = 0.5

    OA.Start()
    return "ok"

def CheckIntegrity(GUI=False):
    tests=[
        CheckIntegrityTopo,
        CheckIntegrity2D
           ]

    for test in tests:
        res = test(GUI=GUI)
        if res.lower() != "ok":
            return res
    return "ok"

if __name__ == '__main__':
    CheckIntegrity(GUI=True)
