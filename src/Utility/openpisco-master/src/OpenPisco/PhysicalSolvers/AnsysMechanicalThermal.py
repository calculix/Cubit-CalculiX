# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np
from typing import Optional

from Muscat.Helpers.Logger import Info
from Muscat.IO.AnsysWriter  import WriteMeshToCDB
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Mesh import Mesh
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter, NodeFilter
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByElementFilter
from Muscat.MeshTools.MeshModificationTools import CleanLonelyNodes
from Muscat.Types import MuscatFloat
import Muscat.Helpers.ParserHelper as PH

from OpenPisco.ExternalTools.AnsysMechanical.AnsysMechanicalInterface import AnsysMechanicalInterface
import OpenPisco.ExternalTools.AnsysMechanical.AnsysMechanicalThermalWriter as AnsysMechanicalThermalWriter
from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
from  OpenPisco.PhysicalSolvers.PhysicalSolversTools import LinToQuadMesh
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import  RegisterClass
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS

"""
Xml interface
<AnsysMechanical id="X" type="thermal"  p="2" >

    <UseTemplateFile name="" />

    <Material    eTag="*everyelement|eTag" conductivity="*1|float" density="*1|float" />
    <Dirichlet   eTag="eTag" value="float" />
    <Radiation     eTag="eTag" emissivity="float" enclosure=int" />
    
</AnsysMechanical>
"""

def CreateAnsysMechanicalThermalPhysicalProblem(ops):

    assert ops["type"] in ["thermal"],"Only  thermal analysis  are allowed: '{}' ".format(ops["type"])
    del(ops['type'])
    
    
    res = AnsysMechanicalThermal()

    if 'dimensionality' in ops:
        del(ops['dimensionality'])
    if 'computeSteadyStateSolution' in ops:
        res.SetSteadyStateComputation(ops['computeSteadyStateSolution'])
        del(ops['computeSteadyStateSolution'])
    for tag,child in ops["children"]:
        if tag.lower() == "material":
            material={}
            for prop in [ "kxx","cp", "density","emissivity"]:
                material[prop] =  PH.ReadFloat(child[prop])
            res.material = material 
        elif tag.lower() == "dirichlet":
            res.dirichlet.append({"eTag":child["eTag"],"temperature":PH.ReadFloat(child["value"])})
        elif tag.lower() == "radiation":
            res.radiation.append({"eTag":child["eTag"], "emissivity": PH.ReadFloat(child["emissivity"]), "enclosure": PH.ReadInt(child["enclosure"])})
        else:
            raise ValueError("This type of object is not supported " + (tag))
    del ops["children"]
    PH.ReadProperties(ops,ops.keys(),res)
    return res

RegisterClass("AnsysMechanical", None,CreateAnsysMechanicalThermalPhysicalProblem)

class AnsysMechanicalThermal(SolverBase):
    """
    .. py:class:: AsterThermal

    Solves a thermal analysis

    - rho C_p T_t - div (lambda nabla T) = 0                        in  (0,t_f) x Omega
    T = T_D                                                         on  (0,t_f) x Gamma_D
    - lambda nabla T = (I - F) q0                                   on  (0,t_f) x Gamma_R
    T(t=0) = T_{init}                                               in  Omega
    (I-(1-eps)F) q0 = eps sigma T^4

    T_t      temporal derivative
    T_{ext}  temperature of the fluid in contact with the structure
    T_D      prescribed temperature
    h>0      convection coefficient
    rho>0    material density
    C_p>0    thermal capacity
    lambda   thermal conducivity
    q0       radiative flux 
    sigma    Stefan-Bolzmann constant 
    eps      emissivity 
    F        view factor operator
    """
    
    def __init__(self):
        super(AnsysMechanicalThermal,self).__init__()
        self.name = "AnsysMechanicalThermal"
        self.solverInterface = AnsysMechanicalInterface()
        self.originalSupport: Mesh = None
        self.cleanMesh = None
        self.material = {}
        self.dirichlet = list()#[['Dirichlet1',(0.,0.,0.)], ['Dirichlet2',(1.,0.,0.)]]
        self.templateFileName = None
        self.conform = True # [False, True]
        self.space = "LagrangeP2"
        self.radiation = list()
        self.tagsToKeep = None
        
    def CreateNodeTagsfromSurfaceTags(self,support):
        tags = list(set(item["eTag"]  for item in self.dirichlet + self.radiation))
        for tag in tags:
            nf = NodeFilter(eTag=tag)
            ids= nf.GetNodesIndices(mesh=support)
            support.nodesTags.CreateTag(tag+"_node").SetIds(ids)
            
    def SolveByLevelSet(self,levelSet):
        self.PreSolver(levelSet)
        return self.Solve()
    
    def PreSolver(self, levelSet):
        self.SetSolverInterface()
        self.WriteComputationalSupport(levelset=levelSet)
        self.solverInterface.CleanWorkingDirectory([".err",".lock",".dat","rth"])
        self.solverInterface.support = self.cleanMesh
        self.WriteParametersInput(fileName=self.workingDirectory+self.solverInterface.filename+".dat")
        
    def Solve(self):
        solverStatus = self.solverInterface.ExecuteAnsysMechanical()
        assert solverStatus == RETURN_SUCCESS, " AnsysMechanical solver has failed."
        return solverStatus


    def WriteComputationalSupport(self,levelset):
        self.PrepareComputationalSupport(levelset=levelset)
        cleanMeshToOriginalNodal = self.cleanMesh.originalIDNodes
        self.cleanMesh = LinToQuadMesh(self.cleanMesh)
        cleanMeshToOriginalNodal = cleanMeshToOriginalNodal[self.cleanMesh.originalIDNodes]
        self.CreateNodeTagsfromSurfaceTags(self.cleanMesh)
        self.cleanMesh = ExtractElementsByElementFilter(self.cleanMesh,ElementFilter(dimensionality = 3 ))
        self.cleanMeshToOriginalNodal = self.cleanMesh.originalIDNodes[cleanMeshToOriginalNodal]
        self.WriteSupport(self.cleanMesh)

    def WriteParametersInput(self,fileName):
        writeFile = AnsysMechanicalThermalWriter.OpenAnsysDatFile(fileName)
        AnsysMechanicalThermalWriter.WriteMeshInput(writeFile,"AnsysMechanicalThermalinput")
        AnsysMechanicalThermalWriter.WriteElementTypesInput(writeFile)
        AnsysMechanicalThermalWriter.WriteMaterialParametersInput(writeFile,self.material)
        AnsysMechanicalThermalWriter.WriteTemperatureBoundaryConditionsInput(writeFile,self.dirichlet)
        AnsysMechanicalThermalWriter.WriteRadiationSurfacesInput(writeFile,self.radiation)
        AnsysMechanicalThermalWriter.WriteRadiationOptionsInput(writeFile,ambientTemperature=22)
        AnsysMechanicalThermalWriter.WriteSolverOptionsInput(writeFile)
        AnsysMechanicalThermalWriter.WriteSolveInput(writeFile)
        AnsysMechanicalThermalWriter.WritePostProcessingInput(writeFile)
        AnsysMechanicalThermalWriter.CloseAnsysDatFile(writeFile)
        


    def SetSolverInterface(self):
        if self.templateFileName is not None:
            self.solverInterface.SetFilename(self.templateFileName)
        else:
            self.solverInterface.SetFilename(self.name)
        uniqueWorkingDirectory=self.GetUniqueWorkingDirectory()
        self.solverInterface.SetWorkingDirectory(uniqueWorkingDirectory)

    def SetTagsToKeep(self,taglist):
        self.tagsToKeep = taglist


    def GetField(self,fieldName:str,fieldSupport:str,index:Optional[int]=None)->np.ndarray:
        """Retrieve field of a given name at a specific location

        Parameters
        ----------
        fieldName : str
            field name
        fieldSupport : str
            field support
        index : int, optional
            field number id (can be time step id for instance), by default None

        Returns
        -------
        np.ndarray
            array containing the field

        Raises
        ------
        KeyError
            raised if field name provided was not found in the mesh support.
        """
        
        #filter analysis results to match self.cleanMesh
        solversupport = self.solverInterface.GetAnsysMechanicalResults()
        support = ExtractElementsByElementFilter(solversupport,ElementFilter(dimensionality = 3 ))
        CleanLonelyNodes(support)
        support.nodeFields = {'TEMP' : np.zeros(support.GetNumberOfNodes(), dtype=MuscatFloat) }
        support.nodeFields['TEMP'] = solversupport.nodeFields['TEMP'][support.originalIDNodes]

        try:
            if fieldSupport=="nodes":
                return support.nodeFields[fieldName]
        except KeyError as e:
            raise Exception("Field "+fieldName+" was not found within support") from e

    def ReorderFieldToComputationalSupport(self,field,onCleanMesh):
        field = field[0:len(self.cleanMeshToOriginalNodal),:]
        if not self.conform or onCleanMesh:
            res = field
        else:
            fieldextended = np.zeros( (self.originalSupport.GetNumberOfNodes(),field.shape[1] ),dtype=MuscatFloat)
            fieldextended[self.cleanMeshToOriginalNodal] = field
            res = fieldextended
        return res

    def GetNodalField(self,name, onCleanMesh=False,index=None):
        field = self.GetField(name,"nodes",index)
        if not self.conform or onCleanMesh:
           return field
        else:
           fieldextendedShape=(self.originalSupport.GetNumberOfNodes(),)+field.shape[1:]
           fieldextended = np.zeros( fieldextendedShape,dtype=MuscatFloat)
           fieldextended[self.cleanMeshToOriginalNodal] = field
           return fieldextended
 
    def GetAuxiliaryField(self, name,on=FN.Centroids,onCleanMesh=False,index=None):
        GetFieldTypeByOn={
                      FN.Nodes:self.GetNodalField,
                      }

        if self.auxiliaryFieldGeneration[name][on]:
            return GetFieldTypeByOn[on](name, onCleanMesh=onCleanMesh,index=index)
        else:
            Info("Field " +str(name)+"on "+str(on)+ "not available")
            return None

    def GetNumberOfSolutions(self):
        return 1

    def WriteSupport(self, support):#,cellFieldsNames=None,cellFields=None):
        self.solverInterface.CleanWorkingDirectory(["input.cdb","solve.out"])
        WriteMeshToCDB(self.workingDirectory+self.solverInterface.filename+"input.cdb", support)

    def SetAuxiliaryFieldGeneration(self, name, on="Centroids", flag=True):
        errorMessage = "This solver"+str(type(self))+" cant generate field:" +str(name)+ " on " + str(on)
        assert self.CanSupplyAuxiliaryField(name,on),errorMessage
        self.auxiliaryFieldGeneration[name][on] = flag
        
    def GetNodalSolution(self,onCleanMesh=False):
        return self.GetNodalField('TEMP', onCleanMesh=onCleanMesh)

from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco.TestData import  GetTestDataPath as GetTestDataPath
from Muscat.IO.XdmfReader import XdmfReader
import typing

def CheckIntegrityThermalAnalysis3D(GUI=False):
    
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    print("tempdir ",TemporaryDirectory.GetTempPath() ) 

    reader = XdmfReader(filename = GetTestDataPath()+"ThermalRadiationInput.xdmf")
    reader.Read()
    grid = reader.xdmf.GetDomain(-1).GetGrid(-1)
    mesh = grid.GetSupport()
    
    print("mesh",mesh)

    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    
    PPM = AnsysMechanicalThermal()
    PPM.dirichlet = [ {"eTag":"left", "temperature":100} , {"eTag":"right", "temperature":200}, {"eTag":"annulus_faces", "temperature":600} ]
    PPM.radiation = [ {"eTag":"annulus_faces", "emissivity": 0.9, "enclosure": 1} , {"eTag":"inner_faces", "emissivity": 0.9, "enclosure": 1} ]
    PPM.material = {"kxx": 11.4,"cp": 0.545,"density": 8190,"emissivity": 0.85}
    
    PPM.SolveByLevelSet(ls)
    assert isinstance(PPM, typing.Hashable), "AnsysMechanical Should be hashable!"
    sol = PPM.GetNodalSolution()
    from Muscat.IO.XdmfWriter import WriteMeshToXdmf as WriteMeshToXdmf
    WriteMeshToXdmf("AnsysMechanicalThermal.xdmf",ls.support,PointFields=[sol],PointFieldsNames = ["temperature"])
    return "ok"        


def CheckIntegrity():
    from OpenPisco.ExternalTools.AnsysMechanical.AnsysMechanicalInterface import SkipAnsysMechanicalTest
    skipTestPhase,message = SkipAnsysMechanicalTest()
    if skipTestPhase:
        return message
    
    totest = [
        CheckIntegrityThermalAnalysis3D,
        ]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"
    
    

if __name__ == '__main__':
    print(CheckIntegrity())# pragma: no cover
