# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

from Muscat.Types import MuscatFloat
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceP1
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.Fields.FieldTools import TransferFEFieldToIPField
import Muscat.Helpers.ParserHelper as PH

import OpenPisco.TopoZones as TZ
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.AsterSolverBase import AsterSolverBase
import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
import OpenPisco.ExternalTools.Aster.AsterThermalWriter as AsterThermalWriter
from OpenPisco.MuscatExtentions.IPFieldIntegration import IPFieldIntegration
from OpenPisco.PhysicalSolvers.AsterAnalysisSolverFactory import RegisterAsterClass


def CreateAsterThermalPhysicalProblem(ops):
    res = AsterThermal()
    del(ops['type'])
    if 'dimensionality' in ops:
        del(ops['dimensionality'])
    if 'computeSteadyStateSolution' in ops:
        res.SetSteadyStateComputation(ops['computeSteadyStateSolution'])
        del(ops['computeSteadyStateSolution'])
    for tag,child in ops["children"]:
        if tag.lower() == "material":
            eTag = child.get('eTag','AllZones')
            material = {"lambda":PH.ReadFloat(child["lambda"]),"rho":PH.ReadFloat(child["rho"]),"cp":PH.ReadFloat(child["cp"]) }
            res.materials.append( [eTag,material])
        elif tag.lower() == "dirichlet":
            eTag = child["eTag"]
            prescribed_temperature={"temperature":child["value"]}
            dirichlet = [eTag,prescribed_temperature]
            res.dirichlet.append(dirichlet)
        elif tag.lower() == "convection":
            eTag = child["eTag"]
            res.convection = [[eTag,child]]
        elif tag.lower() == "source":
            eTag = child.get('eTag','AllZones')
            res.source = [[eTag,{"source":PH.ReadFloat(child["value"])}]]
        elif tag.lower() == "timeparameters":
            res.timeParameters["start"]=PH.ReadFloat(child["start"])
            res.timeParameters["end"]=PH.ReadFloat(child["end"])
            res.timeParameters["starttemperature"]=PH.ReadFloat(child["starttemperature"])
            res.timeParameters["nsteps"]=PH.ReadInt(child["nsteps"])
        else:
            raise ValueError("This type of object is not supported " + (tag))
    del ops["children"]
    PH.ReadProperties(ops,ops.keys(),res)
    return res

class AsterThermalBase(AsterSolverBase):
    def __init__(self):
        super(AsterThermalBase,self).__init__()
        self.materials=[]
        self.convection=[]
        self.dirichlet=[]
        self.source=[]
        self.timeParameters={}

    def WriteParametersInput(self,writeFile):
        AsterThermalWriter.WriteModelisation(writeFile,self.originalSupport.GetElementsDimensionality())
        AsterThermalWriter.WriteMaterialParametersInput(writeFile,self.materials)
        AsterThermalWriter.WriteConvectionParametersInput(writeFile,self.convection)
        AsterThermalWriter.WriteThermalDirichletParametersInput(writeFile,self.dirichlet)
        AsterThermalWriter.WriteSourceParametersInput(writeFile,self.source)
        self.WriteAnalysisParams(writeFile)

class AsterThermal(AsterThermalBase):
    """
    .. py:class:: AsterThermal

    Solves a thermal analysis

    - rho C_p T_t - div (lambda nabla T) = S                        in  (0,t_f) x Omega
    lambda nabla T * n = -h (T-T_{ext})                             on  (0,t_f) x Gamma_N,
    T = T_D                                                         on  (0,t_f) x Gamma_D.
    T(t=0) = T_{init}                                               in  Omega

    T_t      temporal derivative
    S        thermal source
    T_{ext}  temperature of the fluid in contact with the structure
    T_D      prescribed temperature
    h>0      convection coefficient
    rho>0    material density
    C_p>0    thermal capacity
    lambda   thermal conducivity
    """

    def __init__(self):
        super(AsterThermal,self).__init__()
        self.name = 'AsterThermal'
        self.SetSteadyStateComputation()
        self.solverParams = {"theta":0.57}

        FieldsICanSupply = [FN.temperature,FN.flux_temperature]
        for fieldname in FieldsICanSupply:
            self.auxiliaryFieldGeneration[fieldname][FN.Centroids] = False
            self.auxiliaryFieldGeneration[fieldname][FN.Nodes] = False
            self.auxiliaryFieldGeneration[fieldname][FN.IPField] = False

    def WriteAnalysisParams(self,writeFile):
        AsterCommonWriter.WriteVariable(writeFile=writeFile,variableName="steady_state_solution",variableValue=self.computeSteadyStateSolution)
        if not self.computeSteadyStateSolution:
           AsterThermalWriter.WriteTimeParametersInput(writeFile,self.timeParameters)
           AsterCommonWriter.WriteVariable(writeFile=writeFile,variableName="theta",variableValue=self.solverParams["theta"])
    def SetSteadyStateComputation(self,flag=False):
        self.computeSteadyStateSolution = flag

    def GetNodalSolution(self,i=0,onCleanMesh=False):
        if not self.computeSteadyStateSolution:
            assert i<self.timeParameters["nsteps"],"Solution at step "+str(i)+" not available"
            solution = self.GetSolution(name="temperature", onCleanMesh=onCleanMesh,index=i).ravel()
        else:
           solution = self.GetSolution(name="temperature", onCleanMesh=onCleanMesh).ravel()
        return solution

RegisterAsterClass("thermal", AsterThermal,CreateAsterThermalPhysicalProblem)

class AsterThermalEigenValues(AsterThermalBase):
    def __init__(self):
        super(AsterThermalEigenValues,self).__init__()
        self.name = 'AsterThermalEigenValues'
        self.timeParameters={}
        self.modalParams = {}
        self.auxiliaryScalarsbyOrderGeneration = {FN.EigenFreqSquared:False}

        FieldsICanSupply = [FN.temperature]
        for fieldname in FieldsICanSupply:
            self.auxiliaryFieldGeneration[fieldname][FN.Centroids] = False
            self.auxiliaryFieldGeneration[fieldname][FN.Nodes] = False
            self.auxiliaryFieldGeneration[fieldname][FN.IPField] = False

    def GetNumberOfSolutions(self):
        return self.modalParams["Numbermaxfreq"]

    def WriteAnalysisParams(self,writeFile):
        AsterThermalWriter.WriteTimeParametersInputForEigenValue(writeFile,self.timeParameters)
        AsterCommonWriter.WriteVariable(writeFile=writeFile,variableName="NMAX_FREQ",variableValue=self.modalParams['Numbermaxfreq'])

    def GetNodalSolution(self,t=None,onCleanMesh=False):
        if t is None:
           t = self.timeParameters["end"]
        assert t <= self.timeParameters["end"] and t >= self.timeParameters["start"],"time "+str(t)+" out of range."
        support = self.cleanMesh if onCleanMesh else self.originalSupport
        numbering = ComputeDofNumbering(support,LagrangeSpaceP1,fromConnectivity=True)
        temperature_t = np.zeros( (support.GetNumberOfNodes(), ),dtype=MuscatFloat)
        dimensionality = support.GetElementsDimensionality()

        for mode in range(self.GetNumberOfSolutions()):
            res = self.GetSolution(name="MODES___DEPL",onCleanMesh=onCleanMesh).ravel()
            eigenvalue = self.GetAuxiliaryScalarsbyOrders(FN.EigenFreqSquared)[mode]

            # mode(s) normalization
            field = FEField(name="T",
                      mesh=support,
                      space=LagrangeSpaceP1,
                      numbering = numbering,
                      data = res
                      )

            fieldAtGP=TransferFEFieldToIPField(field,ruleName="AsterTetrahedron4PointsLagrangeP1")

            for name,_ in fieldAtGP.mesh.elements.items():
                fieldAtGP.data[name] = np.ascontiguousarray(fieldAtGP.data[name])

            mode_normalization_factor = IPFieldIntegration(fieldAtGP**2,ElementFilter(dimensionality=dimensionality))
            fieldAtGP=(1./mode_normalization_factor)*fieldAtGP
            res=(1./mode_normalization_factor)*res

            # compute coefficient(s) in modal basis
            beta = IPFieldIntegration(fieldAtGP,ElementFilter(dimensionality=dimensionality))
            beta = beta*self.timeParameters["starttemperature"]

            # estimate solution at time t
            dt = t - self.timeParameters["start"]

            for tagname,material in self.materials:
                assert tagname == 'AllZones',"Not coded."
                weight = material["rho"]*material["cp"]
            temperature_t += np.exp(-eigenvalue*dt/weight)*beta*res
        return temperature_t

from Muscat.MeshTools.MeshCreationTools import CreateCube,CreateSquare
from OpenPisco.ExternalTools.Aster.AsterInterface import SkipAsterTest
from OpenPisco.Unstructured.Levelset import LevelSet
from Muscat.Helpers.CheckTools import SkipTest
from OpenPisco.CLApp.XmlToDic import XmlToDic

def CheckIntegrityThermal3D(GUI=False):
    mesh = CreateCube(dimensions=[4,2,2],spacing=[100./19, 10./4, 10./4],origin=[0, 0, 0],ofTetras=True)

    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.ExterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))

    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)

    PPM = AsterThermal()
    material={"lambda":50.0,"rho":7850.0,"cp":1046.0}
    PPM.materials = [['AllZones',material]]
    convect={"h":50.0,"temp_ext":500.0}
    PPM.convection= [['X1',convect]]
    prescribed_temperature={"temperature":50.0}
    PPM.dirichlet = [['X0',prescribed_temperature]]
    PPM.timeParameters={"start":0.0,"end":7200.0,"nsteps":10,"starttemperature":20.0}
    source={"source":1.0}
    PPM.source = [['AllZones',source]]
    PPM.SetAuxiliaryFieldGeneration(FN.flux_temperature,on=FN.IPField)
    PPM.SolveByLevelSet(ls)
    PPM.GetNodalSolution(i=PPM.timeParameters["nsteps"]-1)
    PPM.SetSteadyStateComputation(True)
    PPM.SolveByLevelSet(ls)
    PPM.GetNodalSolution()
    PPM.GetAuxiliaryField(FN.flux_temperature,on=FN.IPField)
    return "ok"

def CheckIntegrityThermal2D(GUI=False):
    teststringField=u"""
    <GeneralAster type="thermal" >
        <Material  lambda="50." rho="7850.0" cp="1046.0"/>
        <Dirichlet eTag="X0" value="50.0"/>
        <Source eTag="Inside3D" value="1.0"/>
        <Convection eTag="X1" h="50." temp_ext="500.0"/>
        <TimeParameters start="0.0" end="7200.0" nsteps="30" starttemperature="20.0"/>
    </GeneralAster>
    """
    teststringField = PH.ApplyGlobalDictionary(teststringField)
    reader = XmlToDic()
    _, ops = reader.ReadFromString(teststringField)
    ops["dimensionality"]=2
    phyProblem=CreateAsterThermalPhysicalProblem(ops)
    phyProblem.SetAuxiliaryFieldGeneration(FN.flux_temperature,on=FN.IPField)
    mesh = CreateSquare(dimensions=[2,3],spacing=[.2,.2],ofTriangles=True)
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet( support=mesh)
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    phyProblem.SolveByLevelSet(ls)
    phyProblem.GetNodalSolution()
    phyProblem.GetAuxiliaryField(FN.flux_temperature,on=FN.IPField)
    return "ok"

def CheckIntegrityThermalEigenValue(GUI=False):
    nx = 10; ny = 10; nz = 10
    lx,ly,lz = 1.,1.,1.
    mesh = CreateCube(dimensions=[nx, ny, nz],spacing=[lx/(nx-1), ly/(ny-1), lz/(nz-1)],origin=[0, 0, 0],ofTetras=True)

    levelset = LevelSet( support=mesh)
    levelset.conform = True
    levelset.phi = np.empty(levelset.support.GetNumberOfNodes())
    levelset.phi.fill(-1)

    tris = levelset.support.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.ExterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))
    tetra = levelset.support.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))

    TPMEv = AsterThermalEigenValues()
    TPMEv.materials = [['AllZones',{"lambda":1.,"rho":1.,"cp":1.}]]
    prescribed_temperature={"temperature":0.0}
    TPMEv.dirichlet = [[TZ.ExterSurf,prescribed_temperature]]
    TPMEv.modalParams = {"Numbermaxfreq":1}
    TPMEv.timeParameters={"start":0.0,"end":1.0,"nsteps":300,"starttemperature":1000.0}
    TPMEv.SetAuxiliaryScalarsbyOrderGeneration(FN.EigenFreqSquared,listSize=1)
    TPMEv.SolveByLevelSet(levelset)
    eigenvalue = TPMEv.GetAuxiliaryScalarsbyOrders(FN.EigenFreqSquared)[0]
    expected = np.pi**2*(1./lx**2 + 1./ly**2 + 1./lz**2)
    if not np.isclose(eigenvalue, expected, rtol=1e-1, atol=1e-1):
       return "not ok"
    TPMEv.GetNodalSolution()
    return "ok"

def CheckIntegrity(GUI=False):
    skipTestPhase,message = SkipAsterTest()
    if skipTestPhase:
        return message
    totest = [CheckIntegrityThermal2D, CheckIntegrityThermal3D, CheckIntegrityThermalEigenValue]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
