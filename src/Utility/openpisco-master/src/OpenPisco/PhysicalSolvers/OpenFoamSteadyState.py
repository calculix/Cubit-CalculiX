# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from typing import Optional
import os
import typing

import numpy as np

from Muscat.Helpers.Logger import Info
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.Types import MuscatFloat
from Muscat.Helpers.Timer import Timer

from OpenPisco import RETURN_SUCCESS
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.MuscatExtentions.FoamReader import ReadExtraField
from OpenPisco.PhysicalSolvers.OpenFoamSteadyBase import OpenFoamSteadyBase

from OpenPisco.ExternalTools.OpenFoam.OpenFoamInterface import OpenFoamInterface,OpenFoamGeneralConfig
from OpenPisco.ExternalTools.OpenFoam.OpenFoamConfigNames import PRESSURE,VELOCITY


class OpenFoamSteadyState(OpenFoamSteadyBase):
    def __init__(self,openFoamConfig: Optional[OpenFoamGeneralConfig]=None):
        super(OpenFoamSteadyState,self).__init__()
        defaultParams = OpenFoamGeneralConfig()
        self.foamGeneralConfig = defaultParams.UpdateFromInstance(other=openFoamConfig)
        self.binaryFormat = self.foamGeneralConfig.writeFormat == "binary"
        self.name = "OpenFoamSteadyStateAnalysis"
        self.toleranceVelocity = 1e-05
        self.tolerancePressure = 1e-06
        self.algorithm = {"SIMPLE":
                            {"nNonOrthogonalCorrectors":5,"consistent":"yes","residualControl":
                                {PRESSURE:1e-5,VELOCITY:1e-6}
                            }
                         }
        self.solverApplication = "simpleFoam"
        self.relaxFactorsValues = {VELOCITY:0.95,PRESSURE:0.95}
        self.initials = {PRESSURE:0.0,VELOCITY:(0.0,0.0,0.0)}
        self.physicalVariables = self.initials.keys()
        
        fieldsICanSupply = [FN.strain,FN.stress,FN.shear_stress]
        for fieldname in fieldsICanSupply:
            self.auxiliaryFieldGeneration[fieldname][FN.Centroids] = False
            self.auxiliaryFieldGeneration[fieldname][FN.Nodes] = False         
            
        self.solverInterface = OpenFoamInterface(self.foamGeneralConfig)
                
    def SolveByLevelSet(self, levelset):
        assert levelset.conform,"Not handled for non conformal case"
        self.SetSolverInterface()
        self.DefineSolverConfig()
        self.PrepareComputationalSupport(levelset=levelset)
        #self.ComputeDomainToTreatLs(levelset)  
        self.CreateCaseDirectories()      
        self.WriteSupport()
        self.WriteBoundaryConditions()
        self.WriteFoamConfiguration()
        with Timer("Solving and postprocessing"):
            self.Solve()
            self.PostSolver()
        Info(Timer.PrintTimes())
        return RETURN_SUCCESS

    def GetNodalSolution(self,onCleanMesh=True):
        solutionAtCell = self.GetSolution()
        solutionAtNodes = {VELOCITY:self.TransportFieldToNode(fieldCellValue=solutionAtCell[VELOCITY],onCleanMesh=onCleanMesh),
                           PRESSURE:self.TransportFieldToNode(fieldCellValue=solutionAtCell[PRESSURE],onCleanMesh=onCleanMesh)}         
        return solutionAtNodes
    
    def DefineSolverConfig(self):
        relativeTolerance = self.foamGeneralConfig.relativeTolerance
        self.solvers = {
                PRESSURE:{"solver":"PCG",
                          "preconditioner":"DIC",
                          "tolerance":self.tolerancePressure,
                          "relTol":relativeTolerance},
                VELOCITY:{"solver":"smoothSolver",
                          "smoother":"symGaussSeidel",
                          "tolerance":self.toleranceVelocity,
                          "relTol":relativeTolerance},
               "pFinal":{"$p":None,"relTol":relativeTolerance}
                       }
        
        self.AddScheme(schemeType="divSchemes",
                       schemeTarget="div(phi,U)",
                       schemeName="bounded Gauss linearUpwind grad(U)")


    def PostSolver(self):
        fieldRequiringVelocityGradient = [FN.strain,FN.stress,FN.shear_stress]
        auxiliaryFieldStatus = [self.auxiliaryFieldGeneration[fieldname][location] 
                                for fieldname in fieldRequiringVelocityGradient 
                                for location in [FN.Nodes,FN.Centroids]]
        if any(auxiliaryFieldStatus):
            self.solverInterface.ExecuteOpenFoamPostProcess("grad(U)")          

    def GetAuxiliaryField(self, name,on=FN.Centroids,onCleanMesh=True):
        GetFieldTypeByOn={
                      FN.Nodes:self.GetNodalField,
                      FN.Centroids:self.GetCellField,
                      }
        if self.auxiliaryFieldGeneration[name][on]:
            return GetFieldTypeByOn[on](name, onCleanMesh=onCleanMesh)
        else:
            Info("Field " +str(name)+"on "+str(on)+ "not available")
            return None        

    def GetNodalField(self,name,onCleanMesh=True):
        field = self.ComputeAuxiliaryField(name)
        if onCleanMesh:
            fieldextended = field
        else:
            fieldNbComponents = field.shape[1],field.shape[2]
            fieldextended = np.zeros( (self.originalSupport.GetNumberOfNodes(),)+fieldNbComponents,dtype=MuscatFloat)
            fieldextended[self.cleanMesh.originalIDNodes] = field
        return fieldextended

    def GetCellField(self,name,onCleanMesh=True):
        field = self.ComputeAuxiliaryField(name)
        if onCleanMesh:
            return field
        else:
            raise NotImplementedError("Can only return field on clean mesh for now")

    def ComputeAuxiliaryField(self,name):
        if name == FN.strain:
            field = self.ComputeStrain()
        elif name == FN.stress:
            field =  self.ComputeStress()
        elif name == FN.shear_stress:
            field = self.ComputeShearStress()
        else:
            raise NotImplementedError("Field not handled yet")
        return field

    def ComputeStress(self):
        shearStressField = self.ComputeShearStress()
        identity = np.broadcast_to(np.identity(3), (shearStressField.shape[0], 3, 3))
        casePath = self.GetCasePath()
        pressure = ReadExtraField(path=casePath,
                                  fieldNames=[PRESSURE],
                                  fieldSupports=[ED.Tetrahedron_4],
                                  binary=self.binaryFormat)[0]
        stressField = shearStressField - np.einsum('ki,kij->kij', pressure, identity)
        return stressField

    def ComputeShearStress(self):
        strainField = self.ComputeStrain()
        shearStressField =  2.0 *self.viscosity*strainField
        return shearStressField

    def ComputeStrain(self):
        gradUVector = self.GetGradientVelocity()
        gradU = gradUVector.reshape(-1,3,3)
        gradU_T = np.transpose(gradU,(0,2,1))
        strainField = (gradU +gradU_T) / 2.0
        return strainField

    def GetGradientVelocity(self):
        casePath = self.GetCasePath()
        return ReadExtraField(path=casePath,
                              fieldNames=["grad(U)"],
                              fieldSupports=[ED.Tetrahedron_4],
                              binary=self.binaryFormat)[0]
    

def CheckIntegrity3DAirFoil(GUI:bool=False):
    from Muscat.MeshTools.MeshCreationTools import CreateCube
    from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometrySphere
    
    from OpenPisco.Unstructured.Levelset import LevelSet
    from OpenPisco.Unstructured.MmgMesher import MmgMesherActionLevelset
    
    length = np.array([8,4,4])
    refinementFactor = 1
    dimensions = refinementFactor*np.array([20, 10, 10])
    spacing = length/(dimensions-1)
    mesh = CreateCube(dimensions=dimensions,spacing=spacing,origin=[0.,0.,0.],ofTetras=True)
    ls = LevelSet(support=mesh)
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.01,"hmax":0.7,"nr":True,"keepGeneratedFiles":True}
    ls.conform = True
    implicitSphere = ImplicitGeometrySphere(center=[2.5, 2., 2.], radius=1)
    implicitSphere.insideOut = True
    ls.phi = implicitSphere.ApplyVector(mesh)
    ls.originalSupport = ls.support
    MmgMesherActionLevelset(ls,opts)
    print(ls.support)

    foamConfig = OpenFoamGeneralConfig(deltaT=0.1, writeInterval =1)
    PPM = OpenFoamSteadyState(openFoamConfig=foamConfig)
    PPM.SetAuxiliaryFieldGeneration(FN.strain,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.stress,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.shear_stress,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.strain,on=FN.Nodes)
    PPM.SetAuxiliaryFieldGeneration(FN.stress,on=FN.Nodes)
    PPM.SetAuxiliaryFieldGeneration(FN.shear_stress,on=FN.Nodes)
    PPM.viscosity = 1e-05

    PPM.dirichlet= {PRESSURE:[{"type":"fixedValue","tag":"X1","value":0.0},
                              {"type":"symmetry","tags":["Y0","Y1","Z0","Z1"]}],
                    VELOCITY:[{"type":"fixedValue","tag":"X0","value":(1.0,0.0,0.0)},
                     {"type":"noSlip","tag":"InterfaceSurf"},
                     {"type":"symmetry","tags":["Y0","Y1","Z0","Z1"]}
                    ]}


    PPM.SolveByLevelSet(ls)
    assert isinstance(PPM, typing.Hashable), "OpenFoamSteadyState Should be hashable!"
    nodalSolution = PPM.GetNodalSolution(onCleanMesh=False)
    print(PPM.GetCasePath())

    from Muscat.IO.XdmfWriter import WriteMeshToXdmf
    resultPath = os.path.join(PPM.GetCasePath(),"AirfoilInitialization.xdmf")
    WriteMeshToXdmf(resultPath,
                    ls.support,
                    PointFields = [ls.phi,nodalSolution[PRESSURE],nodalSolution[VELOCITY]],
                    PointFieldsNames = ["levelset","pressure","velocity"],
                    )

    return "ok"

def CheckIntegrity(GUI=False):
    from OpenPisco.ExternalTools.OpenFoam.OpenFoamInterface import SkipFoamTest
    skipTestPhase,message = SkipFoamTest()
    if skipTestPhase:
        return message

    from Muscat.Helpers.CheckTools import RunListOfCheckIntegrities
    return RunListOfCheckIntegrities([CheckIntegrity3DAirFoil])
        
if __name__ == '__main__':
    print(CheckIntegrity())
