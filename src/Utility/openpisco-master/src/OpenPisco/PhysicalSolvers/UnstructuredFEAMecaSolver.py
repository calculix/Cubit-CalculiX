# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from collections import defaultdict

import numpy as np
from scipy import interpolate
from sympy import Identity

from Muscat.Helpers.Logger import Debug

from Muscat.FE.UnstructuredFeaSym import UnstructuredFeaSym
from Muscat.FE.SymWeakForm import GetField,GetTestField
from Muscat.FE.SymPhysics import MechPhysics
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.FETools import PrepareFEComputation
from Muscat.FE.Fields.FieldTools import FieldsMeshTransportation
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceP0
from Muscat.FE.SymWeakForm import ToVoigtEpsilon, ToVoigtSigma, Strain, Inner, Diag, FromVoigtSigma, GetScalarField
from Muscat.FE.Fields.FieldTools import FillFEField
import Muscat.FE.SymWeakForm as wf
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.Types import MuscatFloat

from OpenPisco.PhysicalSolvers.UnstructuredFEASolverBase import UnstructuredFEASolverBase
from OpenPisco.PhysicalSolvers.UnstructuredFEAThermalSolver  import UnstructuredFEAThermalProblem 
from OpenPisco.PhysicalSolvers.AnsysMechanicalThermal import AnsysMechanicalThermal
import OpenPisco.PhysicalSolvers.FieldsNames as FN
import OpenPisco.TopoZones as TZ


class UnstructuredFEAMecaProblem(UnstructuredFEASolverBase):
    def __init__(self,dim=3):
        super().__init__()

        self.internalSolveur = UnstructuredFeaSym(spaceDim=dim)

        self.auxiliaryScalarGeneration[FN.int_potential_energy] = False
        self.auxiliaryFieldGeneration[FN.potential_energy][FN.Nodes] = False
        self.auxiliaryFieldGeneration[FN.tr_stress_][FN.Nodes] = False
        self.auxiliaryFieldGeneration[FN.tr_strain_][FN.Nodes] = False
        self.auxiliaryFieldGeneration[FN.von_mises][FN.Nodes] = False
        self.auxiliaryFieldGeneration[FN.stress][FN.IPField] = False
        self.auxiliaryFieldGeneration[FN.strain][FN.IPField] = False
        self.auxiliaryFieldGeneration[FN.von_mises][FN.IPField] = False

        self.propsFields = defaultdict(list)
        self.transportFieldOP = FieldsMeshTransportation(oldMesh=None, newMesh=None)
        self.__numberingCache__ = {}
        self.dim = dim
        self.delayedInitialization = None
        self.extraRHSVector = dict()
        self.primalUnknownNames = ["u_"+str(i) for i in range(self.dim)]
        self.dirichletNodeFilters = []
        self.neumannNodeFilters = []
        self.dictProperties = {}

    def GetBoundaryConditionsFilters(self):
        return self.dirichletNodeFilters,self.neumannNodeFilters
    
    def InterpolateRadialTemperature(self,nodes,radius,temperatureField):
        meshRadii = np.sqrt( nodes[:,1]**2 + nodes[:,2]**2)
        f = interpolate.interp1d(radius, temperatureField,fill_value="extrapolate")
        return f(meshRadii)

    def GetInputTemperatureOnNodes(self,nodes):
        if self.inputTemperature.shape[1] == 5:  
            coordinates =  self.inputTemperature[:,1:4]
            temperatureField =  self.inputTemperature[:,4]
            return interpolate.griddata(coordinates, temperatureField,nodes, method="nearest")
        elif self.inputTemperature.shape[1] == 2:
                radius,temperatureField = self.inputTemperature
                return self.InterpolateRadialTemperature(nodes,radius,temperatureField)
        else:
            inputTemperature = self.inputTemperature["data"]
            if inputTemperature.shape[1] == 5:  
                coordinates =  inputTemperature[:,1:4]
                temperatureField =  inputTemperature[:,4]
                return interpolate.griddata(coordinates, temperatureField,outMesh.nodes, method="nearest")
            elif inputTemperature.shape[1] == 2:
                    radius,temperatureField = inputTemperature
                    return self.InterpolateRadialTemperature(outMesh.nodes,radius,temperatureField)
            else:
                raise Exception("Input temperature field has incorrect shape ")
        
    def youngModulusFunctionImplementation(self):
        temperature = self.tabularYoungModulus[:,0]  
        youngModulus  = self.tabularYoungModulus[:,1]  
        return interpolate.interp1d(temperature,youngModulus,fill_value="extrapolate")
    
    def inputTemperatureFunctionImplementation(self):
        coordinates =  self.inputTemperature[:,1:4]
        temperatureField =  self.inputTemperature[:,4]
        return interpolate.NearestNDInterpolator(coordinates,temperatureField) 
        
    def youngModulusFunction(self,points): 
        return  self.fYoung(self.fTemperature(points))
    
    def PreSolver(self,levelSet):
        self.conform = levelSet.conform
        self.ComputeDomainToTreatLs(levelSet)
        self.transportFieldOP.ResetCacheData()
        self.numbering = None

        #TODO I have a problem with the boundary conditions and the use of an old
        # solution vector, soo for the moment I reset the values.
        # This has no impact because we use a direct solver
        self.internalSolveur.Reset()

        mecaPhysics = self.internalSolveur.physics[0]
        mecaPhysics.bilinearWeakFormulations = []
        
        if self.conform :
            tokeep = [ self.DomainToTreat ]
            mecaPhysics.AddBFormulation( self.DomainToTreat, mecaPhysics.GetBulkFormulation(young="young" ) ) 

        else:
            factor = GetField("factor",1,sdim=self.cleanMesh.GetPointsDimensionality())[0]
            mecaPhysics.AddBFormulation( TZ.Bulk, mecaPhysics.GetBulkFormulation(alpha=factor,young="young" )  )
            numbering = ComputeDofNumbering(self.cleanMesh, LagrangeSpaceP0, elementFilter=ElementFilter(eTag=TZ.Bulk) )

            self.internalSolveur.fields["factor"] = FEField(name="factor",
                                       mesh=self.cleanMesh,
                                       space=LagrangeSpaceP0,
                                       numbering=numbering,
                                       data=self.densities)

            tokeep = [TZ.Bulk]

        tokeep.extend(self.tagsToKeep)
        self.internalSolveur.SetMesh(self.cleanMesh)
        self.internalSolveur.ComputeDofNumbering(tokeep)

class MecaThermalStrain(MechPhysics):
    def __init__(self,dim=3):
        super().__init__()
        
    def GetStressVoigt(self, utGlobal, HookeLocalOperator=None, deltaTemperature=None, factor=None):
        """Function to compute the stress using the Inherent Strain, the XLi is the field where the inherent Strain is Active"""
        if deltaTemperature is None:
            deltaTemperature = GetScalarField("deltaTemperature")
            thermalExpansion = GetScalarField("thermalExpansion")

        thermalExpansionOp = Diag([thermalExpansion]*self.spaceDimension)

        if HookeLocalOperator is None:
            HookeLocalOperator = self.GetHookeOperator(GetScalarField("young"), self.coeffs["poisson"], factor)
        return HookeLocalOperator @ ToVoigtEpsilon(Strain(utGlobal,self.spaceDimension) + thermalExpansionOp*deltaTemperature )

    def Stress(self, u, deltaTemperature=None, factor=None):
        stressVoigt = self.GetStressVoigt(u, deltaTemperature=deltaTemperature, factor=factor)
        return FromVoigtSigma(stressVoigt)

    def PostTreatmentFormulations(self):
        """For the moment this work only if GetBulkFormulation is called only once per instance
        the problem is the use of self.Hook"""

        deltaTemperature = GetScalarField("deltaTemperature")
        thermalExpansion = GetScalarField("thermalExpansion")
        thermalExpansionOp = Diag([thermalExpansion]*self.spaceDimension)

        uGlobal = self.primalUnknown
        utLocal = self.materialOrientations * uGlobal

        nodalEnergyT = GetTestField("potential_energy",1,sdim=self.spaceDimension)
        symEnergy = 0.5 * wf.ToVoigtEpsilon(wf.Strain(utLocal, self.spaceDimension) -  thermalExpansionOp*deltaTemperature).T * self.HookeLocalOperator * wf.ToVoigtEpsilon(wf.Strain(utLocal, self.spaceDimension) -  thermalExpansionOp*deltaTemperature) * nodalEnergyT

        trStrainT = GetTestField("tr_strain_", 1,sdim=self.spaceDimension)
        symTrStrain = wf.Trace(wf.Strain(uGlobal, self.spaceDimension)) * trStrainT

        trStressT = GetTestField("tr_stress_", 1,sdim=self.spaceDimension)
        symTrStress = wf.Trace(self.Stress(uGlobal)) * trStressT

        stressT = GetTestField("stress", int(0.5 * self.spaceDimension * (self.spaceDimension + 1)), sdim=self.spaceDimension)
        
        symStress = Inner(ToVoigtSigma(self.Stress(uGlobal)),stressT)

        strainT = GetTestField("strain", int(0.5 * self.spaceDimension * (self.spaceDimension + 1)), sdim=self.spaceDimension)
        symStrain = Inner(ToVoigtEpsilon(wf.Strain(uGlobal)),strainT)

        squaredvonmisesT = GetTestField("squared_von_mises", 1,sdim=self.spaceDimension)
        trStress = wf.Trace(self.Stress(uGlobal))
        DeviatorStress = wf.ToVoigtSigma( self.Stress(uGlobal)) - 1./self.spaceDimension * trStress * wf.ToVoigtSigma( Identity((self.spaceDimension)))
        squaredvonmises = 0.5 * self.spaceDimension * ( DeviatorStress.T * DeviatorStress * squaredvonmisesT )

        postQuantities = {"tr_stress_":symTrStress, "tr_strain_":symTrStrain, "squared_von_mises": squaredvonmises, "potential_energy":symEnergy, "stress":symStress, "strain": symStrain}

        return postQuantities


class UnstructuredFEAMecaThermalStrainProblem(UnstructuredFEAMecaProblem):
    def __init__(self,dim=3):
        super().__init__(dim=dim)
        self.solveThermalProblem = True
        self.inputTemperature = None
        
    def thermalExpansionFunctionImplementation(self):
        temperature = self.tabularThermalExpansion[:,0]  
        thermalExpansion  = self.tabularThermalExpansion[:,1]  
        return interpolate.interp1d(temperature,thermalExpansion,fill_value="extrapolate")
            
    def thermalExpansionFunction(self,points): 
        return  self.fThermalExpansion(self.fTemperature(points))

    def PreSolver(self,levelSet):
        if self.solveThermalProblem:
            self.thermalSolver.PreSolver(levelSet)
        super().PreSolver(levelSet)

    def Solve(self):
        if self.solveThermalProblem:
            self.thermalSolver.Solve()
            
            if  isinstance(self.thermalSolver, UnstructuredFEAThermalProblem ):
                T = self.thermalSolver.GetAuxiliaryField(self.thermalSolver.primalUnknownNames[0],on=FN.FEField)
                self.transportFieldOP.newMesh = self.cleanMesh
                T = self.transportFieldOP.TransportFieldToNewMesh(T)
            elif isinstance(self.thermalSolver, AnsysMechanicalThermal ):
                space, numberings, _, _ = PrepareFEComputation(self.cleanMesh,numberOfComponents=1)
                data = self.thermalSolver.GetNodalSolution() 
                dataOnCleanMesh = np.zeros(self.cleanMesh.GetNumberOfNodes(),dtype=MuscatFloat)
                dataOnCleanMesh = data[self.cleanMesh.originalIDNodes] 
                T = FEField(name="T",mesh=self.cleanMesh,space=space,numbering=numberings[0],data=dataOnCleanMesh )

        else:
            space, numberings, _, _ = PrepareFEComputation(self.cleanMesh,numberOfComponents=1)
            T = FEField(name="T",mesh=self.cleanMesh,space=space,numbering=numberings[0],data=self.GetInputTemperatureOnNodes(nodes=self.cleanMesh.nodes))

        numberingP0 = ComputeDofNumbering(mesh=self.cleanMesh, space= LagrangeSpaceP0)
        Debug("Numbering done")
        for fieldName, fieldDefinition in self.propsFields.items():
            if fieldName == "referenceTemperature":
                res = FEField(name=fieldName, mesh=self.cleanMesh,space=T.space ,numbering=T.numbering)
            else:
                res = FEField(name=fieldName, mesh=self.cleanMesh,space=LagrangeSpaceP0,numbering=numberingP0)
            res.Allocate()
            FillFEField(res,fieldDefinition)
            self.internalSolveur.fields[fieldName] = res
        Debug("FillFEField done")

        referenceTemperature = self.internalSolveur.fields["referenceTemperature"]
        deltaTemperature = T - referenceTemperature
        deltaTemperature.name = "deltaTemperature"
        self.internalSolveur.fields["deltaTemperature"] = deltaTemperature

        Debug(self.internalSolveur.totalNumberOfDof)
        Debug(self.internalSolveur.unknownFields)
        k,f = self.internalSolveur.GetLinearProblem()
        if self.extraRHSVector:
            f+=self.GetExtraRHSTermFromextraRHSVector()

        assert k.count_nonzero() > 0,"Tangent matrix is empty: not allowed"

        self.internalSolveur.ComputeConstraintsEquations()
        Debug(self.internalSolveur.solver.constraints.numberOfEquations)

        self.internalSolveur.Solve(k,f)
        self.internalSolveur.PushSolutionVectorToUnknownFields()

def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
        print(CheckIntegrity())
