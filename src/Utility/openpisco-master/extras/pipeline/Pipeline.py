# -*- coding: utf-8 -*-
"""Note: running this script requires several extra packages not necessarily installed to use OpenPisco: 
- furo, for html style
- psutil, for memory evaluation
- pyvista, in order to be able to export final shape of topology optimization in a nice format
"""
from subprocess import run,check_output
from shutil import copyfile
import time
import re
import os
import traceback
import sys
import inspect
from typing import Iterable,Optional,List,Tuple,Union
from datetime import date
import argparse

try:
    import psutil
except ImportError as error:
    raise Exception("Need psutil library to track memory evolution!") from error

import numpy as np
import matplotlib.pyplot as plt
import matplotlib 

import pyvista as pv

from OpenPisco.Demos import GetDemoPath
OPENPISCO_DEMOS_PATH = GetDemoPath()
OPENPISCO_ROOT_PATH = OPENPISCO_DEMOS_PATH+"../../../"
OPENPISCO_PIPELINE_PATH = OPENPISCO_ROOT_PATH+"extras/pipeline/"

import OpenPisco.CLApp.OpenPiscoCL as OpenPiscoCL

def IsFloat(string:str)->bool:
    try:
        float(string)
        return True
    except ValueError:
        return False

def RetrieveExportedStlFileName(useCaseFile:str)->str:
    localUseCaseFile = useCaseFile.split(os.sep)[-1]
    with open(localUseCaseFile, 'r') as file:
        filedata = file.read()
    actionStart = re.search("SaveShapeToFile", filedata)
    actionEnd = re.search("</Actions>", filedata)
    try:
        actionInformation = filedata[actionStart.start(0):actionEnd.start(0)]
    except AttributeError as error:
        raise Exception("SaveShapeToFile action probably not found in file (should not happen!)") from error
    filenameKeyPosition = re.search("filename=", actionInformation)
    stlFilename = actionInformation[filenameKeyPosition.end(0):].replace('"','').replace('/>','').replace('/n','').strip()
    useCaseName = localUseCaseFile.split(".")[0]
    stlFilename = stlFilename.replace('{INPUTFILE}',useCaseName)
    return stlFilename

def PlotMemoryHistory(inputFile:str,outputFile:str)->Tuple[float,float]:
    with open(inputFile) as f:
        lines = f.readlines()
    splittedLines=[line.split(" ") for line in lines]
    interestingLines=[line for line in splittedLines if 'File' in line]
    times=np.zeros(len(interestingLines))
    memories=np.zeros(len(interestingLines))

    for lineId,line in enumerate(interestingLines):
        information=[data.lstrip('[').rstrip(']').rstrip('MB') for data in line if '[' in data or ']' in data]
        information=[float(data) for data in information if IsFloat(data)]
        times[lineId],memories[lineId]=information

    fig, axis = plt.subplots(1, 1)
    axis.plot(times, memories)
    plt.xlabel("CPU time")
    plt.ylabel("Memory evolution (MB)")
    plt.title("Memory history")
    plt.tight_layout()
    plt.savefig(outputFile)
    plt.close(fig)
    return max(times),max(memories)

def PlotTimePerIterationHistory(inputFile:str,outputFile:str):
    with open(inputFile) as f:
        lines = f.readlines()
    startLineId =  [i for i,x in enumerate(lines) if "***********************************" in x][0]
    lines= lines[startLineId+1:]
    splittedLines=[line.split(" ") for line in lines]
    interestingLines=[line[:3] for line in splittedLines if '|' in line]
    times = [float(line[0].lstrip('[').rstrip(']')) for line in interestingLines]
    times = [instant-times[0] for instant in times]
    iterations = [0]
    for lineId,line in enumerate(interestingLines):
        if lineId==0:
            pass
        else:
            line=line[1:3]
            for termToRemove in ['','|']:
                try:
                    line.remove(termToRemove)
                except:
                    pass
            line=line[0]
            try:
                line=line.lstrip('|')
            except:
                pass
            try:
                line=line.rstrip('|')
            except:
                pass
            iterations.append(int(line))

    nbSubIteration = [iterations.count(index) for index in range(max(iterations))]
    timesPerIteration = []

    startIndex = 0
    for nbIteration in nbSubIteration:
        endIndex = startIndex+nbIteration
        timeForIteration = times[endIndex] - times[startIndex]
        timesPerIteration.append(timeForIteration)
        startIndex +=nbIteration

    fig, axis = plt.subplots(1, 1)
    axis.bar(list(range(len(timesPerIteration))),timesPerIteration)
    plt.xlabel('Iteration ID', fontsize = 15)
    plt.ylabel('CPU time (s)', fontsize = 15)
    plt.title("Time per optimization iteration history")
    plt.savefig(outputFile)
    plt.close(fig)

def RetrieveCriteriaValuesFromFile(inputFile:str)->Tuple[List,List]:
    with open(inputFile) as f:
        lines = f.readlines()
    startLineId =  [i for i,x in enumerate(lines) if "***********************************" in x][0]
    lines= lines[startLineId+1:]
    interestingLines = [line.replace(" ", "").split("|") for line in lines if '|' in line]
    notInterestingStrings = ['cpt', 'iter', 'Gain%', 'StepSize','Objective','\n']
    columnIndices = [termIndices for termIndices,value in enumerate(interestingLines[0]) if value not in notInterestingStrings][1:]
    stepIndices = columnIndices[0]
    criteriaColumnIndices = columnIndices[1:]
    criteriaNames = [interestingLines[0][columnIndice] for columnIndice in criteriaColumnIndices]
    criteriaValues = dict(zip(criteriaNames,[[] for _ in criteriaNames]))
    for line in interestingLines[1:]:
        if line[stepIndices].isdigit():
            for criteriaName,criteriaIndex in zip(criteriaNames,criteriaColumnIndices):
                criteriaValue = line[criteriaIndex]
                criteriaValues[criteriaName].append(float(criteriaValue))
    return criteriaNames,criteriaValues

def PlotCriteriaThroughOptimization(inputFile:str,outputFile:str):
    criteriaNames,criteriaValues = RetrieveCriteriaValuesFromFile(inputFile)
    fig, axis = plt.subplots(len(criteriaNames), 1)

    for criteriaId,criteriaName in enumerate(criteriaNames):
        axis[criteriaId].plot(range(len(criteriaValues[criteriaName])),criteriaValues[criteriaName])
        axis[criteriaId].set_title(criteriaName)
    plt.savefig(outputFile)
    plt.close(fig)

def PlotBarPerBrick(inputFile:str,finalTime:float,debugMessageMapping:dict,outputFile:str):
    with open(inputFile) as f:
        lines = f.readlines()
    filteredLines =  [line for line in lines if "line" in line and "[" in line]
    bricksName = debugMessageMapping.keys()
    timers = dict(zip(bricksName,len(bricksName)*[0]))
    for lineId,line in enumerate(filteredLines):
        for brickName,debugMessage in debugMessageMapping.items():
            if debugMessage in line:
                startTime = float(line.split("[")[1].split("]")[0])
                lineStart = lineId
                endTime = startTime
                for lineForBrickId,lineForBrick in enumerate(filteredLines[lineStart:]):
                    if all(word in lineForBrick for word in debugMessage.split()) and ("done" in lineForBrick or "Done" in lineForBrick):
                        endTime = float(filteredLines[lineStart+lineForBrickId].split("[")[1].split("]")[0])
                        break
                timers[brickName]+=(endTime-startTime)

    timers["other"] = finalTime - sum(timers.values())
    
    fig = plt.figure(figsize = (8, 8))
    labels = timers.keys()
    sizes = [timers[label]/finalTime for label in labels]
    matplotlib.rcParams.update({'font.size': 14})
    percentages = [s * 100 for s in sizes]
    _, ax = plt.subplots(figsize=(10, 5))
    bars = ax.barh(labels, percentages, color='skyblue')

    for bar, pct in zip(bars, percentages):
        if pct < 0.01:
            text = f'{pct:.4f}%'
        else:
            text = f'{pct:.2f}%'

        x_pos = bar.get_width() + 0.5 if bar.get_width() < 95 else bar.get_width() - 5
        align = 'left' if bar.get_width() < 95 else 'right'

        ax.text(x_pos,bar.get_y() + bar.get_height() / 2,text,va='center',ha=align,fontsize=9,rotation=270)

    ax.set_xlabel('Time Percentage (%)')
    ax.set_xlim(0, 100)
    ax.invert_yaxis()
    plt.tight_layout()
    plt.savefig(outputFile)
    plt.close(fig)


def ExportFinalShape(inputFile:str,rotation:Optional[str]=None)->str:
    inputMesh = pv.read(inputFile)
    axisNames = ["x","y","z"]
    suffix = ""
    if rotation in axisNames and rotation is not None:
        if rotation=="x":
            inputMesh = inputMesh.rotate_x(90, inplace=False)
        elif rotation=="y":
            inputMesh = inputMesh.rotate_y(90, inplace=False)
        elif rotation=="z":
            inputMesh = inputMesh.rotate_z(90, inplace=False)
        suffix = "_rotated_"+rotation
    pl = pv.Plotter(off_screen=True)
    pl.add_mesh(inputMesh, smooth_shading=True)
    outputFile = inputFile.split(".")[0]+suffix+".png"
    pl.screenshot(outputFile)
    return outputFile

def GetDebugMessageMappingPerComponent():
    debugMessages = ["Remesh from levelset",
                    "In update all criteria",
                    "In solve physical problems",
                    "in LevelSetUpdate",
                    "Step optim algo"]
    bricksName = ["remesh","criteria update","solvers","levelset update","optim algo"]
    debugMessageMapping = dict(zip(bricksName,debugMessages))
    return debugMessageMapping

def GetDebugMessageMappingPerCommandLine(inputFile):
    with open(inputFile) as f:
        lines = f.readlines()

    debugMessagesMapping = set()
    for lineId,line in enumerate(lines):
        if "line" in line and "[" in line and "Launching" in line and "done" not in line:
            if line.split("Launching")[-1]==' \n':
                appName = lines[lineId+2].split("[")[-1].split(",")[0].split(os.sep)[-1].replace("'","").replace(" ", "")
            else:
                appName = line.split("Launching")[-1].split(os.sep)[-1].replace("'","").replace("\n","").replace(" ", "")
            debugMessage = line.split("INFO -->")[-1]
            debugMessagesMapping.add((appName,debugMessage))

    debugMessagesMapping = {k: v for k, v in debugMessagesMapping}
    return debugMessagesMapping

class PiscoReportGenerator():
    def __init__(self,name:str):
        self.name = name
        self.namingSuffix = {"memory":"_ResultMemory",
                             "timePerIteration":"_ResultPerIteration",
                             "optimCriteria":"_ResultPerCriteria",
                             "timePerComponent":"_ResultPerOptimTopoComponent",
                             "timePerCommandLine":"_ResultPerCommandLineExecutable"}

    def GenerateReport(self,useCases:Iterable):
        introductionText = self.GenerateIntroduction()

        simulationsText = []

        start = time.time()
        for useCase in useCases:
            useCaseName = useCase.split(os.sep)[-1].split(".")[0]
            simulationText = """
Test Case """+useCaseName+"""
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"""
            try:
                finalCriteriaValues = self.RunOptimizationWithPostProcessing(useCaseFile=useCase,useCaseName=useCaseName)
                simulationTextFigure = self.GenerateFigures(useCaseName=useCaseName)
                simulationTextFinalValues= self.GenerateValues(useCaseName=useCaseName,finalCriteriaValues=finalCriteriaValues)
                statusOk,filenames = self.SaveFinalShape(useCaseFile=useCase)
                simulationTextFinalShape = self.GenerateFinalShapeTex(statusOk=statusOk,filenames=filenames,useCaseName=useCaseName)
                simulationText += simulationTextFigure
                simulationText += simulationTextFinalValues
                simulationText +=simulationTextFinalShape
            except Exception as e:
                print(traceback.format_exc())
                simulationText += ["The simulation "+useCaseName+" has failed because "+str(e).replace("'","")]
            simulationsText.extend(simulationText)
        end = time.time() - start

        introductionText+="""
Report generated in """+str(end)+""" seconds

Results
============
"""
        with open(self.name+".rst", 'w') as file:
            file.write(introductionText)


            for simulationText in simulationsText:
                file.write(simulationText)

        currentWorkingDirectory = os.getcwd()
        if currentWorkingDirectory!=OPENPISCO_PIPELINE_PATH:
            onlyfiles = [f for f in os.listdir(currentWorkingDirectory) if os.path.isfile(os.path.join(currentWorkingDirectory, f))]
            onlyRelevantfiles = [f for f in onlyfiles if (f.split(".")[-1]=="png" or f.split(".")[-1]=="rst")]
            for relevantFile in onlyRelevantfiles:
                originPath = os.path.join(currentWorkingDirectory, relevantFile)
                targetPath = os.path.join(OPENPISCO_PIPELINE_PATH, relevantFile)
                copyfile(originPath,targetPath)

        run(["make","-C",OPENPISCO_PIPELINE_PATH,"html"])

    def GenerateIntroduction(self)->str:
        introductionText = self.GenerateBasicIntroduction()
        introductionText += self.GenerateMachineDetails()
        introductionText += self.GenerateCondaEnvDetails()
        introductionText += self.GenerateLastCommitPiscoDetails()
        return introductionText

    def GenerateBasicIntroduction(self)->str:
        today = date.today()
        introductionText = str(today)+"\n"
        introductionText += """ 
############################## 
OpenPisco: No Regression Tests 
############################## 

Introduction
============

The aim of the present document is to provide sharable traces of several examples to make sure there are properly reproducible (results and performances) across incoming releases. \n

For now, we only consider the scripts available in the Demo directory in the OpenSource version and that can be run without any external physical solvers (meaning without dependencies like Freefem, Coda\_Aster or Zset).
For each case and for a given machine, we collect the following items: \n

* Total computation time for the whole optim \n
* Detailed computation time per optimization design step \n
* Memory consumption evolution through the optimization \n
* Criteria evolution through the optimization \n
* Final values of each criteria \n
* Final shape displayed at different angles \n

We also provide the machine characteristics, the description of the conda environment and the OpenPisco version used (last commit of the branch tested). \n
"""
        return introductionText

    def GenerateMachineDetails(self)->str:
        introductionText = """
Machine
============
Characteristics
~~~~~~~~~~~~~~~~
"""
        introductionText +="* Machine Name: "+str(check_output(["hostname"])).lstrip('\'b').rstrip('\'n')+r"""
        """
        lscpuReturn = str(check_output(["lscpu"])).split("\\")[:25]
        lscpuReturn = ["  ".join(lscpuInfo.lstrip('n').lstrip('\'b').split()) for lscpuInfo in lscpuReturn]
        for term in lscpuReturn:
            introductionText += "* "+term+r"""
            """
        return introductionText

    def GenerateCondaEnvDetails(self)->str:
        introductionText = """
Environment \n
~~~~~~~~~~~~~~~~ \n
"""
        condaEnvData = str(check_output(["conda", "list"])).split("\\")
        condaEnvData = ["  ".join(line.replace("#","").lstrip('n').lstrip('\'b').split()) for line in condaEnvData][2:]
        condaEnvData = [packageDescription for packageDescription in condaEnvData if len(packageDescription)]
        for packageCarac in condaEnvData:
            introductionText += "* "+packageCarac+r"""
        """
        return introductionText

    def GenerateLastCommitPiscoDetails(self)->str:
        introductionText = """
OpenPisco repository details
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Last commit in tested branch
"""
        gitCommand = check_output(["git","--git-dir",OPENPISCO_DEMOS_PATH+ "../../../.git","log","-n","1"]).decode().splitlines()
        gitCommand = ["  ".join(line.replace("_",r"\_").replace("#","").lstrip('n').split()) for line in gitCommand ]
        for gitCommandCarac in gitCommand:
            introductionText +="* "+gitCommandCarac+r"""
        """
        introductionText+= r"""
* Branch name
"""
        gitCommand = check_output(["git","--git-dir",OPENPISCO_DEMOS_PATH+ "../../../.git","branch"]).decode().splitlines()
        gitCommand = ["  ".join(line.replace("_",r"\_").replace("#","").lstrip('n').split()) for line in gitCommand ]
        for gitCommandCarac in gitCommand:
            if "*" in gitCommandCarac:
                introductionText +="* "+gitCommandCarac+r"""
                """

        return introductionText

    def GenerateFigures(self,useCaseName:str,)->str:
        figureSuffixes = ["memory","timePerIteration","optimCriteria","timePerComponent","timePerCommandLine"]
        figureNames = ["Memory evolution","Time per iteration","optimCriteria","Time per brick component","Time per command line"]
        simulationText=""
        for figureSuffix,figureName in zip(figureSuffixes,figureNames): 
            simulationText += r"""
                .. figure:: """+useCaseName+self.namingSuffix[figureSuffix]+r""".png
                    :width: 500
                    :align: center
                    :alt: Grid sizes
                    :figclass: align-center

                """+useCaseName+" "+figureName+"\n"
        return simulationText


    def GenerateValues(self,
                          useCaseName:str,
                          finalCriteriaValues:dict)->str:
        simulationText = r"""
            .. csv-table:: Final values for use case  """+useCaseName+r"""
                :header: "Function", "value"
                :widths: 30, 30
            """

        for criteria,value in finalCriteriaValues.items():
            simulationText+="""
                """+criteria+r""","""+str(value)+"\n"

        return simulationText

    def GenerateFinalShapeTex(self,
                              statusOk:bool,
                              filenames:Iterable,
                              useCaseName:str)->str:

        if statusOk:
            simulationText = """
            .. figure:: """+filenames["NoRotation"]+r"""
                :width: 500
                :align: center
                :alt: Grid sizes
                :figclass: align-center

            """+filenames["NoRotation"]+r""" Final shape

            .. figure:: """+filenames["XRotation"]+r"""
                :width: 500
                :align: center
                :alt: Grid sizes
                :figclass: align-center

            """+filenames["XRotation"]+r""" Final shape rotated along x

            .. figure:: """+filenames["YRotation"]+r"""
                :width: 500
                :align: center
                :alt: Grid sizes
                :figclass: align-center

            """+filenames["YRotation"]+r""" Final shape rotated along y

            .. figure:: """+filenames["ZRotation"]+r"""
                :width: 500
                :align: center
                :alt: Grid sizes
                :figclass: align-center

            """+filenames["ZRotation"]+r""" Final shape rotated along z
            """
        else:
            simulationText = "Could not retrieve final stl shape for "+useCaseName+" (stl empty or does not exist for instance)"


        return simulationText

    def RunOptimizationWithPostProcessing(self,useCaseFile:str,useCaseName:str)->dict:
        self.RunOptimization(useCaseFile=useCaseFile)
        logFile = useCaseFile.split(os.sep)[-1]+".log"
        log2File = useCaseName+".log2"
        maxCPUTime = self.PlotAll(inputFileLog=logFile,inputFileLog2=log2File,name=useCaseName)
        _,criteriaValues = RetrieveCriteriaValuesFromFile(inputFile=logFile)
        try:
            finalCriteriaValues = {criteriaName:criteriaValue[-1] for criteriaName,criteriaValue in criteriaValues.items()}
        except IndexError:
            finalCriteriaValues = {criteriaName:"None" for criteriaName in criteriaValues.keys()}

        finalCriteriaValues["CPU time"] = maxCPUTime
        return finalCriteriaValues

    def RunOptimization(self,useCaseFile:str):
        newFile = useCaseFile.split(os.sep)[-1]
        useCaseName = newFile.split(".")[0]
        copyfile(useCaseFile,newFile)

        with open(newFile, 'r') as file:
            filedata = file.read()

        filedata = filedata.replace('debug="False"', 'debug="True"')
        filedata = filedata.replace('debug = "False"', 'debug="True"')
        filedata = filedata.replace('debug= "False"', 'debug="True"')
        filedata = filedata.replace('debug ="False"', 'debug="True"')

        actionsStart = re.search(r"<Actions>", filedata)
        actionsEnd = re.search(r"</Actions>", filedata)

        beforeActions = filedata[:actionsStart.end(0)]
        actions = filedata[actionsStart.end(0):actionsEnd.start(0)]
        afterActions = filedata[actionsEnd.start(0):]

        stlWriteActionToAdd = re.search(r"SaveShapeToFile", filedata) is None
        if stlWriteActionToAdd:
            actions+=r'<Action type="SaveShapeToFile" ls="1" filename="'+useCaseName+r'.stl"/>'

        filedata = beforeActions+actions+afterActions

        with open(newFile, 'w') as file:
            file.write(filedata)

        openPiscoCLPath = inspect.getfile(OpenPiscoCL)
        with open(useCaseName+".log2", "w") as outfile:
             run([sys.executable,openPiscoCLPath, newFile],stderr=outfile)

    def PlotAll(self,inputFileLog:str,inputFileLog2:str,name:str)->float:
        PlotTimePerIterationHistory(inputFile=inputFileLog,outputFile=name+self.namingSuffix["timePerIteration"]+".png")
        PlotCriteriaThroughOptimization(inputFile=inputFileLog,outputFile=name+self.namingSuffix["optimCriteria"]+".png")
        maxCPUTime,_ = PlotMemoryHistory(inputFile=inputFileLog2,outputFile=name+self.namingSuffix["memory"]+".png")

        debugMessageMappingBricks = GetDebugMessageMappingPerComponent()
        PlotBarPerBrick(inputFile=inputFileLog2,
                        debugMessageMapping=debugMessageMappingBricks,
                        finalTime=maxCPUTime,
                        outputFile=name+self.namingSuffix["timePerComponent"]+".png")
        debugMessageMappingCommandLine = GetDebugMessageMappingPerCommandLine(inputFile=inputFileLog2)
        PlotBarPerBrick(inputFile=inputFileLog2,
                        debugMessageMapping=debugMessageMappingCommandLine,
                        finalTime=maxCPUTime,
                        outputFile=name+self.namingSuffix["timePerCommandLine"]+".png")
        return maxCPUTime

    def SaveFinalShape(self,useCaseFile:str)->Tuple[bool,Union[dict,None]]:
        stlFile = RetrieveExportedStlFileName(useCaseFile=useCaseFile)
        if not os.path.exists(stlFile):
            print("Stl file not found")
            return False,None
        meshToPlot = pv.read(stlFile)
        if not len(meshToPlot.points):
            print("Stl file likely to be empty")
            return False,None

        useCaseNames = dict()
        useCaseNames["NoRotation"] = ExportFinalShape(inputFile=stlFile)
        useCaseNames["XRotation"] = ExportFinalShape(inputFile=stlFile,rotation="x")
        useCaseNames["YRotation"] = ExportFinalShape(inputFile=stlFile,rotation="y")
        useCaseNames["ZRotation"] = ExportFinalShape(inputFile=stlFile,rotation="z")
        return True,useCaseNames

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Optional app description')
    parser.add_argument('--usecases', action='append')
    args = parser.parse_args()
    report = PiscoReportGenerator(name="reportopenpisco")

    useCaseFilesDefault  = [
                  "2D/Cantilever_Conform.xml",
                  "2D/Cantilever_Structured.xml",
                  "2D/Cantilever_Unstructured.xml",
                  "Aage_Lazarov_2013/Cantilever.xml",
                  "Allaire_al_2017/Bridge.xml",
                  "Liu_Tovar_2014/Cantilever_T4_C1.xml",
                  "Liu_Tovar_2014/Cantilever_T4_C1NewAPI.xml",
                  "Liu_Tovar_2014/Cantilever_T4_C1_Conforme.xml",
                  "Liu_Tovar_2014/LS_T4_C3.xml",
                  "Liu_Tovar_2014/MBB_T4_C2.xml",
                  "Demos_Aster/Cantilever.xml"
                  ]
    useCaseFiles = useCaseFilesDefault if args.usecases is None else args.usecases

    useCaseFiles = [OPENPISCO_DEMOS_PATH+useCaseFile for useCaseFile in useCaseFiles]
    report.GenerateReport(useCases=useCaseFiles)
