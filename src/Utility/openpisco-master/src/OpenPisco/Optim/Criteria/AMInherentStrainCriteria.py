# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module is dedicated to additive manufacturing inherent strain related criteria
"""

from __future__ import annotations

from typing import Dict,Optional,Optional

import numpy as np

import Muscat.Helpers.ParserHelper as PH
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceP0
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.Fields.FieldTools import TransferFEFieldToIPField
from Muscat.FE.Fields.FieldTools  import NodeFieldToFEField, ElemFieldsToFEField
from Muscat.FE.Fields.IPField import IPField
from Muscat.FE.IntegrationRules import GetIntegrationRuleByName
from Muscat.FE.FETools import PrepareFEComputation
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshTools.MeshFieldOperations import GetFieldTransferOp
from Muscat.Helpers.Logger import Info, Debug

from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco.MuscatExtentions.IPFieldIntegration import IPFieldIntegration
from OpenPisco.PhysicalSolvers.PhysicalSolversTools import  CellToPointData
from OpenPisco.PhysicalSolvers.AMInherentStrainSolver import CreateStructuredAMProcess
from OpenPisco.PhysicalSolvers.AMInherentStrainSolver import AMInherentStrainSolver
from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.PhysicalSolvers.UnstructuredFEAGenericSolver import UnstructuredFEAMecaProblem
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
import OpenPisco.TopoZones as TZ
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS

def CreateTopoCriteriaAMInherentDisp(ops:Dict)->TopoCriteriaAMInherentDisp:
    """Create Additive manufacturing inherent displacement criteria

    Parameters
    ----------
    ops : Dict
        criteria parameters

    Returns
    -------
    TopoCriteriaAMInherentDisp
        instance of criteria

    Raises
    ------
    KeyError
        If beta value not provided
    """
    res = TopoCriteriaAMInherentDisp()
    try:
        params={
                "beta":PH.ReadFloat(ops["beta"])
                  }
    except KeyError as e:
        raise KeyError("parameter "+e.args[0]+" not properly defined for this criteria")

    for parameterName in params.keys():
        del(ops[parameterName])
    res.beta=params["beta"]
    PH.ReadProperties(ops,ops.keys(),res)
    return res

class TopoCriteriaAMInherentDisp(PhysicalCriteriaBase):
     def __init__(self, other:Optional[TopoCriteriaAMInherentDisp]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaAMInherentDisp], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaAMInherentDisp, self).__init__(other)

        if other is None:
            self.SetName("AMInherentStrainDisp")

            self.dir = np.array([0,0,1])
            self.f_val = -1
            self.beta = 1e-7
            self.fieldTransferMethod = "Interp/Clamp"
        else:
            self.dir = np.copy(other.dir)
            self.f_val = other.f_val
            self.beta = other.beta
            self.fieldTransferMethod = other.fieldTransferMethod

     def UpdateValues(self,levelSet:LevelSet)->int:
        r"""Update criteria value/sensitivity

        Objective
        j(u_i) = |max(0, u_i.e_d)|^2
        ==>
        j_approx(u_i) = u_i.e_d^2e^2u_i.e_d\beta / (1 + e^u_i.e_d\beta)^2

        J_2(\omega) = \Sigma_i
                                (\int_{\omega_i}
                                j_approx(u_i)
                                dx)

        We compute the value of the derivative of J

        J2'(\omega)(\theta) =  \Sigma_i \int_{\partial \omega_i}
                                        (
                                            |u_i.e_d|^2 + A(e(u_i) + epsi*):e(p_i)
                                            ) \theta*n ds
                                
        Parameters
        ----------
        levelSet : LevelSet
             levelset object, not actually used in this criteria. We rely on the computational level set of the underlying physical problem
        
        Returns
        -------
        int
             RETURN_SUCCESS is expected if problem suceeded 
        """
        ls = self.problem.computationLevelSet
        computationMesh = ls.support
        usedNodes = np.zeros(computationMesh.GetNumberOfNodes())
        origin,_ = computationMesh.ComputeBoundingBox()
        adjointSolveur = self.CreateAdjointProblem()
        adjointSolveur.internalSolveur.SetMesh(computationMesh)
        adjointSolveur.internalSolveur.ComputeDofNumbering()
        adjointSolveurPhysic=adjointSolveur.internalSolveur.physics[0]

        deltaZ = self.problem.nz
        if self.problem.useProjectionInitialization:
            transOP =  self.problem.GetProjectionInitialization(computationMesh, deltaZ)

        oldSol = np.zeros_like((computationMesh.nodes))
        negPhi = self.problem.computationLevelSet.phi <= 0
        numberingP0 = ComputeDofNumbering(computationMesh, LagrangeSpaceP0)

        JprimeTot = 0
        accumulatedJs = np.zeros((computationMesh.GetNumberOfNodes(),1))

        for layer in range(self.problem.numberOfStepZ):
            directSolution=self.problem.GetNodalSolutionLayer(layer=layer)
            u_i = np.dot(directSolution,self.dir)
            js = np.maximum(0.,u_i) ** 2

            Info("Layer: "+str(layer+1))
            usedNodes.fill(0)
            ls.phi[:] = self.problem.phiByLayer[layer]

            zBulk = origin[2] + deltaZ*(layer+1)
            bulkZone = lambda xyz,zBulkVal = zBulk: xyz[:,2]-(zBulkVal+self.problem.layerDetectionTol)
            bulkElementFilter = ElementFilter(dimensionality=3)
            bulkElementFilter.SetZonesTreatment("CENTER")
            bulkElementFilter.zones = [bulkZone]

            for _,data,ids in bulkElementFilter(computationMesh):
                negPhiIds = np.nonzero(np.sum(negPhi[data.connectivity],axis=1) > 0)[0]
                inside3dIds = np.intersect1d(negPhiIds,ids,assume_unique=True)
                data.tags.CreateTag(TZ.Inside3D,False).SetIds(inside3dIds)
                usedNodes[data.connectivity[inside3dIds,:].ravel()] = True

            js1 = js.reshape((len(js),1))
            js1XLi= js1 * usedNodes[:,None]
            accumulatedJs += js1XLi
            #Compute Rhs adjoint
            #jsPrime = (2.0 / self.beta) * np.log(1 + np.exp( u_i * self.beta)) / (1 + np.exp(u_i *self.beta))
            jsPrime =  2 * np.maximum(0.,u_i)

            u_vol_x = -jsPrime * self.dir[0]
            u_vol_y = -jsPrime * self.dir[1]
            u_vol_z = -jsPrime * self.dir[2]
            u_vol_dir = np.vstack([u_vol_x,u_vol_y,u_vol_z])
            for axisId,axisName in enumerate(["x","y","z"]):
                computationMesh.nodeFields["u_vol_"+axisName] = u_vol_dir[axisId]
                direction=[0]*3
                direction[axisId]=1
                weakVolumicForce=adjointSolveurPhysic.GetForceFormulation(direction,flux="u_vol_"+axisName)
                adjointSolveurPhysic.AddLFormulation( TZ.Inside3D, weakVolumicForce)
                feFields = NodeFieldToFEField(computationMesh, {"u_vol_"+axisName:u_vol_dir[axisId]})
                adjointSolveur.internalSolveur.fields.update(feFields)

            Debug("Solve adjoint problem")
            adjointSolveur.PreSolver(ls)
            adjointSolveur.AddField(self.problem.XliByLayer[layer])
            if self.problem.useProjectionInitialization:
                oldSol = transOP.dot(oldSol)
                adjointSolveur.SetNodalSolution(oldSol)
            adjointSolveur.Solve()
            Debug("Solve adjoint problem Done")

            #Reset solver rhs to original value: add opposite volumic force to weak formulation
            for axisId,axisName in enumerate(["x","y","z"]):
                direction=[0]*3
                direction[axisId]=-1
                weakVolumicForce=adjointSolveurPhysic.GetForceFormulation(direction,flux="u_vol_"+axisName)
                adjointSolveurPhysic.AddLFormulation( TZ.Inside3D, weakVolumicForce)
                feFields = NodeFieldToFEField(computationMesh, {"u_vol_"+axisName:u_vol_dir[axisId]})
                adjointSolveur.internalSolveur.fields.update(feFields)

            XliField = adjointSolveur.GetAuxiliaryField("XLi", on=FN.IPField)
            adjointSolution = [ adjointSolveur.GetAuxiliaryField("u_"+str(i),on =FN.FEField)  for i in range(3) ]
            inside3DElementFilter = ElementFilter(eTag=TZ.Inside3D,dimensionality=3)
            strain = self.problem.ComputeStrain(numberingP0, adjointSolveurPhysic, adjointSolution, XliField, on=inside3DElementFilter,computationMesh=computationMesh)
            strainElem = np.array([s.GetCellRepresentation()*self.problem.bulkAlphaByLayer[layer] for s in strain ])

            stress = self.problem.stressByLayer[layer]
            stressElem = np.array([s.GetCellRepresentation()*self.problem.bulkAlphaByLayer[layer] for s in stress ])

            energyElem = np.sum(stressElem.T * strainElem.T, axis=1)
            computationMesh.elemFields["energy"] = energyElem
            energyElemField = ElemFieldsToFEField(computationMesh, {"energy":energyElem})
            energyAtIp = TransferFEFieldToIPField(energyElemField["energy"],ruleName="LagrangeIsoParamQuadrature",elementFilter=ElementFilter(dimensionality=3,eTag="3D"))
            energyElem = {ED.Tetrahedron_4:energyAtIp.data[ED.Tetrahedron_4].ravel()}
            energyAtNodes = CellToPointData(computationMesh,energyElem)

            JPrimeLayer = js + energyAtNodes
            JprimeTot += JPrimeLayer

        computationMesh.nodeFields["js"] = accumulatedJs
        originalSupport = ls.support
        originalSupport.nodeFields["JprimeTot"] = JprimeTot

        js_field = NodeFieldToFEField(computationMesh, {"js":accumulatedJs})
        js_ip = TransferFEFieldToIPField(js_field["js"],ruleName="LagrangeIsoParamQuadrature",elementFilter=ElementFilter(dimensionality=3,eTag="3D"))
        tointegrate= IPField("",mesh=computationMesh,rule=GetIntegrationRuleByName("ElementCenter"))
        tointegrate.Allocate()
        tointegrate.SetDataFromNumpy(js_ip.data[ED.Tetrahedron_4].flatten(),elementFilter=ElementFilter(dimensionality=3,eTag="3D"))
        ff = ElementFilter(dimensionality=3)
        criteriaValueTot = IPFieldIntegration(tointegrate, ff)
        self.f_val = criteriaValueTot
        Info(self.f_val)

        space, numberings, _, _ = PrepareFEComputation(originalSupport,numberOfComponents=1)
        tempfield = FEField("",mesh=originalSupport,space=space,numbering=numberings[0])
        targetSupport = self.problem.initialSupport
        op, _, _ = GetFieldTransferOp(tempfield,targetSupport.nodes,method="Interp/Clamp" )
        projectedJprimeTot = op.dot(JprimeTot)
        self.fSensitivity_val = projectedJprimeTot

        return RETURN_SUCCESS

     def CreateAdjointProblem(self)->UnstructuredFEAMecaProblem:
        """Create adjoint problem

        -div(A*e(p_i)) = -j'(u_i)  // volume loading
        p = 0                      // Dirichlet zero
        (A*e(p))*n = 0             // no Newmann

        Returns
        -------
        UnstructuredFEAMecaProblem
             adjoint problem
        """
        adjointSolveur =UnstructuredFEAMecaProblem()
        adjointSolveur.tagsToKeep = self.problem.tagsToKeep
        young = self.problem.propsFields["young"][0][1]
        poisson = self.problem.propsFields["poisson"][0][1]
        adjointSolveur.internalSolveur.constants["young"] = young
        adjointSolveur.internalSolveur.constants["poisson"] = poisson
        phys = self.problem.mecaInherentStrain
        phys.inherentStrainsCoefficients = np.array([0, 0, 0])
        adjointSolveur.internalSolveur.physics.append(phys)

        for dirichlet in self.problem.dirichletBCs:
            adjointSolveur.internalSolveur.solver.constraints.AddConstraint(dirichlet)

        return adjointSolveur

     def GetNumberOfSolutions(self)->int:
        """Get number of criterion solutions

        Returns
        -------
        int
            number of criterion solution to be considered (here there is a single solution)
        """
        return 1
     
     def GetCriteriaSolutionName(self,i:int=0)->str:
         """Get criteria field name

         Parameters
         ----------
         i : int
             i-th solution of the problem, default is 0

         Returns
         -------
         str
             field name corresponding to index (here there is a single solution)
         """
         return "Dep"


     def GetCriteriaSolution(self,i:int=0)->np.ndarray:
        """Get criteria field solution value

        Parameters
        ----------
        i : int
            i-th solution of the problem, default is 0

        Returns
        -------
        np.ndarray
            the i-th solution of the problem (here there is a single solution)
        """
        originalSupport = self.problem.computationLevelSet.support
        nbNodes=originalSupport.GetNumberOfNodes()
        dispLayerNormalSum=np.zeros(nbNodes)
        for layerId in range(self.problem.numberOfStepZ):
            disp = self.problem.GetNodalSolutionLayer(layer=layerId)
            dispLayerNormal = np.dot(disp,self.dir)
            dispLayerNormalSum+=dispLayerNormal

        targetSupport = self.problem.initialSupport
        space, numberings, _, _ = PrepareFEComputation(originalSupport,numberOfComponents=3)
        tempfield = FEField("",mesh=originalSupport,space=space,numbering=numberings[0])
        op, _, _ = GetFieldTransferOp(tempfield,targetSupport.nodes,method=self.fieldTransferMethod )
        projSol = op.dot(dispLayerNormalSum)
        return projSol

RegisterCriteriaClass("AMInherentStrainDisp", TopoCriteriaAMInherentDisp, CreateTopoCriteriaAMInherentDisp)


class TopoCriteriaAMIStrainVonMises(PhysicalCriteriaBase):
     def __init__(self, other:Optional[TopoCriteriaAMIStrainVonMises]=None):
        super(TopoCriteriaAMIStrainVonMises, self).__init__(other)
        """Constructor of criteria

        Parameters
        ----------
        other : Optional[TopoCriteriaAMIStrainVonMises], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        if other is None:
            self.SetName("AMIStrainVonMises")
            self.alpha = 2.
            self.f_val = -1
            self.K1 = 0.
        else:
            self.f_val = other.f_val
            self.fSensitivity_val = np.copy(other.fSensitivity_val)
            self.alpha = PH.ReadFloat(other.alpha)
            self.K1 =  PH.ReadFloat(other.K1)

     def ComputeValue(self)->float:
        """Compute objective value

         Returns
         -------
         float
             objective value
        """
        tointegrate= IPField("",mesh=self.problem.vonMisesByLayer[0].mesh,rule=GetIntegrationRuleByName("ElementCenter"))
        tointegrate.Allocate()
        accumulatedVonMises =  [ self.problem.vonMisesByLayer[i].data for i in range(self.problem.numberOfStepZ)]
        tointegrate.SetDataFromNumpy(abs( sum(accumulatedVonMises) )**(2*self.alpha))

        res = IPFieldIntegration(tointegrate,ElementFilter(dimensionality=3))

        young = self.problem.GetMaterialPropertyCoefficient('young')
        poisson = self.problem.GetMaterialPropertyCoefficient('poisson')
        mu = young/(1+2*poisson)
        self.K1 = 4*mu*res**(1./self.alpha -1)
        return res**(1.0/self.alpha)

     def CreateAdjointProblem(self)->AMInherentStrainSolver:
        """Create adjoint problem

         Returns
         -------
         AMInherentStrainSolver
             adjoint problem instance
        """
        young = self.problem.GetMaterialPropertyCoefficient('young')
        poisson = self.problem.GetMaterialPropertyCoefficient('poisson')
        data = {
        "type":'IStrains',
        "nx":self.problem.nx,
        "ny":self.problem.ny,
        "nz":self.problem.nz,
        "IStrains":'0. 0. 0.',
        # TO DO : read blocks from direct problem
        'children':[
        ('Material',{'young': young, 'poisson': poisson, 'density': '1'}),
        ('Block', {'on': 'Z0', 'block': '0 1 2'}),
        ('Block', {'on': 'X0', 'block': '0'}),
        ('Block', {'on': 'Y0', 'block': '1'})]}

        return CreateStructuredAMProcess(data)

     def ComputeAdjointPreSigma(self,adjointProblem:AMInherentStrainSolver):
        """Compute adjoint problem pre-constraint state

         Parameters
         ----------
         adjointProblem : AMInherentStrainSolver
             adjoint problem
        """
        adjointProblem.addPreconstraintFormulation = True
        accumulatedVonMisesnorm =  [ abs(self.problem.vonMisesByLayer[layer].data)**(2*self.alpha-2) for layer in range(self.problem.numberOfStepZ)]
        for i in range(6):
            accumulateddeviatoricStress =   [self.problem.deviatoricstressByLayer[layer][i].GetCellRepresentation() 
                                             for layer in range(self.problem.numberOfStepZ)]
            adjointProblem.elemFields["PreSigma_"+str(i)] = self.K1*accumulatedVonMisesnorm[i]*sum(accumulateddeviatoricStress)

     def ComputeSensitivity(self,adjointProblem:AMInherentStrainSolver,levelSet:LevelSet)->np.ndarray:
        """Compute sensitivity

        Parameters
        ----------
        adjointProblem : AMInherentStrainSolver
             adjoint problem
        levelSet : LevelSet
             levelset object

        Returns
        -------
        np.ndarray
             sensitivity
        """
        computationMesh = self.problem.computationLevelSet.support
        zAtIP = self.problem.GetZPosAtIntegrationPoints(computationMesh)
        accumulatedJPrime = 0.
        origin, boundingMax = levelSet.support.ComputeBoundingBox()
        numberOfStepZ = self.problem.nz 
        deltaZ = (boundingMax[2]-origin[2])/(numberOfStepZ-1)

        for layer in range(numberOfStepZ):
            if layer == 20 :
                break

            zBulk = origin[2] + deltaZ*(layer+1)
            bulkAlpha = (zAtIP.GetCellRepresentation() < zBulk)*1.0

            strain = adjointProblem.strainByLayer[layer]
            strainElem = np.array([s.GetCellRepresentation()*bulkAlpha for s in strain ])
            stress = self.problem.stressByLayer[layer]
            stressElem = np.array([s.GetCellRepresentation()*bulkAlpha for s in stress ])
            energyElem = np.sum(stressElem.T * strainElem.T, axis=1)
            vonMisesNorm = (1./self.alpha)*self.f_val**(1-self.alpha)*abs(self.problem.vonMisesByLayer[layer].data)**(2*self.alpha)
            energyElem += vonMisesNorm
            accumulatedJPrime += energyElem
       
        energyField = ElemFieldsToFEField(computationMesh, {"energy":accumulatedJPrime})
        energy_ip = TransferFEFieldToIPField(energyField["energy"],ruleName="LagrangeIsoParamQuadrature",elementFilter=ElementFilter(dimensionality=3,eTag="3D"))
        energy_tetra = energy_ip.data[ED.Tetrahedron_4]
        field = {ED.Tetrahedron_4:energy_tetra.ravel()}
        Jprimenodal = CellToPointData(computationMesh,field)

        originalSupport = computationMesh
        targetSupport = levelSet.support
        space, numberings, _, _ = PrepareFEComputation(originalSupport,numberOfComponents=1)
        tempfield = FEField("",mesh=originalSupport,space=space,numbering=numberings[0])
        op, _, _ = GetFieldTransferOp(tempfield,targetSupport.nodes,method=self.fieldTransferMethod )
        projectedJprimeTot = op.dot(Jprimenodal)

        return projectedJprimeTot

     def UpdateValues(self,levelSet:LevelSet)->int:
        """Update criteria value/sensitivity
                  
        Parameters
        ----------
        levelSet : LevelSet
             levelset object
        
        Returns
        -------
        int
             RETURN_SUCCESS is expected if problem suceeded 
        """
        self.f_val = self.ComputeValue()
        adjointProblem = self.CreateAdjointProblem()
        self.ComputeAdjointPreSigma(adjointProblem)
        adjointProblem.SolveByLevelSet(levelSet)
        self.fSensitivity_val = self.ComputeSensitivity(adjointProblem,levelSet)
        return RETURN_SUCCESS

     def GetNumberOfSolutions(self)->int:
         """Get number of underlying fields

         Returns
         -------
         int
             number of solution fields to be considered (here there is a single solution)
         """
         return 1
     
     def GetCriteriaSolutionName(self,i:int=0)->str:
         """Get criteria field name

         Parameters
         ----------
         i : int
             i-th solution of the problem, default is 0

         Returns
         -------
         str
             field name corresponding to index (here there is a single solution)
         """
         return "Dep"

     def GetCriteriaSolution(self,i:int=0)->np.ndarray:
         """Get criteria field solution value

         Parameters
         ----------
         i : int
            i-th solution of the problem, default is 0

         Returns
         -------
         np.ndarray
            the i-th solution of the problem (here there is a single solution)
         """
         return self.problem.GetNodalSolution()

RegisterCriteriaClass("AMIStrainVonMises", TopoCriteriaAMIStrainVonMises)

def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
