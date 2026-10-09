# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module is dedicated to the buckling criterion for linear elasticity
"""
from __future__ import annotations
from typing import Optional

import numpy as np

from Muscat.Types import MuscatFloat
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshTools.MeshInspectionTools import GetVolumePerElement

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from  OpenPisco.PhysicalSolvers.PhysicalSolversTools import CellToPointData,SymMatrixToMandelRepresentation
from OpenPisco.Optim.Criteria.PhyMecaCriteria import HookeIsoModified
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL
from OpenPisco.Unstructured.Levelset import LevelSet

class TopoCriteriaBuckling(PhysicalCriteriaBase):
     def __init__(self, other:Optional[TopoCriteriaBuckling]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaBuckling], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaBuckling, self).__init__(other)

        self.update = True
        self.iterationCritLoad = 8
        self.loadcritNumber = None
        if other is None:
            self.SetName("Buckling")
        else:
            self.loadcritNumber = other.loadcritNumber
            self.iterationCritLoad = other.iterationCritLoad
            self.f_val = other.f_val
            self.fSensitivity_val = np.copy(other.fSensitivity_val)

     def GetNumberOfSolutions(self)->int:
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions
        """
        return 4

     def GetCriteriaSolution(self,i:int)->np.ndarray:
        """Retrieve the solution of index i. The index i should be strictly smaller than the total number of solutions (here 4).
        The nodal solutions retrieved are:
        - The criterion sensitivity
        - The von-mises stress field value
        - The static displacement
        - The buckling mode

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        np.ndarray
            solution value
        """
        vmisesAtNodes = self.problem.GetAuxiliaryField(FN.von_mises,on=FN.Nodes)
        raw_sensitivity = self.fSensitivity_val
        buckling_mode = self.problem.GetBucklingMode()
        disp_static = self.problem.GetStaticLinearDisplacement()
        solution=[
                    raw_sensitivity,
                    vmisesAtNodes,
                    disp_static,
                    buckling_mode,
                   ]
        return solution[i]

     def GetCriteriaSolutionName(self,i:int)->str:
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
        solutionName=[
                    "raw_sensitivity",
                    "Vonmises",
                    "disp_static",
                    "buckling_mode",
                    ]
        return solutionName[i]

     def SetAuxiliaryQuantities(self,levelSet:LevelSet):
         """Define auxiliary physical quantities for a linear buckling problem, required to compute the criteria

         They are the following:
         - The critical load
         - The stress tensor
         - The elastic energy
         - The strain buckling
         - The strain green buckling
         - The buckling mode
         - The von-mises stress field

         Parameters
         ----------
         levelSet : LevelSet
             levelset object
         """
         self.loadcritNumber = self.problem.loadcritParams["NumbermaxLoadCrit"]
         self.problem.SetAuxiliaryScalarsbyOrderGeneration(FN.load_crit,listSize=self.loadcritNumber)
         self.problem.SetAuxiliaryFieldGeneration(FN.stress,on=FN.Centroids)
         self.problem.SetAuxiliaryFieldGeneration(FN.elastic_energy_buckling,on=FN.Centroids)
         self.problem.SetAuxiliaryFieldGeneration(FN.strain_buckling,on=FN.Centroids)
         self.problem.SetAuxiliaryFieldGeneration(FN.strain_green_buckling,on=FN.Centroids)
         self.problem.SetAuxiliaryFieldGeneration(FN.mode_buckling,on=FN.Nodes)
         self.problem.SetAuxiliaryFieldGeneration(FN.von_mises,on=FN.Nodes)

     def UpdateValues(self,levelSet:LevelSet):
        r"""Update value and sensitivity of the criterion for the current levelset

        At each update, both a linear buckling problem and the associated adjoint problem must be solved. 
        
        We provide the expression of the adjoint problem below
        sigma(u) = A*e(u)
        div(sigma(p)) = alpha**-1*div( A*Q(u_1,u_1))                 // Omega
        p = 0                                      // Gamma_D
        (sigma(p))*n =  alpha**-1*A*Q(u_1,u_1)*n                     // Gamma_N U Gamma

        The criteria value is given by
        J(\Omega)=\gamma^{-1}(\Omega)
        where gamma is the critical load coefficient.

        Parameters
        ----------
        point : LevelSet
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        lcoef_crit = self.problem.GetArrayofLoadMultiplier()
        mask_lcrit = lcoef_crit>0.
        for _ in range(self.iterationCritLoad):
            if not mask_lcrit.any():
              self.loadcritNumber = 2*self.loadcritNumber
              self.problem.loadcritParams['NumbermaxLoadCrit'] = self.loadcritNumber
              self.problem.SetAuxiliaryScalarsbyOrderGeneration(FN.load_crit,listSize=self.loadcritNumber)
              if self.problem.SolveByLevelSet(levelSet) != RETURN_SUCCESS:
                 print("Error solving " + str(self.problem) )
                 return RETURN_FAIL
              lcoef_crit = self.problem.GetArrayofLoadMultiplier()
              mask_lcrit = lcoef_crit>0.
        lcoef_crit_min  = np.min(lcoef_crit[mask_lcrit])
        assert lcoef_crit_min>0,'Critical load is negative : number of mode has to be increased!'
        coef_lcrit = lcoef_crit_min

        ######### assembly matrix A*Q ##########################

        Vmatrix = np.array([[ 1., -0.5, -0.5,0.,0.,0.],
               [ -0.5, 1., -0.5,0.,0.,0.],
               [ -0.5, -0.5, 1., 0,0.,0],
               [ 0., 0., 0.,3., 0., 0],
               [ 0., 0., 0.,0., 3., 0],
               [ 0., 0., 0.,0., 0., 3.]],dtype=MuscatFloat)

        sigma = self.problem.GetAuxiliaryField(FN.stress,on=FN.Centroids,onCleanMesh=True)
        strain = self.problem.GetAuxiliaryField(FN.strain_buckling,on=FN.Centroids,onCleanMesh=True)
        strain_green = self.problem.GetAuxiliaryField(FN.strain_green_buckling,on=FN.Centroids,onCleanMesh=True)
        elastic_energy = self.problem.GetAuxiliaryField(FN.elastic_energy_buckling,on=FN.Centroids,onCleanMesh=True)
        elemtype = ED.Tetrahedron_4
        tetra = self.problem.cleanMesh.GetElementsOfType(elemtype)
        sigma = sigma[elemtype]
        strain = strain[elemtype]
        strain_green = strain_green[elemtype]
        elastic_energy = elastic_energy[elemtype]
        vol = GetVolumePerElement(self.problem.cleanMesh, ElementFilter(elementType=elemtype))
        alpha = np.sum (elastic_energy.ravel()*vol, axis = 0)
        youngField = self.problem.YoungField[elemtype].ravel()
        poissonField = self.problem.PoissonField[elemtype].ravel()
        Q  = np.zeros_like(sigma)
        AQ = np.zeros_like(sigma)
        strain_inel = np.zeros_like(sigma)
        for tet in range(tetra.GetNumberOfElements()):
            strain_inel[tet,0,:] = strain_green[tet,0,:] - strain[tet,0,:]
            Q[tet,0,:] = np.matmul(np.transpose(strain_inel[tet,0,:]),Vmatrix)
            C = HookeIsoModified(youngField[tet],poissonField[tet],3)
            AQ[tet,0,:] = np.matmul(C,Q[tet,0,:])/alpha

        AdjLoading = {}
        AdjLoading[elemtype] = AQ
        if self.adjointProblem is None:
            self.SetAdjointProblemLikeDirect(self.problem.loadcritParams)
        self.adjointProblem.conform=self.problem.conform
        if self.problem.templateFileName is not None:
            self.adjointProblem.templateFileName = self.problem.templateFileName+"_AsterBucklingCriticLoadAdjoint"
        else:
            self.adjointProblem.templateFileName = "AsterBucklingCriticLoadAdjoint"
        self.adjointProblem.SetSolverInterface()
        self.adjointProblem.SetAuxiliaryFieldGeneration(FN.strain,on=FN.Centroids)
        self.adjointProblem.SetSupport(self.problem.originalSupport)
        self.adjointProblem.cleanMeshToOriginalNodal = self.problem.cleanMeshToOriginalNodal
        self.adjointProblem.cleanMesh = self.problem.cleanMesh
        self.adjointProblem.materials = self.problem.materials
        self.adjointProblem.dirichlet= self.problem.dirichlet
        self.adjointProblem.problems = self.problem.problems
        self.adjointProblem.WriteSupport(self.problem.cleanMesh,
                                  CellFieldsNames = ["AQ"],
                                  CellFields=[AdjLoading],
                                  phi = levelSet.phi)
        self.adjointProblem.residual = self.problem.residual
        self.adjointProblem.SolveWithRelaxedResidual()

        epsiadj = self.adjointProblem.GetAuxiliaryField(FN.strain,on=FN.Centroids,onCleanMesh=True)
        epsiadj = epsiadj[elemtype]
        sensitivityAtTetraData = np.zeros(tetra.GetNumberOfElements(), dtype=MuscatFloat)

        for tet in range(tetra.GetNumberOfElements()):
            sigmamandel =  SymMatrixToMandelRepresentation(sigma[tet,0,:])
            strain_inelmandel =  SymMatrixToMandelRepresentation(strain_inel[tet,0,:])
            epsiadjmandel =  SymMatrixToMandelRepresentation(epsiadj[tet,0,:])
            prodinel = np.dot(sigmamandel,strain_inelmandel)
            prodadj = np.dot(sigmamandel,epsiadjmandel)
            sensitivityAtTetraData[tet] = (elastic_energy[tet,0] + (prodinel*coef_lcrit) - (prodadj*(coef_lcrit**2)))*(-1/(coef_lcrit**2))

        sensitivityAtTetra = {elemtype:sensitivityAtTetraData}
        sensitivity = CellToPointData(self.adjointProblem.cleanMesh,sensitivityAtTetra)
        self.fSensitivity_val = np.zeros(levelSet.support.GetNumberOfNodes() ,dtype=MuscatFloat)
        self.fSensitivity_val[self.problem.cleanMesh.originalIDNodes] = sensitivity
        self.f_val = 1/coef_lcrit
        return RETURN_SUCCESS

RegisterCriteriaClass("Buckling", TopoCriteriaBuckling)

def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
