# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
r"""AsterInterface

This is the dedicated interface for the external physical solver Code\_Aster. 
Its main role is to facilitate the interactions with Code\_Aster, either by providing features to build the command line required to run the solver or to retrieve field computed by said solver.
"""

import os
import os.path
import shutil
import csv
import subprocess
from typing import List,Tuple,Iterable,Optional

import numpy as np

from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
from Muscat.Helpers.IO.Which import Which
from Muscat.Helpers.Logger import Info, Error
import Muscat.Helpers.ParserHelper as PH
import Muscat.MeshContainers.ElementsDescription as ED

from OpenPisco.ExternalTools.Aster import GetScriptsPath as GetScriptsPath
from OpenPisco.ExternalTools.AppExecutableTools import AppExecutableBase, CommandLineBuilder
from OpenPisco.MuscatExtentions.MedReader import MedReader
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL

#Name of the required scalar value in csv file by type of post-processing
keysbyValue={}
keysbyValue[FN.int_potential_energy]='TRAV_ELAS'
keysbyValue[FN.EigenFreqSquared]='EigenFreqSquared'
keysbyValue[FN.ERP]='ERPABS'
keysbyValue[FN.eigen_Frequencies]="eigen_Frequencies"
keysbyValue[FN.GeneDisp]="GeneDisp"
keysbyValue[FN.load_crit]='CHAR_CRIT'
keysbyValue[FN.ModalCumulMassFractiondX] = 'ModalCumulMassFractiondX'
keysbyValue[FN.ModalCumulMassFractiondY] = 'ModalCumulMassFractiondY'
keysbyValue[FN.ModalCumulMassFractiondZ] = 'ModalCumulMassFractiondZ'
keysbyValue[FN.ModalLastEnergyRatio] = 'ModalLastEnergyRatio'

AsterElemNameToRuleNames = {}
AsterElemNameToRuleNames[ED.Tetrahedron_10] = ['T10_____FPG5']
AsterElemNameToRuleNames[ED.Tetrahedron_4] = ['TE4_____FPG1','TE4_____FPG4']
AsterElemNameToRuleNames[ED.Triangle_3] = ['TR3_____FPG1']
AsterElemNameToRuleNames[ED.Triangle_6] = ['TR6_____FPG3']


AsterComputedFieldsSupport={
                  FN.potential_energy:FN.IPField,
                  FN.strain:FN.IPField,
                  FN.strain_xx:FN.IPField,
                  FN.strain_yy:FN.IPField,
                  FN.strain_zz:FN.IPField,
                  FN.strain_xy:FN.IPField,
                  FN.strain_xz:FN.IPField,
                  FN.strain_yz:FN.IPField,
                  FN.stress:FN.IPField,
                  FN.stress_xx:FN.IPField,
                  FN.stress_yy:FN.IPField,
                  FN.stress_zz:FN.IPField,
                  FN.stress_xy:FN.IPField,
                  FN.stress_xz:FN.IPField,
                  FN.stress_yz:FN.IPField,
                  FN.von_mises:FN.IPField,
                  FN.flux_temperature:FN.IPField,
                  FN.EigenFreqSquared_sensitivity:FN.Nodes,
                  FN.ERP_sensitivity:FN.Nodes,
                  FN.ERP_density:FN.Nodes,
                  FN.dominant_SolutionMode:FN.Nodes,
                  FN.grad_Ure:FN.Nodes,
                  FN.grad_Uim:FN.Nodes,
                  FN.AllModeDEPL:FN.Nodes,
                  FN.elastic_energy_buckling:FN.IPField,
                  FN.strain_buckling:FN.IPField,
                  FN.strain_green_buckling:FN.IPField,
                  FN.mode_buckling:FN.Nodes,
                  FN.temperature:FN.Nodes,
                  FN.zzError:FN.Centroids,
}

def GetScalarFromFile(fileName:str,scalarName:str)->List:
    r"""Get scalar value from file, typically a post processing value computed by Code\_Aster

    Parameters
    ----------
    fileName : str
        input file, assumption it is in the .csv format
    scalarName : str
        scalar name to be retrieved

    If the scalar is not found in the file, an assertion error is raised.

    Here is an example of such a file
    #
    #--------------------------------------------------------------------------------
    ##ASTER 17.02.22 CONCEPT 00000036 CALCULE LE 27/10/2025 A 15:06:07 DE TYPE        
    NUME_ORDRE,INST,TRAV_ELAS,TRAV_REEL
            1, 0.00000E+00, 1.51764E-05, 0.00000E+00

    Returns
    -------
    List
        scalar value corresponding to scalar name
    """
    value = []
    keyNameExist = False
    counter = 0
    lines = []
    with open(fileName) as csvfile:
        reader=csv.reader(csvfile)
        for row in reader:
            counter+=1
            lines.append(row)
            if scalarName in row:
                index = row.index(scalarName)
                icounter=counter
                keyNameExist=True

    assert keyNameExist,"The name "+scalarName+" was not found in "+fileName

    for i in range(icounter,len(lines)):
        if len(lines[i]):
            val = PH.ReadFloat(lines[i][index])
            value.append(val)
        else:
            break
    return value

class AsterPrecisionError(Exception):
    r"""Should be raised when the solution returned by Code_Aster is not accurate enough"""
    pass

class AsterExecutableNotFound(Exception):
    r"""Should be raised when the Code\_Aster executable was not found"""
    pass

asterdefaultExec = "as_run"
asterExecOptions = []
if 'ASTER_EXECUTABLE' not in os.environ.keys():
    asterExec = asterdefaultExec
else:
    asterExecCustom = os.environ['ASTER_EXECUTABLE']
    if len(asterExecCustom.split())>1:
        asterExecCustom = asterExecCustom.split()
        asterExec = asterExecCustom[0]
        asterExecOptions = asterExecCustom[1:]
    else:
        asterExec = asterExecCustom

ASTER_AVAILABLE = Which(asterExec) 
class AsterInterface(AppExecutableBase):
    r"""
    .. py:class:: AsterInterface

    Interface for the finite element solver Code\_Aster
    """

    def __init__(self):
        super(AsterInterface, self).__init__(asterExec)
        self.extrafiles=False
        self.support=None
        self.reader = MedReader()
        self.filepath = GetScriptsPath()
        self.keepStdErr = False
        self.fileCsvname = ""
        self.fileoutname = ""
        self.memoryLimit = None

    def SetExtraFiles(self,extrafiles:bool):
        """Whether to include additional file to build study

        Parameters
        ----------
        extrafiles : bool
            True to include additional files to study.
        """
        self.extrafiles=extrafiles

    def _executeSubprocess(self, cmd:Iterable)->str:
        """Run the solver by using the built command line

        Parameters
        ----------
        cmd : Iterable
            commnad line description to run the solver

        Returns
        -------
        str
            returned state after running the command line
        """
        #TODO: Should be possible to use run instead but annoying bug to solve first
        return subprocess.check_output(' '.join(cmd), shell=True,stderr=subprocess.STDOUT)

    def ComputeMemoryLimit(self):
        """When running the solver, the user is expected to provide beforehand the memory limit. 
        Rather than letting the user estimates the value, it is computed automatically.
        """
        cmdBuilder = CommandLineBuilder(self.GetAppName())
        for option in asterExecOptions:
            cmdBuilder.argumentAdd(option)
        cmdBuilder.argumentAdd("--info")
        cmd = cmdBuilder.result()
        try:
            out = self._runCommand(cmd)
            import io
            keyword = "interactif_memmax : "
            for line in io.StringIO(out):
                if line.startswith(keyword):
                   self.memoryLimit = line.rstrip()[len(keyword):]
        except Exception as e:
            Info("CodeAster warning : unable to estimate memory limit.")
            Info(e)

    def IsAsterStudyFileExtension(self,fileName:str)->bool:
        r"""Check whether a file is a study file (only .comm and .export are considered as such)

        Parameters
        ----------
        fileName : str
            fileName name

        Returns
        -------
        bool
            True if the input file is a Code\_Aster study file, else False
        """
        for ext in ['.comm','.export']:
            if fileName.endswith(ext): return True
        return False

    def GetStudyFilesFromPath(self,path:str)->List[str]:
        """Retrieve file names within a path associated to a specific study

        Parameters
        ----------
        path : str
            location of studies

        Returns
        -------
        List[str]
            Collection of file names associated to a study.
        """
        studyFiles = [fileName for fileName in os.listdir(path) if fileName.startswith(self.filename+'.') and self.IsAsterStudyFileExtension(fileName)]
        if self.extrafiles:
           studyFiles+=[fileName for fileName in os.listdir(path) if fileName.startswith(self.filename) and self.IsAsterStudyFileExtension(fileName)]
        return studyFiles

    def GetSolverComputedError(self)->float:
        """Retrieve solver computed error after the computation

        Returns
        -------
        float
            computed error
        """
        import re
        logfile=self._filePath(self.filename+".log")
        with open(logfile) as myfile:
            targets = list(set([line for line in myfile if "Erreur calcul" in line]))
            errorComputed=[float(x) for x in re.findall(r'-?\d+.?\d*(?:[Ee]-\d+)?', targets[0])][0]
        return errorComputed


    def ExecuteAster(self)->int:
        r"""Run Code\_Aster executable

        Returns
        -------
        int
            RETURN_SUCCESS is sucess, RETURN_FAIL if failure

        Raises
        ------
        AsterExecutableNotFound
            Raised when the executable is not found
        AsterPrecisionError
            Raised when the solver accuracy is not satisfactory
        """
        source = os.getcwd()
        studyFiles = self.GetStudyFilesFromPath(source)
        if not len(studyFiles):
            Info("Study not found in current directory; looking at templates ")
            source = self.filepath
            studyFiles = self.GetStudyFilesFromPath(source)
        assert len(studyFiles)>0,"Aster files corresponding to "+self.filename+" were not found."
        for f in studyFiles:
            if f.endswith(".export"):
                fileReader = open(os.path.join(source, f),"r+")
                fileWriter =open(os.path.join(self.workingDirectory, f) ,"w")
                lines = fileReader.readlines()
                for line in lines:
                    if line.find("P memory_limit ")>-1 and self.memoryLimit is not None:
                        fileWriter.write(line.replace(line,"P memory_limit "+str(self.memoryLimit)+"\n"))
                    else:
                        fileWriter.write(line)
                fileReader.close()
                fileWriter.close()
            else:
                shutil.copy(os.path.join(source,f), self.workingDirectory)

        cmdBuilder = CommandLineBuilder(self.GetAppName())
        for option in asterExecOptions:
            cmdBuilder.argumentAdd(option)
        cmdBuilder.argumentAdd(self._filePath(self.filename+".export"))
        #hack to redirect aster stderr
        if not self.keepStdErr:
           cmdBuilder.argumentAdd("2> /dev/null")
        cmd = cmdBuilder.result()

        #Run executable
        try:
            self._runCommand(cmd)
        except:
            workDir=self.workingDirectory
            asterLogFiles=[os.path.join(workDir, f) for f in os.listdir(workDir) if f.endswith(self.filename+'.log')]
            if len(asterLogFiles)==1:
                logfile=asterLogFiles[0]
            elif not asterLogFiles:
                if not ASTER_AVAILABLE:
                    raise AsterExecutableNotFound("Aster executable not found")
                raise Exception("Problem arising before Aster log was produced")
            else:
                logfile=asterLogFiles[0]
                Info("Warning: several log files found, only look at" + logfile +" file")

            with open(logfile) as myfile:
                if 'La solution du système linéaire est trop imprécise' in myfile.read():
                    raise AsterPrecisionError("Precision problem when solving FEM")
                else:
                    myfile.seek(0)
                    last_lines = myfile.readlines()[-200:-1]
                    for line in last_lines:
                        print(line)
                    Error("Aster executable ("+" ".join(cmd)+") had failed for the reason stated above")
                    return RETURN_FAIL
        return RETURN_SUCCESS

    def CleanWorkingDirectory(self,extensions:Iterable):
        """Clean working directory, remove specific files with provided extensions

        Parameters
        ----------
        extensions : Iterable
            collection of extension to be remove from the working directory
        """
        filesToDelete = [self.filename+extension for extension in extensions]
        self.DeleteFiles(filesToDelete)

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
        self.fileoutname = self._filePath(self.filename+"output.rmed")
        self.support = self.reader.ReadExtraFields(self.support,self.fileoutname,fieldNames=[fieldName],fieldSupports=[fieldSupport],fieldNumOrders=[index])

        try:
            if fieldSupport=="nodes":
                return self.support.nodeFields[fieldName]
            else:
                return self.support.elemFields[fieldName]
        except KeyError as e:
            raise Exception("Field "+fieldName+" was not found within support") from e

    def GetIntegrationRuleFor(self,name:str)->Tuple[np.ndarray,np.ndarray]:
        """Get integration rule from rule name

        Parameters
        ----------
        name : str
            integration rule name

        Returns
        -------
        Tuple[np.ndarray,np.ndarray]
            Integration rule integration point localization and weights 
        """
        locname, _, gscoo , wg = self.reader.ReadGaussPointsLocalization(self.fileoutname)
        assert locname in AsterElemNameToRuleNames[name],"Cannot find integration rule for this element: "+str(name)
        return gscoo, wg

    def GetScalar(self,name:str)->float:
        """Get scalar from name

        Parameters
        ----------
        name : str
            scalar name

        Returns
        -------
        float
            scalar value
        """
        return self.GetScalarsbyOrders(name)[0]

    def GetScalarsbyOrders(self,name:str,listSize:int=1)->float:
        """Get scalar by order

        Parameters
        ----------
        name : str
            scalar name
        listSize : int, optional
            size of collection of scalars expected, by default 1

        Returns
        -------
        float
            scalar values
        """
        self.fileCsvname = self._filePath(self.filename+"_objective.csv")
        keyName = keysbyValue[name]
        value = GetScalarFromFile(fileName=self.fileCsvname,scalarName=keyName)
        assert len(value)==listSize,("Given list size ("+str(listSize)+") is different from the size of scalar list computed by Code_Aster : ",len(value))
        return value

    def GetScalarbyLoadCase(self,name:str,loadcase:str):
        """Get scalar by load case

        Parameters
        ----------
        name : str
            scalar name
        loadcase : str
            load cases names

        Returns
        -------
        float
            scalar values
        """
        self.fileCsvname = self._filePath(self.filename+"_objective.csv")
        keyName = keysbyValue[name]+"_"+str(loadcase)
        value = GetScalarFromFile(fileName=self.fileCsvname,scalarName=keyName)
        return value[0]

def SkipAsterTest():
    """Allow to skip Code_Aster test if dependencies are missing."""
    from Muscat.Helpers.CheckTools import SkipTest
    if SkipTest("ASTER_NO_FAIL"):
        return True,"skip"

    from OpenPisco.MuscatExtentions.MedTools import MEDAvailable
    if not MEDAvailable:
        return True,"skip MED not Available"

    from OpenPisco.ExternalTools.Aster.AsterInterface import ASTER_AVAILABLE
    if not ASTER_AVAILABLE:
        return True,"skip Aster not Available"

    return False,"Aster test can be run"



def PrepareLinToQuadStudy()->Tuple[AsterInterface,str]:
    """Convert a study based on a linear interpolation (P1) to a quadratic interpolation (P2) for shape functions.

    Returns
    -------
    Tuple[AsterInterface,str]
        interface and study path
    """
    import OpenPisco.TopoZones as TZ
    from Muscat.MeshTools.MeshCreationTools import CreateCube

    mesh = CreateCube(dimensions=[3,3,3],spacing=[1./2, 1./2, 1./2],origin=[0, 0, 0],ofTetras=True)
    nbtets = mesh.GetElementsOfType(ED.Tetrahedron_4).GetNumberOfElements()
    mesh.GetElementsOfType(ED.Tetrahedron_4).GetTag(TZ.Bulk).SetIds(np.arange(nbtets) )

    solverInterface = AsterInterface()
    solverInterface.SetFilename("AsterLinToQuad")
    path=TemporaryDirectory.GetTempPath()
    solverInterface.SetWorkingDirectory(path)
    solverInterface.ComputeMemoryLimit()
    from OpenPisco.MuscatExtentions.MedWriter import WriteMed
    WriteMed(path+solverInterface.filename+".med",mesh)
    return solverInterface, path

def CheckIntegrity_LinToQuad(GUI=False):
    """
    Convert a linear mesh into a quadratic one.
    The extra nodes added for the quadratic elements are stored after the original ones in the mesh nodes storage.
    The following line allows to retrive the original nodes in the quadratic mesh
    quadMesh.nodes[0:inputmesh.GetNumberOfNodes(),:]
    The order of elements is left unchanged.
    For more information see https://www.code-aster.org/V2/doc/default/fr/man_u/u4/u4.23.02.pdf
    """
    skipTestPhase,message = SkipAsterTest()
    if skipTestPhase:
        return message

    solverInterface, path = PrepareLinToQuadStudy()
    solverInterface.ExecuteAster()
    from OpenPisco.MuscatExtentions.MedReader import ReadMed
    newMesh = ReadMed(path+solverInterface.filename+".rmed")
    nbTet10 = newMesh.GetElementsOfType(ED.Tetrahedron_10)
    assert nbTet10.GetNumberOfElements()>0, "There should be only Tet10 elements"
    nbTet4 = newMesh.GetElementsOfType(ED.Tetrahedron_4)
    assert nbTet4.GetNumberOfElements()==0, "There should not be any Tet4 element"
    return "ok"

def CheckIntegrity_AsterExecutableNotFound():
    os.environ["ASTER_EXECUTABLE"] = "as_run_dummy"

    from OpenPisco.MuscatExtentions.MedTools import MEDAvailable
    if not MEDAvailable:
        return "skip MED not Available"

    solverInterface, _ = PrepareLinToQuadStudy()
    try:
        solverInterface.ExecuteAster()
    except AsterExecutableNotFound:
        print("Aster executable not found")
    return "ok"

def CheckIntegrity():
    totest = [
        CheckIntegrity_AsterExecutableNotFound,
        CheckIntegrity_LinToQuad,
              ]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
