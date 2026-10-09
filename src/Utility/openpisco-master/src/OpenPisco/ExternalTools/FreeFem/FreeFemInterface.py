# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
"""FreeFemInterface

This interface is specifically designed for the external physical solver FreeFem. Its primary role is to streamline interactions with FreeFem, whether by offering tools to construct the necessary command line for running the solver or by retrieving the fields computed by the solver.
"""

import numpy as np
import os
import shutil

from Muscat.Helpers.IO.Which import Which

from OpenPisco.ExternalTools.FreeFem import GetScriptsPath as GetScriptsPath
from OpenPisco.ExternalTools.AppExecutableTools import AppExecutableBase, CommandLineBuilder
import OpenPisco.PhysicalSolvers.FieldsNames as FN
import OpenPisco.Unstructured.MetricFieldsNames as MFN

#outputs that the solver can generate
FileNamebyFieldName={}
FileNamebyFieldName[FN.potential_energy]="freefeminput.elasticenergy.sol"
FileNamebyFieldName[FN.von_mises]="freefeminput.vonmises.sol"
FileNamebyFieldName[FN.stress]="freefeminput.stress.sol"
FileNamebyFieldName[MFN.metric]="mshmetdata.sol"

FileNamebyScalarName={}
FileNamebyScalarName[FN.int_potential_energy]="intelasticenergy.txt"

freefemExecDefault = "FreeFem++"
freefemExecOptions = []
if 'FREEFEM_EXECUTABLE' not in os.environ.keys():
    freefemExec = freefemExecDefault
else:
    freefemExecCustom = os.environ['FREEFEM_EXECUTABLE']
    if len(freefemExecCustom.split())>1:
        freefemExecCustom = freefemExecCustom.split()
        freefemExec = freefemExecCustom[0]
        freefemExecOptions = freefemExecCustom[1:]
    else:
        freefemExec = freefemExecCustom

FREEFEM_AVAILABLE = Which(freefemExec) 


class FreeFemInterface(AppExecutableBase):
    """
    .. py:class:: FreeFemInterface

    Interface for the finite element solver Freefem++
    """
    def __init__(self):
        super(FreeFemInterface, self).__init__(freefemExec)

    def DeleteGeneratedFiles(self):
        """Delete specific generated files after run"""
        filesToDelete = ["freefeminput.depl.sol","freefeminput.sol"]
        self.DeleteFiles(filesToDelete)

    def ExecuteFreefem(self):
        """Run Freefem++ executable"""
        source = GetScriptsPath()
        assert len(self.filename),"No freefem filename given"
        filesFound = [f for f in os.listdir(source) if f.startswith(self.filename)]
        assert len(filesFound)<=1,"Ambiguity: Only one file at most should match "+self.filename
        if len(filesFound)==1:
           freefemFile=filesFound[0]
           shutil.copy(os.path.join(source,freefemFile), self.workingDirectory )
        cmdBuilder = CommandLineBuilder(self.GetAppName())
        for option in freefemExecOptions:
            cmdBuilder.argumentAdd(option)
        cmdBuilder.optionAdd("nw")
        cmdBuilder.argumentAdd(self._fileName(self.filename))
        cmd = cmdBuilder.result()
        self._runCommand(cmd)

    def GetSolution(self, i:int=0)->np.ndarray:
        """Retrieve solution

        Parameters
        ----------
        i : int, optional
            solution index, by default 0

        Returns
        -------
        np.ndarray
            solution at index i
        """
        self.fileoutname = self._filePath("freefeminput.depl.sol")
        self.reader.SetFileName(self.fileoutname)
        res = self.reader.Read()
        return res.nodeFields["SolAtVertices"+str(i)]

    def GetNodalField(self,name:str)->np.ndarray:
        """Retrieve nodal field

        Parameters
        ----------
        name : str
            field name

        Returns
        -------
        np.ndarray
            nodal field value
        """
        self.fileoutname = self._filePath(FileNamebyFieldName[name])
        self.reader.SetFileName(self.fileoutname)
        res = self.reader.Read()
        if name == FN.stress:
           np.hstack((res.nodeFields["SolAtVertices0"], res.nodeFields["SolAtVertices1"]))
        if name=='metric' and "SolAtVertices1" in res.nodeFields and "SolAtVertices2" in res.nodeFields:
           return np.vstack([res.nodeFields["SolAtVertices0"][:,0],res.nodeFields["SolAtVertices1"][:,0],res.nodeFields["SolAtVertices2"][:,0]]).T
        return res.nodeFields["SolAtVertices0"][:,0]

    def GetScalar(self,name:str)->float:
        """Retrieve scalar

        Parameters
        ----------
        name : str
            scalar name

        Returns
        -------
        float
            scalar value
        """
        return float(np.loadtxt(self._filePath(FileNamebyScalarName[name])))


def CheckIntegrity(GUI=False):
    return "ok"
