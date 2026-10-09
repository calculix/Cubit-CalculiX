# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations
from typing import Optional,Dict

import numpy as np

from Muscat.Types import MuscatFloat
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshTools.MeshInspectionTools import GetVolumePerElement
from Muscat.MeshTools.MeshFieldOperations import PointToCellData

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco import RETURN_SUCCESS

def CreateTopoCriteriaComplianceRobustLoadWorstCase(ops:Optional[Dict]=None):
    """Create compliance robust load worst case criterion

    The xml version expected is
    <Objective  type="ComplianceRobustLoadWorstCase"  name="ComplianceRobustLoadWorstCase" useProblem="1"/>


    Parameters
    ----------
    ops : Dict
        modal criterion parameter, NUM_MODE_OPTIM (mandatory) refers to the mode arising from the modal analysis to be optimized.
        For instance, NUM_MODE_OPTIM="1" means the criteria take into account the first mode.

    Returns
    -------
    TopoCriteriaComplianceRobustLoadWorstCase
        Compliance robust load worst case criterion instance
    """
    res=TopoCriteriaComplianceRobustLoadWorstCase()
    return res

class TopoCriteriaComplianceRobustLoadWorstCase(PhysicalCriteriaBase):
    """
    Physical criterion for the Von Mises criterion in linear elasticity
    """
    def __init__(self, other:Optional[TopoCriteriaComplianceRobustLoadWorstCase]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaComplianceRobustLoadWorstCase], optional
            another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaComplianceRobustLoadWorstCase, self).__init__(other)
        self.f_val = 0.0
        self.fSensitivity_val = None
        self.update = True
        self.alphamax = None
        self.amplitude = None
        self.defaultSeed = 42
        if other is None:
            self.SetName("ComplianceRobustLoadWorstCase")
        else:
            self.f_val = other.f_val
            self.fSensitivity_val = np.copy(other.fSensitivity_val)
            self.point = other.point
            self.alphamax = other.alphamax
            self.amplitude = other.amplitude

    def GetNumberOfSolutions(self):
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions
        """
        return 1

    def GetCriteriaSolution(self,i:int)->np.ndarray:
        """Get criteria solution

        Parameters
        ----------
        i : int
            solution id for the physical problem

        Returns
        -------
        np.ndarray
            i-th solution of the physical problem
        """
        return self.problem.GetNodalSolution()

    def GetCriteriaSolutionName(self,i:int)->str:
        return "Dep"

    def refFrameOneVector(self,v1:np.ndarray)->np.ndarray:
        """Build new vector reference frame from canonical vector frame

        Parameters
        ----------
        v1 : np.ndarray
            shape (3,)

        Returns
        -------
        np.ndarray
            shape (3,3)
        """
        axes_glob = [(1.,0,0),(0,1.,0),(0,0,1.)]
        v1_normal= v1/np.linalg.norm(v1)
        v1_tuple = tuple(v1_normal)
        v1_in_glob = v1_tuple in axes_glob
        if v1_in_glob:
           aa = axes_glob
           aa.remove(v1_tuple)
           v2_tuple = aa[0]
           v2_normal = np.array(v2_tuple)
           v3_normal = np.cross(v1_normal,v2_normal)
        else:
           if v1_tuple[0]==0:
              v2_tuple = (1.,0,0)
              v2_normal = np.array(v2_tuple)
              v3_normal = np.cross(v1_normal,v2_normal)
           else:
              v2 = np.cross(v1_normal,np.array([1.,0,0]))
              v2_normal = v2/np.linalg.norm(v2)
              v3_normal = np.cross(v1_normal,v2_normal)
        return np.array([v1_normal,v2_normal,v3_normal])

    def forceConique(self,v1:np.ndarray,alpha:float,beta:float)->np.ndarray:
        """Compute conique force

        Parameters
        ----------
        v1 : np.ndarray
            force, shape (3,)
        alpha : float
            angle relative to x of local coordinate rad
        beta : float
            angle relative to y of local coordinate rad

        Returns
        -------
        np.ndarray
            new force expressed in a different frame, shape (3,)
        """
        refloc = self.RefFrameOneVector(v1)
        refloc = np.transpose(refloc)
        v1_norm = np.linalg.norm(v1)
        f1x1 = v1_norm*np.cos(alpha)
        f1y1 = v1_norm*np.sin(alpha)*np.cos(beta)
        f1z1 = v1_norm*np.sin(alpha)*np.sin(beta)
        f1_loc = np.array([f1x1,f1y1,f1z1])
        f1_glob = np.dot(refloc,f1_loc)
        return f1_glob

    def magnitudePerturbed(self,v1:np.ndarray,alphamax:float,amplitude:float)->float:
        """Compute perturbation between nominal and conique force 

        Parameters
        ----------
        v1 : np.ndarray
            nominal load, shape (3,)
        alphamax : float
            factor of perturbed force on conique force
        amplitude : float
            angle relative to x and y of local coordinate rad

        Returns
        -------
        float
            distance between conic force and nominal force
        """
        fc = self.ComputeForceConique(v1,alphamax,alphamax)*amplitude
        fp = fc - v1
        return np.linalg.norm(fp)

    def UpdateNeumann(self)->Dict:
        """Update neumann condition

        Returns
        -------
        Dict
            Updated neumann condition
        """
        neumann = self.problem.neumann
        neumann_n = dict()
        for problemId in self.problem.problems:
            if problemId in neumann.keys():
                neumann_n[problemId] = []
                for idl,loadcase in enumerate(neumann[problemId]):
                    load = np.array(loadcase[1])
                    alphamax = self.alphamax[problemId]
                    amplitude= self.amplitude[problemId]
                    rng = np.random.default_rng()
                    alpha,beta = rng.uniform(-alphamax[idl],alphamax[idl],2)
                    fc = self.forceConique(load,alpha,beta)
                    fp = (fc - load)*amplitude[idl]
                    fa = load+fp
                    neumann_n[problemId].append([loadcase[0],list(fa)])
        return neumann_n

    def SetAuxiliaryQuantities(self,levelSet:LevelSet):
        """Define auxiliary physical quantites to be computed by the physical analysis for this criterion.
        The following fields are to be computed by the underlying solver:
        - The potential_energy at nodes
        - the stress at centroids
        
        The integral of the potential energy over the domain is also to be computed by the physical solver.

        Parameters
        ----------
        levelSet : LevelSet
            levelset object
        """
        self.problem.neumman = self.UpdateNeumann()
        self.problem.SetAuxiliaryFieldGeneration(FN.potential_energy,on=FN.Nodes)
        self.problem.SetAuxiliaryScalarGeneration(FN.int_potential_energy)
        self.problem.SetAuxiliaryFieldGeneration(FN.stress,on=FN.Centroids)

    def UpdateValues(self,levelSet:LevelSet):
        """Update value and sensitivity of the criterion for the current levelset

        At each update, a linear elasticity problem is solved. 
        
        The adjoint problem is given by

        div(sigma(p)) = 0                 // Omega
        p   = 0                // Gamma_D
        (sigma(p))*n   = 4*u                     // Gamma_N U Gamma

        Parameters
        ----------
        point : LevelSet
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        self.point = levelSet

        self.fSensitivity_val = np.zeros_like(levelSet.phi)
        displ = self.problem.GetNodalSolution()
        elastic_energy = self.problem.GetAuxiliaryField(FN.potential_energy,on=FN.Centroids,onCleanMesh=True)
        sigma = self.problem.GetAuxiliaryField(FN.stress,on=FN.Centroids,onCleanMesh=True)
        int_elastic_energy = self.problem.GetAuxiliaryScalar(FN.int_potential_energy)
        vol = GetVolumePerElement(self.problem.cleanMesh, ElementFilter(elementType=ED.Tetrahedron_4))

        tris = self.problem.cleanMesh.GetElementsOfType(ED.Triangle_3)
        surfconn = tris.connectivity
        Un = np.zeros((tris.GetNumberOfElements(),3), dtype=MuscatFloat)

        for triangle in range(tris.GetNumberOfElements()):
            Un[triangle,:] = np.mean(displ[surfconn[triangle,:]])

        AdjLoading = {}
        AdjLoading[ED.Triangle_3] = 4*Un
        if self.adjointProblem is None:
            self.SetAdjointProblemLikeDirect()
        self.adjointProblem.conform=self.problem.conform
        self.adjointProblem.solverInterface.filename="AsterComplianceRobustLoadWorstCaseAdjoint"
        self.adjointProblem.SetSupport(self.problem.originalSupport)
        self.adjointProblem.cleanMesh = self.problem.cleanMesh
        self.adjointProblem.materials = self.problem.materials
        self.adjointProblem.dirichlet= self.problem.dirichlet
        self.adjointProblem.problems = self.problem.problems
        self.adjointProblem.WriteSupport(self.problem.cleanMesh,
                                    CellFieldsNames = ["Un"],
                                    CellFields=[AdjLoading],
                                    phi = levelSet.phi)
        self.adjointProblem.Solve()

        epsiadj = self.adjointProblem.GetAuxiliaryField(FN.strain,on=FN.Centroids,onCleanMesh=True)
        Q = self.adjointProblem.GetNodalSolution()
        tetra = self.adjointProblem.cleanMesh.GetElementsOfType(ED.Tetrahedron_4)
        conn = tetra.connectivity
        sensitivityAtTetra = np.zeros(tetra.GetNumberOfElements(), dtype=MuscatFloat)

        Id = self.problem.problems[0]
        neumanns = self.problem.neumann[Id]
        alphamax = self.alphamax[Id]
        amplitude= self.amplitude[Id]
        
        U = PointToCellData(self.problem.cleanMesh,displ)
        for tet in range(tetra.GetNumberOfElements()):
            prodadj = np.dot(sigma[tet,:],epsiadj[tet,:])
            tot = 0
            for i,neumann in enumerate(neumanns):
                ids = tris.GetTag(neumann[0]).GetIds()
                U_norm = np.linalg.norm(U[ids])
                g = np.array(neumann[1])
                m = self.magnitudePerturbed(g,alphamax[i],amplitude[i])
                tot+= -(m/(2*U_norm))*prodadj

            sensitivityAtTetra[tet] = elastic_energy[tet] + tot

        sensitivity = np.zeros(self.adjointProblem.cleanMesh.GetNumberOfNodes() ,dtype=MuscatFloat)
        weight = np.zeros(self.adjointProblem.cleanMesh.GetNumberOfNodes() ,dtype=MuscatFloat)

        for tet in range(tetra.GetNumberOfElements()):
            sensitivity[conn[tet,:]] += vol[tet]*sensitivityAtTetra[tet]
            weight[ conn[tet,:] ] += vol[tet]

        for p in range(self.adjointProblem.cleanMesh.GetNumberOfNodes()):
            tot = 0
            for i,neumann in enumerate(neumanns):
                ids = tris.GetTag(neumann[0]).GetIds()
                U_norm = np.linalg.norm(U[ids])
                g = np.array(neumann[1])
                m = self.magnitudePerturbed(g,alphamax[i],amplitude[i])
                tot+=(m/(2*U_norm))*np.dot(g,Q[p]) + np.dot(U[p],U[p])*m/U_norm

            if weight[p]> 1e-6:
                sensitivity[p] = tot+sensitivity[p]/weight[p]

        tot=0
        for i,neumann in enumerate(neumanns):
            ids = tris.GetTag(neumann[0]).GetIds()
            U_norm = np.linalg.norm(U[ids])
            g = np.array(neumann[1])
            m = self.magnitudePerturbed(g,alphamax[i],amplitude[i])
            tot+=m*U_norm

        self.f_val = int_elastic_energy + tot
        self.fSensitivity_val = np.zeros(levelSet.support.GetNumberOfNodes() ,dtype=MuscatFloat)
        self.fSensitivity_val[self.problem.cleanMesh.originalIDNodes] = sensitivity

        return RETURN_SUCCESS

RegisterCriteriaClass("ComplianceRobustLoadWorstCase", TopoCriteriaComplianceRobustLoadWorstCase)

def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
