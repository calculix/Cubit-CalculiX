# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module is dedicated to the Equivalent Radiated Power (ERP).
It is a simplified method to gain information about maximum dynamic radiation of panels for excitations in frequency response analysis.
"""
from __future__ import annotations

from typing import Dict,Tuple,Optional

import numpy as np
import math

import Muscat.Helpers.ParserHelper as PH
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Mesh import Mesh
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.Types import MuscatFloat
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceP1
from Muscat.FE.DofNumbering import ComputeDofNumbering

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS
from OpenPisco.Unstructured.LevelsetTools import ComputeMeanCurvature
from OpenPisco.Unstructured.Levelset import triangle_3d_areas,LevelSet
from OpenPisco.Unstructured.LevelsetTools import ComputeGradientOnTetrahedrons

def CellToPointDataOnSurface(support:Mesh,surfaceTag:str,field:np.ndarray)->np.ndarray:
    """Project cell-valued field to nodes belonging to a surface (return 0 for a node outside the target surface)

    Parameters
    ----------
    support : Mesh
        support mesh
    surfaceTag : str
        surface tag name
    field : np.ndarray
        field to be projected

    Returns
    -------
    np.ndarray
        nodal field
    """
    nodalField = np.zeros( (support.GetNumberOfNodes(),3), dtype = MuscatFloat)
    weight = np.zeros( support.GetNumberOfNodes(), dtype = MuscatFloat)
    tria = support.GetElementsOfType(ED.Triangle_3)
    elems=tria.tags[surfaceTag]
    conn = tria.connectivity       
    weightAtElement = triangle_3d_areas(support.nodes[conn,:])
    for elId in elems.GetIds():
        nodalField[conn[elId,:]] += field[elId]*weightAtElement[elId]
        weight[ conn[elId,:] ] += weightAtElement[elId] 
    ids = np.nonzero (weight > 0)[0]
    redWeight=np.expand_dims(weight[ids], axis=1)
    nodalField[ids] = nodalField[ids]/redWeight
    return nodalField

def triangle_3d_normals(coords:np.ndarray)->np.ndarray:
    """Compute normals for triangular element nodes coordinates

    Parameters
    ----------
    coords : np.ndarray
        nodes coordinates for each triangle

    Returns
    -------
    np.ndarray
        triangles normal vectors coordinates
    """
    assert coords.shape[-1] == 3 and coords.shape[-2] == 3
    return np.cross(
        coords[..., 1, :] - coords[..., 0, :],
        coords[..., 2, :] - coords[..., 0, :])

def CreateTopoCriteriaERP(ops:Dict)->TopoCriteriaERP:
    """Create Equivalent Radiated Power criterion

    The xml version expected is
    <Objective  type="ERP"  name="ERP" useProblem="1" density_flu="1.189e-12" veloc_flu="3.43e5" radia_fact="1.0" radia_Surface="ET3"/>

    Parameters
    ----------
    ops : Dict
        criterion parameters, all parameters described below are mandatory:
        - "density_flu": air density
        - "veloc_flu": sound velocity
        - "radia_fact": radiation factor
        - "radia_Surface": radiation region of interest

    Returns
    -------
    TopoCriteriaERP
        ERP criterion instance

    Raises
    ------
    ValueError
        At least one mandatory parameter was not provided.
    """
    res=TopoCriteriaERP()
    try:
        params={
                "density_flu":PH.ReadFloat(ops["density_flu"]),
                "veloc_flu":PH.ReadFloat(ops["veloc_flu"]),
                "radia_fact":PH.ReadFloat(ops["radia_fact"]),
                "radia_Surface" :PH.ReadString(ops["radia_Surface"])
                  }
    except KeyError as e:
        raise ValueError("parameter "+e.args[0]+" not properly defined for this criteria")

    if "zone" in ops:
        res.nonOptimSurfZone=ops["zone"]
        del(ops["zone"])

    if "optimizableRadiatingSurface" in ops:
        res.optimizableRadiatingSurface=PH.ReadBool(ops["optimizableRadiatingSurface"])
        del(ops["optimizableRadiatingSurface"])

    for parameterName in params.keys():
        del(ops[parameterName])
    res.params=params
    PH.ReadProperties(ops,ops.keys(),res)
    return res

class TopoCriteriaERP(PhysicalCriteriaBase):
    """
    Physical criterion for the ERP in harmonic analysis.
    """
    def __init__(self,other:Optional[TopoCriteriaERP]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaERP], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaERP, self).__init__(other)
        self.update = True

        if other is None:
            self.SetName("ERP")
            self.problem = None
            self.f_val = 0.0
            self.fSensitivity_val = None
            self._params = None
            self.onzone=None
            self.optimizableRadiatingSurface=False
            self.nonOptimSurfZone = None
            self.pureSurfacicOptim = False
        else:
            self.f_val = other.f_val
            self.fSensitivity_val = np.copy(other.fSensitivity_val)
            self._params = other._params
            self.onzone = other.onzone 
            self.optimizableRadiatingSurface=other.optimizableRadiatingSurface
            self.nonOptimSurfZone = other.nonOptimSurfZone
            self.pureSurfacicOptim = other.pureSurfacicOptim

    @property
    def params(self):
        """Criterion parameters getter"""
        return self._params

    @params.setter
    def params(self, value:Dict):
        """Criterion parameters setter

        Parameters
        ----------
        value : Dict
            parameters value

        Raises
        ------
        ValueError
            If a parameter provided is not available
        ValueError
            If a mandatory parameter is missing
        """
        expectedCriteriaParameters=["density_flu","veloc_flu","radia_fact","radia_Surface"]
        #Pseudo Invariants
        if any(Parameter not in expectedCriteriaParameters for Parameter in value.keys()):
            raise ValueError("The following parameters are not available for the ERP criteria ",value.keys()-set(expectedCriteriaParameters))
        elif any(Parameter not in value.keys() for Parameter in expectedCriteriaParameters):
            raise ValueError("The following parameters were expected for the ERP criteria but were not provided",set(expectedCriteriaParameters)-value.keys())
        self._params = value

    def GetNumberOfSolutions(self)->int:
        """Get number of criterion solutions, correspond to the frequency discretization within the physical analysis

        Returns
        -------
        int
            number of solutions
        """
        return self.problem.GetNumberOfSolutions()

    def GetCriteriaSolution(self,i:int,onCleanMesh:Optional[bool]=False)->np.ndarray:
        """Retrieve criterion solution

        Parameters
        ----------
        i : int
            i-th solution of the physical problem
        onCleanMesh : Optional[bool], optional
            whether to retrieve the solution on the whole computational domain (False) or only on the negative part of the levelset (True), by default False

        Returns
        -------
        np.ndarray
            i-th solution of the physical problem
        """
        return self.problem.GetNodalSolution(i,onCleanMesh=onCleanMesh)

    def GetCriteriaSolutionName(self,i:int)->str:
        """Retrieve criterion solution name

        Parameters
        ----------
        i : int
            i-th solution of the physical problem

        Returns
        -------
        str
            i-th solution name
        """
        return f"DepErp_{i}"

    def GetScalarsOutputs(self)->Dict[str,float]:
        """Retrieve scalar outputs from problem. The quantity retrieved are:
        - The modal cumulative mass fraction along each principal directions (X,Y,Z), used to evaluate whether the modal basis quanlity is large enough to consider it an acceptable approximation
        - The eigen frenquencies for each mode within the modal basis

        Returns
        -------
        Dict[str,float]
            Scalar outputs by scalar outputs respective names
        """
        scalarOutputs={}
        if self.problem.name=='AsterHarmonic':
            modalCumulMassdX=self.problem.GetAuxiliaryScalar(FN.ModalCumulMassFractiondX)
            modalCumulMassdY=self.problem.GetAuxiliaryScalar(FN.ModalCumulMassFractiondY)
            modalCumulMassdZ=self.problem.GetAuxiliaryScalar(FN.ModalCumulMassFractiondZ)
            scalarOutputs={
                              "modalCumulMassdX": modalCumulMassdX,
                              "modalCumulMassdY": modalCumulMassdY,
                              "modalCumulMassdZ": modalCumulMassdZ,
                           }
            eigenFreqs = self.problem.GetAuxiliaryScalarsbyOrders(FN.eigen_Frequencies)
            for eigenNum,eigenFreq in enumerate(eigenFreqs):
                scalarOutputs["eigenFreq_"+str(eigenNum)]=eigenFreq

        return scalarOutputs

    def SetAuxiliaryQuantities(self,levelSet:LevelSet):
        """Define auxiliary physical quantites to be computed by the physical analysis for the ERP criterion.
        The following fields are to be computed by the underlying solver:
        - The Equivalent Radiated Power density
        - The Equivalent Radiated Power sensitivity (rely on the physical solver as it leverages on the modal decomposition for both direct and adjoint problem)
        
        The following scalars are to be computed by the underlying solver:
        - The Equivalent Radiated Power value
        - The eigen frenquencies for each mode within the modal basis computed
        - The modal cumulative mass fraction along each principal directions (X,Y,Z) for the AsterHarmonic analysis

        Parameters
        ----------
        levelSet : LevelSet
            levelset object

        """
        if not self.pureSurfacicOptim:
            self.problem.SetAuxiliaryFieldGeneration(FN.ERP_sensitivity,on=FN.Nodes)
        self.problem.SetAuxiliaryFieldGeneration(FN.ERP_density,on=FN.Nodes)
        self.problem.SetAuxiliaryScalarsbyOrderGeneration(FN.ERP,listSize=self.problem.GetNumberOfSolutions()/2)
        if self.problem.name=='AsterHarmonic':
            self.problem.SetAuxiliaryScalarsbyOrderGeneration(FN.eigen_Frequencies,listSize=self.problem.harmonicParams["Numbermaxfreq"])
            self.problem.SetAuxiliaryScalarGeneration(FN.ModalCumulMassFractiondX)
            self.problem.SetAuxiliaryScalarGeneration(FN.ModalCumulMassFractiondY)
            self.problem.SetAuxiliaryScalarGeneration(FN.ModalCumulMassFractiondZ)
        assert self.params is not None,"This criteria can not be computed without its associated parameters."
        self.problem.criteriaParams=self.params

        if self.nonOptimSurfZone is None and self.IsSurfaceOptimizable(levelSet):
            raise ValueError("Radiating surface is not properly followed through iteration")
        elif self.nonOptimSurfZone is not None and self.IsSurfaceOptimizable(levelSet):
            self.problem.followRadiaTag = True
            self.problem.fixedZone = self.nonOptimSurfZone

    def UpdateValues(self,levelSet:LevelSet)->int:
        r"""Update value and sensitivity of the criterion for the current levelset

        At each update, the following frequency response problem must be solved. 

        .. math::

            \begin{equation}
                \label{u}
                \left\{ \begin{array}{rlll}
                    \rho \ddot{u} + c\dot{u} - \text{div}(\sigma (u)) &= & 0 & \text{in }\Omega \times \mathbb{R}^{+}, \\
                    u &= & 0 & \text{on }\Gamma_D\times \mathbb{R}^{+},\\
                    \sigma(u) \cdot n &= &g(t) & \text{on }\Gamma_N\times \mathbb{R}^{+},\\
                    \sigma(u) \cdot n &= &0 & \text{on }\Gamma_R \cup \Gamma_M \times\mathbb{R}^{+}.
                \end{array}
                \right.
            \end{equation}

        In order to compute the Equivalent radiated power sensitivity, the adjoint problem is actually also solved by using the direct problem modal basis decomposition, 
        as described in Grégoire Allaire, Georgios Michailidis. Modal basis approaches in shape and topology optimization of frequency response problems. 
        International Journal for Numerical Methods in Engineering, 2018, 113 (8), pp.1258-1299. ⟨10.1002/nme.5504⟩. ⟨hal-01354162⟩

        The criteria is computed as follows

        .. math::

            J(\Omega) = \frac{1}{\omega_{max}-\omega_{min}}\int_{\omega_{min}}^{\omega_{max}}  \frac{1}{2} f \rho_f c_f \int_{S^e_k} |(\dot{u}_\Re +i \dot{u}_\Im)\cdot n|^2 \:ds \:d\omega,

        Parameters
        ----------
        point : LevelSet
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        #Compute value of criteria
        self.f_val = self.ComputeObjective()

        #Compute Sensitivity
        self.fSensitivity_val=self.ComputeSensitivity(levelSet)
        return RETURN_SUCCESS

    def ComputeObjective(self)->float:
        """Compute criterion value by averaging erp over range of frequency

        Returns
        -------
        float
            criterion value
        """
        erps = self.problem.GetAuxiliaryScalarsbyOrders(FN.ERP)
        harmonicParams=self.problem.harmonicParams
        pulsations=[2*math.pi*(harmonicParams["freqExcitMin"]+delta*harmonicParams["freqExcitStep"]) for delta in range(len(erps))]
        avg_erp=np.trapz(erps,x=pulsations)/(2*math.pi*harmonicParams["freqExcitMax"]-2*math.pi*harmonicParams["freqExcitMin"])
        return avg_erp

    def SolutionAndGradientFromDisp(self,solId:int)->Tuple[Dict,Dict]:
        """Compute solution and gradient of solution at nodes and split each of their respective component

        Parameters
        ----------
        solId : int
            i-th physical problem solution

        Returns
        -------
        Tuple[Dict,Dict]
            i-th physical problem solution by components (3) and i-th physical problem solution gradient by component (9)
        """
        solution=self.GetSolution(solId,onCleanMesh=True)
        numByComponent={"x":0,"y":1,"z":2}
        solByComponent={}
        derivByComponent={}
        for name,numId in numByComponent.items():
            solCompo=solution[:,numId]
            solByComponent["u"+name]=solCompo
            dsol_dnum =ComputeGradientOnTetrahedrons(self.problem.cleanMesh,solCompo)
            for nameDeriv,numIdDeriv in numByComponent.items():
                derivByComponent["u"+name+nameDeriv]=dsol_dnum[:,numIdDeriv]
        return solByComponent,derivByComponent

    def ExtendFieldFromCleanMeshToMesh(self,field:np.ndarray,nbdofPerNodes:int,extendedMesh:Mesh)->np.ndarray:
        """Extend field from clean Mesh (mesh corresponding to the negative part of the level set) to full mesh by adding zeros

        Parameters
        ----------
        field : np.ndarray
            field
        nbdofPerNodes : int
            number of degree of freedom per node
        extendedMesh : Mesh
            Extended mesh

        Returns
        -------
        np.ndarray
            Extended field
        """
        usedNodes = len(self.problem.cleanMeshToOriginalNodal)
        if nbdofPerNodes==1:
            extendedField = np.zeros(extendedMesh.GetNumberOfNodes() ,dtype=MuscatFloat)
            extendedField[self.problem.cleanMeshToOriginalNodal] = field[0:usedNodes]
        else:
            extendedField = np.zeros((extendedMesh.GetNumberOfNodes(),nbdofPerNodes) ,dtype=MuscatFloat)
            extendedField[self.problem.cleanMeshToOriginalNodal] = field[0:usedNodes,:]
        return extendedField


    def SensitivityGradJ(self,extendedMesh:Mesh,sol:Dict,grad:Dict)->np.ndarray:
        r"""Compute one component of the surface sensitivity of the objective for a i-th solution

        Parameters
        ----------
        extendedMesh : Mesh
            Extended mesh
        sol : Dict
            i-th solution of the physical problem
        grad : Dict
            gradient of solution provided

        .. math::

        surf_{SensitivityGradJ} = ((\nabla U)^T (U\cdot normal)\cdot normal)\cdot normal

        Returns
        -------
        np.ndarray
            surface sensitivity component
        """
        #U
        #Concatenation of displacements (nbDof vector)
        ux,uy,uz=sol["ux"],sol["uy"],sol["uz"]
        disp=np.stack((ux,uy,uz),axis=0).transpose()

        #dj_Erp=(U.n)n
        normals=self.normalAtNodes
        proj_un=np.einsum('ij,ij->i', disp, normals)
        dj_Erp =np.einsum("i,ij->ij",proj_un,normals)

        #grad(U).transpose() 
        exx,exy,ezx=grad["uxx"],grad["uxy"],grad["uzx"]
        ex=np.stack((exx,exy,ezx),axis=0).transpose()
        eyx,eyy,eyz=grad["uyx"],grad["uyy"],grad["uyz"]
        ey=np.stack((eyx,eyy,eyz),axis=0).transpose()
        ezx,ezy,ezz=grad["uzx"],grad["uzy"],grad["uzz"]
        ez=np.stack((ezx,ezy,ezz),axis=0).transpose()
        gradTranspose=np.dstack((ex,ey,ez))

        #(grad(U).transpose()dj_Erp).n
        grad_JERP=np.einsum('kij,kj->ki', gradTranspose, dj_Erp)
        surfSensitivity=np.einsum('ij,ij->i', grad_JERP, normals)

        surfSensitivity=self.ExtendFieldFromCleanMeshToMesh(field=surfSensitivity,nbdofPerNodes=1,extendedMesh=extendedMesh)
        return surfSensitivity


    def SensitivityJ(self,extendedMesh:Mesh,sol_re:Dict,sol_imag:Dict)->np.ndarray:
        r"""Compute another component of the sensitivity for the i-th solution

        .. math::

        surf_{SensitivityJ} = \|U \cdot normal\|^2 div(normal)

        Parameters
        ----------
        extendedMesh : Mesh
            extended Mesh
        sol_re : Dict
            real i-th solution
        sol_imag : Dict
            imaginary i-th solution

        Returns
        -------
        np.ndarray
            surface sensitivity component
        """
        ux,uy,uz=sol_re["ux"],sol_re["uy"],sol_re["uz"]
        disp_re=np.stack((ux,uy,uz),axis=0).transpose()
        ux,uy,uz=sol_imag["ux"],sol_imag["uy"],sol_imag["uz"]
        disp_im=np.stack((ux,uy,uz),axis=0).transpose()

        normals=self.normalAtNodes
        ure_n=np.einsum('ij,ij->i', disp_re, normals)
        uim_n=np.einsum('ij,ij->i', disp_im, normals)
        ucomp_n=ure_n+1j*uim_n
        u_sqmodulus=np.absolute(ucomp_n)**2
        u_sqmodulus=self.ExtendFieldFromCleanMeshToMesh(field=u_sqmodulus,nbdofPerNodes=1,extendedMesh=extendedMesh)
        curvatureTerm=np.einsum('i,i->i',u_sqmodulus,self.meanCurveTria)
        return curvatureTerm

    def IsSurfaceOptimizable(self,levelSet:LevelSet)->bool:
        """Check whether radiating surface is optimizable

        Parameters
        ----------
        levelSet : LevelSet
            levelset object

        Returns
        -------
        bool
            True if the radiating surface is optimizable, otherwise False
        """
        if self.optimizableRadiatingSurface:
            return True 
        else:
            ff = ElementFilter( zone = self.onzone,zones=[])
            ff.SetDimensionality(2)
            for _,_,ids in ff(levelSet.support):
                onzoneTria=ids

            tris = levelSet.support.GetElementsOfType(ED.Triangle_3)
            surfacicTag = tris.tags[self.params["radia_Surface"]]
            radiaIds=surfacicTag.GetIds()

            intersect=set(radiaIds) & set(onzoneTria)
            compInter=set(radiaIds)
            surfaceOptimizable= (intersect!=compInter)
            return surfaceOptimizable

    def ComputeNormalsAtNodes(self):
        """Compute triangular element normals at nodes"""
        mesh=self.PhyProblemMesh
        tria = mesh.GetElementsOfType(ED.Triangle_3)
        surfconns = tria.connectivity
        surfPointsInElems = mesh.nodes[surfconns]
        normals=triangle_3d_normals(surfPointsInElems)
        normals=normals.transpose()/np.linalg.norm(normals,axis=1)
        normals[np.isnan(normals)] = 0.0
        normals=normals.transpose()
        self.normalAtNodes=CellToPointDataOnSurface(mesh,self.params["radia_Surface"],normals)

    def ComputeSensitivity(self,levelSet:LevelSet)->np.ndarray:
        r"""Compute sensitivity.
        There are two components:
        -The first one is computed by the physical solver (volumic contribution)
        -The second one is done "by hand", assuming the radiating surface is optimizable.

        The latter has to take into account each excitation frequencies considered in the original physical problem
        Based on the notation previously introduced in the respective methods, the expression of the sensitivity is given by

        .. math::

        sensitivity = sensitivity_{volumic} + sensitivity_{surfacic}

        with 

        .. math::

        sensitivity_{surfacic} =  \rho v_{air} r_{factor}  \int_{\omega_{init}}^{\omega_{final}} \omega^2 (surf_{SensitivityGradJ_Re} + surf_{SensitivityGradJ_Im} + 0.5 surf_{SensitivityJ})


        Parameters
        ----------
        levelSet : LevelSet
            levelset object

        Returns
        -------
        np.ndarray
            complete sensitivity value
        """
        #Classical component of ERP sensitivity
        if not self.pureSurfacicOptim:
            res = self.problem.GetAuxiliaryField(FN.ERP_sensitivity,on="Nodes").flatten()
        else:
            res = np.zeros((levelSet.support.GetNumberOfNodes()))

        if self.IsSurfaceOptimizable(levelSet):
            #Build ERP extra term for optimizable radiating surface
            self.PhyProblemMesh=self.problem.cleanMesh
            mesh=levelSet.support
            self.numbering = ComputeDofNumbering(mesh,LagrangeSpaceP1,fromConnectivity=True)

            self.ComputeNormalsAtNodes()
            self.meanCurveTria=ComputeMeanCurvature(levelSet)

            #Compute surface sensitivity for each frequency
            nbfreq=int(self.GetNumberOfSolutions()/2)
            harmonicParams=self.problem.harmonicParams
            factorERP=self.params["density_flu"]*self.params["veloc_flu"]*self.params["radia_fact"]
            pulsations=[2*math.pi*(harmonicParams["freqExcitMin"]+delta*harmonicParams["freqExcitStep"]) for delta in range(nbfreq)]
            surfSensitivity = np.zeros((nbfreq,mesh.GetNumberOfNodes()))

            for i,pulsation in enumerate(pulsations):
                multfactor=factorERP*pulsation**2
                sol_re,grad_re=self.SolutionAndGradientFromDisp(solId=2*i)
                derSensitivRe=self.SensitivityGradJ(extendedMesh=mesh,sol=sol_re,grad=grad_re)
                sol_im,grad_im=self.SolutionAndGradientFromDisp(solId=2*i+1)
                derSensitivIm=self.SensitivityGradJ(extendedMesh=mesh,sol=sol_im,grad=grad_im)
                curvSensitiv=self.SensitivityJ(extendedMesh=mesh,sol_re=sol_re,sol_imag=sol_im)
                surfSensitivity[i,:]=multfactor*(derSensitivRe+derSensitivIm+0.5*curvSensitiv)

            #Integration on all pulsations
            avg_surfSensitivityNodes=np.trapz(surfSensitivity,x=pulsations,axis=0)

            #Remove surfacic contribution of radiating surface in ERP sensitivity
            tris = mesh.GetElementsOfType(ED.Triangle_3)
            surfacicTag = tris.tags[self.params["radia_Surface"]]
            surfconns = tris.connectivity[surfacicTag.GetIds(),:]
            res[surfconns]=0

            #Compute full sensitivity
            res+=avg_surfSensitivityNodes

        return res

RegisterCriteriaClass("ERP", TopoCriteriaERP,CreateTopoCriteriaERP)


def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
