# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from dataclasses import dataclass,asdict
from typing import Optional,Iterable
import subprocess
import os 
from copy import deepcopy
from collections import OrderedDict
import shutil

from Muscat.Helpers.IO.Which import Which
from Muscat.Helpers.Logger import ForcePrint, Error, Info,Warning
from Muscat.Helpers.CPU import GetNumberOfAvailableCores

from OpenPisco.ExternalTools.AppExecutableTools import AppExecutableBase, CommandLineBuilder
from OpenPisco import RETURN_SUCCESS,RETURN_FAIL
from OpenPisco.MuscatExtentions.FoamWriter import CreateHeader
from OpenPisco.MuscatExtentions.FoamReader import GetLastStepDirectoryInPath

class OpenFOAMConvergenceError(Exception):
    pass

class OpenFOAMDirectConvergenceError(OpenFOAMConvergenceError):
    pass

class OpenFOAMAdjointConvergenceError(OpenFOAMConvergenceError):
    pass

class OpenFOAMNoResidualInfoError(Exception):
    pass

class OpenFOAMCrashError(Exception):
    pass

class OpenFOAMIllPosedError(Exception):
    pass

class OpenFOAMCoreDumpedError(Exception):
    pass

@dataclass
class OpenFoamGeneralConfig:
    startFrom :str = "latestTime"
    startTime :float = 0
    stopAt :str = "endTime"
    endTime : float = 1.0
    deltaT : float = 1.0
    writeControl :str = "timeStep"
    writeInterval : float = 500
    purgeWrite : float = 1
    writeFormat :str = "binary"
    writePrecision :int = 6
    writeCompression :str = "off"
    timeFormat :str = "general"
    timePrecision :int = 6
    runTimeModifiable :str = "true"  
    relativeTolerance :float = 0.0   
    continuityError :float = 1e-4
    variableError :float = 1e-5
    enableParallelism : bool = False
    numberOfSubDomains :int = 1
    optimizeRenumbering : bool = True
    enableCheckError : bool = True
    enableCheckMesh : bool = True
    computeSensitivity : bool = False
    plotFoamQuantities : bool = False

    def UpdateFromDict(self, params:dict):
        """Update instance from dict

        :param params: new parameters
        :type params: dict
        :return: instance
        :rtype: OpenFoamGeneralConfig
        """
        originalParams = self.ToDict()
        originalParams.update(params)
        return type(self)(**originalParams)

    def UpdateFromInstance(self,other:Optional["OpenFoamGeneralConfig"]=None):
        """Update instance from another instance

        :param other: another instance
        :type other: OpenFoamGeneralConfig,optional
        :return: instance
        :rtype: OpenFoamGeneralConfig
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

EXCLUDE_PARAM_NAMES = ['relativeTolerance', 'continuityError', 'variableError', 
                      'optimizeRenumbering', 'enableCheckError', 'enableCheckMesh','numberOfSubDomains', 
                      'enableParallelism', 'computeSensitivity']

parallelJobsDefaultExec = "mpirun"

def ParseInfoFromLogFile(logFile:str,extraQuantities:Iterable,startingFromLineId:int=0):
    with open(logFile,'r') as fileContent:
        fileContent = fileContent.readlines()

    residualByVariable = {}
    lineId = 0
    Info("Start reading from line "+str(startingFromLineId))
    for lineId,line in enumerate(fileContent[startingFromLineId:]):
        lineIdCurrent = lineId+startingFromLineId
        if "Solving for" in line:
            variable = line.split(",")[0].split(" ")[-1]+"_error"
            value = float(line.split(",")[-2].split("=")[-1])
        elif line.startswith("time step continuity errors"):
            previousLine = fileContent[lineIdCurrent-1]
            if "AdjointProblem" not in previousLine:
                suffix = "Direct_"
            else:
                suffix = "AdjointProblem_"+previousLine.split("AdjointProblem_")[1].split(",")[0]+"_"
            variable = suffix+"global_continuity_error"
            value = float(line.split(",")[-2].split("=")[-1])
        elif any(line.startswith(quantity) for quantity in extraQuantities):
            variable = line.split("=")[0] if "=" in line else line.split()[0]
            variable = variable.strip()
            value = float(line.split()[-1])
        elif "core dumped" in line:
            ForcePrint(fileContent[lineIdCurrent-30:lineIdCurrent])
            raise OpenFOAMCoreDumpedError("Core dumped detected, context above")
        elif "reached max. number of iterations" in line:
            Warning(line.split()[0]+" Reached maximum number of iteration prescribed (problem may not have fully converged)")
            continue
        elif "renumbering" in line:
            #See information about renumbering effect
            for i in range(3):
                Info(fileContent[lineIdCurrent+i])
            continue
        elif "Failed" in line and "mesh checks" in line:
            Warning("OpenFOAM mesh verification failure (may not be critical): "+line)
            continue
        elif "Signal: Floating point exception" in line:
            Warning("OpenFOAM Crashed : Floating point exception")
            OpenFOAMHasCrashed = True
            break
        else:
            OpenFOAMHasCrashed = False
            continue

        try:
            residualByVariable[variable].append(value)
        except KeyError:
            residualByVariable[variable]=[value]
    return residualByVariable,lineId,OpenFOAMHasCrashed

def CopyCleanConfig(originalPath,copyPath):
    os.makedirs(copyPath)
    def ignoreFiles(dir, files):
        topLevelFilter = os.path.abspath(dir) == os.path.abspath(originalPath)
        if topLevelFilter:
            return [file for file in files if file not in ["constant", "system", "0"]]
        return []
    shutil.copytree(originalPath,copyPath, ignore=ignoreFiles,dirs_exist_ok=True)

def ComputeDirectorySize(path):
    total_size = 0
    for dirpath, _, filenames in os.walk(path):
        for f in filenames:
            fp = os.path.join(dirpath, f)
            suitableFile = not os.path.islink(fp) and os.path.exists(fp)
            if suitableFile:
                total_size += os.path.getsize(fp)
    return total_size

class OpenFoamInterface(AppExecutableBase):
    """
    .. py:class:: OpenFoamInterface

    Interface for the finite volume solver OpenFoam
    """

    def __init__(self,foamGeneralConfig: Optional[OpenFoamGeneralConfig]=None):
        super(OpenFoamInterface, self).__init__("None")
        defaultGeneralParams = OpenFoamGeneralConfig()
        self.foamGeneralConfig = defaultGeneralParams.UpdateFromInstance(other=foamGeneralConfig)
        self.residualByVariable = dict()
        self.extraQuantitiesToTrack = []
        self.startingFromLineId = 0
        self.logFile = "OpenFoamStudy.log"

    def GetAppName(self):
        workingDir = self.GetWorkingDirectory()
        applicationFile = os.path.join(workingDir,"system","controlDict")
        with open(applicationFile, "r") as file:
            fileContent = file.readlines()
        appNameLine = [line for line in fileContent if "application" in line]
        assert len(appNameLine), "OpenFOAM Application for problem not specified"
        appName = appNameLine[0].split()[-1].replace(";","")
        return appName

    def ExecuteOpenFoam(self):
        if self.foamGeneralConfig.optimizeRenumbering:
            self.ExecuteSequentialFoamUtility("renumberMesh")

        if self.foamGeneralConfig.enableCheckMesh:
            self.ExecuteSequentialFoamUtility("checkMesh")

        meshDetails = self.ParseMeshAnalysis()
        Info(meshDetails)

        if self.foamGeneralConfig.enableParallelism:
            self.RunInParallel()
        else:
            self.RunInSequential()

        if self.foamGeneralConfig.optimizeRenumbering:
            self.RevertToOriginalNumbering()

        """
        try:
            logFile = self.GetLogFilePath()
            self.CheckOpenFOAMCrash(logFile)
        except OpenFOAMNoResidualInfoError:

            logFile = self.GetLogFilePath()
            with open(logFile) as myfile:
                last_lines = myfile.readlines()[-200:-1]
                for line in last_lines:
                    ForcePrint(line)
                Error("OpenFOAM had (probably) failed for the reason stated above in "+logFile)
                return RETURN_FAIL"""
                
        try:
            logFile = self.GetLogFilePath()
            self.CheckOpenFOAMCrash(logFile)
        except OpenFOAMNoResidualInfoError:
            return RETURN_FAIL
        
        return RETURN_SUCCESS

    def RevertToOriginalNumbering(self):
        cmdBuilder = self.BuildCmdFromUtilityName("mapFields")
        cmdBuilder.optionAdd("case")
        workingDirectory = self.GetWorkingDirectory()
        copyPath = os.path.join(workingDirectory,"..","OpenFOAMCleanCopy")
        cmdBuilder.argumentAdd(copyPath)
        cmdBuilder.optionAdd("sourceTime")
        lastStepDirectory = GetLastStepDirectoryInPath(workingDirectory).split(os.sep)[-1]
        cmdBuilder.argumentAdd(lastStepDirectory)
        cmdBuilder.argumentAdd(workingDirectory)

        CopyCleanConfig(workingDirectory,copyPath)
        assert ComputeDirectorySize(copyPath)>0, "Copied analysis directory is empty, should not happen"
        cmdBuilder.optionAdd("consistent")
        cmd = " ".join(cmdBuilder.result())
        self._runCommand(cmd)

        reorderFieldDirPath = os.path.join(copyPath,"0")
        lastStepDirectoryPath = os.path.join(workingDirectory,lastStepDirectory)
        for fileName in os.listdir(reorderFieldDirPath):
            targetFile = os.path.join(reorderFieldDirPath,fileName)
            originalFile = os.path.join(lastStepDirectoryPath,fileName)
            shutil.copyfile(targetFile, originalFile)
        shutil.rmtree(copyPath)

    def ParseMeshAnalysis(self):
        logFile = self.GetLogFilePath()
        captureCaracteristicsActivated = False
        meshDetails = ""
        with open(logFile,'r') as fileContent:
            fileContent = fileContent.readlines()
            for line in fileContent[self.startingFromLineId:]:
                if line.startswith("Checking topology"):
                    captureCaracteristicsActivated = True
                if line.startswith("Mesh") and captureCaracteristicsActivated:
                    meshDetails+=line
                    break
                elif captureCaracteristicsActivated:
                    meshDetails+=line

        meshDetails="\n".join(list(OrderedDict.fromkeys(meshDetails.split("\n"))))
        return meshDetails

    def CleanWorkingDirectory(self):
        """To use in order to remove all directories create during OpenFOAM execution
        """
        workingDirectory = self.GetWorkingDirectory()
        allDirectories = [name for name in os.listdir(workingDirectory) if os.path.isdir(os.path.join(workingDirectory, name))]
        directoryToKeep = ["0","constant","system"]
        directoriesToClean = list(set(allDirectories) - set(directoryToKeep))
        Info("Cleaning analysis directories: "+", ".join(directoriesToClean))
        self.DeleteDirectories(directoriesToClean)
        filesToRename = ["OpenFoamStudy.log"]
        self.RenameFiles(filesToRename)

    def CleanAdjointWorkingDirectory(self,copyId:int,adjointPhysicalVariables:Iterable):
        """To use when the adjoint solver fails. 
        The aim is to remove the diverging adjoint solution on disk (else OpenFOAM will use it as a starting point and fails again) 
        but to keep the direct solution as an initialization to accelerate the convergence (should take a single iteration, significantly save computation time)
        """
        workingDirectory = self.GetWorkingDirectory()
        lastStepDirectory = str(max([float(directory) for directory in os.listdir(workingDirectory) if directory.replace('.','',1).isdigit()]))
        solutionDirectoryPath = os.path.join(workingDirectory,lastStepDirectory)        
        if not os.path.exists(solutionDirectoryPath):
            lastStepDirectory = str(int(float(lastStepDirectory)))
            solutionDirectoryPath = os.path.join(workingDirectory,lastStepDirectory)        

        adjointFieldsFiles = [name for name in os.listdir(solutionDirectoryPath) for variable in adjointPhysicalVariables if variable in name or "Adjoint" in name]
        adjointFieldsFiles = [os.path.join(lastStepDirectory,adjointFieldFile) for adjointFieldFile in  adjointFieldsFiles]
        self.DeleteFiles(adjointFieldsFiles)

        processorDirectories = [name for name in os.listdir(workingDirectory) if os.path.isdir(os.path.join(workingDirectory, name)) and "processor" in name]
        Info("Cleaning directories for adjoints: ",", ".join(processorDirectories))
        self.DeleteDirectories(processorDirectories)

        originPath = "0"
        for fieldToCopy in adjointPhysicalVariables:
            fieldOriginaPath = os.path.join(originPath,fieldToCopy)
            fieldTargetPath = os.path.join(lastStepDirectory,fieldToCopy)
            self.CopyFile(sourceFilePath=fieldOriginaPath,targetFilePath=fieldTargetPath)

        #Plots for direct problems will be overwritten, allow to keep original convergence information (as direct problem should converge in a single iteration)
        plotFiles = [name for name in os.listdir(workingDirectory) if name.endswith(".png")]
        plotFilesDirect = [plotFileName for plotFileName in plotFiles if "Adjoint" not in plotFileName]
        for plotFileDirect in plotFilesDirect:
            if any(char.isdigit() for char in plotFileDirect):
                continue
            newName = plotFileDirect.split(".")[0]+"_"+str(copyId)+".png"
            self.CopyFile(sourceFilePath=plotFileDirect,targetFilePath=newName)

    def RunInSequential(self):
        self.ExecuteSequentialFoamUtility(self.GetAppName())
        Info("Compute sensitivity required by analysis: "+str(self.foamGeneralConfig.computeSensitivity))
        if self.foamGeneralConfig.computeSensitivity:
            self.ExecuteSequentialFoamUtility("computeSensitivities")
        return RETURN_SUCCESS

    def RunInParallel(self):
        self.WriteDecompositionStrategy()
        self.ExecuteSequentialFoamUtility("decomposePar")
        self.ExecuteParallelFoamUtility(self.GetAppName())
        Info("Compute sensitivity required by analysis: "+str(self.foamGeneralConfig.computeSensitivity))
        if self.foamGeneralConfig.computeSensitivity:
            #compute sensitivities (OpenFOAM does not write them all automatically in purge mode)
            self.ExecuteParallelFoamUtility("computeSensitivities")
        self.ExecuteSequentialFoamUtility("reconstructPar")

    def ExecuteOpenFoamPostProcess(self,functionToCompute):
        cmdBuilder = self.BuildCmdFromUtilityName(self.GetAppName())
        cmdBuilder.optionAdd("postProcess")
        cmdBuilder.optionAdd("func")
        cmdBuilder.argumentAdd("\""+functionToCompute+"\"")
        cmdBuilder.optionAdd("case")
        cmdBuilder.argumentAdd(self.GetWorkingDirectory())
        cmd = " ".join(cmdBuilder.result())
        self._runCommand(cmd)
        return RETURN_SUCCESS

    def WriteDecompositionStrategy(self):
        filename = "decomposeParDict"
        keyWordLocation = "system"  
        casePath = self.GetWorkingDirectory()          
        filePath = os.path.join(casePath,keyWordLocation,filename)
        nbCores = GetNumberOfAvailableCores()
        if self.foamGeneralConfig.numberOfSubDomains>nbCores:
            Warning("Not enough cores for this configuration, only "+str(nbCores)+" cores detected (expect the unexpected)")
            
        with open(filePath, "w+") as foamfile:
            header = CreateHeader(filePath=keyWordLocation,keyWordClass="dictionary",keyWordObject=filename)
            foamfile.write(header)
            foamfile.write("\n\n")
            foamfile.write("numberOfSubdomains "+str(self.foamGeneralConfig.numberOfSubDomains)+";\n\n")
            foamfile.write("method             scotch;\n\n")
            foamfile.write("// ************************************************************************* //")

    def ExecuteSequentialFoamUtility(self,foamUtilityName):
        cmdBuilder = self.BuildCmdFromUtilityName(foamUtilityName)
        cmdBuilder.optionAdd("case")
        cmdBuilder.argumentAdd(self.GetWorkingDirectory())
        cmd = " ".join(cmdBuilder.result())
        self._runCommand(cmd)

    def ExecuteParallelFoamUtility(self,foamUtilityName):
        if 'PARALLELRUN_EXECUTABLE' in os.environ.keys():
            parallelJobsExec = os.environ['PARALLELRUN_EXECUTABLE']
            paraExecCustom = parallelJobsExec.split()
            paramExec = paraExecCustom[0]
            cmdBuilder= self.BuildCmdFromUtilityName(paramExec)
            try:
                paraExecOptions = paraExecCustom[1:]
                for paraExecOption in paraExecOptions:
                    cmdBuilder.argumentAdd(paraExecOption)
            except IndexError:
                pass
        else:
            cmdBuilder = self.BuildCmdFromUtilityName(parallelJobsDefaultExec)

        if 'PARALLELRUN_OPTIONS' in os.environ.keys():
            parallelRunOptions = os.environ['PARALLELRUN_OPTIONS']
            parallelRunOptions = parallelRunOptions.split()
            try:
                for parallelRunOption in parallelRunOptions:
                    cmdBuilder.argumentAdd(parallelRunOption)
            except IndexError:
                pass

        cmdBuilder.optionAdd("np")
        cmdBuilder.argumentAdd(str(self.foamGeneralConfig.numberOfSubDomains))
        cmdBuilder.argumentAdd(foamUtilityName)
        cmdBuilder.optionAdd("case")
        cmdBuilder.argumentAdd(self.GetWorkingDirectory())
        cmdBuilder.optionAdd("parallel")
        cmd = " ".join(cmdBuilder.result())
        self._runCommand(cmd)

    def _executeSubprocess(self, cmd):
        logFile = open(self.GetLogFilePath(), "a")
        output = subprocess.run(cmd, shell=True,stdout=logFile,stderr=logFile)
        logFile.close()
        return output

    def CheckOpenFOAMCrash(self,logFile):
        residualByVariable,numberOfLinesParsed,OpenFOAMHasCrashed = ParseInfoFromLogFile(logFile,self.extraQuantitiesToTrack,self.startingFromLineId)
        if OpenFOAMHasCrashed:
            raise OpenFOAMCrashError("OpenFOAM has crashed")

    def CheckConvergence(self,logFile):
        try:
            residualByVariable,numberOfLinesParsed = ParseInfoFromLogFile(logFile,self.extraQuantitiesToTrack,self.startingFromLineId)
        except OpenFOAMCoreDumpedError as e:
            raise Exception("Something went very wrong during OpenFOAM run, see the log"+logFile) from e
        self.startingFromLineId += numberOfLinesParsed

        self.residualByVariable = deepcopy(residualByVariable)
        suspiciousVariablesConvergence = [variableName 
                                          for variableName,variableValues in residualByVariable.items() 
                                          if len(variableValues)==1 and abs(variableValues[0]) == 0]
        if suspiciousVariablesConvergence:
            raise OpenFOAMIllPosedError("Problem likely to be ill posed for variable(s): "+", ".join(suspiciousVariablesConvergence))
        
        if self.foamGeneralConfig.plotFoamQuantities:
           self.PlotFoamQuantities(residualByVariable)

        if self.foamGeneralConfig.enableCheckError:
            self.CheckContinuityError(residualByVariable)
            self.CheckVariableConvergence(residualByVariable)

    def CheckContinuityError(self,residualByVariable):
        continuityErrorVariables =  sorted([variableName for variableName in residualByVariable.keys() if "global_continuity_error" in variableName],reverse=True)
        for continuityErrorVariable in continuityErrorVariables:
            try:
                continuityErrors = residualByVariable[continuityErrorVariable]
            except KeyError:
                raise ValueError("Could not find residual information in logFile ("+str(self.GetLogFilePath())+"). Should not happen, OpenFOAM run may have failed")
            for timeStep,continuityError in enumerate(continuityErrors):
                if abs(continuityError)>self.foamGeneralConfig.continuityError:
                    errorMessage = "Continuity error ("+str(continuityError)+") larger than threshold "\
                        +str(self.foamGeneralConfig.continuityError)+" at time step "+str(timeStep+1)
                    if "Direct" in continuityErrorVariable:
                        raise OpenFOAMDirectConvergenceError(errorMessage+" for direct problem")
                    else:
                        raise OpenFOAMAdjointConvergenceError(errorMessage+" for adjoint problem")
            residualByVariable.pop(continuityErrorVariable)

    def CheckVariableConvergence(self,residualByVariable):
        strippedQuantities = ["".join(extraQuantity.split()) for extraQuantity in self.extraQuantitiesToTrack]
        for variableName,variableResiduals in residualByVariable.items():
            if "".join(variableName.split()) in strippedQuantities:
                continue
            finalResidual = variableResiduals[-1]
            if finalResidual>self.foamGeneralConfig.variableError:
                errorMessage = "Variable "+variableName+" final residual ("+str(finalResidual)+") larger than threshold\
                    "+str(self.foamGeneralConfig.variableError)
                if "AdjointProblem" not in variableName:
                    raise OpenFOAMDirectConvergenceError(errorMessage)
                else:
                    raise OpenFOAMAdjointConvergenceError(errorMessage)

    def PlotFoamQuantities(self,residualByVariable):
        try:
            import matplotlib.pyplot as plt
            for variableName,residualValue in residualByVariable.items():
                plt.figure()
                plt.xlabel('iteration')
                plt.ylabel(variableName)
                plt.plot(residualValue)
                if min(residualValue)>0:
                    plt.yscale('log')
                plt.tight_layout()
                variableName = ''.join(variableName.split()).replace("(","_").replace(")","_")
                outputPath = os.path.join(self.GetWorkingDirectory(),"OpenFoam_"+variableName+".png")
                plt.savefig(outputPath)
                plt.close()
        except ImportError:
            print("matplotlib  module not found : unable to plot foam quantities.")

    def RetrieveProblemConvergenceIteration(self,problemName):
        workingDir = self.GetWorkingDirectory()
        filename = os.path.join(workingDir,self.logFile)

        pattern1 = problemName+" solution converged in"
        pattern2 = problemName+" solution reached max. number of iterations"

        fileDirectory = None
        with open(filename, "r") as fp:
            allLines = fp.readlines()
            for lineId,line in enumerate(allLines):
                if pattern1 in line:
                    fileDirectory = [word for word in line.split() if word.replace('.','',1).isdigit()][0]
                    break
                elif pattern2 in line:
                    relevantBlock = allLines[lineId-20:lineId+1]
                    try:
                        relevantLine = [line for line in reversed(relevantBlock) if "Time =" in line and "ClockTime" not in line][0]
                        fileDirectory = [word for word in relevantLine.split() if word.replace('.','',1).isdigit()][0]
                        break
                    except IndexError:
                        break
        assert fileDirectory is not None, problemName+" may have failed to converge or the directory was not found, see the .log"
        return fileDirectory


    def BuildCmdFromUtilityName(self,foamUtilityName):
        if 'OPENFOAM_EXECUTABLE' in os.environ.keys():
            foamExecCustom = os.environ['OPENFOAM_EXECUTABLE']
            foamExecCustom = foamExecCustom.split()
            foamExec = foamExecCustom[0]
            cmdBuilder = CommandLineBuilder(foamExec)
            try:
                foamExecOptions = foamExecCustom[1:]
                for foamExecOption in foamExecOptions:
                    cmdBuilder.argumentAdd(foamExecOption)
            except IndexError:
                pass
            cmdBuilder.argumentAdd(foamUtilityName)
        else:
            cmdBuilder = CommandLineBuilder(foamUtilityName)
        return cmdBuilder

    def GetLogFilePath(self):
        return os.path.join(self.GetWorkingDirectory(),self.logFile)
    
    def GetResiduals(self):
        return self.residualByVariable

    def AddQuantityToTrack(self,quantityToTrack):
        self.extraQuantitiesToTrack.append(quantityToTrack)

def SkipFoamTest():
    from Muscat.Helpers.CheckTools import SkipTest
    if SkipTest("FOAM_NO_FAIL"):
        return True,"skip"

    #We assume that checking one executable is enough to test if OpenFoam is available
    if 'OPENFOAM_EXECUTABLE' in os.environ.keys():
        executabletest = os.environ['OPENFOAM_EXECUTABLE'].split()[0]
    else:
        executabletest = "simpleFoam"
    if not Which(executabletest):
        return True,"skip: OpenFoam not Available"


    return False,"Foam test can be run"

def CheckIntegrity_GeneralConfig(GUI=False):
    foamConfig = OpenFoamGeneralConfig()
    updatedConfiguration = {"deltaT":0.0001,"relativeTolerance":0.1}
    newConfig = foamConfig.UpdateFromDict(updatedConfiguration)
    assert newConfig.deltaT==updatedConfiguration["deltaT"]
    assert newConfig.relativeTolerance==updatedConfiguration["relativeTolerance"]

    return "ok"

def CheckIntegrity_Log(GUI=False):
    from OpenPisco.TestData import GetTestDataPath
    dataPath = os.path.join(GetTestDataPath(),"OpenFoamStudy.log")
    extraQuantities = ["drag","lift"]
    residualInfo,_ = ParseInfoFromLogFile(logFile=dataPath,extraQuantities=extraQuantities)
    assert all([field in residualInfo for field in extraQuantities]),"drag and lift values should have been parsed correctly"
    for variableName,variableValue in residualInfo.items():
        assert not any(i.isdigit() for i in variableName), "Variable name should not contain any digit"
        assert all(isinstance(e, (int, float)) for e in variableValue),"Variable value should only contain digits"

    interface = OpenFoamInterface()
    interface.CheckConvergence(dataPath)  

    return "ok"

def CheckIntegrity(GUI=False):
    from Muscat.Helpers.CheckTools import RunListOfCheckIntegrities
    return RunListOfCheckIntegrities([CheckIntegrity_GeneralConfig])

if __name__ == '__main__':
    print(CheckIntegrity())  