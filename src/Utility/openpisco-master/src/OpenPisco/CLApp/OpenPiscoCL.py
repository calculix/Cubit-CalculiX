# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

"""OpenPiscoCL

Allow to read, parse and run utilities from OpenPisco with the command line using a domain specific langage (DSL).
"""
import sys
import os
from math import log10
import traceback
from typing import Optional,Dict

import numpy as np

from Muscat.Helpers.PrintBypass import PrintBypass as PB
from Muscat.Helpers.TextFormatHelper import TFormat
import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.IO.PathController import PathController
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
import Muscat.Helpers.Logger as Logger
from Muscat.Helpers.LocalVariables import AddToGlobalDictionary, globalDict

from Muscat.IO.ProxyWriter import ProxyWriter
from Muscat.IO.IOFactory import CreateWriter, InitAllWriters

from Muscat.ImplicitGeometry.ImplicitGeometryBase import ImplicitGeometryDelayedInit, ImplicitGeometryCachedData
from Muscat.ImplicitGeometry.ImplicitGeometryOperators import ImplicitGeometryUnion
from Muscat.ImplicitGeometry.ImplicitGeometryFactory import InitAllImplicitGeometry

from Muscat.MeshTools.MeshCreationTools import CreateCube, CreateSquare
from Muscat.MeshTools.MeshCreationTools import QuadToLin

from OpenPisco import RETURN_SUCCESS
from OpenPisco.CLApp.PlugIn import PluginLoader
from OpenPisco.CLApp.XmlToDic import XmlToDic
from OpenPisco.Optim.Problems.ProblemFactory import InitAllProblems
from OpenPisco.Optim.Criteria.CriteriaFactory import InitAllCriteria
from OpenPisco.Optim.Algorithms import OptimAlgoFactory
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import InitAllPhysicalSolvers,Create
from OpenPisco.Optim.Problems.ProblemFactory import Create as CreateProblem

from OpenPisco.Optim.Problems.OptimProblemDerived import OptimProblemPenalized,OptimProblemSquared

InitAllImplicitGeometry()

from Muscat.IO.UniversalReader import InitAllReaders, ReadMesh
InitAllReaders()
from Muscat.IO.UniversalWriter import InitAllWriters
InitAllWriters()

import OpenPisco.MuscatExtentions.MedReader
import OpenPisco.MuscatExtentions.MedWriter

from Muscat.FE.Spaces.FESpaces import InitAllSpaces

InitAllSpaces()

if "OpenPiscoPlugins" in os.environ:
    from OpenPisco.CLApp.PlugIn import PluginLoader
    plugins = os.environ["OpenPiscoPlugins"].split(os.pathsep)
    for plugin in plugins:
        PluginLoader("Plugin", {"name": plugin})

from OpenPisco.TestData import GetTestDataPath

AddToGlobalDictionary("PISCO_TEST_DATA", GetTestDataPath())

from Muscat.TestData import GetTestDataPath

AddToGlobalDictionary("MUSCAT_TEST_DATA", GetTestDataPath())
AddToGlobalDictionary("CWD", os.getcwd())


class MainApp:
    """Run OpenPisco from the command line while relying on the DSL"""
    def __init__(self):
        super(MainApp, self).__init__()
        self.dimensionality = 3
        self.zones = dict()
        self.grids = dict()
        self.levelSets = dict()
        self.physicalProblems = dict()
        self.optimProblems = dict()
        self.outputs = dict()
        self.optimAlgos = dict()
        self.actions = {"children": []}
        self.beforeActionCallBack = None
        self.closeOutputFilesIfSIGINT = True
        self.filepath = ""
        self.useProxyWriter = None
        self.inputReader = XmlToDic()
        TemporaryDirectory.prefix = "OpenPisco_Temp_"

    def ReadFile(self, filename:str):
        """Read file

        Parameters
        ----------
        filename : str
            file path
        """
        PathController.SetWorkingDirectoryUsingFile(filename)

        self.filepath = PathController.GetWorkingDirectory()

        f = open(filename, "r", errors="ignore")
        string = f.read()
        f.close()

        cleanfilename, fileExtension = os.path.splitext(filename)
        AddToGlobalDictionary("INPUTFILE", os.path.basename(cleanfilename))
        AddToGlobalDictionary("INPUTFILEPATH", os.path.dirname(filename))

        self.SetInternalReaderTo(fileExtension, filename=filename)
        try:
            self.ReadText(string)
        except:
            print(f"Error Reading file {filename} ")
            raise

    def SetInternalReaderTo(self, fileExtension:str, filename:Optional[str]=None):
        """Set internal reader state to handle DSL accordingly based on file extension

        Parameters
        ----------
        fileExtension : str
            DSL type to use, only .xml and .openpisco are allowed for now
        filename : Optional[str], optional
            file name to consider as an input, by default None
        """
        errorMessage = "Only .pisco and .xml extensions are supported"
        assert fileExtension in [".xml", ".pisco"], errorMessage
        if fileExtension == ".xml":
            from OpenPisco.CLApp.XmlToDic import XmlToDic

            self.inputReader = XmlToDic()

        elif fileExtension == ".pisco":
            from OpenPisco.CLApp.PiscoToDic import PiscoToDic

            self.inputReader = PiscoToDic()

        if filename is None:
            self.inputReader.filenamePath = PathController.GetWorkingDirectory()
        else:
            self.inputReader.filenamePath = PathController.GetFullPathCurrentDirectoryUsingFile(filename)

    def TextToDict(self, string:str)->Dict:
        """Convert text in dictionary

        Parameters
        ----------
        string : str
            text to convert

        Returns
        -------
        Dict
            converted string
        """
        string = PH.ApplyGlobalDictionary(string)
        _, _dict = self.inputReader.ReadFromString(string)
        return _dict

    def ReadText(self, string:str):
        """Read text

        Parameters
        ----------
        string : str
            string
        """
        dic = self.TextToDict(string)
        self.LoadFromDict(dic)

    def LoadFromDict(self, dic:Dict):
        """Load from dict

        Parameters
        ----------
        dic : Dict
            _description_
        """
        # for the moment we remove the desc tags for the dict
        def Cleandesc(di):
            if "desc" in di.keys():
                di.pop("desc")
            if "children" in di:
                for _, v1 in di["children"]:
                    Cleandesc(v1)

        Cleandesc(dic)

        self.dimensionality = int(dic.get("dim", 3))
        if "debug" in dic and PH.ReadBool(dic["debug"]):
            Logger.VerboseMuscatOutput()

        if "WD" in dic:
            if dic["WD"] == "TEMP":
                TemporaryDirectory.path = None
                path = TemporaryDirectory.GetTempPath()
            elif dic["WD"] == "RAM":
                TemporaryDirectory.path = None
                path = TemporaryDirectory.GetTempPath(onRam=True)
            else:
                TemporaryDirectory.SetTempPath(dic["WD"])
                path = TemporaryDirectory.GetTempPath()
            PathController.SetWorkingDirectory(path)

        print("Using temporary directory: " + str(TemporaryDirectory.GetTempPath()))

        ReadingOrder = [
            {"Globals": self.ReadGlobals},
            {"Plugins": self.LoadPlugins},
            {"Grids": self.ReadGrids},
            {"LevelSets": self.ReadLevelSets},
            {"Zones": self.ReadZones, "Outputs": self.ReadOutputs},
            {"PhysicalProblems": self.ReadPhysicalSolvers},
            {"OptimProblems": self.ReadOptimizationProblems},
            {"TopologicalOptimizations": self.ReadOptimizationAlgorithms},
            {"AutoActions": self.ReadAndRunAutoActions},
            {"Actions": self.ReadActions},
        ]

        for order in ReadingOrder:
            for tag, child in dic["children"]:
                if tag in order:
                    Logger.ForcePrint("Reading " + str(tag))
                    order[tag](child)
                    Logger.ForcePrint("Reading " + str(tag) + " Done")

    def ReadGlobals(self, gs:Dict):
        """Read global native types

        Parameters
        ----------
        gs : Dict
            data
        """
        types = {"Str": str, "Bool": bool, "Float": float, "Int": int}

        if "children" in gs:
            for tag, data in gs["children"]:
                t = types[tag]
                name = PH.ReadString(data["name"])
                val = PH.Read(data["val"], t())
                PH.AddToGlobalDictionary(name, str(val))

    def ReadAndRunAutoActions(self, Rs:Dict):
        """Read and run all actions

        Parameters
        ----------
        Rs : Dict
            collection of actions
        """
        self.inputReader.ReplaceObjectFromAlmanac(Rs, self)
        for _, child in Rs["children"]:
            self.RunAction(child)

    def RunPythonFunction(self, tag, data):
        if not tag.lower().startswith("python"):
            raise RuntimeError(f"Cant treat tag of type {tag} with data {data}")

        if "CDATA" not in data:
            data["CDATA"] = ""

        if "function" in data:
            functionName = data["function"]
            func = globalDict.variables[functionName]
            func(self, data)
        elif "exec" in data and PH.ReadBool(data["exec"]):
            local = {"mainApp": self, "tag": tag, "data": data, "PH": PH}
            local.update(**data)
            exec(data["CDATA"], globals(), local)
        else:
            raise RuntimeError("Need a function to run")

    def ReadActions(self, Rs:Dict):
        """Read new actions and add them to current existing actions

        Parameters
        ----------
        Rs : Dict
            new actions to read
        """
        self.inputReader.ReplaceObjectFromAlmanac(Rs, self)
        if "children" in Rs:
            self.actions["children"].extend(Rs["children"])

    def ReadOutput(self, tag, O):
        if tag.lower().startswith("python"):
            self.RunPythonFunction(tag, O)
            return

        ID = int(O["id"])
        del O["id"]

        tname = PH.ReadString(O["name"])
        O.pop("name", None)

        if tname.lower() == "proxy":
            writer = ProxyWriter()
            writer.writers = [self.outputs[i] for i in PH.ReadInts(O["outputs"])]
            O.pop("outputs", None)
        else:
            if "inWorkingDirectory" in O and PH.ReadBool(O["inWorkingDirectory"]):
                name = PathController.GetFullFilenameOnWorkingDirectory(tname)
            elif "inTempDirectory" in O and PH.ReadBool(O["inTempDirectory"]):
                name = TemporaryDirectory.GetFullFilenameOnTempDirectory(tname)
            else:
                name = PathController.GetFullFilenameCurrentDirectory(tname)

            O.pop("inWorkingDirectory", None)
            O.pop("inTempDirectory", None)

            print("Opening file : " + name)
            InitAllWriters()

            writer = CreateWriter(name)
            writer.SetFileName(name)
            if not writer.canHandleTemporal:
                print(TFormat.InRed("This type of writer" + str(type(writer)) + " cant handle temporal data "))
                print(TFormat.InRed("Only the last output will be saved"))
            else:
                writer.SetTemporal()
            if hasattr(writer, "SetHdf5") and callable(writer.SetHdf5):
                writer.SetHdf5()
            else:
                writer.SetBinary()
            writer.automaticOpen = True

            if "append" in O and PH.ReadBool(O["append"]):
                writer.SetAppendMode()
            O.pop("append", None)

        PH.ReadProperties(O, O.keys(), writer)

        if self.useProxyWriter is None:
            self.outputs[ID] = writer
        else:
            self.outputs[ID] = self.useProxyWriter(writer)

    def ReadOutputs(self, outputs:Dict):
        for tag, data in outputs["children"]:
            self.ReadOutput(tag, data)

    def ReadImplicitGeometryZones(self, attrs:Dict):
        # the order is important
        tagsToRead = ["OnZone", "OffZone"]
        res = []
        for tag in tagsToRead:
            if tag in attrs:
                Ids = PH.ReadInts(attrs[tag])
                zone = ImplicitGeometryCachedData(ImplicitGeometryUnion([self.zones[x] for x in Ids]))
            else:
                zone = None
            res.append(zone)
        return res

    def CreateOptimizationProblem(self, op:Dict):
        name = op["type"]
        del op["type"]
        OnZone, OffZone = self.ReadImplicitGeometryZones(op)
        op["OnZone"] = OnZone
        op["OffZone"] = OffZone
        levelset = op["ls"]
        del op["ls"]
        op["point"] = levelset
        res = CreateProblem(name, op)
        return res

    def _isPrefixPresent(self, prefix:str, string:str):
        return string.startswith(prefix)

    def ReadOptimizationProblems(self, OPs:Dict):
        # import the class to populate the factory
        InitAllProblems()
        InitAllCriteria()
        self.inputReader.ReplaceObjectFromAlmanac(OPs, self)

        for tag, OP in OPs["children"]:
            self.ReadOptimizationProblem(tag, OP)

    def ReadOptimizationProblem(self, tag, OP:Dict):
        if tag.lower().startswith("python"):
            self.RunPythonFunction(tag, OP)
            return

        ID = int(OP["id"])
        del OP["id"]

        father = None

        while ("type" in OP) and len(OP["type"]):
            if self._isPrefixPresent("Penalized", OP["type"]):
                res = OptimProblemPenalized()
                for key in list(OP.keys()):
                    if self._isPrefixPresent("Penal", key):
                        cname = key[len("Penal") :]
                        penal = PH.ReadFloat(OP[key])
                        del OP[key]
                        res.SetPenal(cname, penal)

                OP["type"] = OP["type"][len("Penalized") :]

            elif self._isPrefixPresent("Squared", OP["type"]):
                res = OptimProblemSquared()
                OP["type"] = OP["type"][len("Squared") :]
            else:
                res = self.CreateOptimizationProblem(OP)

            if father is None:
                father = res
                self.optimProblems[ID] = res
            else:
                father.SetInternalOptimProblem(res)
                father = res

    def ReadOptimizationAlgorithms(self, OAs:Dict):
        OptimAlgoFactory.InitAllAlgos()
        for tag, OA in OAs["children"]:
            self.ReadOptimizationAlgorithm(tag, OA)

    def ReadOptimizationAlgorithm(self, tag, OA:Dict):
        if tag.lower().startswith("python"):
            self.RunPythonFunction(tag, OA)
            return

        ID = int(OA["id"])
        del OA["id"]

        self.inputReader.ReplaceObjectFromAlmanac(OA, self)
        self.optimAlgos[ID] = OptimAlgoFactory.Create(tag, ops=OA)

    def ReadPhysicalSolvers(self, PPs:Dict):
        InitAllPhysicalSolvers()
        for tag, PP in PPs["children"]:
            self.ReadPhysicalSolver(tag, PP)

    def ReadPhysicalSolver(self, tag, PP:Dict):
        if tag.lower().startswith("python"):
            self.RunPythonFunction(tag, PP)
            return

        ID = int(PP["id"])
        del PP["id"]
        res = None

        self.inputReader.ReplaceObjectFromAlmanac(PP, self)

        PP["dimensionality"] = self.dimensionality
        res = Create(tag, ops=PP)

        assert res is not None, "Physical problem " + tag + " could not be created."
        self.physicalProblems[ID] = res

    def LoadPlugins(self, plugins):
        for name, plugin in plugins["children"]:
            PluginLoader(name, plugin)

    def ReadZones(self, zones:Dict):
        for tag, zone in zones["children"]:
            self.ReadZone(tag, zone)

    def ReadZone(self, tag:str, zone:Dict):
        if tag.lower().startswith("python"):
            self.RunPythonFunction(tag, zone)
            return

        ID = int(zone["id"])
        del zone["id"]

        assert ID not in self.zones, "Zone :" + str(ID) + " already exist/"

        if "z" in zone:
            z = PH.ReadInts(zone["z"])
            zone["Zones"] = [self.zones[x] for x in z]
            del zone["z"]

        if "ls" in zone:
            lsId = PH.ReadInt(zone["ls"])
            zone["ls"] = self.levelSets[lsId]

        if "support" in zone:
            lsId = PH.ReadInt(zone["support"])
            zone["support"] = self.grids[lsId]

        for zoneName, nameInOps in zip(["z1", "z2"], ["Zone1", "Zone2"]):
            if zoneName in zone:
                z = PH.ReadInt(zone[zoneName])
                zone[nameInOps] = self.zones[z]

                del zone[zoneName]

        if "filename" in zone:
            filename = zone["filename"]

            if not os.path.isabs(PH.ReadString(filename)):
                filename = self.filepath + filename
                zone["filename"] = filename

        # no need to use the inside out this is done inside the internal ImplicitGeometry
        self.zones[ID] = ImplicitGeometryDelayedInit(tag, zone)

    def ReadLevelSets(self, levelsets:Dict):
        from OpenPisco.ConceptionFieldFactory import InitAllConceptionField

        InitAllConceptionField()
        for tag, ls in levelsets["children"]:
            self.ReadLevelSet(tag, ls)

    def ReadLevelSet(self, tag:str, ls:Dict):
        from OpenPisco.ConceptionFieldFactory import Create as CreateConceptionField

        self.inputReader.ReplaceObjectFromAlmanac(ls, self)
        if tag.lower().startswith("python"):
            self.RunPythonFunction(tag, ls)
            return
        ID = int(ls["id"])
        ls["dimensionality"] = self.dimensionality
        self.levelSets[ID] = CreateConceptionField(tag, ops=ls)

    def ReadGrids(self, grids:Dict):
        for tag, grid in grids["children"]:
            self.ReadGrid(tag, grid)

    def ReadGrid(self, tag:str, grid:Dict):
        if tag.lower().startswith("python"):
            self.RunPythonFunction(tag, grid)
        else:
            ID = PH.ReadInt(grid["id"])
            
        if tag == "Grid":
            n = PH.ReadInts(grid["n"])
            if grid["origin"] == "auto":
                bmin = np.ones(self.dimensionality) * 1e100
                zones = PH.ReadInts(grid["zones"])
                for z in zones:
                    bmin = np.minimum(bmin, self.zones[z].GetBoundingMin())
                origin = bmin
            else:
                origin = PH.ReadFloats(grid["origin"])

            if grid["length"] == "auto":
                bmin = np.ones(self.dimensionality) * 1e100
                bmax = np.ones(self.dimensionality) * -1e100
                zones = PH.ReadInts(grid["zones"])
                for z in zones:
                    bmin = np.minimum(bmin, self.zones[z].GetBoundingMin())
                    bmax = np.maximum(bmax, self.zones[z].GetBoundingMax())
                length = bmax - bmin
            else:
                length = PH.ReadFloats(grid["length"])

            # For retrocompatibility, ofTetras should be replaced by ofSimplex
            if "ofTetras" in grid:
                grid["ofSimplex"] = grid.pop("ofTetras")
            ofSimplex = PH.ReadBool(grid.get("ofSimplex", False))

            if len(n) == 3:
                self.grids[ID] = CreateCube(dimensions=n, spacing=length / (n - 1), origin=origin, ofTetras=ofSimplex)
            elif len(n) == 2:
                self.grids[ID] = CreateSquare(dimensions=n, spacing=length / (n - 1), origin=origin, ofTriangles=ofSimplex)
            else:
                raise ValueError("Wrong grid size")
            if ofSimplex:
                self.grids[ID].props = {}

            print(self.grids[ID])

        elif tag == "Mesh":
            filename = PH.ReadString(grid["filename"])
            if not os.path.isabs(filename):
                filename = self.filepath + filename
                
            if "timeToRead" in grid:
                timeToRead = PH.ReadInt(grid["timeToRead"])
                res = ReadMesh(filename,timeToRead=timeToRead)
            else:
                res = ReadMesh(filename)
            print("Reading file : {}".format(filename))
            self.grids[ID] = QuadToLin(res)
            self.grids[ID].originalIDNodes = res.originalIDNodes[self.grids[ID].originalIDNodes]
            self.grids[ID].ConvertDataForNativeTreatment()
            if len(res.elemFields.keys()) > 0:
                self.grids[ID].elemFields = res.elemFields
            if len(res.nodeFields.keys()) > 0:
                self.grids[ID].nodeFields = res.nodeFields

    def Start(self):
        from OpenPisco.Actions.ActionBase import ActionBase

        # to reset the Need To Interrupt
        ActionBase.NeedToInterrupt()

        if "children" in self.actions:
            for _, child in self.actions["children"]:
                if ActionBase.NeedToInterrupt():
                    print("Run actions interrupted")
                    break
                res = self.RunAction(child)
                if res is None:
                    print("Please correct action " + str(child) + " to return a 0/1")

                if res != RETURN_SUCCESS:
                    print("Run Time error")
                    break
            else:
                print("Done")

    def RunAction(self, child:Dict):
        if self.beforeActionCallBack is not None:
            self.beforeActionCallBack()
        print("Running action : " + child["type"])

        from OpenPisco.Actions.ActionFactory import Create, InitAllActions

        InitAllActions()

        try:
            obj = Create(child["type"])
        except BaseException as e:
            print("unable to find object in the factory. Using old actions")
            print("Active actions are : ")
            from OpenPisco.Actions.ActionFactory import ActionFactory

            print(str(ActionFactory.keys()))
            raise e
        else:
            return obj.Apply(self, child)

    def CloseAllOutputs(self):
        for o in self.outputs:
            Logger.Debug(o)
            self.outputs[o].Close()

    def __str__(self):
        res = "MainApp:\n"
        res += " Dimensionality " + str(self.dimensionality) + "\n"
        res += " Number of Grids : " + str(len(self.grids)) + " \n"
        for g in self.grids:
            res += "(ID=" + str(g) + ") " + str(self.grids[g])
        res += " Number of Zones : " + str(len(self.zones)) + "\n"
        for z in self.zones:
            res += "(ID=" + str(z) + ") " + str(self.zones[z])
        return res


def CheckIntegrity():
    myApp2 = MainApp()
    from OpenPisco.TestData import GetTestDataPath

    path = GetTestDataPath()
    myApp2.ReadFile(path + "TestInput.xml")
    print(myApp2)
    return "ok"


def RunSimulation(filename=None):
    if filename is None:
        if len(sys.argv) > 1:
            filename = sys.argv[1]
        else:
            print("Need a file ")

    with PB() as f:
        try:
            f.CopyOutputToDisk(filename=filename + ".log", filenameCerr=filename + ".cerr.log")
            with open(filename) as f:
                print(f"Input Filename : {filename}")
                lines = f.readlines()
                assert len(lines), f"Empty Filename ({filename})"
                paddinSize = int(log10(len(lines))) + 1
                print("\n" + "".join([TFormat.Left(f"{i+1}", fill=" ", width=paddinSize) + l for i, l in enumerate(lines)]))

            myApp = MainApp()
            myApp.ReadFile(filename)
            myApp.Start()
        except Exception:
            tb = traceback.format_exc()
            print(tb)


if __name__ == "__main__":
    from Muscat.Helpers.Timer import Timer

    a = Timer("RunSimulation")

    if len(sys.argv) > 1:
        a.Start()
        RunSimulation(sys.argv[1])
        a.Stop()
        print(a.PrintTimes())
        sys.exit()
    else:
        print(CheckIntegrity())
