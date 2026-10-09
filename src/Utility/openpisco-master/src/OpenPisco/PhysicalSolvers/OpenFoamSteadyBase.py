# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import os
from pathlib import Path
from collections.abc import Iterable
import shutil

import numpy as np

import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.Helpers.Logger import Warning,Info
from Muscat.MeshContainers.Filters.FilterObjects import NodeFilter

from OpenPisco.BaseElements import elementsTypeSupportedForDimension
from OpenPisco.MuscatExtentions.FoamWriter import FoamWriter,CreateHeader,ENDLINE
from OpenPisco.MuscatExtentions.FoamReader import ReadExtraField
from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
from OpenPisco.PhysicalSolvers.PhysicalSolversTools import CellToPointData
from OpenPisco import RETURN_SUCCESS

from OpenPisco.ExternalTools.OpenFoam.OpenFoamInterface import EXCLUDE_PARAM_NAMES, OpenFOAMConvergenceError
from OpenPisco.ExternalTools.OpenFoam.OpenFoamConfigNames import PRESSURE,VELOCITY

defaultSchemes  = {
            "ddtSchemes" : [("default","steadyState")],
            "gradSchemes" : [("default","Gauss linear")],
            "divSchemes" : [("default","Gauss linear")],
            "laplacianSchemes" : [("default","Gauss linear corrected")],
            "interpolationSchemes" : [("default","linear")],
            "snGradSchemes" : [("default","corrected")]
        }

class OpenFoamSteadyBase(SolverBase):
    def __init__(self):
        super(OpenFoamSteadyBase,self).__init__()
        self.writer = FoamWriter
        self.compressMeshFiles = False
        self.caseFullPath = None
        self.solverApplication = ""
        self.simulationType = "laminar"
        self.foamSchemes = defaultSchemes
        self.relaxationFactors = dict()
        self.boundaryPatchType = dict()
        self.physicalVariables = []
        self.dirichlet = dict()
        self.neumann   = dict()
        self.viscosity = None

    def Solve(self):
        try:
            solverStatus = self.solverInterface.ExecuteOpenFoam()
        except OpenFOAMConvergenceError as e:
            raise Exception('OpenFOAM fail to converge for this analysis, see the plots in '+self.GetCasePath()) from e
        assert solverStatus == RETURN_SUCCESS, " OpenFoam solver has failed."

    def AddScheme(self,schemeType,schemeTarget,schemeName):
        Info("Current schemes: "+str(self.foamSchemes[schemeType]))
        schemeAllTargets = [singleScheme[0] for singleScheme in self.foamSchemes[schemeType]]
        schemeNames = [singleScheme[1] for singleScheme in self.foamSchemes[schemeType]]

        # problem at second iteration : schemes already exist
        # assert schemeTarget not in schemeAllTargets, "Can not add target"+schemeTarget+" to schemeType"+schemeType+", target already exist"
        # suggestion : just ignore if scheme exists rather than exit

        if schemeTarget in schemeAllTargets and schemeNames[schemeAllTargets.index(schemeTarget)]!=schemeName:
            currentSchemeName = schemeNames[schemeAllTargets.index(schemeTarget)]
            Warning("Can not add target "+schemeTarget+" to schemeType "+schemeType\
                    +" with "+schemeName+". Scheme "+currentSchemeName+" already present")
            Warning("Added scheme ("+str(schemeTarget)+"/"+str(schemeName)+") will be ignored")
            return None
        self.foamSchemes[schemeType] = list(set(self.foamSchemes[schemeType]+[(schemeTarget,schemeName)] ))

    def ReplaceScheme(self,schemeType,schemeTarget,schemeName):
        schemeAllTargets = [singleScheme[0] for singleScheme in self.foamSchemes[schemeType]]
        assert schemeTarget in schemeAllTargets, "Can not replace target "+schemeTarget+" in schemeType"+schemeType+", target does not exist"
        indexToReplace = schemeAllTargets.index(schemeTarget)
        self.foamSchemes[schemeType].pop(indexToReplace)
        self.AddScheme(schemeType,schemeTarget,schemeName)

    def GetScheme(self,schemeType,schemeTarget):
        try:
            schemeindex = [singleScheme[0] for singleScheme in self.foamSchemes[schemeType]].index(schemeTarget)
        except ValueError as e:
            raise Exception("schemeTarget "+schemeTarget+" was not found in foam schemes") from e
        return self.foamSchemes[schemeType][schemeindex][-1]

    def GetSolution(self):
        casePath = self.GetCasePath()
        physicalVariables = ["p","U"]
        fieldsValues = ReadExtraField(path=casePath,
                                      fieldNames=physicalVariables,
                                      fieldSupports=[ED.Tetrahedron_4,ED.Tetrahedron_4],
                                      binary=self.binaryFormat)
        results = {fieldName:fieldValue for fieldName,fieldValue in zip(physicalVariables,fieldsValues)}
        return results

    def TransportFieldToNode(self,fieldCellValue,onCleanMesh=True):
        dimensionality =  self.originalSupport.GetElementsDimensionality()
        elemtype = elementsTypeSupportedForDimension[dimensionality]
        supportNumberOfNodes = self.cleanMesh.GetNumberOfNodes()
        if fieldCellValue.shape[1]>1:
            fieldAtNodes = np.zeros((supportNumberOfNodes,fieldCellValue.shape[1]))
            for i in range(fieldCellValue.shape[1]):
                fieldComponentOnCell = {elemtype:fieldCellValue[:,i]} 
                fieldComponentOnPoint = CellToPointData(self.cleanMesh,fieldComponentOnCell,self.originalSupport.GetElementsDimensionality(),weightbyElementSize=False)
                fieldAtNodes[:,i] = fieldComponentOnPoint
        else:
            fieldComponentOnCell = {elemtype:fieldCellValue} 
            fieldAtNodes = CellToPointData(self.cleanMesh,fieldComponentOnCell,self.originalSupport.GetElementsDimensionality(),weightbyElementSize=False) 

        if not onCleanMesh:
            fieldAtNodes = self.ExtendingNodalFieldToOriginalSupport(fieldAtNodes)  
        return fieldAtNodes     

    def SetSolverInterface(self):
        self.solverInterface.SetFilename(self.name)
        uniquePath = self.GetUniqueWorkingDirectory()
        self.caseFullPath = os.path.join(uniquePath,self.name)
        self.solverInterface.SetWorkingDirectory(self.caseFullPath)

    def WriteFoamConfiguration(self):
        self.WriteSimulationType()
        self.WriteTransportProperties()
        self.WriteApplicationConfiguration()
        self.WriteSchemesConfiguration()
        self.WriteSolverConfiguration()

    def WriteSupport(self):
        self.PrepareBoundaryConditions()   
        boundaryPatchType = self.DefineBoundaryPatchType() 
        casePath = self.GetCasePath()

        writer = self.writer(location=os.path.join(casePath,"constant","polyMesh"),
                             compressMeshFiles=self.compressMeshFiles)
        writer.Write(self.cleanMesh,self.tagsToKeep,boundaryPatchType)

    def GetBoundaryConditionsFilters(self):
        inletFilters = [NodeFilter(eTag=boundary["tag"]) for boundary in self.dirichlet["U"] if boundary["type"] == "fixedValue"]
        outletFilters = [NodeFilter(eTag=boundary["tag"]) for boundary in self.dirichlet["p"] if boundary["type"] == "fixedValue"]
        return inletFilters,outletFilters

    def PrepareBoundaryConditions(self):
        physicsFormated = {PRESSURE:{"boundary":{},"internal":self.initials[PRESSURE]},
                           VELOCITY:{"boundary":{},"internal":self.initials[VELOCITY]}}
        allTags = self.AssignRegularBoundaryConditions(physicsFormated)
        self.AddZeroGradientBoundaryConditions(physicsFormated,allTags)

        self.boundaryConditions = physicsFormated
        self.tagsToKeep = list(set(allTags))

    def AssignRegularBoundaryConditions(self,physicsFormated):
        allTags = []
        for variable,conditions in self.dirichlet.items():
            for condition in conditions:
                value  = condition['value'] if "value" in condition else None
                if "tag" in condition:
                    singleTag = condition["tag"]
                    physicsFormated[variable]['boundary'][singleTag] = {'type': condition['type'], 'value': value}
                    allTags.append(singleTag)
                elif "tags" in condition:
                    for singleTag in condition["tags"]:
                        physicsFormated[variable]['boundary'][singleTag] = {'type': condition['type'], 'value': value}
                        allTags.append(singleTag)
                else:
                    raise NotImplementedError("Case not handled yet!")
        return allTags

    def AddZeroGradientBoundaryConditions(self,physicsFormated,allTags):
        for variable,conditions in physicsFormated.items():
            boundaryConditions = conditions["boundary"]
            for tag in allTags:
                if tag not in boundaryConditions.keys():
                    boundaryConditions[tag] = {'type': 'zeroGradient', 'value': None}

    def DefineBoundaryPatchType(self):
        boundaryPatchByType = dict()
        for physicalParam in [PRESSURE,VELOCITY]:
            for boundary in self.dirichlet[physicalParam]:
                if "tag" in boundary:
                    tag = boundary["tag"]
                    boundaryPatchByType[tag] = self.BoundaryPatchByType(boundaryType=boundary["type"])
                elif "tags" in boundary:
                    tags = boundary["tags"]
                    for tag in tags:
                        boundaryPatchByType[tag] = self.BoundaryPatchByType(boundaryType=boundary["type"])
                else:
                    raise ValueError("Either tag or tags should be used for boundary condition")
        return boundaryPatchByType

    def BoundaryPatchByType(self,boundaryType):
        if boundaryType == "symmetry":
            return "symmetry"
        elif boundaryType == "noSlip" or boundaryType == "slip":
            return "wall"
        else:
            return "patch"

    def WriteBoundaryConditions(self):
        variableConditions = self.boundaryConditions[PRESSURE]
        self.WriteVariableBoundaryConditions(variableConditions=variableConditions,
                                                  variableName=PRESSURE,
                                                  fieldType="volScalarField",
                                                  dimensions=[0,2,-2,0,0,0,0])

        variableConditions = self.boundaryConditions[VELOCITY]
        self.WriteVariableBoundaryConditions(variableConditions=variableConditions,
                                                  variableName=VELOCITY,
                                                  fieldType="volVectorField",
                                                  dimensions=[0,1,-1,0,0,0,0])

    def CreateCaseDirectories(self):
        rootPath = self.GetCasePath()
        if os.path.exists(rootPath):
            shutil.rmtree(rootPath)

        foamPath1 = os.path.join(rootPath,"constant","polyMesh")
        foamPath2 = os.path.join(rootPath,"system")
        foamPath3 = os.path.join(rootPath,"0")
        for foamPath in [foamPath1,foamPath2,foamPath3]:
            Path(foamPath).mkdir(parents=True, exist_ok=True)

    def WriteSchemesConfiguration(self):
        filename = "fvSchemes"
        keyWordLocation = "system"  
        casePath = self.GetCasePath()          
        filePath = os.path.join(casePath,keyWordLocation,filename)
        with open(filePath, "w+") as foamfile:
            header = CreateHeader(filePath=keyWordLocation,keyWordClass="dictionary",keyWordObject=filename)
            foamfile.write(header)
            foamfile.write("\n\n")
            for schemeName,schemes in self.foamSchemes.items():
                foamfile.write(schemeName)
                foamfile.write("\n"+"{"+"\n")
                for scheme in schemes:
                    schemeTarget,schemeType = scheme
                    foamfile.write("\t"+schemeTarget+"\t"+schemeType+";\n")
                foamfile.write("}"+"\n\n")            
            foamfile.write(ENDLINE)
        
    def WriteSolverConfiguration(self):
        filename = "fvSolution"
        keyWordLocation = "system"
        casePath = self.GetCasePath()          
        filePath = os.path.join(casePath,keyWordLocation,filename)
        with open(filePath, "w+") as foamfile:
            header = CreateHeader(filePath=keyWordLocation,
                                  keyWordClass="dictionary",
                                  keyWordObject=filename)
            foamfile.write(header)
            foamfile.write("\n\n")
            self.WriteSolvers(foamfile)
            self.WriteAlgorithm(foamfile)
            self.WriteRelaxationFactors(foamfile)

    def WriteSolvers(self,foamfile):
        foamfile.write("solvers")
        foamfile.write("\n"+"{"+"\n")
        for keys in self.solvers.keys():
            foamfile.write("\t"+str(keys)+"\n")
            foamfile.write("\t"+"{"+"\n")
            for key, value in self.solvers[keys].items():
                foamfile.write("\t\t"+str(key)+"\t"+str(value)+";\n")
            foamfile.write("\t"+"}"+"\n\n")
        foamfile.write("}"+"\n\n")

    def WriteAlgorithm(self,foamfile):
        algorithm = list(self.algorithm.keys())[0]
        foamfile.write(algorithm)
        foamfile.write("\n"+"{"+"\n")
        for keys, values in self.algorithm[algorithm].items():
            if isinstance(values, dict):
                foamfile.write("\n\t"+str(keys)+"\n")
                foamfile.write("\t"+"{"+"\n")
                for key, value in self.algorithm[algorithm][keys].items():
                    foamfile.write("\t\t"+str(key)+"\t"+str(value)+";\n")
                foamfile.write("\t"+"}"+"\n\n")
            else:
                foamfile.write("\t"+str(keys)+"\t"+str(values)+";\n")
        foamfile.write("}"+"\n\n")

    def WriteRelaxationFactors(self,foamfile):
        foamfile.write("relaxationFactors")
        foamfile.write("\n"+"{"+"\n")
        relaxFactorsEquations = {"\""+variable+".*\"":relaxationFactor for variable,relaxationFactor in self.relaxFactorsValues.items()}
        relaxFactorsEquations = {"equations":relaxFactorsEquations}
        for keys, _ in relaxFactorsEquations.items():
            foamfile.write("\t"+str(keys)+"\n")
            foamfile.write("\t"+"{"+"\n")
            for key, value in relaxFactorsEquations[keys].items():
                if key[0] == "'":
                    foamfile.write("\t\t"+'"'+str(key[1:-1])+'"'+"\t"+str(value)+";\n")
                else:
                    foamfile.write("\t\t"+str(key)+"\t"+str(value)+";\n")
            foamfile.write("\t"+"}"+"\n\n")
        foamfile.write("}"+"\n\n")
        foamfile.write(ENDLINE)


    def WriteSimulationType(self):
        filename = "turbulenceProperties"
        keyWordLocation = "constant"
        casePath = self.GetCasePath()
        filePath = os.path.join(casePath,keyWordLocation,filename)
        with open(filePath, "w+") as foamfile:
            header = CreateHeader(filePath=keyWordLocation,keyWordClass="dictionary",keyWordObject=filename)
            foamfile.write(header)
            foamfile.write("\n\n")
            foamfile.write("simulationType"+"\t"+self.simulationType+";")
            foamfile.write("\n\n")
            foamfile.write(ENDLINE)
        
    def WriteTransportProperties(self):
        filename = "transportProperties"
        keyWordLocation = "constant"
        casePath = self.GetCasePath()    
        filePath = os.path.join(casePath,keyWordLocation,filename)
        header = CreateHeader(filePath=keyWordLocation,keyWordClass="dictionary",keyWordObject=filename)
        with open(filePath, "w+") as foamfile:
            foamfile.write(header)
            foamfile.write("\n\n")
            foamfile.write("transportModel"+"\tNewtonian;")
            foamfile.write("\n\n")
            foamfile.write("nu                                "+str(self.viscosity)+";")
            foamfile.write("\n\n")
            foamfile.write(ENDLINE)
        
    def WriteApplicationConfiguration(self):
        filename = "controlDict"
        keyWordLocation = "system"
        casePath = self.GetCasePath()    
        filePath = os.path.join(casePath,keyWordLocation,filename)
        with open(filePath, "w+") as foamfile:
            header = CreateHeader(filePath=keyWordLocation,keyWordClass="dictionary",keyWordObject=filename)
            foamfile.write(header)
            generalConfig = self.foamGeneralConfig.ToDict()
            assert len(self.solverApplication), "Solver application must be redefined by derived class"
            generalConfig["application"] = self.solverApplication
            for paramName,paramValue in generalConfig.items():
                if paramName not in EXCLUDE_PARAM_NAMES:
                    foamfile.write("\n\n")
                    foamfile.write(paramName+"\t"+str(paramValue)+";")
            foamfile.write("\n\n")
            foamfile.write("\n\n")
            foamfile.write(ENDLINE)

    def WriteVariableBoundaryConditions(self,variableConditions,variableName,fieldType,dimensions):
        casePath = self.GetCasePath()
        keyWordLocation = "0"
        filePath = os.path.join(casePath,keyWordLocation,variableName)
        with open(filePath, "w+") as foamfile:
            header = CreateHeader(filePath=keyWordLocation,keyWordClass=fieldType,keyWordObject=variableName)
            foamfile.write(header)
            foamfile.write("\n\n")
            foamfile.write("dimensions"+"\t"+"["+" ".join([str(i) for i in dimensions])+"]"+";"+"\n\n")
            if isinstance(variableConditions["internal"], Iterable): 
                foamfile.write("internalField   uniform"+" "+"("+" ".join([str(i) for i in variableConditions["internal"]])+")"+";"+"\n\n")
            else: 
                foamfile.write("internalField   uniform"+" "+str(variableConditions["internal"])+";"+"\n\n")
                
            foamfile.write("boundaryField"+"\n")
            foamfile.write("{"+"\n")
            for key, value in variableConditions["boundary"].items():
                foamfile.write("\t"+key+"\n")
                foamfile.write("\t"+"{"+"\n")
                innerKeys = list(value.keys())
                if value[innerKeys[1]] is None:
                    arg1= str(innerKeys[0])
                    arg2= str(value[innerKeys[0]])
                    foamfile.write("\t"+arg1+"\t"+arg2+";\n")
                else:
                    for innerKey in innerKeys:
                        arg1 = str(innerKey)
                        arg2 = value[innerKey]
                        if arg1 == "value":
                            if isinstance(arg2, Iterable): 
                                foamfile.write("\t"+arg1+"\t"+"uniform"+"\t"+"("+" ".join([str(i) for i in arg2])+")"+";\n")
                            else: 
                                foamfile.write("\t"+arg1+"\t"+"uniform"+"\t"+str(arg2)+";\n")
                                
                        else:
                            foamfile.write("\t"+arg1+"\t"+str(arg2)+";\n")
                foamfile.write("\t"+"}"+"\n")
            foamfile.write("}"+"\n\n")
            foamfile.write(ENDLINE)

    def GetCasePath(self):
        return self.caseFullPath 