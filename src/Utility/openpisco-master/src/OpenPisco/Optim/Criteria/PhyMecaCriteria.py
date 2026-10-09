# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations

from typing import Dict,List,Tuple,Optional
import copy

import numpy as np
from sympy import tensorproduct, tensorcontraction

import Muscat.Helpers.ParserHelper as PH
from Muscat.Types import MuscatFloat
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter, ZT_ALL_NODES
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByElementFilter,GetVolume
from Muscat.FE.Fields.IPField import IPField
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceP0
from Muscat.FE.Fields.FEField import FEField

from Muscat.FE.SymWeakForm import Strain, FromVoigtSigma, GetField
from Muscat.FE.MaterialHelp import HookeIso
from Muscat.ImplicitGeometry.ImplicitGeometryOperators import ImplicitGeometryInsideOut

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
from OpenPisco.Optim.Criteria.SemiInfiniteCriteria import SemiInfiniteCriteriaBase
from OpenPisco.BaseElements import elementsTypeSupportedForDimension
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import PhysicalSolverFactory, InitAllPhysicalSolvers
from OpenPisco.PhysicalSolvers.PhysicalSolversTools import ComputeSymMatrixProduct
from OpenPisco.PhysicalSolvers.AsterStatic import AsterStatic, GetMaterialFields
from OpenPisco.MuscatExtentions.IPFieldIntegration import IPFieldIntegration
from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco import RETURN_SUCCESS

DesignSpaceSupportbyFESpaceName = {}
DesignSpaceSupportbyFESpaceName["LagrangeP1"] = ED.Tetrahedron_4
DesignSpaceSupportbyFESpaceName["LagrangeP2"] = ED.Tetrahedron_10

def HookeIsoModified(E:float,nu:float,dim:int)->np.ndarray:
    """Apply proper modification to Hooke law for an isotropic material, depending on problem dimension

    Parameters
    ----------
    E : float
        young modulus
    nu : float
        poisson ratio
    dim : int
        problem dimension

    Returns
    -------
    np.ndarray
        hooke operator for isotropic material

    Raises
    ------
    ValueError
        if dimension provided is neither 2 or 3.
    """
    C = HookeIso(E,nu,dim)
    if dim==3:
        C[3,3] *= 2
        C[4,4] *= 2
        C[5,5] *= 2
    elif dim==2:
        C[2,2] *= 2
    else:
        raise ValueError("Case not covered for dimension "+str(dim))
    return C

def CreateTopoCriteriaVonMises(ops:Dict)->TopoCriteriaVonMises:
    """Create Von mises criterion

    The simplest xml version expected is
    <Constraint type="VonMises" name="VonMises" useProblem="1" UpperBound="1e7" CriteriaOffZone="13"/>
    </Constraint>

    other options involves the very nature of semi infinite criterion

    This version enable a regionalization approach with interlacing submethod and 3 regions decomposition
    <Constraint type="VonMises" name="VonMises" useProblem="1" UpperBound="0.1*11387960" CriteriaOffZone="13" >
    <useMethod type="Regionalization" submethod="Interlacing" numberOfRegions="3"/>
    </Constraint>

    Parameters
    ----------
    ops : Dict
        criterion parameter

    Returns
    -------
    TopoCriteriaVonMises
        VonMises criterion instance
    """
    method = None
    methodParams={}

    if "children" in ops:
        for tag,child in ops["children"]:
            if tag.lower() == "usemethod":
                method = PH.ReadString(child['type'])
                submethod = PH.ReadString(child['submethod'])
                if method.lower()=="regionalization" and "numberOfRegions" in child:
                    methodParams["numberOfRegions"] = PH.ReadInt(child['numberOfRegions'])
        del ops["children"]
    if method is None:
        res = TopoCriteriaVonMises()
    else:
        res = TopoCriteriaVonMises.CriteriaOperation(method,submethod=submethod,params=methodParams)

    if 'zone' in ops:
        zone = ops['zone']
        res.offZone = ImplicitGeometryInsideOut(zone)
        del(ops['zone'])
    PH.ReadProperties(ops,ops.keys(),res)
    return res

class TopoCriteriaVonMises(SemiInfiniteCriteriaBase):
    """
    Physical criterion for the Von Mises criterion in linear elasticity
    """
    def __init__(self, other:Optional[TopoCriteriaVonMises]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaVonMises], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaVonMises, self).__init__(other)

        self.update = True
        self.localValueSupport = FN.IPField

        if other is None:
            self.SetName("VonMises")
            self.f_val = -1
            self.alpha = 2.
            self.fSensitivity_val = np.array([])
            self.offZone = None #implicit zone that should be excluded in the global measurement of stress
            self.firstTime = True
            self.hasUpperBound = False
            self.factor = 1.
            self.automaticNormalization = False
            self.dimensionalitySupport= -1
            self.adjointSolver = 'GeneralAster'
        else:
            self.f_val = other.f_val
            self.alpha = PH.ReadFloat(other.alpha)
            self.fSensitivity_val = np.copy(other.fSensitivity_val)
            self.offZone = other.offZone
            self.firstTime = other.firstTime
            self.factor = other.factor
            self.hasUpperBound = other.hasUpperBound
            self.firstTime = other.firstTime
            self.automaticNormalization = other.automaticNormalization
            self.dimensionalitySupport = other.dimensionalitySupport
            self.adjointSolver = other.adjointSolver
        self.computationalElementFilter = None

    def SetUpperBound(self,bound:float):
        """Set upper bound for inequality constraint

        Parameters
        ----------
        bound : float
            upper bound value
        """
        if self.automaticNormalization:
            self.hasUpperBound = True
            self.factor = PH.ReadFloat(bound)
            self.upperBound = 1.
        else:
            super(TopoCriteriaVonMises, self).SetUpperBound(bound)

    def GetNumberOfSolutions(self):
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions
        """
        return 3

    def GetCriteriaSolution(self,i:int):
        """Retrieve the solution of index i. The index i should be strictly smaller than the total number of solutions (here 3).

        The nodal solution fields are the following:
        - The static displacement of a linear elasticity problem
        - The von mises field
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
        disp=self.problem.GetNodalSolution()
        vmisesAtNodes = self.problem.GetAuxiliaryField(FN.von_mises,on=FN.Nodes)
        disp_adjoint = self.adjointProblem.GetNodalSolution()
        solution=[
                disp,
                vmisesAtNodes,
                disp_adjoint,
                ]
        return solution[i]

    def GetCriteriaSolutionName(self,i:int):
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
                    "Dep",
                    "Vmises",
                    "Dep_Adjoint_VonMises"
                    ]
        return solutionName[i]

    def GetScalarsOutputs(self)->Dict[str,float]:
        """Retrieve scalar outputs from problem. The quantity retrieved is the maximal von mises evaluated at integration points

        Returns
        -------
        Dict[str,float]
            Scalar outputs by scalar outputs respective names
        """
        self.problem.SetAuxiliaryFieldGeneration(FN.von_mises,on=self.localValueSupport)
        vmises = self.problem.GetAuxiliaryField(FN.von_mises,on=self.localValueSupport,onCleanMesh=True)
        dimensionality = self.problem.cleanMesh.GetElementsDimensionality()
        data = vmises.data[elementsTypeSupportedForDimension[dimensionality]]
        scalarOutput={"maxVonMisesAtIP": np.max(data)}
        return scalarOutput

    def CreateAdjointProblemDictProperties(self)->Dict:
        """Create adjoint problem parameters description

        Returns
        -------
        Dict
            adjoint problem parameters description
        """
        adjointDict = copy.deepcopy(self.problem.dictProperties)
        adjointDict["type"] = "static_elastic"
        for tag,mat in adjointDict["children"]:
            if tag in ["Force","Material","ThermalSolver"]:
                adjointDict["children"].remove((tag,mat))
        if self.adjointSolver=="GeneralAster":
            adjointDict["hasPreStress"] = True
        adjointDict["type"] = 'static_elastic'
        for name in ["solveThermalProblem","linearSolverName"]:
            if name in adjointDict.keys():
                del adjointDict[name]
        return adjointDict

    def SetAdjointProblem(self):
        """Set adjoint problem, only AsterSolver and Muscat Unstructured solver are supported"""
        adjointProblemdictProperties = self.CreateAdjointProblemDictProperties()
        InitAllPhysicalSolvers()
        factory = PhysicalSolverFactory()
        availableAdjointSolvers=["GeneralAster","Unstructured"]
        assert self.adjointSolver in availableAdjointSolvers,"Adjoint solver not available"
        self.adjointProblem = factory.Create(self.adjointSolver,ops=adjointProblemdictProperties)
        self.adjointProblem.conform=self.problem.conform
        self.adjointProblem.SetAuxiliaryFieldGeneration(FN.strain,on=FN.IPField)


    def UpdateAndSolveAdjointProblem(self,
                                     materialFieldsNames:List[str],
                                     materialFields:List[np.ndarray],
                                     AdjointLoadFieldName:str,
                                     AdjointLoadField:np.ndarray,
                                     levelSet:LevelSet):
        """Update and solve adjoint problem

        Parameters
        ----------
        materialFieldsNames : List[str]
            material field names
        materialFields : List[np.ndarray]
            material field value
        AdjointLoadFieldName : str
            adjoint loading field name
        AdjointLoadField : np.ndarray
            adjoint loading field value
        levelSet : LevelSet
            levelset object
        """
        self.adjointProblem.SetSupport(self.problem.originalSupport)
        self.adjointProblem.cleanMesh = self.problem.cleanMesh
        self.adjointProblem.cleanMeshToOriginalNodal = self.problem.cleanMeshToOriginalNodal
        
        if self.adjointSolver == "GeneralAster":
            self.adjointProblem.WriteSupport(self.problem.cleanMesh,
                                             cellFieldsNames = materialFieldsNames + [AdjointLoadFieldName],
                                             cellFields=materialFields + [AdjointLoadField] )
            self.adjointProblem.SolveStandard()

        elif self.adjointSolver == "Unstructured":
            dim = self.problem.cleanMesh.GetElementsDimensionality()
            assert dim==3,"Unstructured adjoint not coded in 2D"
            mecaPhysics = self.adjointProblem.internalSolveur.physics[0]
            e = Strain(mecaPhysics.primalTest,dim)
            preSigma = GetField("PreStress",6,sdim=dim)
            wf = tensorcontraction(tensorproduct(FromVoigtSigma(preSigma),e),(0,3),(1,2))
            mecaPhysics.AddLFormulation(self.computationalElementFilter,wf)
            numbering = ComputeDofNumbering(self.problem.cleanMesh, LagrangeSpaceP0)
            preSigmafield = []
            for comp in range(6):
                fieldComponent = FEField(mesh=self.problem.cleanMesh, space=LagrangeSpaceP0, numbering=numbering)
                fieldComponent.Allocate()
                fieldComponent.data = fieldComponent.data.flatten()
                for name,_,_ in self.computationalElementFilter(self.problem.cleanMesh):
                    if name in AdjointLoadField:
                        field = AdjointLoadField[name]  
                        datanumbering = numbering[name]
                        fieldComponent.data[datanumbering] = field[...,comp]
                        fieldComponent.name = "PreStress_"+str(comp)
                preSigmafield.append({AdjointLoadFieldName+"_"+str(comp):fieldComponent})
                
            for field in preSigmafield:
                self.adjointProblem.internalSolveur.fields.update(field)
                
            self.adjointProblem.propsFields = {}
            for key in ["young","poisson"]:
                self.adjointProblem.internalSolveur.fields[key] = self.problem.internalSolveur.fields[key]

            self.adjointProblem.PreSolver(levelSet)
            self.adjointProblem.Solve()
            self.adjointProblem.PostSolver()

    def SetAuxiliaryQuantities(self,levelSet:LevelSet):
        """Define auxiliary physical quantites to be computed by the physical analysis for the ERP criterion.
        The following fields are to be computed by the underlying solver:
        - The stress tensor at integration points
        - the von-mises value at integration points and at nodes
        
        Parameters
        ----------
        levelSet : LevelSet
            levelset object
        """
        self.problem.SetAuxiliaryFieldGeneration(FN.stress,on=FN.IPField)
        self.problem.SetAuxiliaryFieldGeneration(FN.von_mises,on=self.localValueSupport)
        self.problem.SetAuxiliaryFieldGeneration(FN.von_mises,on=FN.Nodes)

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

        In order to compute the criterion sensitivity, the adjoint problem use a particular instance of the linear elasticity problem with a prescribed loading depending on the direct problem solution.

        The adjoint problem to be solved is described as follows
        
        sigma(u) = A*e(u)
        -div(sigma(p)) = div( A*j'(sigma(u))                 // Omega
        p = 0                                      // Gamma_D
        (sigma(p))*n =  -A*j'(sigma(u)*n                     // Gamma_N U Gamma

        equivalent to

        int_Omega { \\sigma(p) : \epsi (v) } = - int_Omega { A*j'(sigma(u) : \epsi(v) }

        Then, the criterion value is given by

        .. math::

            J(\Omega) = \big( \frac{ \int_{\Omega}{ \sigma_{vm}^\alpha  \:dx} }{ \int_{\Omega}{ \:dx}  }\big)^{\frac{1}{\alpha}}

        Parameters
        ----------
        levelSet : LevelSet
            levelset object

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        if self.firstTime:
            if self.automaticNormalization:
                self.InitialiseNormalizationFactor()
            if self.dimensionalitySupport == -1:
                self.dimensionalitySupport = self.problem.cleanMesh.GetElementsDimensionality()
            self.SetAdjointProblem()
            self.firstTime = False

        self.localValues=self.ComputeLocalValues()
        self.computationalElementFilter=self.ComputeComputationalElementFilter()

        self.volume = GetVolume(ExtractElementsByElementFilter(self.problem.cleanMesh,self.computationalElementFilter))
        self.f_val = self.ComputeObjective()
        self.fSensitivity_val=self.ComputeSensitivity(levelSet)
        return RETURN_SUCCESS

    def ComputeLocalValues(self)->float:
        """Compute local values to be aggregated"""
        return self.problem.GetAuxiliaryField(FN.von_mises,on=self.localValueSupport,onCleanMesh=True)

    def ComputeComputationalElementFilter(self)->ElementFilter:
        """Compute element filter if needed. The underlying idea is to avoid taking into account specific zones for this criterion.

        Returns
        -------
        ElementFilter
            Element filter
        """
        if self.offZone is not None:
            return ElementFilter(dimensionality=self.dimensionalitySupport,zone=[self.offZone],zonesTreatment=ZT_ALL_NODES)
        else:
            return ElementFilter(dimensionality=self.dimensionalitySupport)

    def ComputeObjective(self)->float:
        """Compute objective

        Returns
        -------
        float
            objective value
        """
        vmises = self.localValues
        res = IPFieldIntegration((vmises)**self.alpha,self.computationalElementFilter)
        res *= 1.0/self.factor**self.alpha
        self.internalintegral = res
        return (res/self.volume)**(1.0/self.alpha)

    def InitialiseNormalizationFactor(self):
        """Initialize normalization factor (must be done once at the first topology optimization iteration)"""
        if not self.hasUpperBound:
            for elementName,_,ids in ElementFilter(dimensionality=self.problem.cleanMesh.GetElementsDimensionality())(self.problem.cleanMesh):
                data = self.localValues.data[elementName]
                self.factor = np.max(data[ids])

    def GetStressTensorData(self,sigma:np.ndarray)->np.ndarray:
        r"""Retrieve stress tensor data (standardization between Code\_Aster and Muscat)

        Parameters
        ----------
        sigma : np.ndarray
            stress tensor

        Returns
        -------
        np.ndarray
            stress tensor
        """
        if sigma.shape[2] == 4: # code aster
            sigmadata = np.empty((sigma.shape[0], sigma.shape[1], 3))
            sigmadata[:, :, 0] = sigma[:, :, 0]
            sigmadata[:, :, 1] = sigma[:, :, 1]
            sigmadata[:, :, 2] = sigma[:, :, 3]
            return sigmadata
        elif sigma.shape[2] == 3: # muscat
            return np.mean(sigma, axis=1, keepdims=True)
        else:
            return sigma

    def GetPreStressData(self,Ajprime:np.ndarray)->np.ndarray:
        """Define pre-constraint for adjoint problem

        Parameters
        ----------
        Ajprime : np.ndarray
            Pre-constraint

        Returns
        -------
        np.ndarray
            Pre-constraint
        """
        if Ajprime.shape[2] == 3:
            prestress = np.zeros((Ajprime.shape[0], Ajprime.shape[1], 4))
            prestress[:, :, 0] = Ajprime[:, :, 0]
            prestress[:, :, 1] = Ajprime[:, :, 1]
            prestress[:, :, 3] = Ajprime[:, :, 2]
            return prestress
        else:
            return Ajprime

    def ComputeAdjointLoad(self,sigma:np.ndarray,materialFields:Tuple[Dict,Dict])->Tuple[str,Dict]:
        """Compute adjoint load value

        Parameters
        ----------
        sigma : np.ndarray
            stress tensor
        materialFields : Tuple[Dict,Dict]
            material fields

        Returns
        -------
        Tuple[str,Dict]
            field name and load values
        """
        ######### assembly prestress matrix A*j'(sigma(u) ##########################

        Vmatrix3D = np.array([[ 1., -0.5, -0.5,0.,0.,0.],
               [ -0.5, 1., -0.5,0.,0.,0.],
               [ -0.5, -0.5, 1., 0,0.,0],
               [ 0., 0., 0.,3., 0., 0],
               [ 0., 0., 0.,0., 3., 0],
               [ 0., 0., 0.,0., 0., 3.]],dtype=MuscatFloat)

        Vmatrix2D = np.array([[ 1.,-0.5,0.],
               [ -0.5, 1., 0.],
               [ 0., 0.,1.5],],dtype=MuscatFloat)

        VmatrixByDim={"2":Vmatrix2D,"3":Vmatrix3D}

        vmises = self.localValues
        AdjLoading = {}
        youngFieldbyElemNames,poissonFieldbyElemNames = materialFields
        dim = self.problem.cleanMesh.GetElementsDimensionality()

        for name,_,idx in self.computationalElementFilter(self.problem.cleanMesh):
            sigmadata =  self.GetStressTensorData(sigma.data[name])
            vmisesdata = vmises.data[name]
            jprime = np.zeros_like(sigmadata)
            Ajprime = np.zeros_like(sigmadata)
            
            if self.adjointSolver == "Unstructured":
                numbering = ComputeDofNumbering(self.problem.cleanMesh, LagrangeSpaceP0)
                datanumbering = numbering[name]
                youngField = youngFieldbyElemNames[datanumbering].flatten()
                poissonField = poissonFieldbyElemNames[datanumbering].flatten()
            else:
                youngField = youngFieldbyElemNames[name].ravel()
                poissonField = poissonFieldbyElemNames[name].ravel()

            ngauss = sigmadata.shape[1]

            for el in idx:
                C = HookeIsoModified(youngField[el],poissonField[el],dim=dim)
                for ip in range(ngauss):
                    jprime[el,ip,:] = np.transpose(sigmadata[el,ip,:]) @ VmatrixByDim[str(dim)]
                    jprime[el,ip,:] *= self.alpha*(vmisesdata[el,ip].ravel()**(self.alpha-2))/(self.factor)**self.alpha
                    Ajprime[el,ip,:] = np.matmul(C,jprime[el,ip,:])

            AdjLoading[name] = - self.GetPreStressData(Ajprime)

        return "PreStress", AdjLoading

    def ComputeSensitivity(self,levelSet:LevelSet)->np.ndarray:
        """Compute sensitivity

        Parameters
        ----------
        levelSet : LevelSet
            levelset object

        Returns
        -------
        np.ndarray
            sensitivity value
        """
        sigma = self.problem.GetAuxiliaryField(FN.stress,on=FN.IPField,onCleanMesh=True)
        vmises = self.localValues
        
        if self.adjointSolver == "GeneralAster":
            materials=[]
            for tag,child in self.problem.dictProperties["children"]:
                if tag.lower() == "material":
                    material={"young":PH.ReadFloat(child["young"]), "poisson":PH.ReadFloat(child["poisson"])}
                    materials.append( [child.get("eTag","AllZones"),material])
            materialFieldsNames,materialFields = GetMaterialFields(support=self.problem.cleanMesh,
                                                                                    phi=levelSet.phi,
                                                                                    materials=materials,
                                                                                    levelset=levelSet,
                                                                                    eVoid=self.problem.eVoid)
        elif self.adjointSolver == "Unstructured":
            materialFieldsNames=[]
            youngField = self.problem.internalSolveur.fields['young'].data.flatten()
            poissonField = self.problem.internalSolveur.fields['poisson'].data.flatten()
            materialFields = [youngField,poissonField]
            
        AdjointLoadFieldName, AdjointLoadField = self.ComputeAdjointLoad(sigma,materialFields)  
        
        self.UpdateAndSolveAdjointProblem(materialFieldsNames,materialFields,AdjointLoadFieldName,AdjointLoadField,levelSet)

        strain = self.adjointProblem.GetAuxiliaryField(FN.strain,on=self.localValueSupport,onCleanMesh=True)
        energy = IPField("energy",mesh=self.problem.cleanMesh,rule=vmises.rule)
        energy.Allocate()

        for elementName,elems,ids in ElementFilter(dimensionality=self.problem.cleanMesh.GetElementsDimensionality())(self.problem.cleanMesh):
            sigmadata = self.GetStressTensorData(sigma.data[elementName])
            straindata = self.GetStressTensorData(strain.data[elementName])
            ngauss= straindata.shape[1]
            energydata = np.zeros( (elems.GetNumberOfElements(),ngauss), dtype=MuscatFloat)
            for tet in ids:
                for ip in range(ngauss):
                    energydata[tet,ip] = ComputeSymMatrixProduct(sigmadata[tet,ip,:], straindata[tet,ip,:])

            energy.data[elementName] = energydata

        sensitivityIPField = (vmises/self.factor)**self.alpha + energy
        sensitivity = sensitivityIPField.GetPointRepresentation()
        sensitivity = (sensitivity*self.volume - np.ones_like(sensitivity)*self.internalintegral)/(self.volume)**2
        sensitivity *= (self.f_val**(1-self.alpha))*1.0/self.alpha

        fSensitivity_val = np.zeros(levelSet.support.GetNumberOfNodes(),dtype=MuscatFloat)
        usedNodes = len(self.problem.cleanMeshToOriginalNodal)
        fSensitivity_val[self.problem.cleanMeshToOriginalNodal] = sensitivity[0:usedNodes]
        return fSensitivity_val

RegisterCriteriaClass("VonMises",TopoCriteriaVonMises,CreateTopoCriteriaVonMises)


def CreateTopoCriteriaNodalTargetDisp(ops:Dict):
    """Create nodal target displacement criterion

    A xml version expected would be
    <Constraint type="NodalTargetDisp" name="NodalTargetDisp" useProblem="1" u0="1" dir="1 0 0" nTag="MyNodalTag" />
    </Constraint>

    Parameters
    ----------
    ops : Dict
        criterion parameter

    Returns
    -------
    TopoCriteriaVonMises
        VonMises criterion instance
    """
    res = TopoCriteriaNodalTargetDisp()
    if "dir" in ops:
        res.dir = np.array(PH.ReadFloats(ops["dir"]))
        del ops['dir']
    if "u0" in ops:
        res.u0 = PH.ReadFloat(ops["u0"])
        del ops['u0']
    if "nTag" in ops:
        res.nTag = PH.ReadString(ops["nTag"])
        del ops['nTag']
    PH.ReadProperties(ops,ops.keys(),res)
    return res

class TopoCriteriaNodalTargetDisp(PhysicalCriteriaBase):
    """
    Physical criterion for the nodal target displacement in linear elasticity
    """
    def __init__(self, other:Optional[TopoCriteriaNodalTargetDisp]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaNodalTargetDisp], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaNodalTargetDisp, self).__init__(other)

        if other is None:
            self.SetName("NodalTargetDisp")
            self.nTag = None
            self.alpha = 2
            self.dir =None
            self.u0 = 0.
            self.f_val = -1
        else:
            self.nTag = other.nTag
            self.alpha = other.alpha
            self.dir = np.array(PH.ReadFloats(other.dir))
            self.u0 = other.u0
            self.f_val = other.f_val
            self.fSensitivity_val = np.copy(other.fSensitivity_val)

    def SetAuxiliaryQuantities(self,levelSet:LevelSet):
        """Define auxiliary physical quantities for a linear elasticity problem, required to compute the criteria

        Here, the stress is computed at the integration points.

        Parameters
        ----------
        levelSet : LevelSet
            levelset object
        """
        self.problem.SetAuxiliaryFieldGeneration(FN.stress,on=FN.IPField)

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

        In order to compute the criterion sensitivity, the adjoint problem use a particular instance of the linear elasticity problem with a prescribed nodal loading 
        depending on the direct problem solution.

        Then, the criterion value is given by

        .. math::

            J(\Omega) = (\int_{nTag}  |u*dir -u0|^2 dx)

        Parameters
        ----------
        levelSet : LevelSet
            levelset object

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        solution = self.problem.GetNodalSolution(onCleanMesh=True)

        #retrieve node coordinates from nTag
        assert self.nTag in self.problem.cleanMesh.nodesTags,"Nodal Tag "+self.nTag+" not found in mesh"
        ids = self.problem.cleanMesh.nodesTags[self.nTag].GetIds()
        assert len(ids)!=0,"Need a non empty nodal Tag "+self.nTag+" tu update values of Target Displacement criterion "
        errorMessage = "Nodal target displacement criterion: need to use Code_Aster to solve the physical problem"
        assert isinstance(self.problem, AsterStatic),errorMessage

        solutionatnode = solution[ids,:]
        disperror = (np.dot(solutionatnode,self.dir) - self.u0)[0]
        self.f_val = disperror**2
        self.fSensitivity_val = np.zeros_like(levelSet.phi)
        sigma = self.problem.GetAuxiliaryField(FN.stress,on=FN.IPField,onCleanMesh=True)
        elemtype = DesignSpaceSupportbyFESpaceName[self.problem.space]
        sigmadata = sigma.data[elemtype]
        ngauss= sigmadata.shape[1]

        if self.adjointProblem is None:
            self.SetAdjointProblemLikeDirect()
        self.adjointProblem.conform=self.problem.conform
        self.adjointProblem.space=self.problem.space
        self.adjointProblem.SetSupport(self.problem.originalSupport)
        self.adjointProblem.SetAuxiliaryFieldGeneration(FN.strain,on=FN.IPField)
        self.adjointProblem.templateFileName = self.problem.templateFileName
        self.adjointProblem.SetSolverInterface()
        self.adjointProblem.cleanMesh = self.problem.cleanMesh
        self.adjointProblem.cleanMeshToOriginalNodal = self.problem.cleanMeshToOriginalNodal
        self.adjointProblem.materials = self.problem.materials
        self.adjointProblem.dirichlet= self.problem.dirichlet
        self.adjointProblem.problems = self.problem.problems
        loadcase = np.array([0.,0.,0.],dtype=MuscatFloat)
        loadcase[:] = -2*disperror*self.dir
        self.adjointProblem.forcenodale= {"idx1": [[self.nTag,loadcase]]}
        self.adjointProblem.WriteSupport(self.problem.cleanMesh,
                                         phi = levelSet.phi)

        self.adjointProblem.residual = self.problem.residual
        self.adjointProblem.maxAllowedRelaxedResidual = self.problem.maxAllowedRelaxedResidual
        self.adjointProblem.SolveWithRelaxedResidual()
        strain = self.adjointProblem.GetAuxiliaryField(FN.strain,on=FN.IPField,onCleanMesh=True)
        straindata = strain.data[elemtype]
        tetra = self.adjointProblem.cleanMesh.GetElementsOfType(elemtype)

        sensitivityAtTetra = np.zeros( (tetra.GetNumberOfElements(),ngauss), dtype=MuscatFloat)
        for tet in range(tetra.GetNumberOfElements()):
            for ip in range(ngauss):
                prod = np.dot(sigmadata[tet,ip,:][0:3],straindata[tet,ip,:][0:3])+2*np.dot(sigmadata[tet,ip,:][3:],straindata[tet,ip,:][3:])
                sensitivityAtTetra[tet,ip] = prod

        ipfield = IPField("sensitivity",mesh=self.adjointProblem.cleanMesh,ruleName=self.problem.space)
        ipfield.Allocate()
        ipfield.rule[elemtype] = strain.rule[elemtype]
        ipfield.data[elemtype] = sensitivityAtTetra

        sensitivity = ipfield.GetPointRepresentation()

        self.fSensitivity_val = np.zeros(levelSet.support.GetNumberOfNodes() ,dtype=MuscatFloat)
        usedNodes = len(self.problem.cleanMeshToOriginalNodal)
        self.fSensitivity_val[self.problem.cleanMeshToOriginalNodal] = sensitivity[0:usedNodes]
        return RETURN_SUCCESS

    def GetNumberOfSolutions(self):
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions
        """
        return 1

    def GetCriteriaSolution(self,i:int):
        """Retrieve the solution of index i. The index i should be strictly smaller than the total number of solutions (here 1).

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

    def GetCriteriaSolutionName(self,i:int):
        """Get criterion solution name

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

RegisterCriteriaClass("NodalTargetDisp", TopoCriteriaNodalTargetDisp,CreateTopoCriteriaNodalTargetDisp)

def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
