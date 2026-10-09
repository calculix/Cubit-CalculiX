# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import abc
import numpy as np

from Muscat.Types import MuscatFloat
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshContainers.Mesh import Mesh
from Muscat.Helpers.Logger import Info, Warning

from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
from OpenPisco.PhysicalSolvers.PhysicalSolversTools import IpToNodesData
from OpenPisco.ExternalTools.Aster.AsterInterface import AsterInterface, AsterComputedFieldsSupport,AsterPrecisionError
import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
from OpenPisco.MuscatExtentions import MedWriter
from OpenPisco.MuscatExtentions import MedReader
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS

class BoundaryConditionDefinitionError(Exception):
    def __init__(self, message): 
        fullMessage =   "Need "+message+" for this boundary condition" 
        super().__init__(fullMessage)

class AsterSolverBase(SolverBase):
    def __init__(self):
        super(AsterSolverBase,self).__init__()
        self.name = ""
        self.solverInterface = AsterInterface()
        self.solverInterface.ComputeMemoryLimit()
        self.originalSupport: Mesh = None
        self.cleanMesh = None
        self.writer = MedWriter.MedWriter()
        self.materials = list() #[["AllZones",(1,0.3)]]
        self.dirichlet = list()#[['Dirichlet1',(0.,0.,0.)], ['Dirichlet2',(1.,0.,0.)]]
        self.neumann = dict()#{"idx1": [['Neumann1',(0.,0.,0.)],['Neumann2',(0.,0.,0.)]], "idx2": [['Neumann3',(0.,0.,0.)]] }
        self.forcenodale = dict()
        self.problems = list() #["idx1",]
        self.bodyforce = dict()
        self.templateFileName = None
        self.conform = True # [False, True]
        self.space = "LagrangeP1"
        self.eVoid = 1e-3 # Parameter for Ersatz material
        self.auxiliaryScalarsbyOrderlistSize = dict()
        self.tagsToKeep = None

    def SolveByLevelSet(self, levelset):
        self.SetSolverInterface()
        self.WriteComputationalSupport(levelset=levelset)
        solverStatus = self.SolveStandard()
        return solverStatus

    def WriteComputationalSupport(self,levelset):
        self.PrepareComputationalSupport(levelset=levelset)
        self.WriteSupport(self.cleanMesh)

    def SolveWithRelaxedResidual(self):
        #Allow to relax the residual when the solution of the linear system is not accurate enough
        originalRes = self.residual
        precisionError=False

        try:
            exit_status = self.Solve()
        except AsterPrecisionError:
            precisionError = True
            errorComputed = self.solverInterface.GetSolverComputedError()
            if errorComputed <self.maxAllowedRelaxedResidual:
                Info("Fail when solving FEM with residual " +str(self.residual)+"; change it value to: "+str(errorComputed*1.01))
                self.residual = errorComputed*1.01
            else:
                message = "Current residual: "+str(self.residual)+", Max Allowed Relaxed Residual"+str(self.maxAllowedRelaxedResidual)
                raise AsterPrecisionError("Solution of linear elasticity not accurate enough ("+message
                                +"). To continue try to increase the parameter RESI_RELA in the Aster study.")

        if precisionError:
            exit_status = self.Solve()
            self.residual = originalRes
        return exit_status

    def SolveStandard(self):
        return self.Solve()

    def Solve(self):
        self.solverInterface.CleanWorkingDirectory(["param.in",".comm",".export","_objective.csv"])
        filename = self.solverInterface.filename+"param.in"
        writeFile = AsterCommonWriter.OpenAsterParamFile(fileName=self.solverInterface.workingDirectory+filename)
        self.WriteParametersInput(writeFile)
        self.WriteAuxiliaryInputs(writeFile)
        AsterCommonWriter.CloseAsterParamFile(writeFile)
        self.solverInterface.support = self.cleanMesh
        solverStatus = self.solverInterface.ExecuteAster()
        assert solverStatus == RETURN_SUCCESS, " Aster solver has failed."
        return solverStatus

    @abc.abstractmethod
    def WriteParametersInput(self,writeFile):
        pass

    def SetSolverInterface(self):
        if self.templateFileName is not None:
           self.solverInterface.SetFilename(self.templateFileName)
        else:
           self.solverInterface.SetFilename(self.name)
        uniqueWorkingDirectory=self.GetUniqueWorkingDirectory()
        self.solverInterface.SetWorkingDirectory(uniqueWorkingDirectory)

    def SetTagsToKeep(self,taglist):
        self.tagsToKeep = taglist

    def GetSolution(self,name, onCleanMesh=False,index=None):
        solution = self.GetField(name,"nodes",index)
        res = self.ReorderFieldToComputationalSupport(field=solution,onCleanMesh=onCleanMesh)
        return res

    def GetField(self,fieldName,fieldSupport,index=None):
        return self.solverInterface.GetField(fieldName,fieldSupport,index)

    def ReorderFieldToComputationalSupport(self,field,onCleanMesh):
        field = field[0:len(self.cleanMeshToOriginalNodal),:]
        if not self.conform or onCleanMesh:
            res = field
        else:
            fieldextended = np.zeros( (self.originalSupport.GetNumberOfNodes(),field.shape[1] ),dtype=MuscatFloat)
            fieldextended[self.cleanMeshToOriginalNodal] = field
            res = fieldextended
        return res

    def TryToGetNodalField(self,name, onCleanMesh = True,index=None):
        try:
            return self.GetNodalField(name, onCleanMesh, index)
        except MedReader.NodalFieldNotFoundError:
            Info("Warning :Nodalfield "+name+" not found! Searching field with the same name at IP Points and transport it to nodes")
            IPfield = self.GetIPField(name,onCleanMesh=True)
            dim = IPfield.mesh.GetElementsDimensionality()
            fieldAtPoints = IpToNodesData(IPfield,dim)
            if onCleanMesh:
                 return fieldAtPoints
            else:
                 fieldextended = np.zeros( (self.originalSupport.GetNumberOfNodes(), ),dtype=MuscatFloat)
                 fieldextended[self.cleanMeshToOriginalNodal] = fieldAtPoints[0:len(self.cleanMeshToOriginalNodal)]
                 return fieldextended

    def GetNodalField(self,name, onCleanMesh=False,index=None):
        field = self.GetField(name,"nodes",index)
        if not self.conform or onCleanMesh:
           return field
        else:
           fieldextendedShape=(self.originalSupport.GetNumberOfNodes(),)+field.shape[1:]
           fieldextended = np.zeros( fieldextendedShape,dtype=MuscatFloat)
           fieldextended[self.cleanMeshToOriginalNodal] = field
           return fieldextended

    def GetCellField(self,name, onCleanMesh = False,index=None):
        data = self.GetIPField(name=name, onCleanMesh = onCleanMesh).data
        dimensionality = self.originalSupport.GetElementsDimensionality()
        for key,field in data.items():
            if ED.dimensionality[key] !=dimensionality:
               continue
            assert field.shape[1]==1,"The field asked is not constant by element. Call the function GetIPField instead"
        return data

    def GetIPField(self,name,onCleanMesh=False,index=None,dim=None):
        if dim is None:
           dimensionality = self.originalSupport.GetElementsDimensionality()
        else:
           dimensionality = dim

        from Muscat.FE.Fields.IPField import IPField
        if onCleanMesh:
           support = self.cleanMesh
        else:
           support = self.originalSupport

        ipfield = IPField(name,mesh=support,ruleName="".join(["Aster",self.space]))
        ipfield.Allocate()

        for elementName,elems,_ in ElementFilter(dimensionality=dimensionality)(self.cleanMesh):
            nbElements = elems.GetNumberOfElements()
            if nbElements ==0:
               continue
            field = self.GetField(name,elementName)
            nval, ngauss, ncomp = field.shape
            if ncomp ==1:
               field = field.reshape(nval,ngauss)
               fieldextended = np.zeros( (nbElements,ngauss),dtype=MuscatFloat)
            else:
               fieldextended = np.zeros( (nbElements,ngauss,ncomp ),dtype=MuscatFloat)
            cleanMeshelements = self.cleanMesh.GetElementsOfType(elementName)
            if onCleanMesh:
               ipfield.data[elementName]=field
            else:
               fieldextended[cleanMeshelements.originalIds] = field
               ipfield.data[elementName]=fieldextended
        return ipfield

    def GetAuxiliaryField(self, name,on=FN.Centroids,onCleanMesh=False,index=None):
        GetFieldTypeByOn={
                      FN.Centroids:self.GetCellField,
                      FN.Nodes:self.TryToGetNodalField,
                      FN.IPField:self.GetIPField
                      }

        if self.auxiliaryFieldGeneration[name][on]:
            return GetFieldTypeByOn[on](name, onCleanMesh=onCleanMesh,index=index)
        else:
            Info("Field " +str(name)+"on "+str(on)+ "not available")
            return None

    def GetDictOfAuxiliaryFieldsbyLoadCase(self, name,on=FN.Centroids,onCleanMesh=False,index=None):
        GetFieldTypeByOn={
                      FN.Centroids:self.GetCellField,
                      FN.Nodes:self.TryToGetNodalField,
                      FN.IPField:self.GetIPField
                      }
        fields = {}
        if self.auxiliaryFieldGeneration[name][on]:
            for loadcase in self.problems:
               fields[str(loadcase)] = GetFieldTypeByOn[on](name+"_"+str(loadcase), onCleanMesh=onCleanMesh,index=index)
            return fields
        else:
            Warning("FieldsbyLoadCase " +str(name)+"on "+str(on)+ "not available")
            return None

    def GetAuxiliaryFieldForLoadCase(self, name,loadcase,on=FN.Centroids,onCleanMesh=False,index=None):
        GetFieldTypeByOn={
                      FN.Centroids:self.GetCellField,
                      FN.Nodes:self.TryToGetNodalField,
                      FN.IPField:self.GetIPField
                      }

        if self.auxiliaryFieldGeneration[name][on]:
            for load in self.problems:
                if load==loadcase:
                    return  GetFieldTypeByOn[on](name+"_"+loadcase, onCleanMesh=onCleanMesh,index=index)
        else:
            Warning("FieldsbyLoadCase "+str(name)+" on "+str(on)+"for load case "+str(loadcase)+" not available")
            return None

    def CanSupplyAuxiliaryScalarsbyOrder(self, name):
        return name in self.auxiliaryScalarsbyOrderGeneration

    def SetAuxiliaryScalarsbyOrderGeneration(self, name, listSize=1, flag=True):
        assert self.CanSupplyAuxiliaryScalarsbyOrder(name),"This solver cant generate scalar:" +str(name)
        self.auxiliaryScalarsbyOrderGeneration[name] = flag
        self.auxiliaryScalarsbyOrderlistSize[name]=listSize

    def GetAuxiliaryScalarsbyOrderGeneration(self, name):
        return self.auxiliaryScalarsbyOrderGeneration.get(name, False)

    def GetAuxiliaryScalarsbyOrders(self, name):
        if self.auxiliaryScalarsbyOrderGeneration[name]:
            return self.GetScalarsbyOrders(name)
        else:
            Warning("AuxiliaryScalarByOrder "+str(name)+" not available")
            return None

    def GetScalarsbyOrders(self,name):
        listSize=self.auxiliaryScalarsbyOrderlistSize[name]
        return self.solverInterface.GetScalarsbyOrders(name,listSize=listSize)

    #### post scalar ####
    def GetAuxiliaryScalar(self, name):
        if self.auxiliaryScalarGeneration[name]:
            return self.GetScalar(name)
        else:
            Warning("AuxiliaryScalar "+str(name)+" not available")
            return None

    def GetDictOfAuxiliaryScalarByLoadCase(self,name):
        res={}
        if self.auxiliaryScalarGeneration[name]:
           for loadcase in self.problems:
               res[str(loadcase)] = self.solverInterface.GetScalarbyLoadCase(name,loadcase)
           return res
        else:
            Warning("AuxiliaryScalarByLoadCase "+str(name)+" not available")
            return None

    def GetAuxiliaryScalarForLoadCase(self,name,loadcase):
        if self.auxiliaryScalarGeneration[name]:
            for load in self.problems:
                if load ==loadcase:
                    return self.solverInterface.GetScalarbyLoadCase(name,loadcase)
        else:
            Warning("AuxiliaryScalar "+str(name)+" ForLoadCase "+str(loadcase)+" not available")
            return None

    def GetScalar(self,name):
        return self.solverInterface.GetScalar(name)

    def GetNumberOfSolutions(self):
        return len(self.problems)

    def WriteSupport(self, support,cellFieldsNames=None,cellFields=None):
        self.solverInterface.CleanWorkingDirectory(["input.med","output.rmed"])
        self.writer.SetBinary(True)
        self.writer.Open(self.workingDirectory+self.solverInterface.filename+"input.med")
        self.writer.Write(support,
                          CellFieldsNames=cellFieldsNames,
                          CellFields=cellFields)
        self.writer.Close()

    def GetMaterialPropertiesValues(self,name,zone='AllZones'):
        for tagname,material in self.materials:
            if tagname == zone:
               return material[name]

    def SetAuxiliaryFieldGeneration(self, name, on="Centroids", flag=True):
        errorMessage = "This solver"+str(type(self))+" cant generate field:" +str(name)+ " on " + str(on)
        assert self.CanSupplyAuxiliaryField(name,on),errorMessage
        self.auxiliaryFieldGeneration[name][on] = flag

    def WriteAuxiliaryInputs(self,writeFile):
        writeFile.write("scalars=[]\n")
        writeFile.write("scalars=scalars+"+str([scalar for scalar in self.auxiliaryScalarsbyOrderGeneration.keys() if self.auxiliaryScalarsbyOrderGeneration[scalar]])+"\n")
        writeFile.write("scalars=scalars+"+str([scalar for scalar in self.auxiliaryScalarGeneration.keys() if self.auxiliaryScalarGeneration[scalar]])+"\n")

        auxiliaryCombo=[{"field":field,"support":support}
                        for field in self.auxiliaryFieldGeneration.keys()
                        for support in self.auxiliaryFieldGeneration[field].keys()
                        if self.auxiliaryFieldGeneration[field][support]
                        ]

        for auxiliarysingleCombo in auxiliaryCombo:
            assert auxiliarysingleCombo["field"] in AsterComputedFieldsSupport.keys(), "Field "+auxiliarysingleCombo["field"]+" does not exist within those supported by AsterInterface"
            if auxiliarysingleCombo["support"] not in AsterComputedFieldsSupport[auxiliarysingleCombo["field"]]:
                Info("The field "+auxiliarysingleCombo["field"]+" is only computed by Aster at "+', '.join(AsterComputedFieldsSupport[auxiliarysingleCombo["field"]]))
                if auxiliarysingleCombo["support"]==FN.Nodes and FN.IPField==AsterComputedFieldsSupport[auxiliarysingleCombo["field"]]:
                    Info("However, since the field is available at IP, the transport IP->Nodes will be performed by the platform")
                elif auxiliarysingleCombo["support"]==FN.Centroids and FN.IPField==AsterComputedFieldsSupport[auxiliarysingleCombo["field"]] and self.space=="LagrangeP1":
                    Info("However, since the field is available at IP, the transport IP->Centroids will be performed by the platform")
                else:
                    raise TypeError("Nothing we can do about it: abort")

        asterFieldGenerated=list([auxiliary["field"] for auxiliary in auxiliaryCombo])

        for cpt in range(len(asterFieldGenerated)):
            for compname in FN.TensorFieldsComponentsNames:
                if asterFieldGenerated[cpt].endswith(compname):
                   asterFieldGenerated[cpt] = asterFieldGenerated[cpt][0:-3]

        asterFieldGenerated = set(asterFieldGenerated)
        writeFile.write("fields="+str(asterFieldGenerated)+"\n")

def CheckIntegrity():
    from OpenPisco.ExternalTools.Aster.AsterInterface import SkipAsterTest
    skipTestPhase,message = SkipAsterTest()
    if skipTestPhase:
        return message

    return 'ok'

if __name__ == '__main__':
    print(CheckIntegrity())# pragma: no cover
