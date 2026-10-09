# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

import Muscat.MeshContainers.ElementsDescription as ED
import Muscat.Helpers.ParserHelper as PH

import OpenPisco.TopoZones as TZ
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.AsterSolverBase import AsterSolverBase,BoundaryConditionDefinitionError
import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
import OpenPisco.ExternalTools.Aster.AsterThermalWriter as AsterThermalWriter
import OpenPisco.ExternalTools.Aster.AsterThermoElasticWriter as AsterThermoElasticWriter
from  OpenPisco.PhysicalSolvers.PhysicalSolversTools import CellToPointData
from OpenPisco.PhysicalSolvers.AsterAnalysisSolverFactory import RegisterAsterClass
from OpenPisco.PhysicalSolvers.AsterStatic import GetMaterialFields

def CreateAsterThermoElasticProblem(ops):
    res = AsterThermoElastic()
    del(ops['type'])
    dim = PH.ReadInt(ops.get('dimensionality',"3"))
    if 'dimensionality' in ops:
        del(ops['dimensionality'])
    if 'computeThermalSteadyStateSolution' in ops:
        res.SetThermalSteadyStateComputation(ops['computeThermalSteadyStateSolution'])
    for tag,child in ops["children"]:
        if tag.lower() == "material":
            eTag = child.get('eTag','AllZones')
            material = {"lambda":PH.ReadFloat(child["lambda"]),"rho":PH.ReadFloat(child["rho"]),"cp":PH.ReadFloat(child["cp"]),
                        "young":PH.ReadFloat(child["young"]),"poisson":PH.ReadFloat(child["poisson"]),"thermalExpansion":PH.ReadFloat(child["thermalExpansion"]),"referenceTemperature":PH.ReadFloat(child["referenceTemperature"]) }
            res.materials.append( [eTag,material])
        elif tag.lower() == "prescribedtemperature":
            eTag = child["eTag"]
            prescribed_temperature={"temperature":child["value"]}
            dirichlet = [eTag,prescribed_temperature]
            res.therdirichlet.append(dirichlet)
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
        elif tag.lower() == "dirichlet":
            eTag = child["eTag"]
            dofs = PH.ReadInts(child["dofs"])
            val = PH.ReadFloat(child["value"])
            U=[]
            for i in range(dim):
                if i in dofs:
                   U.append(val)
                else:
                   U.append(None)
            res.dirichlet.append([eTag,U])
        elif tag.lower() == "force":
            res.neumann["1"] = []
            res.problems.append("1")
            eTag = child["eTag"]
            if "value" in child:
                forceValues = PH.ReadFloats(child["value"])
            elif "phi" in child and "theta" in child:
                if "mag" in child:
                    forceValues = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"],child["mag"]])
                else:
                    forceValues = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"]],True )
            else:
                raise BoundaryConditionDefinitionError("a vector (value) or Euler angles (phi and theta)")
            res.neumann["1"].append([eTag,forceValues])
        else:
            raise NotImplementedError("This type of object is not supported " + (tag))
    del ops["children"]
    PH.ReadProperties(ops,ops.keys(),res)
    return res

class AsterThermoElastic(AsterSolverBase):
    """
    .. py:class:: AsterThermoElastic

    Solves a weakly-coupled thermomecanical analysis

    - rho C_p T_t - div (lambda nabla T) = S                        in  (0,t_f) x Omega
    lambda nabla T * n = -h (T-T_{ext})                             on  (0,t_f) x Gamma_C,
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

    -div(sigma(u,Delta T)) = f                                              in Omega
    sigma(u, Delta T ) = sigma_el + sigma_th= A(e(u) - alpha(Delta T) I)
    u = 0                                                                   on Gamma_D
    (sigma(u,Delta T))*n =  g                                               on Gamma_N U Gamma

    thermalExpansion    thermal expansion coefficient
    Delta T = T(t_f,x) - T_{ref}
    T_{ref}  reference temperature
    """

    def __init__(self):
        super(AsterThermoElastic,self).__init__()
        self.name = 'AsterThermoElastic'
        FieldsICanSupply = [FN.potential_energy,FN.von_mises, FN.stress, FN.strain ]
        for componentName in FN.TensorFieldsComponentsNames:
            FieldsICanSupply.append(FN.stress+"_"+componentName)
            FieldsICanSupply.append(FN.strain+"_"+componentName)

        for fieldname in FieldsICanSupply:
            self.auxiliaryFieldGeneration[fieldname][FN.Centroids] = False
            self.auxiliaryFieldGeneration[fieldname][FN.Nodes] = False
            self.auxiliaryFieldGeneration[fieldname][FN.IPField] = False
        self.auxiliaryScalarGeneration = {FN.int_potential_energy:False}

        self.residual = 1e-5
        self.maxAllowedRelaxedResidual = 1e-3
        self.hasPreStress = False
        self.SetThermalSteadyStateComputation()
        self.materials=[]
        self.convection=[]
        self.therdirichlet=[]
        self.source=[]
        self.timeParameters={}

    def SetThermalSteadyStateComputation(self,flag=False):
        self.computeThermalSteadyStateSolution = flag

    def WriteComputationalSupport(self, levelset):
        assert levelset.conform,"GeneralAster: Thermoelastic analysis not available in non conformal setting "
        self.PrepareComputationalSupport(levelset=levelset)
        if self.space == "LagrangeP2":
           from  OpenPisco.PhysicalSolvers.PhysicalSolversTools import LinToQuadMesh
           self.cleanMesh = LinToQuadMesh(self.cleanMesh)
        materialFieldsNames,materialFields = GetMaterialFields(support=self.cleanMesh,
                                                                phi=levelset.phi,
                                                                materials=self.materials,
                                                                levelset=levelset,
                                                                eVoid=self.eVoid)
        self.WriteSupport(self.cleanMesh,cellFieldsNames=materialFieldsNames,cellFields=materialFields)

    def SolveStandard(self):
        return self.SolveWithRelaxedResidual()

    def WriteParametersInput(self,writeFile):
        assert len(self.problems) <= 1,"GeneralAster: Sorry multi loads problem not implemented yet "
        dimensionality = self.originalSupport.GetElementsDimensionality()
        AsterCommonWriter.WriteDirichletParametersInput(writeFile,self.dirichlet,dimensionality)
        AsterCommonWriter.WriteNeumannParametersInput(writeFile,self.neumann,dimensionality,self.problems)
        AsterCommonWriter.WriteNodalForcesParametersInput(writeFile,self.forcenodale,dimensionality,self.problems)
        AsterCommonWriter.WriteBodyForcesParametersInput(writeFile,self.bodyforce,dimensionality,self.problems)
        AsterThermoElasticWriter.WriteMecaModelisation(writeFile,dimensionality)
        AsterThermoElasticWriter.WriteThermalModelisation(writeFile,dimensionality)
        AsterThermoElasticWriter.WriteMaterialParametersInput(writeFile,self.materials)
        AsterThermalWriter.WriteConvectionParametersInput(writeFile,self.convection)
        AsterThermalWriter.WriteThermalDirichletParametersInput(writeFile,self.therdirichlet)

        if not self.computeThermalSteadyStateSolution:
           AsterThermalWriter.WriteTimeParametersInput(writeFile,self.timeParameters)
        AsterThermalWriter.WriteSourceParametersInput(writeFile,self.source)
        AsterCommonWriter.WriteVariable(writeFile=writeFile,variableName="steady_state_solution",variableValue=self.computeThermalSteadyStateSolution)
        AsterCommonWriter.WriteVariable(writeFile=writeFile,variableName="resi_rela",variableValue=self.residual)
        AsterCommonWriter.WriteVariable(writeFile=writeFile,variableName="hasPreStress",variableValue=self.hasPreStress)


    def GetNodalSolution(self,onCleanMesh=False):
        self.u = self.GetSolution(name="DEPL",onCleanMesh=onCleanMesh)[:,0:3]
        return self.u

RegisterAsterClass("static_thermoelastic",AsterThermoElastic,CreateAsterThermoElasticProblem)

from Muscat.MeshTools.MeshCreationTools import CreateCube, CreateSquare
from OpenPisco.ExternalTools.Aster.AsterInterface import SkipAsterTest
from OpenPisco.CLApp.XmlToDic import XmlToDic
from OpenPisco.Unstructured.Levelset import LevelSet

def CheckIntegrity3D(GUI=False):
    mesh = CreateCube(dimensions=[5,3,3],spacing=[1./4, 1./2, 1./2],origin=[0, 0, 0],ofTetras=True)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.ExterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))

    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)

    PPM = AsterThermoElastic()
    PPM.SetAuxiliaryScalarGeneration(FN.int_potential_energy)
    PPM.SetAuxiliaryFieldGeneration(FN.potential_energy,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.stress,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.strain_xy,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.von_mises,on=FN.Centroids)

    PPM.dirichlet= [['X0',(0.,0.,0.)]]
    PPM.problems = ['idx1']
    PPM.neumann= {"idx1": [['X1',(0.,0.,-0.1)]]}

    material={"lambda":50.0,"rho":7850.0,"cp":1046.0,"young":210000.0,"poisson":0.3,"thermalExpansion":11E-6,"referenceTemperature":20.0}
    PPM.materials = [['AllZones',material]]
    convect={"h":50.0,"temp_ext":500.0}
    PPM.convection= [['X0',convect]]
    prescribed_temperature={"temperature":50.0}
    PPM.therdirichlet = [['X1',prescribed_temperature]]
    PPM.timeParameters={"start":0.0,"end":7200.0,"nsteps":300,"starttemperature":20.0}
    sour={"source":1.0}
    PPM.source = [['AllZones',sour]]
    PPM.SolveByLevelSet(ls)

    PPM.GetAuxiliaryField(FN.potential_energy,on=FN.Centroids)
    PPM.GetNodalSolution()
    PPM.GetAuxiliaryField(FN.stress,on=FN.Centroids)
    vmises= PPM.GetAuxiliaryField(FN.von_mises,on=FN.Centroids)
    CellToPointData(ls.support,vmises)
    return "ok"

def CheckIntegrity2D(GUI=False):
    teststringField=u"""
    <GeneralAster type="static_thermoelastic" >
        <Material  lambda="50." rho="7850.0" cp="1046.0" young="210000.0" poisson="0.3" thermalExpansion="11E-6" referenceTemperature="20.0"/>
        <PrescribedTemperature eTag="X1" value="50.0"/>
        <Source eTag="Inside3D" value="1.0"/>
        <Convection eTag="X0" h="50." temp_ext="500.0"/>
        <Dirichlet eTag="X0" dofs="0 1" value="0.0"/>
        <Force eTag="X1" value="0. -0.1" />
        <TimeParameters start="0.0" end="7200.0" nsteps="30" starttemperature="20.0"/>
    </GeneralAster>
    """
    teststringField = PH.ApplyGlobalDictionary(teststringField)
    reader = XmlToDic()
    _, ops = reader.ReadFromString(teststringField)
    ops["dimensionality"]=2
    phyProblem=CreateAsterThermoElasticProblem(ops)
    mesh = CreateSquare(dimensions=[2,3],spacing=[.2,.2],ofTriangles=True)
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet( support=mesh)
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    ls.conform = True
    phyProblem.SolveByLevelSet(ls)
    return "ok"

def CheckIntegrity(GUI=False):

    skipTestPhase,message = SkipAsterTest()
    if skipTestPhase:
        return message
    totest = [CheckIntegrity3D,CheckIntegrity2D]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
