# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import os 
from typing import Optional,get_type_hints
from dataclasses import dataclass,asdict

import numpy as np

from Muscat.Helpers.Timer import Timer
from Muscat.Helpers.Logger import Info
import Muscat.Helpers.ParserHelper as PH

from Muscat.Types import MuscatFloat

from OpenPisco import RETURN_SUCCESS,RETURN_FAIL

from OpenPisco.MuscatExtentions.FoamWriter import CreateHeader
from OpenPisco.MuscatExtentions.FoamReader import ReadExtraField
from OpenPisco.PhysicalSolvers.OpenFoamSteadyState import OpenFoamSteadyBase,PRESSURE,VELOCITY
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import  RegisterClass
import OpenPisco.ExternalTools.OpenFoam.OpenFoamInterface as foamInterface
from OpenPisco.ExternalTools.OpenFoam.OpenFoamConfigNames import PRESSURE_ADJ,VELOCITY_ADJ,DIV_VELOCITY,DIV_VELOCITY_ADJOINT,FOAM_FIRST_ORDER_DIV_SCHEME,FOAM_SECOND_ORDER_DIV_SCHEME


def CreateFoamSteadyStateComplete(ops):
    """
    Constructor for a OpenFoamSteadyStateComplete

    Parameters
    ----------
    ops
        A dictionary-like object


    Returns
    -------
    OpenFoamSteadyStateComplete
        A populated `OpenFoamSteadyStateComplete` instance
    """
    viscosity = ops['viscosity']
    del(ops['viscosity'])

    dirichlet = dict()
    foamGeneralConfig = foamInterface.OpenFoamGeneralConfig()
    steadyStateCompleteConfig = OpenFoamSteadyStateCompleteConfig()

    for tag,child in ops["children"]:
        if tag =="Dirichlet":
            physicalVariable = child['variable']
            assert physicalVariable in [PRESSURE,VELOCITY], "variable "+physicalVariable+" not handled yet"
            child.pop('variable')
            if "tags" in child:
                child["tags"] = PH.ReadStrings(child["tags"])
            if "value" in child:
                child["value"] = PH.ReadFloats(child["value"]) if len(child["value"].split(" "))>1 else PH.ReadFloat(child["value"])
            try:
                dirichlet[physicalVariable].append(child)
            except KeyError:
                dirichlet[physicalVariable] = [child]
        elif "Config" in tag:
            if tag =="GeneralConfig":
                configType = foamInterface.OpenFoamGeneralConfig
            elif tag=="SteadStateConfig":
                configType = OpenFoamSteadyStateCompleteConfig
            else:
                raise ValueError("Config "+tag+" not handled yet")

            configParamValue = dict()
            paramTypeByField = get_type_hints(configType)
            for paramName,paramValueAsStr in child.items():
                try:
                    paramType = paramTypeByField[paramName]
                except KeyError as err:
                    raise Exception(paramName+" not part of "+configType.__name__) from err
                paramValue = ParseByType(paramValueAsStr,paramType)
                configParamValue[paramName] = paramValue

            if tag =="GeneralConfig":
                foamGeneralConfig = configType(**configParamValue)
            elif tag=="SteadStateConfig":
                steadyStateCompleteConfig = configType(**configParamValue)
        else:
            raise ValueError("Tag "+tag+" not recognized")
    
    problemInstance = OpenFoamSteadyStateComplete(openFoamConfig=foamGeneralConfig,
                                                  steadyStateCompleteConfig=steadyStateCompleteConfig)
 
    problemInstance.viscosity = viscosity
    problemInstance.dirichlet = dirichlet

    return problemInstance

def ParseByType(paramValueAsStr,paramType):
    if paramType==str:
        paramValue = PH.ReadString(paramValueAsStr)
    elif paramType==bool:
        paramValue = PH.ReadBool(paramValueAsStr)
    elif paramType==int:
        paramValue = PH.ReadInt(paramValueAsStr)
    elif paramType==float:
        paramValue = PH.ReadFloat(paramValueAsStr)
    elif paramType==tuple:
        paramValue = PH.ReadInts(paramValueAsStr)
    else:
        raise ValueError("Type not recognized")
    return paramValue

@dataclass
class OpenFoamSteadyStateCompleteConfig:
    divSchemes :str = FOAM_SECOND_ORDER_DIV_SCHEME+" gradUConv"
    divSchemesAdjoint :str = FOAM_SECOND_ORDER_DIV_SCHEME+" gradUaConv"
    #divSchemesAdjoint :str = FOAM_FIRST_ORDER_DIV_SCHEME
    laplacianSchemes :str = "Gauss linear"
    toleranceVelocity : float = 1e-05
    tolerancePressure : float = 1e-06
    residualControlDirect : float = 1e-07
    residualControlAdjoint : float = 1e-07
    maxNumberIterationsPrimalSolver :int = 20
    maxNumberIterationsAdjointSolver :int = 10
    numberOfNonOrthogonalCorrectorsDirect :int = 2
    numberOfNonOrthogonalCorrectorsAdjoint :int = 2
    relaxFactorVelocity : float = 0.95
    relaxFactorPressure : float = 0.95
    relaxFactorVelocityAdj : float = 0.3 
    relaxFactorPressureAdj : float = 0.3 
    relativeToleranceVelocity : float = 0.0
    relativeTolerancePressure : float = 0.0
    smoothSensitivities : bool = False
    minimalRelaxationVelocity : float = 0.1
    minimalRelaxationPressure : float = 0.1
    decreasingRelaxationCoefficient : float = 0.7
    relaxationTrials :int = 6
    disableATCModel : bool = False


    def UpdateFromDict(self, params:dict):
        """Update instance from dict

        :param params: new parameters
        :type params: dict
        :return: instance
        :rtype: OpenFoamSteadyStateCompleteConfig
        """
        originalParams = self.ToDict()
        originalParams.update(params)
        return type(self)(**originalParams)

    def UpdateFromInstance(self,other:Optional["OpenFoamSteadyStateCompleteConfig"]=None):
        """Update instance from another instance

        :param other: another instance
        :type other: OpenFoamSteadyStateCompleteConfig,optional
        :return: instance
        :rtype: OpenFoamSteadyStateCompleteConfig
        """
        if other is None:
            newInstance = self
        else:
            params = other.ToDict()
            newInstance = self.UpdateFromDict(params)
        return newInstance

    def ToDict(self) -> dict:
        """Convert to dict

        :return: instance as a dict
        :rtype: dict
        """
        return asdict(self)

    def __str__(self)->str:
        """Overload str to get a class representation

        :return: string representation of the class
        :rtype: str
        """
        sInfo = ""
        constructorParams = self.ToDict()
        for paramName,paramVal in constructorParams.items():
            sInfo+="\t"+str(paramName)+": "+str(paramVal)+"\n"
        return sInfo

class OpenFoamSteadyStateComplete(OpenFoamSteadyBase):
    def __init__(self,
                 openFoamConfig: Optional[foamInterface.OpenFoamGeneralConfig]=None,
                 steadyStateCompleteConfig: Optional[OpenFoamSteadyStateCompleteConfig]=None):
        super(OpenFoamSteadyStateComplete,self).__init__()
        self.name = "OpenFoamSteadyStateCompleteAnalysis"
        self.problemNamePrefix = "AdjointProblem"
        self.solverApplication = "adjointOptimisationFoam"

        defaultGeneralParams = foamInterface.OpenFoamGeneralConfig()
        openFoamConfig.computeSensitivity = True
        self.foamGeneralConfig = defaultGeneralParams.UpdateFromInstance(other=openFoamConfig)
        self.binaryFormat = self.foamGeneralConfig.writeFormat == "binary"
        self.solverInterface = foamInterface.OpenFoamInterface(openFoamConfig)

        defaultAnalysisParams = OpenFoamSteadyStateCompleteConfig()
        self.analysisParams = defaultAnalysisParams.UpdateFromInstance(other=steadyStateCompleteConfig)
        self.InitializeSolverConfiguration()
        self.physicalVariables = self.initials.keys()
        self.auxiliaryScalarGeneration = {"drag":False,"lift":False,"PtLosses":False}
        self.auxiliaryFieldGeneration["drag_sensitivity"][FN.Nodes] = False
        self.auxiliaryFieldGeneration["lift_sensitivity"][FN.Nodes] = False
        self.auxiliaryFieldGeneration["PtLosses_sensitivity"][FN.Nodes] = False
        
        self.viscosity = None
        self.optimCriteria = set()
        self.minimalRelaxationVelocity = self.analysisParams.minimalRelaxationVelocity
        self.minimalRelaxationPressure = self.analysisParams.minimalRelaxationPressure
        self.decreasingRelaxationCoefficient = self.analysisParams.decreasingRelaxationCoefficient
        self.relaxationTrials = self.analysisParams.relaxationTrials

    def InitializeSolverConfiguration(self):
        self.AddScheme(schemeType="divSchemes",
                       schemeTarget=DIV_VELOCITY,
                       schemeName=self.analysisParams.divSchemes)  
        self.initials = {PRESSURE:0.0,PRESSURE_ADJ:0.0,VELOCITY:(0.0,0.0,0.0),VELOCITY_ADJ:(0.0,0.0,0.0)}
        self.InitializeRelaxationFactors()
        self.InitializeAlgorithm()
        self.physicalVariables = self.initials.keys()

    def InitializeRelaxationFactors(self):
        self.relaxFactorsValues = {VELOCITY:self.analysisParams.relaxFactorVelocity,
                                   PRESSURE:self.analysisParams.relaxFactorPressure,
                                   VELOCITY_ADJ:self.analysisParams.relaxFactorVelocityAdj,
                                   PRESSURE_ADJ:self.analysisParams.relaxFactorPressureAdj}

    def InitializeAlgorithm(self):
        self.algorithm = {"SIMPLE":
                            {"nNonOrthogonalCorrectors":self.analysisParams.numberOfNonOrthogonalCorrectorsDirect,"consistent":"yes","residualControl":
                                {PRESSURE:self.analysisParams.residualControlDirect,VELOCITY:self.analysisParams.residualControlDirect}
                            }
                          }
        self.algorithmAdjoint = {"SIMPLE":
                            {"nNonOrthogonalCorrectors":self.analysisParams.numberOfNonOrthogonalCorrectorsAdjoint,"consistent":"yes","residualControl":
                                {PRESSURE_ADJ:self.analysisParams.residualControlAdjoint,VELOCITY_ADJ:self.analysisParams.residualControlAdjoint}
                            }
                          }

    def SolveByLevelSet(self, levelset):
        assert levelset.conform,"Not handled for non conformal case"
        self.SetSupport(levelset.support)
        self.SetSolverInterface()
        self.DefineSolverConfig()
        self.PrepareComputationalSupportWithoutTags(levelset=levelset)
        #self.ComputeDomainToTreatLs(levelset)  
        self.CreateCaseDirectories()      
        self.WriteSupport()
        
        self.WriteFoamConfiguration()
        self.WriteBoundaryConditions()
        self.WriteAdjointBoundaryConditions()
        self.WriteAdjointRASModel()
        self.WriteOptimiserConfiguration()

        Info("Problem description "+str(self))
        with Timer("Solving and postprocessing"):
            self.Solve()
        return RETURN_SUCCESS
        
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
            self.WriteRelaxationFactors(foamfile)

    def GetNodalSolution(self,onCleanMesh=True):
        solutionAtCell = self.GetSolution()
        solutionAtNodes = {physicalVariable:self.TransportFieldToNode(fieldCellValue=solutionAtCell[physicalVariable],onCleanMesh=onCleanMesh)
                           for physicalVariable in solutionAtCell.keys()}         
        return solutionAtNodes

    def AddCriterion(self,criteria):
        criteriaName = criteria.GetName()
        self.SetAuxiliaryScalarGeneration(criteriaName)
        self.solverInterface.AddQuantityToTrack(criteriaName)
        self.solverInterface.AddQuantityToTrack("Max mag ("+VELOCITY_ADJ+"AdjointProblem_"+criteriaName+")")
        self.solverInterface.AddQuantityToTrack("Max mag ("+PRESSURE_ADJ+"AdjointProblem_"+criteriaName+")")
        self.optimCriteria.add(criteria)

    def DefineSolverConfig(self):
        solvers = {
                PRESSURE:{"solver":"PCG",
                          "preconditioner":"DIC",
                          "tolerance":self.analysisParams.tolerancePressure,
                          "relTol":self.analysisParams.relativeTolerancePressure},
                PRESSURE_ADJ:{"solver":"PCG",
                          "preconditioner":"DIC",
                          "tolerance":self.analysisParams.tolerancePressure,
                          "relTol":self.analysisParams.relativeTolerancePressure},
                VELOCITY:{"solver":"PBiCGStab;",
                          "preconditioner":"DILU",
                          "tolerance":self.analysisParams.toleranceVelocity,
                          "relTol":self.analysisParams.relativeToleranceVelocity},
                VELOCITY_ADJ:{"solver":"PBiCGStab",
                          "preconditioner":"DILU",
                          "tolerance":self.analysisParams.toleranceVelocity,
                          "relTol":self.analysisParams.relativeToleranceVelocity},
                       }
        self.solvers = {"\""+variable+".*\"":solverParameters for variable,solverParameters in solvers.items()}

        self.AddScheme(schemeType="divSchemes",
                       schemeTarget=DIV_VELOCITY_ADJOINT,
                       schemeName=FOAM_SECOND_ORDER_DIV_SCHEME+" gradUaConv")
        self.AddScheme(schemeType="gradSchemes",
                       schemeTarget="gradUConv",
                       schemeName="cellLimited Gauss linear 0.5")
        self.AddScheme(schemeType="gradSchemes",
                       schemeTarget="gradUaConv",
                       schemeName="cellLimited Gauss linear 0.5")

    def WriteAdjointBoundaryConditions(self):
        adjointPressureConditions = dict()
        adjointPressureConditions['internal'] = self.boundaryConditions[PRESSURE]['internal']
        adjointPressureConditions['boundary'] = dict()

        adjointVelocityConditions = dict()
        adjointVelocityConditions['internal'] = self.boundaryConditions[VELOCITY]['internal']
        adjointVelocityConditions['boundary'] = dict()

        directPressureBoundaryConditions = self.boundaryConditions[PRESSURE]['boundary']
        directVelocityBoundaryConditions = self.boundaryConditions[VELOCITY]['boundary']
        assert set(list(directPressureBoundaryConditions.keys()))==set(list(directVelocityBoundaryConditions.keys()))

        for boundaryTag in directPressureBoundaryConditions.keys():
            velocityBoundaryType = directVelocityBoundaryConditions[boundaryTag]["type"]
            pressureBoundaryType = directPressureBoundaryConditions[boundaryTag]["type"]
            if velocityBoundaryType=="fixedValue" and pressureBoundaryType=="zeroGradient":
                adjointVelocityConditions['boundary'][boundaryTag]={"type":"adjointInletVelocity",'value': (0.0, 0.0, 0.0)}
                adjointPressureConditions['boundary'][boundaryTag]={"type":"zeroGradient",'value': None}
            elif velocityBoundaryType=="zeroGradient" and pressureBoundaryType=="fixedValue":
                adjointVelocityConditions['boundary'][boundaryTag]={"type":"adjointOutletVelocity",'value': (0.0, 0.0, 0.0)}
                adjointPressureConditions['boundary'][boundaryTag]={"type":"adjointOutletPressure",'value': 0}
            elif velocityBoundaryType=="noSlip" and pressureBoundaryType=="zeroGradient":
                adjointVelocityConditions['boundary'][boundaryTag]={"type":"adjointWallVelocity",'value': (0.0, 0.0, 0.0)} #bug with distributor case. But necessary for lift case !
                #adjointVelocityConditions['boundary'][boundaryTag]={"type":"noSlip",'value': (0.0, 0.0, 0.0)} #stabilizes distributor case
                adjointPressureConditions['boundary'][boundaryTag]={"type":pressureBoundaryType,'value': None}
            elif velocityBoundaryType=="slip" and pressureBoundaryType=="zeroGradient":
                adjointVelocityConditions['boundary'][boundaryTag]={"type":"slip",'value': None}
                adjointPressureConditions['boundary'][boundaryTag]={"type":pressureBoundaryType,'value': None}
            elif (velocityBoundaryType=="symmetry" and pressureBoundaryType=="symmetry") or (velocityBoundaryType=="zeroGradient" and pressureBoundaryType=="zeroGradient"):
                adjointVelocityConditions['boundary'][boundaryTag]={"type":velocityBoundaryType,'value': None}
                adjointPressureConditions['boundary'][boundaryTag]={"type":pressureBoundaryType,'value': None}
            else:
                raise NotImplementedError("Sorry, combination ("+velocityBoundaryType+","+pressureBoundaryType+") not handled yet")

        self.WriteVariableBoundaryConditions(variableConditions=adjointPressureConditions,
                                                  variableName=PRESSURE_ADJ,
                                                  fieldType="volScalarField",
                                                  dimensions=[0,2,-2,0,0,0,0])
        self.WriteVariableBoundaryConditions(variableConditions=adjointVelocityConditions,
                                                  variableName=VELOCITY_ADJ,
                                                  fieldType="volVectorField",
                                                  dimensions=[0,1,-1,0,0,0,0])

    def WriteOptimiserConfiguration(self):
        filename = "optimisationDict"
        keyWordLocation = "system"
        casePath = self.GetCasePath()          
        filePath = os.path.join(casePath,keyWordLocation,filename)
        with open(filePath, "w+") as foamfile:
            header = CreateHeader(filePath=keyWordLocation,keyWordClass="dictionary",keyWordObject=filename)
            foamfile.write(header)
            foamfile.write("\n\n")
            foamfile.write("optimisationManager    singleRun;\n\n")
            self.WritePrimalSolver(foamfile)
            self.WriteAdjointSolver(foamfile)
            self.WriteOptimisationOutput(foamfile)

    def WritePrimalSolver(self,foamfile):
        algo = list(self.algorithm.keys())[0]
        foamfile.write("primalSolvers\n{\n\t DirectProblem\n\t{\n")
        foamfile.write("\t\t active \t\t true;\n")
        foamfile.write("\t\t type \t\t incompressible;\n")
        foamfile.write("\t\t solver \t\t" + algo.lower()+";\n")
        foamfile.write("\t\t solutionControls\n")
        foamfile.write("\t\t {\n\t\t\t nNonOrthogonalCorrectors "+str(self.algorithm[algo]["nNonOrthogonalCorrectors"])+";\n")
        foamfile.write("\t\t\t consistent "+str(self.algorithm[algo]["consistent"])+";\n")
        foamfile.write("\t\t\t nIters "+str(self.analysisParams.maxNumberIterationsPrimalSolver)+";\n")
        foamfile.write("\t\t\t residualControl\n\t\t\t {\n")
        foamfile.write("\t\t\t\t \""+PRESSURE+".*\"\t\t "+str(self.algorithm[algo]["residualControl"][PRESSURE])+";\n")
        foamfile.write("\t\t\t\t \""+VELOCITY+".*\"\t\t "+str(self.algorithm[algo]["residualControl"][VELOCITY])+";\n")
        foamfile.write("\t\t\t}\n\t\t}\n\t}\n}\n\n")

    def WriteAdjointSolver(self,foamfile):
        foamfile.write("adjointManagers\n{\n\t adjManager1\n\t{\n")
        foamfile.write("\t\t primalSolver \t\t DirectProblem;\n")
        foamfile.write("\t\t adjointSolvers\n")
        foamfile.write("\t\t {\n")
        algo = list(self.algorithm.keys())[0]
        self.problemNameMapping = dict()
        Info("Number of OpenFOAM adjoint problems: "+str(len(self.optimCriteria)))
        for optimCriterion in self.optimCriteria:
            adjointProblemName = self.problemNamePrefix+"_"+optimCriterion.GetName()
            self.problemNameMapping[optimCriterion.GetName()] = adjointProblemName
            optimCriterion.adjointProblemName = adjointProblemName
            foamfile.write("\t\t\t "+adjointProblemName+"\n\t\t\t{\n")
            foamfile.write("\t\t\t\t active\t\ttrue;\n")
            foamfile.write("\t\t\t\t type\t\tincompressible;\n")
            foamfile.write("\t\t\t\t solver\t\tadjointSimple;\n")
            foamfile.write("\t\t\t\t useSolverNameForFields\t\ttrue;\n") #always true, avoids problem when there is only one adjoint to compute
            foamfile.write("\t\t\t\t objectives\n\t\t\t\t{\n")
            foamfile.write("\t\t\t\t\t type\t\tincompressible;\n")
            foamfile.write("\t\t\t\t\t objectiveNames\n\t\t\t\t\t{\n")
            criteriaData = optimCriterion.ToSolverFormat()
            foamfile.write(criteriaData)
            foamfile.write("\n\t\t\t\t\t}\n\t\t\t\t}\n")
            foamfile.write("\t\t\t\t ATCModel\n\t\t\t\t{\n")
            ATCModelStatus = "cancel" if self.analysisParams.disableATCModel else "standard"
            foamfile.write("\t\t\t\t\t ATCModel \t\t"+ATCModelStatus+";\n\t\t\t\t}\n")
            foamfile.write("\t\t\t\t solutionControls\n")
            foamfile.write("\t\t {\n\t\t\t nNonOrthogonalCorrectors "+str(self.algorithmAdjoint[algo]["nNonOrthogonalCorrectors"])+";\n")
            foamfile.write("\t\t\t consistent "+str(self.algorithmAdjoint[algo]["consistent"])+";\n")
            foamfile.write("\t\t\t nIters "+str(self.analysisParams.maxNumberIterationsAdjointSolver)+";\n")
            foamfile.write("\t\t\t printMaxMags true;\n")
            foamfile.write("\t\t\t\t\t residualControl\n\t\t\t\t\t {\n")
            foamfile.write("\t\t\t\t\t\t \""+PRESSURE_ADJ+".*\"\t\t "+str(self.algorithmAdjoint[algo]["residualControl"][PRESSURE_ADJ])+";\n")
            foamfile.write("\t\t\t\t\t\t \""+VELOCITY_ADJ+".*\"\t\t "+str(self.algorithmAdjoint[algo]["residualControl"][VELOCITY_ADJ])+";\n\t\t\t\t\t }\n")
            foamfile.write("\t\t\t\t}\n\t\t\t}")
        foamfile.write("\n\t\t}\n\t}\n}\n\n")

    def WriteOptimisationOutput(self,foamfile):
        foamfile.write("optimisation\n{\n\t designVariables\n\t{\n")
        foamfile.write("\t\t sensitivityType \t\t surfacePoints;\n")
        foamfile.write("\t\t includeMeshMovement \t\t false;\n")
        smoothing = "true" if self.analysisParams.smoothSensitivities else "false"
        foamfile.write("\t\t smoothSensitivities \t\t"+smoothing+";\n")
        '''designVariablePatches = set(tuple(optimCriterion.patches) for optimCriterion in self.optimCriteria)
        if not len(designVariablePatches):
            designVariablePatches = ["InterfaceSurf"]
        else:
            designVariablePatches = [list(term)[0] for term in designVariablePatches]'''
        if "InterfaceBars" in self.tagsToKeep:
            designVariablePatches = ["InterfaceBars"]
        elif "InterfaceSurf" in self.tagsToKeep:
            designVariablePatches = ["InterfaceSurf"]
        else:
            raise ValueError("Pathes should contain either InterfaceSurf (3D) or InterfaceBars(2D)")
        foamfile.write("\t\t patches \t\t(" +" ".join(designVariablePatches)+");\n\t}\n}\n")

    def WriteAdjointRASModel(self):
        filename = "adjointRASProperties"
        keyWordLocation = "constant"
        casePath = self.GetCasePath()    
        filePath = os.path.join(casePath,keyWordLocation,filename)
        header = CreateHeader(filePath=keyWordLocation,keyWordClass="dictionary",keyWordObject=filename)
        with open(filePath, "w+") as foamfile:
            foamfile.write(header)
            foamfile.write("\n\n")
            foamfile.write("adjointRASModel   adjointLaminar;")
            foamfile.write("\n\n")
            foamfile.write("adjointTurbulence off;")
            foamfile.write("\n")
        
    def Solve(self):
        newVelocityRelaxFactor = self.analysisParams.relaxFactorVelocity
        newPressureRelaxFactor = self.analysisParams.relaxFactorPressure
        newVelocityAdjRelaxFactor = self.analysisParams.relaxFactorVelocityAdj
        newPressureAdjRelaxFactor = self.analysisParams.relaxFactorPressureAdj
        solverStatus = RETURN_FAIL
        for trialId in range(self.relaxationTrials):
            try:
                Info("Trial "+str(trialId))
                Info("Velocity relaxation factor: "+str(newVelocityRelaxFactor))
                Info("Pressure relaxation factor: "+str(newPressureRelaxFactor))
                Info("Adjoint velocity relaxation factor: "+str(newVelocityAdjRelaxFactor))
                Info("Adjoint pressure relaxation factor: "+str(newPressureAdjRelaxFactor))
                solverStatus = self.solverInterface.ExecuteOpenFoam()
                break
            except foamInterface.OpenFOAMDirectConvergenceError:
                status = self.DirectSolverRelaxation(trialId)
                if status =="stop":
                    break
            except (foamInterface.OpenFOAMAdjointConvergenceError,foamInterface.OpenFOAMCoreDumpedError):
                status = self.AdjointSolverRelaxation(trialId)
                if status =="stop":
                    break
            except (foamInterface.OpenFOAMCrashError):
                status = self.SetRobustSchemes(trialId)
                if trialId > 0: #try only once setting more robust schemes, then quit
                    break

        self.ReinitializeSolverConfiguration()
        if solverStatus!=RETURN_SUCCESS:
            raise foamInterface.OpenFOAMConvergenceError('OpenFOAM fail to converge for this analysis, see the plots in '+self.GetCasePath())

    def SetRobustSchemes(self,trialId):
        Info("OpenFOAM has crashed, set more robust Schemes")
        self.WriteSolverConfiguration()
        self.ReplaceScheme(schemeType="gradSchemes",
                            schemeTarget="gradUConv",
                            schemeName="faceLimited Gauss linear 1.0")
        self.ReplaceScheme(schemeType="gradSchemes",
                            schemeTarget="gradUaConv",
                            schemeName="faceLimited Gauss linear 1.0")
        self.ReplaceScheme(schemeType="gradSchemes",
                            schemeTarget="default",
                            schemeName="faceLimited Gauss linear 1.0")
        self.ReplaceScheme(schemeType="laplacianSchemes",
                            schemeTarget="default",
                            schemeName="Gauss linear limited 0.333")
        self.ReplaceScheme(schemeType="snGradSchemes",
                            schemeTarget="default",
                            schemeName="limited 0.333")
        self.WriteSchemesConfiguration()
        #Clean analysis corrupted output before next run (else, OpenFOAM might fails by using last solution computed as an initialization)
        self.solverInterface.CleanWorkingDirectory()
        return "ok"
    
    def DirectSolverRelaxation(self,trialId):
        Info("Direct solver had failed for trial, changing divergence scheme to first order for more stability")
        self.ReplaceScheme(schemeType="divSchemes",
                            schemeTarget=DIV_VELOCITY,
                            schemeName=FOAM_FIRST_ORDER_DIV_SCHEME)
        self.WriteSchemesConfiguration()

        Info("Increase number of non-orthogonal corrections for more stability")
        self.algorithm["SIMPLE"]["nNonOrthogonalCorrectors"] = self.analysisParams.numberOfNonOrthogonalCorrectorsDirect+5
        self.WriteOptimiserConfiguration()
        result = self.ComputeNewRelaxationFactors(failSolverType="Direct",
                                                  trialId=trialId,
                                                  velocityRelaxFactor=self.analysisParams.relaxFactorVelocity,
                                                  pressureRelaxFactor=self.analysisParams.relaxFactorPressure)
        if result == "stop":
            return result
        else:
            newVelocityRelaxFactor,newPressureRelaxFactor = result

        self.relaxFactorsValues = {VELOCITY:newVelocityRelaxFactor,
                                    PRESSURE:newPressureRelaxFactor,
                                    VELOCITY_ADJ:self.analysisParams.relaxFactorVelocityAdj,
                                    PRESSURE_ADJ:self.analysisParams.relaxFactorPressureAdj}
        self.WriteSolverConfiguration()
        #Clean analysis corrupted output before next run (else, OpenFOAM might fails by using last solution computed as an initialization)
        self.solverInterface.CleanWorkingDirectory()
        return "ok"
        
    def AdjointSolverRelaxation(self,trialId):
        Info("Adjoint solver(s) had failed for trial, changing divergence scheme to first order for more stability")
        self.ReplaceScheme(schemeType="divSchemes",
                            schemeTarget=DIV_VELOCITY_ADJOINT,
                            schemeName=FOAM_FIRST_ORDER_DIV_SCHEME)
        self.WriteSchemesConfiguration()

        Info("Increase number of non-orthogonal corrections for more stability")
        self.algorithmAdjoint["SIMPLE"]["nNonOrthogonalCorrectors"] = self.analysisParams.numberOfNonOrthogonalCorrectorsAdjoint+5
        self.WriteOptimiserConfiguration()
        Info("Decrease relaxation factors for more stability")
        result = self.ComputeNewRelaxationFactors(failSolverType="Adjoint solver(s)",
                                                  trialId=trialId,
                                                  velocityRelaxFactor=self.analysisParams.relaxFactorVelocityAdj,
                                                  pressureRelaxFactor=self.analysisParams.relaxFactorPressureAdj)
        if result == "stop":
            return result
        else:
            newVelocityAdjRelaxFactor,newPressureAdjRelaxFactor = result

        self.relaxFactorsValues = {VELOCITY:self.analysisParams.relaxFactorVelocity,
                                    PRESSURE:self.analysisParams.relaxFactorPressure,
                                    VELOCITY_ADJ:newVelocityAdjRelaxFactor,
                                    PRESSURE_ADJ:newPressureAdjRelaxFactor}
        self.WriteSolverConfiguration()
        #Clean only adjoint corrupted output before next run (else, OpenFOAM might fails by using last existing solution computed). 
        self.solverInterface.CleanAdjointWorkingDirectory(copyId=trialId,adjointPhysicalVariables=[VELOCITY_ADJ,PRESSURE_ADJ])
        return "ok"

    def ComputeNewRelaxationFactors(self,failSolverType,trialId,velocityRelaxFactor,pressureRelaxFactor):
        Info(failSolverType+" has failed for relaxation factors for velocity/pressure provided. Initial values were: ("\
                +str(self.analysisParams.relaxFactorVelocityAdj)+","+str(self.analysisParams.relaxFactorPressureAdj)+")")
        newVelocityRelaxFactor = velocityRelaxFactor*self.decreasingRelaxationCoefficient**(trialId)
        newPressureRelaxFactor = pressureRelaxFactor*self.decreasingRelaxationCoefficient**(trialId)

        if newVelocityRelaxFactor<=self.minimalRelaxationVelocity and newPressureRelaxFactor <=self.minimalRelaxationPressure:
            Info(failSolverType+" has failed for all trials, velocity and pressure relaxed factors reached value lower than threshold  ("\
                +str(self.minimalRelaxationVelocity)+","+str(self.minimalRelaxationPressure)+")")
            return "stop"
        newVelocityRelaxFactor = max(self.minimalRelaxationVelocity,newVelocityRelaxFactor)
        newPressureRelaxFactor = max(self.minimalRelaxationPressure,newPressureRelaxFactor)
        return newVelocityRelaxFactor,newPressureRelaxFactor

    def ReinitializeSolverConfiguration(self):
        self.InitializeRelaxationFactors()
        self.solverInterface.startingFromLineId = 0
        self.ReplaceScheme(schemeType="divSchemes",
                            schemeTarget=DIV_VELOCITY,
                            schemeName=self.analysisParams.divSchemes)
        self.ReplaceScheme(schemeType="divSchemes",
                            schemeTarget=DIV_VELOCITY_ADJOINT,
                            schemeName=self.analysisParams.divSchemesAdjoint)
        self.InitializeAlgorithm()

    def GetAuxiliaryScalar(self, name):
        if self.auxiliaryScalarGeneration[name]:
            try:
                scalarValue = self.GetScalar(name)
            except FileNotFoundError as err:
                casePath = self.GetCasePath()
                raise Exception("Criteria "+name+" was likely not computed,\
                                 analysis may have failed, see log in "+casePath) from err
            return scalarValue
        else:
            Warning("AuxiliaryScalar "+str(name)+" not available")
            return None

    def GetScalar(self,name):
        casePath = self.GetCasePath()
        fileName = name+self.problemNameMapping[name]
        objectiveDirectory = os.path.join(casePath,"optimisation","objective")
        lastDirectory = str(max([float(directory) for directory in os.listdir(objectiveDirectory) if directory.replace('.','',1).isdigit()]))
        filePath = os.path.join(objectiveDirectory,lastDirectory,fileName)
        if not os.path.exists(filePath):
            lastDirectory = str(int(float(lastDirectory)))
            filePath = os.path.join(objectiveDirectory,lastDirectory,fileName)

        with open(filePath, 'r') as objectiveFile:
            scalarValue = float(list(objectiveFile)[-1].split()[1])
        return scalarValue

    def GetAuxiliaryField(self, name,on=FN.Nodes,onCleanMesh=True):
        assert on == FN.Nodes, "Auxiliary field retrievement only implemented for nodes"
        GetFieldTypeByOn={
                      FN.Nodes:self.GetNodalField,
                      }
        if self.auxiliaryFieldGeneration[name][on]:
            return GetFieldTypeByOn[on](name, onCleanMesh=onCleanMesh)
        else:
            Info("Field " +str(name)+"on "+str(on)+ "not available")
            return None  

    def GetNodalField(self,name,onCleanMesh=True):
        if "sensitivity" in name:
            criteriaName = name.split("_")[0]
            problemName = self.problemNameMapping[criteriaName]
            fieldName = "pointSensNormal"+problemName+"SI" 
        else:
            fieldName = name

        field = self.RetrieveExtraSolutionField(fieldName)

        if onCleanMesh:
            fieldextended = field
        else:
            fieldNbComponents = (field.shape[1],field.shape[2]) if len(field.shape)==3 else field.shape[1],
            fieldextended = np.zeros( (self.originalSupport.GetNumberOfNodes(),)+fieldNbComponents,dtype=MuscatFloat)
            fieldextended[self.cleanMesh.originalIDNodes] = field
        return fieldextended

    def RetrieveExtraSolutionField(self,fieldName):
        casePath = self.GetCasePath()
        field = ReadExtraField(path=casePath,
                                fieldNames=[fieldName],
                                fieldSupports=[FN.Nodes],
                                binary=self.binaryFormat)[0]
        return field

    def GetProblemName(self,fieldName):
        for criterionName in ["drag","lift","PtLosses"]:
            if criterionName in fieldName:
                return self.problemNameMapping[criterionName]
            
        raise ValueError(fieldName+ "not found in results")

    def __str__(self):
        problemInfo = "Instance of "+type(self).__name__+"\n"
        problemInfo +="Schemes\n"
        for schemeName,schemes in self.foamSchemes.items():
            problemInfo +="\t Scheme "+schemeName[:schemeName.index("Schemes")]+"\n"
            for scheme in schemes:
                problemInfo += "\t\t"+str(scheme)+"\n"
        problemInfo +="Physical setting\n"
        problemInfo +="Simulation type: "+self.simulationType+"\n"
        problemInfo +="Viscosity: "+str(self.viscosity)+"\n"
        problemInfo +="Dirichlet boundary conditions: \n"
        for dirichletVariableName,dirichletVariableConditions in self.dirichlet.items():
            problemInfo +="    Variable name: "+dirichletVariableName+"\n"
            for dirichletVariableCondition in dirichletVariableConditions:
                problemInfo +="\t Condition type: "+dirichletVariableCondition["type"]
                if "tag" in dirichletVariableCondition:
                    problemInfo +=" Tag: "+dirichletVariableCondition["tag"]
                else:
                    problemInfo +=" Tags: "+" ".join(dirichletVariableCondition["tags"])
                if "value" in dirichletVariableCondition:
                    problemInfo +=" value: "+str(dirichletVariableCondition["value"])
                problemInfo +="\n"
        problemInfo +="\n\n"

        problemInfo +="Analysis parameters\n"
        problemInfo+=str(self.analysisParams)+"\n\n"
        problemInfo +="General parameters\n"
        problemInfo+=str(self.foamGeneralConfig)
        return problemInfo

RegisterClass("FoamSteadyStateComplete", None,CreateFoamSteadyStateComplete)


def CheckIntegrityAirFoil(GUI:bool=False):
    from Muscat.MeshTools.MeshCreationTools import CreateCube
    from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometrySphere
    from Muscat.MeshContainers.Filters.FilterTools import NodeFilter
    import Muscat.MeshContainers.ElementsDescription as ED
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory

    from OpenPisco.ExtractConnexPart import GetNodeToElementContainerConnectivity
    from OpenPisco.Unstructured.Levelset import LevelSet
    from OpenPisco.Unstructured.MmgMesher import MmgMesherActionLevelset
    from OpenPisco.Optim.Criteria.CriteriaFactory import Create
    import OpenPisco.Optim.Criteria.PhyFluidCriteria

    tempdir = TemporaryDirectory.GetTempPath()
    print(tempdir)
    length = np.array([8,4,4])
    refinementFactor = 1
    dimensions = refinementFactor*np.array([20, 10, 10])
    spacing = length/(dimensions-1)
    mesh = CreateCube(dimensions=dimensions,spacing=spacing,origin=[0.,0.,0.],ofTetras=True)
    ls = LevelSet(support=mesh)
    print(mesh)
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.5,"hmax":1,"nr":True,"keepGeneratedFiles":True}
    ls.conform = True
    implicitSphere = ImplicitGeometrySphere(center=[2.5, 2., 2.], radius=1)
    implicitSphere.insideOut = True
    ls.phi = implicitSphere.ApplyVector(mesh)
    ls.originalSupport = ls.support
    MmgMesherActionLevelset(ls,opts)
    print("Computing steadystate 3D conform ...")
    print(ls.support)

    #Configuration based on description provided here
    #https://www.cfd-online.com/Forums/openfoam-solving/182268-airfoil-3d-simulation-using-openfoam.html

    foamConfig = foamInterface.OpenFoamGeneralConfig(enableParallelism=True,numberOfSubDomains=6,enableCheckError=True)
    divSchemesFirstOrder=FOAM_FIRST_ORDER_SCHEME

    foamSteadyStateConfig = OpenFoamSteadyStateCompleteConfig(divSchemes=divSchemesFirstOrder,
                                                              maxNumberIterationsPrimalSolver=500,
                                                              maxNumberIterationsAdjointSolver=10,
                                                              residualControlDirect=1e-5,
                                                              relaxationTrials=1)
    PPM = OpenFoamSteadyStateComplete(openFoamConfig=foamConfig,steadyStateCompleteConfig=foamSteadyStateConfig)
    divPhiU = DIV_VELOCITY
    divergenceScheme = "divSchemes"
    assert PPM.GetScheme(schemeType=divergenceScheme,schemeTarget=divPhiU)==divSchemesFirstOrder, "div scheme not set correctly"

    divSchemes=FOAM_SECOND_ORDER_DIV_SCHEME+" gradUConv"
    PPM.ReplaceScheme(schemeType=divergenceScheme,schemeTarget=divPhiU,schemeName=divSchemes)
    assert PPM.GetScheme(schemeType=divergenceScheme,schemeTarget=divPhiU)==divSchemes, "div scheme not set correctly"
    PPM.ReplaceScheme(schemeType=divergenceScheme,schemeTarget=divPhiU,schemeName=divSchemesFirstOrder)


    PPM.dirichlet= {PRESSURE:[{"type":"fixedValue","tag":"X1","value":0.0},
                              {"type":"symmetry","tags":["Y0","Y1","Z0","Z1"]}],
                    VELOCITY:[{"type":"fixedValue","tag":"X0","value":(1.0,0.0,0.0)},
                     {"type":"noSlip","tag":"InterfaceSurf"},
                     {"type":"symmetry","tags":["Y0","Y1","Z0","Z1"]}
                    ]}
    PPM.viscosity = 0.1


    dragParameters = {      "name":"drag",
                            "patches":["InterfaceSurf"],
    }

    dragCriteria = Create(name="drag",ops=dragParameters)
    dragCriteria.problem = PPM
    dragCriteria.SetAuxiliaryQuantities(ls)

    liftParameters = {      "name":"lift",
                            "patches":["InterfaceSurf"],
    }

    liftCriteria = Create(name="lift",ops=liftParameters)
    liftCriteria.problem = PPM
    liftCriteria.SetAuxiliaryQuantities(ls)

    with Timer("Solving and postprocessing"):
        PPM.SolveByLevelSet(ls)

    print(Timer.PrintTimes())
    dragCriteria.UpdateValues(ls)
    # print(dragCriteria.GetValue())

    liftCriteria.UpdateValues(ls)
    # print(liftCriteria.GetValue())

    solution = PPM.GetSolution()

    # print(PPM)

    def GetTetraIdsForTag(eTag):
        nodalFilter = NodeFilter(eTag=eTag)
        nodeIds = nodalFilter.GetNodesIndices(mesh)
        bodyElements = mesh.GetElementsOfType(ED.Tetrahedron_4)
        nodetoelementconn=GetNodeToElementContainerConnectivity(mesh=mesh,elements=bodyElements,maxNumConnections=200)
        tetraIdsForBoundaryNodes = np.unique(nodetoelementconn[nodeIds].flatten())[1:]
        return tetraIdsForBoundaryNodes

    tetraIdForBoundaryNodes = GetTetraIdsForTag(eTag="X1")
    pressureSolution = solution[PRESSURE]
    # expectedPressureValue = PPM.dirichlet[PRESSURE][0]["value"]
    # print(np.mean(pressureSolution[tetraIdForBoundaryNodes]))
    # assert abs(np.mean(pressureSolution[tetraIdForBoundaryNodes])-expectedPressureValue)<1e-8

    tetraIdForBoundaryNodes = GetTetraIdsForTag(eTag="X0")
    # print(np.mean(solution[VELOCITY],axis=0))
    # expectedVelocityMagnitudeValue = np.linalg.norm(PPM.dirichlet[VELOCITY][0]["value"])
    # print(np.mean(velocityMagnitudeSolution[tetraIdForBoundaryNodes],axis=1))
    # assert abs(np.mean(velocityMagnitudeSolution[tetraIdForBoundaryNodes])-expectedVelocityMagnitudeValue)<1e-6


    from Muscat.IO.XdmfWriter import WriteMeshToXdmf
    resultPath = os.path.join(PPM.GetCasePath(),"AirfoilInitialization.xdmf")
    WriteMeshToXdmf(resultPath,
                    ls.support,
                    PointFields = [dragCriteria.GetSensitivity()],
                    PointFieldsNames = ["Sensitivity"],
                    )
    return "ok"

def DebugAirfoil():
    from Muscat.MeshTools.MeshCreationTools import CreateCube
    from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometrySphere
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory

    from OpenPisco.Unstructured.Levelset import LevelSet
    from OpenPisco.Unstructured.MmgMesher import MmgMesherActionLevelset
    from OpenPisco.Optim.Criteria.CriteriaFactory import Create
    import OpenPisco.Optim.Criteria.PhyFluidCriteria

    length = np.array([8,4,4])
    refinementFactor = 1
    dimensions = refinementFactor*np.array([20, 10, 10])
    spacing = length/(dimensions-1)
    mesh = CreateCube(dimensions=dimensions,spacing=spacing,origin=[0.,0.,0.],ofTetras=True)
    ls = LevelSet(support=mesh)
    print(mesh)
    opts = {"iso":0.0,"hmax":0.8,"hmin":0.04,"hausd":0.05,"nr":True}
    ls.conform = True
    implicitSphere = ImplicitGeometrySphere(center=[2.5, 2., 2.], radius=1)
    implicitSphere.insideOut = True
    ls.phi = implicitSphere.ApplyVector(mesh)
    ls.originalSupport = ls.support
    MmgMesherActionLevelset(ls,opts)
    print(ls.support)

    tempdir = TemporaryDirectory.GetTempPath()
    print(tempdir)

    foamConfig = foamInterface.OpenFoamGeneralConfig(enableParallelism=True,numberOfSubDomains=6,enableCheckError=True,optimizeRenumbering=True)
    foamSteadyStateConfig = OpenFoamSteadyStateCompleteConfig(maxNumberIterationsPrimalSolver=3000,
                                                              maxNumberIterationsAdjointSolver=3000,
                                                              relativeToleranceVelocity=0.1,
                                                              relativeTolerancePressure=0.01,
                                                              relaxFactorVelocityAdj=0.3,
                                                              relaxFactorPressureAdj=0.3,
                                                              relaxFactorPressure=0.7,
                                                              relaxFactorVelocity=0.7,
                                                              residualControlDirect=1e-07,
                                                              residualControlAdjoint=1e-07,
                                                              toleranceVelocity=1e-9,
                                                              tolerancePressure=1e-9,
                                                              numberOfNonOrthogonalCorrectorsDirect=3,
                                                              relaxationTrials=3)
    
    PPM = OpenFoamSteadyStateComplete(openFoamConfig=foamConfig,steadyStateCompleteConfig=foamSteadyStateConfig)
    PPM.dirichlet= {PRESSURE:[{"type":"fixedValue","tag":"X1","value":0.0},
                              {"type":"symmetry","tags":["Y0","Y1","Z0","Z1"]}],
                    VELOCITY:[{"type":"fixedValue","tag":"X0","value":(1.0,0.0,0.0)},
                     {"type":"noSlip","tag":"InterfaceSurf"},
                     {"type":"symmetry","tags":["Y0","Y1","Z0","Z1"]}
                    ]}
    PPM.viscosity = 0.1

    dragParameters = {"name":"drag","patches":["InterfaceSurf"]}

    dragCriteria = Create(name="drag",ops=dragParameters)
    dragCriteria.problem = PPM
    dragCriteria.SetAuxiliaryQuantities(ls)

    liftParameters = {"name":"lift","direction":[0.0,-1.0,0.0],"patches":["InterfaceSurf"]}

    liftCriteria = Create(name="lift",ops=liftParameters)
    liftCriteria.problem = PPM
    liftCriteria.SetAuxiliaryQuantities(ls)

    PPM.SolveByLevelSet(ls)
    dragCriteria.UpdateValues(ls)
    liftCriteria.UpdateValues(ls)


def CheckIntegrity(GUI=False):
    from OpenPisco.ExternalTools.OpenFoam.OpenFoamInterface import SkipFoamTest
    skipTestPhase,message = SkipFoamTest()

    if skipTestPhase:
        return message

    from Muscat.Helpers.CheckTools import RunListOfCheckIntegrities
    return RunListOfCheckIntegrities([CheckIntegrityAirFoil])

if __name__ == '__main__':
    # print(CheckIntegrity())
    DebugAirfoil()
