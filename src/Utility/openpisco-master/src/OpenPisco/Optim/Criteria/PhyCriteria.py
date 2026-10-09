# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module is dedicated to the the most common physical criteria in solid mechanics for topology optimization
"""
from __future__ import annotations
from typing import Optional

import numpy as np

from Muscat.Types import MuscatFloat
import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.Logger import Info
from Muscat.FE.SymWeakForm import GetField, GetTestField, Strain, ToVoigtEpsilon
from Muscat.FE.MaterialHelp import HookeIso
from Muscat.FE.Fields.FieldTools import VectorToFEFieldsData
from Muscat.FE.Fields.FEField  import FEField
from Muscat.FE.Integration import IntegrateGeneral
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.Spaces.FESpaces import ConstantSpaceGlobal
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
from OpenPisco.Unstructured.Levelset import LevelSet
import OpenPisco.TopoZones as TZ
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS

class TopoCriteriaCompliance(PhysicalCriteriaBase):
    """
    Physical criterion for the compliance in linear elasticity
    """
    def __init__(self, other:Optional[TopoCriteriaCompliance]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaCompliance], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaCompliance, self).__init__(other)
        self.update = True

        if other is None:
            self.SetName("Compliance")
            self.f_val = -1.
        else:
            self.point = other.point
            self.f_val = other.f_val
            self.fSensitivity_val = other.fSensitivity_val

    def GetNumberOfSolutions(self)->int:
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions
        """
        return 1

    def GetCriteriaSolution(self,i:int)->np.ndarray:
        """Retrieve the solution of index i. The index i should be strictly smaller than the total number of solutions (here 1).
        The nodal solution is the static displacement of a linear elasticity problem
        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        np.ndarray
            solution value
        """
        return self.problem.GetNodalSolution()

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
        return "Dep"

    def SetAuxiliaryQuantities(self,levelSet:LevelSet):
        """Define auxiliary physical quantities for a linear elasticity problem, required to compute the criteria

        They are the following:
        - The potential energy
        - The integral of potential energy over the domain

        Parameters
        ----------
        levelSet : LevelSet
            levelset object
        """
        self.problem.SetAuxiliaryFieldGeneration(FN.potential_energy,on=FN.Nodes)
        self.problem.SetAuxiliaryScalarGeneration(FN.int_potential_energy)

    def UpdateValues(self,point:LevelSet):
        r"""Update value and sensitivity of the criterion for the current levelset

        At each update, the following linear elasticity problem must be solved. 

        .. math::
            \begin{equation}
            \left\{
            \begin{aligned}
            \text {div} (\sigma(u)) + f &= 0  \qquad &\text{ in } \Omega,\\
            \sigma(u) \cdot n &= g  \qquad &\text{ on } \Gamma_N, \\
            \sigma(u) \cdot n &= 0  \qquad &\text{ on } \Gamma, \\
            u &= 0\qquad &\text{ on  } \Gamma_D.\\
            \end{aligned}
            \right.
            \end{equation}

        As the compliance criterion with linear elasticity is auto-adjoint, only a single physical problem update is needed. Its value is given by
        .. math::

            J(\Omega) = \int_{\Omega}{\sigma(u):\varepsilon (u)\:dx}

        Parameters
        ----------
        point : LevelSetBase
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        self.point = point

        self.f_val = self.problem.GetAuxiliaryScalar(FN.int_potential_energy)*2
        self.fSensitivity_val = self.problem.GetAuxiliaryField(FN.potential_energy,on=FN.Nodes)*2
        self.fSensitivity_val = -self.fSensitivity_val.flatten()
        return RETURN_SUCCESS

RegisterCriteriaClass("Compliance", TopoCriteriaCompliance)

class TopoCriteriaDisplacementDeviation(PhysicalCriteriaBase):
    """
    Physical criterion for the displacement deviation in linear elasticity
    """
    def __init__(self, other:Optional[TopoCriteriaDisplacementDeviation]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaDisplacementDeviation], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super().__init__(other)

        if other is None:
            self.SetName("DisplacementDeviation")
            self.nTag1 = None
            self.nTag2 = None
            self.__dir = np.array([1,0,0],dtype=MuscatFloat)
            self.f_val = -1.
            self.absValue = False
        else:
            self.nTag1 = other.nTag1
            self.nTag2 = other.nTag2
            self.__dir = np.copy(other.__dir)
            self.f_val = other.f_val
            self.absValue = other.absValue

    def SetDeviationDirection(self, data:str):
        """Define deviation direction

        Parameters
        ----------
        data : str
            deviation direction data
        """
        val = PH.ReadFloats(data)
        u0 = np.linalg.norm(val)
        self.__dir = val/u0

    def UpdateValues(self,levelSet:LevelSet)->int:
        r"""Update value and sensitivity of the criterion for the current levelset

        At each update, the following linear elasticity problem must be solved. 

        .. math::
            \begin{equation}
            \left\{
            \begin{aligned}
            \text {div} (\sigma(u)) + f &= 0  \qquad &\text{ in } \Omega,\\
            \sigma(u) \cdot n &= g  \qquad &\text{ on } \Gamma_N, \\
            \sigma(u) \cdot n &= 0  \qquad &\text{ on } \Gamma, \\
            u &= 0\qquad &\text{ on  } \Gamma_D.\\
            \end{aligned}
            \right.
            \end{equation}
        
        The criterion value is given by
        
        .. math::

            J(\Omega) = |u_{nTag}*dir -u_0|^2

        Parameters
        ----------
        levelSet : LevelSet
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        cleanMesh = self.problem.internalSolveur.unknownFields[0].mesh
        cleanMeshSpace = self.problem.internalSolveur.unknownFields[0].space
        cleanMeshNumbering = self.problem.internalSolveur.unknownFields[0].numbering
        self.u_num = self.problem.GetNodalSolution().copy()

        # save
        uSave = self.problem.internalSolveur.unknownFields
        saveOldSol = self.problem.internalSolveur.sol
        # put new fields in the internal solver
        # we do this to reuse the same solver for the adjoint problem
        self.problem.internalSolveur.unknownFields = [f.copy() for f in self.problem.internalSolveur.unknownFields]
        u = np.array(uSave)

        id1 = cleanMesh.nodesTags[self.nTag1].GetIds()
        assert len(id1) == 1, "Case not handled"

        dof1 = cleanMeshNumbering.GetDofOfPoint(id1)
        u1 = np.array([ self.problem.internalSolveur.unknownFields[x].data[dof1] for x in range(3)])

        if self.nTag2 is not None:
            id2 = cleanMesh.nodesTags[self.nTag2].GetIds()
            assert len(id2) == 1, "the ntag must contain only one node"
            dof2 = cleanMeshNumbering.GetDofOfPoint(id2)
            u2 = np.array([ self.problem.internalSolveur.unknownFields[x].data[dof2] for x in range(3)])
        else:
            u2 = np.array([0,0,0], dtype=MuscatFloat)

        if self.absValue :
            sign = np.sign((u1-u2).dot(self.__dir))
        else:
            sign = 1

        self.f_val = sign*((u1-u2).dot(self.__dir))
        nbdofs = cleanMeshNumbering.size
        f = np.zeros(nbdofs * 3, dtype=MuscatFloat)
        f[[0*nbdofs+dof1,1*nbdofs+dof1,2*nbdofs+dof1]] = self.__dir*sign

        if self.nTag2 is not None:
            f[[0*nbdofs+dof2,1*nbdofs+dof2,2*nbdofs+dof2]] = -self.__dir*sign

        self.problem.internalSolveur.Resolve(f)
        self.problem.internalSolveur.PushSolutionVectorToUnkownFields()

        self.p_num = self.problem.GetNodalSolution()

        fields = []

        for i,f in  enumerate(self.problem.internalSolveur.unknownFields):
            f_field = f.copy()
            f_field.name = "p_"+str(i)
            fields.append(f_field)

        VectorToFEFieldsData(self.problem.internalSolveur.sol,fields)

        self.problem.internalSolveur.sol = saveOldSol
        fields.extend(u)

        ### symbolic part ###
        p = GetField("p",3,sdim=self.cleanMesh.GetPointsDimensionality())
        u = GetField("u",3,sdim=self.cleanMesh.GetPointsDimensionality())

        ### P1 interpolation
        p1space = GetField("P1",1,sdim=self.cleanMesh.GetPointsDimensionality())
        p1spaceTest = GetTestField("P1",1,sdim=self.cleanMesh.GetPointsDimensionality())

        mecaPhysics = self.problem.internalSolveur.physics[0]
        AOperator = HookeIso(GetField("young",1,sdim=self.cleanMesh.GetPointsDimensionality())[0],mecaPhysics.coeffs["poisson"],dim=3)

        if levelSet.conform:
            factor= 1
        else:
            factor = GetField("factor",1,sdim=self.cleanMesh.GetPointsDimensionality())[0]

        numwform = factor*ToVoigtEpsilon(Strain(p)).T*AOperator*ToVoigtEpsilon(Strain(u))*p1spaceTest + p1space.T*p1spaceTest

        grad = FEField("P1",
                      cleanMesh,
                      cleanMeshSpace,
                      cleanMeshNumbering)

        fields.extend(list(self.problem.internalSolveur.fields.values()))

        m,f = IntegrateGeneral( mesh= cleanMesh,
                               wform= numwform,
                               constants= {},
                               fields= fields,
                               unknownFields= [grad]  ,
                               testFields= None,
                               elementFilter = ElementFilter(tag=self.problem.tagForPostProcess),
                               integrationRuleName="NodalEvalGeo",
                               onlyEvaluation=True)

        diag = m.diagonal()
        diag[diag==0] = 1
        f /= diag

        grad.data = f

        self.fSensitivity_val = np.zeros_like(levelSet.phi)
        self.fSensitivity_val[self.problem.cleanMeshToOriginalNodal] = -grad.GetPointRepresentation()

        self.problem.internalSolveur.unknownFields = uSave
        self.problem.internalSolveur.PushUnkownFieldsToSolutionVector()
        return RETURN_SUCCESS

    def GetNumberOfSolutions(self)->int:
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions
        """
        return 2

    def GetCriteriaSolution(self,i:int)->np.ndarray:
        """Retrieve the solution of index i. The index i should be strictly smaller than the total number of solutions (here 4).
        The nodal solutions retrieved are:
        - The solution of the direct problem (displacement, for linear elasticity)
        - The solution of the adjoint problem

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        np.ndarray
            solution value
        """
        criteriaSolution = [self.u_num,self.p_num]
        return criteriaSolution[i]

    def GetCriteriaSolutionName(self,i:int)->str:
        """Get criterion solution name, either the "Dep" (direct solution) or "Adjoint" (adjoint solution)

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        str
            solution name
        """
        criteriaSolutionName = ["Dep","Adjoint"]
        return criteriaSolutionName[i]

RegisterCriteriaClass("DisplacementDeviation", TopoCriteriaDisplacementDeviation)

class TopoCriteriaTargetDisp(PhysicalCriteriaBase):
    """
    Physical criterion for the target displacement in linear elasticity
    """
    def __init__(self, other:Optional[TopoCriteriaTargetDisp]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaTargetDisp], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaTargetDisp, self).__init__(other)
        self.__alpha = 2

        if other is None:
            self.SetName("TargetDisp")
            ### parameter of the compliant mechanism
            self.eTag = None
            self.__dir = np.array([1,0,0],dtype=MuscatFloat)
            self.__u0 = 0.
            self.f_val = -1.
        else:
            self.eTag = other.eTag
            self.__dir = np.copy(other.__dir)
            self.__u0 = other.__u0
            self.f_val = other.f_val

    def SetTargetDisplacement(self, data:str):
        """Set target displacement

        Parameters
        ----------
        data : str
            target displacement data
        """
        val = PH.ReadFloats(data)
        self.__u0 = np.linalg.norm(val)
        self.__dir = val/self.__u0

    def UpdateValues(self,levelSet:LevelSet):
        r"""Update value and sensitivity of the criterion for the current levelset

        At each update, the following linear elasticity problem must be solved. 

        .. math::
            \begin{equation}
            \left\{
            \begin{aligned}
            \text {div} (\sigma(u)) + f &= 0  \qquad &\text{ in } \Omega,\\
            \sigma(u) \cdot n &= g  \qquad &\text{ on } \Gamma_N, \\
            \sigma(u) \cdot n &= 0  \qquad &\text{ on } \Gamma, \\
            u &= 0\qquad &\text{ on  } \Gamma_D.\\
            \end{aligned}
            \right.
            \end{equation}

        The criterion is given by
        J_2(\omega) = (\int_{\omega} k(x) |u-u0|^{\alpha} dx)^\frac{1}{\alpha}
        Update internal quantities
        must compute the value of J_2
        
        The adjoint problem can be formulated as follows
        -div(A*e(p)) = -C_0 k(x) |u -u0|**(\alpha-2) *(u-u0)  // volume loading
                   p = 0                                      // Dirichlet zero
          (A*e(p))*n = 0                                      // no Newmann

        note (u-u0) -> disp_error

        We compute the value of the derivative of J2

        J2'(\omega)(\theta) =  \int_{\omega_N} (  C0/{\alpha}*|u-u0|^2  +       # A)
                                                    A*e(u)*e(p) +                  # B)
                                                -f*p         +                  # C)
                                                -d(g*p)/dn   +                  # D)
                                                -H*g*p                          # E)
                                                ) \theta*n ds +
                                \int_{\omega_D} (  C0/{\alpha}*|u-u0|^2  +       # F)
                                                    A*e(u)*e(p)                    # G)
                                                ) \theta*n ds

        Parameters
        ----------
        point : LevelSetBase
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        cleanMesh = self.problem.internalSolveur.unknownFields[0].mesh
        cleanMeshSpace = self.problem.internalSolveur.unknownFields[0].space
        cleanMeshNumbering = self.problem.internalSolveur.unknownFields[0].numbering
        self.u_num = self.problem.GetNodalSolution().copy()

        # save
        uSave = self.problem.internalSolveur.unknownFields
        saveOldSol = self.problem.internalSolveur.sol
        # put new fields in the internal solver
        # we do this to reuse the same solver for the adjoint problem
        self.problem.internalSolveur.unknownFields = [f.copy() for f in self.problem.internalSolveur.unknownFields]

        # we generate a full field
        u = np.array(uSave)
        disp_error = np.dot(u, self.__dir ) - self.__u0
        disp_error.name = "disp_error"
        C0_arg = np.abs( disp_error )**2
        C0_arg.name = "C0_arg"

        """ Generation of a weak formulation for the integration of c0_args """
        """Sym part """
        gt = GetTestField("gt",1,sdim=cleanMesh.GetPointsDimensionality()) ## unique test function to compute integrals
        C0_arg_sym = GetField("C0_arg",1,sdim=cleanMesh.GetPointsDimensionality()) 
        wf = gt*C0_arg_sym
        g_numbering = ComputeDofNumbering(cleanMesh,ConstantSpaceGlobal)

        gt_num = FEField("gt", cleanMesh, ConstantSpaceGlobal, g_numbering,data=None)
        _,J2_arg = IntegrateGeneral( mesh=cleanMesh, wform=wf, constants={}, fields=[C0_arg], unknownFields=[gt_num], elementFilter= ElementFilter(tag=self.eTag) )
        self.f_val = J2_arg[0]**(1./self.__alpha)
        Info(self.f_val)

        C0 =J2_arg[0]**((1./self.__alpha )-1.)

        #########################   computation of the adjoint problem  #########################
        adjoint_volume_loading  = -C0*disp_error
        adjoint_volume_loading.name = "avj"
        mecaPhysics = self.problem.internalSolveur.physics[0]
        sym_adjoint_volume_loading = (ElementFilter(tag=self.eTag), mecaPhysics.GetForceFormulation(self.__dir,"avj")  )

        self.problem.AddField(adjoint_volume_loading)
        self.problem.Resolve([sym_adjoint_volume_loading])

        self.p_num = self.problem.GetNodalSolution()
        fields = []
        for i,f in  enumerate(self.problem.internalSolveur.unknownFields):
            f_field = f.copy()
            f_field.name = "p_"+str(i)
            fields.append(f_field)

        VectorToFEFieldsData(self.problem.internalSolveur.sol,fields)
        self.problem.internalSolveur.sol = saveOldSol
        fields.extend(u)

        ### symbolic part ###
        p = GetField("p",3,sdim=cleanMesh.GetPointsDimensionality()) 
        u = GetField("u",3,sdim=cleanMesh.GetPointsDimensionality()) 

        ### P1 interpolation
        p1space = GetField("P1",1,sdim=cleanMesh.GetPointsDimensionality()) 
        p1spaceTest = GetTestField("P1",1,sdim=cleanMesh.GetPointsDimensionality()) 

        AOperator = HookeIso(GetField("young",1,sdim=cleanMesh.GetPointsDimensionality())[0],mecaPhysics.coeffs["poisson"],dim=3)

        if levelSet.conform:
            factor= 1
        else:
            factor = GetField("factor",1,sdim=cleanMesh.GetPointsDimensionality())[0]

        numwform = factor*ToVoigtEpsilon(Strain(p)).T*AOperator*ToVoigtEpsilon(Strain(u))*p1spaceTest + p1space.T*p1spaceTest
        ufs = FEField("P1",
                      cleanMesh,
                      cleanMeshSpace,
                      cleanMeshNumbering)

        fields.extend(list(self.problem.internalSolveur.fields.values()))

        m,f = IntegrateGeneral( mesh= cleanMesh,
                               wform= numwform,
                               constants= {},
                               fields= fields,
                               unknownFields= [ufs]  ,
                               testFields= None,
                               elementFilter = ElementFilter( tag=TZ.Bulk),
                               integrationRuleName="NodalEvalGeo",
                               onlyEvaluation=True)

        diag = m.diagonal()
        diag[diag==0] = 1
        f /= diag
        grad = FEField("grad", cleanMesh,
                               cleanMeshSpace,
                               cleanMeshNumbering,
                               data=f)

        self.fSensitivity_val = np.zeros_like(levelSet.phi)
        self.fSensitivity_val[self.problem.cleanMeshToOriginalNodal] = grad.GetPointRepresentation()
        self.problem.internalSolveur.unknownFields = uSave
        self.problem.internalSolveur.PushUnkownFieldsToSolutionVector()
        return RETURN_SUCCESS

    def GetNumberOfSolutions(self)->int:
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions
        """
        return 2

    def GetCriteriaSolution(self,i:int)->np.ndarray:
        """Retrieve the solution of index i. The index i should be strictly smaller than the total number of solutions (here 4).
        The nodal solutions retrieved are:
        - The solution of the direct problem (displacement, for linear elasticity)
        - The solution of the adjoint problem

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        np.ndarray
            solution value
        """
        criteriaSolution = [self.u_num,self.p_num]
        return criteriaSolution[i]

    def GetCriteriaSolutionName(self,i:int)->str:
        """Get criterion solution name, either the "Dep" (direct solution) or "Adjoint" (adjoint solution)

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        str
            solution name
        """
        criteriaSolutionName = ["Dep","Adjoint"]
        return criteriaSolutionName[i]

RegisterCriteriaClass("TargetDisp", TopoCriteriaTargetDisp)

def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
