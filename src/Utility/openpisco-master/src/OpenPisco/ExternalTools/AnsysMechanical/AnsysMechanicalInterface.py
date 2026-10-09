# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
r"""AnsysMechanicalInterface

This is the dedicated interface for the external physical solver AnsysMechanical. 
Its main role is to facilitate the interactions with AnsysMechanical, either by providing features to build the command line required to run the solver or to retrieve field computed by said solver.
"""

import os
import os.path
import shutil
import subprocess
from typing import List,Iterable
import re

import numpy as np

from Muscat.Helpers.IO.Which import Which
from Muscat.Helpers.Logger import Info
import Muscat.Helpers.ParserHelper as PH
from Muscat.IO.RstReader import RstReader, ReadRst
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Mesh import Mesh

from OpenPisco.ExternalTools.AnsysMechanical import GetScriptsPath as GetScriptsPath
from OpenPisco.ExternalTools.AppExecutableTools import AppExecutableBase, CommandLineBuilder
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL

#Name of the required scalar value in csv file by type of post-processing
keysbyValue={FN.temperature:'TEMP'}

AnsysMechanicalComputedFieldsSupport={
                  FN.temperature:FN.Nodes,
}


class AnsysMechanicalExecutableNotFound(Exception):
    r"""Should be raised when the AnsysMechanical executable was not found"""
    pass

ansysmechanicaldefaultExec = "ansys261"
ANSYSMECHANICAL_AVAILABLE = Which(ansysmechanicaldefaultExec) 


class AnsysMechanicalInterface(AppExecutableBase):
    r"""
    .. py:class:: AnsysMechanicalInterface

    Interface for the finite element solver AnsysMechanical
    """

    def __init__(self):
        super(AnsysMechanicalInterface, self).__init__(ansysmechanicaldefaultExec)
        self.extrafiles=False
        self.support=None
        self.reader = RstReader()
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
        return subprocess.check_output(' '.join(cmd), shell=True,stderr=subprocess.STDOUT)

   
    def IsAnsysMechanicalStudyFileExtension(self,fileName:str)->bool:
        r"""Check whether a file is a study file (only .dat are considered as such)

        Parameters
        ----------
        fileName : str
            fileName name

        Returns
        -------
        bool
            True if the input file is a study file, else False
        """
        if fileName.endswith('.dat'): return True
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
        studyFiles = [fileName for fileName in os.listdir(path) if fileName.startswith(self.filename+'.') and self.IsAnsysMechanicalStudyFileExtension(fileName)]
        if self.extrafiles:
           studyFiles+=[fileName for fileName in os.listdir(path) if fileName.startswith(self.filename) and self.IsAnsysMechanicalStudyFileExtension(fileName)]
        return studyFiles


    def ExecuteAnsysMechanical(self)->int:
        r"""Run AnsysMechanical executable

        Returns
        -------
        int
            RETURN_SUCCESS if success, RETURN_FAIL if failure

        Raises
        ------
        AnsysMechanicalExecutableNotFound
            Raised when the executable is not found
        """
        
        source = os.getcwd()
        studyFiles = self.GetStudyFilesFromPath(source)
        if not len(studyFiles):
            Info("Study not found in current directory; looking at templates ")
            source = self.filepath
            studyFiles = self.GetStudyFilesFromPath(source)
        for f in studyFiles:
            shutil.copy(os.path.join(source,f), self.workingDirectory)

        cmdBuilder = CommandLineBuilder(self.GetAppName())
        cmdBuilder.optionAdd("i")
        cmdBuilder.argumentAdd(self._filePath(self.filename+".dat"))
        cmdBuilder.optionAdd("o")
        cmdBuilder.argumentAdd(self._filePath("solve.out"))
        cmdBuilder.optionAdd("b")
        cmdBuilder.optionAdd("dir")
        cmdBuilder.argumentAdd(self.workingDirectory)
        cmdBuilder.optionAdd("j")
        cmdBuilder.argumentAdd(self.filename)
        cmd = cmdBuilder.result()

        #Run executable
        try:
            self._runCommand(cmd)
        except:
            if not ANSYSMECHANICAL_AVAILABLE:
                raise AnsysMechanicalExecutableNotFound("Ansys Mechanical executable not found")
            
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
        
    def GetAnsysMechanicalResults(self)-> Mesh:
        """Read the mesh and solution fields (nodal, elemental and elemental-nodal) from a rth file.
        """
        return ReadRst(self._filePath(self.filename+".rth"))

        
    def ReadViewFactorsMatrix(self,filename):
        """
        Parse an ANSYS ASCII view factor file.
        """
        
        header = re.compile(r"Element number =\s*(\d+)\s*Face Number\s*(\d+)\s*TOTAL=\s*([0-9Ee+\-.]+)")

        sources = []
        totals = []
        rows = []

        with open(filename, "r") as f:
            lines = f.readlines()

        i = 0
        while i < len(lines):
            m = header.search(lines[i])
            if not m:
                i += 1
                continue

            elem = int(m.group(1))
            face = int(m.group(2))
            total = float(m.group(3))

            sources.append((elem, face))
            totals.append(total)

            i += 1
            values = []

            while i < len(lines):
                if header.search(lines[i]):
                    break

                values.extend(float(x) for x in lines[i].split())
                i += 1

            rows.append(values)

        VF = np.array(rows)

        Info("Matrix shape:", VF.shape)
        Info("Number of source surfaces:", len(sources))
        Info("Total :", len(totals))
    
        return VF,sources,totals


def SkipAnsysMechanicalTest():
    """Allow to skip AnsysMechanical test if dependencies are missing."""
    from Muscat.Helpers.CheckTools import SkipTest
    if SkipTest("ANSYSMECHANICAL_NO_FAIL"):
        return True,"skip"

    if not ANSYSMECHANICAL_AVAILABLE:
        return True,"skip AnsysMechanical not Available"

    return False,"AnsysMechanical test can be run"


def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
