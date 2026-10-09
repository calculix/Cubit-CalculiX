# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import os

import numpy as np
from copy import deepcopy

from Muscat.Helpers import CPU
from Muscat.Types import MuscatIndex, MuscatFloat
import Muscat.MeshTools.MeshModificationTools as UMMT
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.Helpers.IO.Which import Which

from OpenPisco.ExternalTools.AppExecutableTools import AppExecutableBase, CommandLineBuilder
import OpenPisco.TopoZones as TZ
from OpenPisco.BaseElements import elementsTypeSupportedForDimension
from Muscat.Helpers.Logger import Debug

mshdistExec = "mshdist"
meshExtension = ".mesh"

MSHDIST_AVAILABLE = Which(mshdistExec)

class Meshdist(AppExecutableBase):
    def __init__(self):
        super(Meshdist, self).__init__(mshdistExec)
        self.originalSupport = None
        self.support = None
        self.surfacesupport = None
        self.baseFilename = "box"
        self.keepGeneratedFiles = False
        self.SetBinaryFlag(True)
        self.reader.SetBinary(False)

    def __del__(self):
        self.DeleteGeneratedFiles([self.baseFilename+"mesh"])

    def DeleteGeneratedFiles(self, filesToDelete=None):
        if filesToDelete is None:
            filesToDelete = []
            filesToDelete.extend(  self.baseFilename+ext for ext in  [ '.sol','_contour.mesh', '.solb.sol'])
        self.DeleteFiles(filesToDelete=filesToDelete)

    def SetMesh(self, mesh):
        self.SetSupport(mesh)
        self.WriteSupport()

    def SetSupport(self, _support):
        self.originalSupport = _support
        from Muscat.MeshTools.MeshInspectionTools import  ExtractElementsByElementFilter
        from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
        dimensionality = _support.GetElementsDimensionality()
        if dimensionality==3:
            self.support = ExtractElementsByElementFilter(_support, ElementFilter(dimensionality=dimensionality) )
        elif dimensionality==2:
            self.support=_support

    def SetSurfaceSupport(self, _support):
        self.surfacesupport = _support

    def WriteSupport(self,writeInsideZone=False):
        self.DeleteGeneratedFiles()

        baseFileName = self._fileName(self.baseFilename)
        import tempfile
        fp = tempfile.mkstemp(suffix=baseFileName, prefix=None, dir=self.GetWorkingDirectory(), text=True)
        self.baseFilename = fp[1].split(os.sep)[-1]
        self.writer.Open(self._filePath(self.baseFilename + meshExtension))

        dimensionality = self.support.GetElementsDimensionality()
        elemRefNumber = np.zeros(self.support.GetNumberOfElements(), dtype=MuscatIndex)
        # we need to label the inside tetras/trias with 3 only when computing distance to a subdomain
        elementType=elementsTypeSupportedForDimension[dimensionality]
        if writeInsideZone and TZ.Inside3D in self.support.GetElementsOfType(elementType).tags :
           elements = self.support.GetElementsInTag(TZ.Inside3D)
           elemRefNumber[elements] = TZ.MG_MINUS
        self.writer.Write(self.support, elemRefNumber=elemRefNumber)
        self.writer.Close()

    def SetPhi(self, levelset):
        self.WriteLevelSet(levelset)

    def WriteLevelSet(self,levelset):
        phiOnSupport = levelset[self.support.originalIDNodes]
        self.writer.SetFileName(self._filePath(self.baseFilename +meshExtension))
        self.writer.OpenSolutionFile(self.support)
        self.writer.WriteSolutionsFields(self.support, [phiOnSupport])
        self.writer.Close()

    def WriteSurfaceSupport(self):
        self.writer.Open(self._filePath(self.baseFilename+"_contour.mesh"))
        self.writer.Write(self.surfacesupport)
        self.writer.Close()

    def GenerateDist(self, ls=True, it=None):
        cmdBuilder = CommandLineBuilder(self.GetAppName())
        cmdBuilder.argumentAdd(self._fileName(self.baseFilename+meshExtension))
        if it is not None:
            cmdBuilder.optionAdd("it", arg=str(it))

        if not ls:
            cmdBuilder.argumentAdd(self._fileName(self.baseFilename+"_contour.mesh"))
            cmdBuilder.optionAdd("noscale")

        with CPU.ComputingCores() as cpus:
            cmdBuilder.optionAdd("ncpu", arg=str(cpus.nbCoresAllocated))
            cmdBuilder.optionAdd("fmm")

            cmd = cmdBuilder.result()
            self._executeCommand(cmd)

    def GenerateDistSubDomain(self):
        cmdBuilder = CommandLineBuilder(self.GetAppName())
        cmdBuilder.argumentAdd(self._fileName(self.baseFilename+meshExtension))
        cmdBuilder.optionAdd("dom")
        cmd = cmdBuilder.result()
        self._executeCommand(cmd)

    def GetNewPhi(self):
        solution = self.reader.ReadExtraFields(self._filePath(self.baseFilename+".sol",binary=False))
        if not self.keepGeneratedFiles:
            self.DeleteGeneratedFiles()
        return np.asarray(solution["SolAtVertices0"].ravel(), dtype=MuscatFloat)


def MeshDistUpdatePhi(levelset, it=None):
    obj = Meshdist()
    obj.SetSupport(levelset.support)
    obj.WriteSupport()
    phitmp = levelset.phi
    bmin = np.amin(phitmp)
    bmax = np.amax(phitmp)
    obj.WriteLevelSet(phitmp)
    obj.GenerateDist(ls=True, it=it)
    phi = obj.GetNewPhi()
    amin = np.amin(phi)
    amax = np.amax(phi)
    Debug("min max before " + str([bmin,bmax]))
    Debug("min max after " + str([amin,amax]))
    levelset.phi.ravel()[:] = phi.ravel()

import Muscat.ImplicitGeometry.ImplicitGeometryObjects as ImplicitGeometry
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
from Muscat.TestData import GetTestDataPath
from Muscat.IO.StlReader import ReadStl
from Muscat.IO.XdmfWriter import WriteMeshToXdmf
import OpenPisco.Unstructured.Levelset as LS
from Muscat.MeshTools.MeshCreationTools import CreateSquare,CreateCube
from Muscat.IO.XdmfWriter import WriteMeshToXdmf


def CheckIntegrity_mshdist():
    mesh = CreateCube(dimensions=[21, 21, 21],spacing=[0.07, 0.07, 0.07],origin=[-0.7, -0.7, -0.7],ofTetras=True)
    surf = ReadStl(GetTestDataPath() + "stlsphere.stl")
    UMMT.CleanDoubleNodes(surf)

    obj = Meshdist()
    obj.SetSupport(mesh)
    obj.SetSurfaceSupport(surf)
    obj.WriteSupport()
    obj.WriteSurfaceSupport()
    obj.GenerateDist(ls=False, it=10)
    phi = obj.GetNewPhi()

    WriteMeshToXdmf(TemporaryDirectory.GetTempPath() + "TestMeshdist1.xdmf", mesh, PointFields=[phi], PointFieldsNames=["phi"])

    mask = (np.linalg.norm(mesh.nodes,axis=1) - 0.5) < 0
    phi[mask] *= -1

    obj.WriteSupport()
    obj.WriteLevelSet(phi)
    obj.GenerateDist(ls=True, it=10)
    phi = obj.GetNewPhi()

    WriteMeshToXdmf(TemporaryDirectory.GetTempPath() + "TestMeshdist2.xdmf", mesh, PointFields=[phi],PointFieldsNames=["phi"])

    ls = LS.LevelSet()
    ls.SetSupport(mesh)
    ls.phi = phi
    MeshDistUpdatePhi(ls, it=10)

    WriteMeshToXdmf(TemporaryDirectory.GetTempPath() + "TestMeshdist3.xdmf", mesh, PointFields=[ls.phi], PointFieldsNames=["phi"])
    return "ok"

def CheckIntegrity_GenerateDistToSubdomain():
    UM = CreateCube(dimensions=[5,5,5],spacing=[1./2,1./2,1./2],origin=[0, 0, 0],ofTetras=True)
    newnodes = np.empty((UM.GetNumberOfNodes()+1,3),dtype=MuscatFloat)
    newnodes[0:UM.GetNumberOfNodes(),:] = UM.nodes
    newnodes[-1,:] = [-1,-1,-1]
    UM.nodes = newnodes
    tetra = UM.GetElementsOfType(ED.Tetrahedron_4)
    X = UM.nodes[tetra.connectivity,0]
    left = np.nonzero( np.all(X<=1.,axis=1) )[0]
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(left)
    obj = Meshdist()
    obj.keepGeneratedFiles = True
    obj.SetSupport(UM)
    obj.WriteSupport(writeInsideZone=True)
    obj.GenerateDistSubDomain()
    obj.GetNewPhi()
    return "ok"

def CheckIntegrity_meshdist2d():
    mesh = CreateSquare(origin=[0.,0.],dimensions=[40,40],spacing=[1./39,1./39],ofTriangles=True)
    from OpenPisco.Unstructured.Levelset import LevelSet
    levelset = LevelSet(support=mesh)

    levelset.phi = ImplicitGeometry.ImplicitGeometrySphere(center=[.5,.5,0.],radius=.25).ApplyVector(mesh)
    phi_before_redistancing = deepcopy(levelset.phi)
    MeshDistUpdatePhi(levelset, it=10)
    phi_after_redistancing = deepcopy(levelset.phi)

    levelset.phi = levelset.support.nodes[:,0] - 0.5
    elems = levelset.support.GetElementsOfType(ED.Triangle_3)
    vals = levelset.phi[elems.connectivity]
    insideids = np.nonzero(np.all(vals<=0,axis=1))[0]
    elems.tags.CreateTag(TZ.Inside3D,False).SetIds(insideids)
    obj = Meshdist()
    obj.keepGeneratedFiles = True
    obj.SetSupport(levelset.support)
    obj.WriteSupport(writeInsideZone=True)
    obj.GenerateDistSubDomain()
    phi = obj.GetNewPhi()
    np.testing.assert_almost_equal(phi,levelset.phi,decimal=3)
    WriteMeshToXdmf(TemporaryDirectory.GetTempPath() + "TestMeshdist2d.xdmf",
                    levelset.support,
                    PointFields=[phi_before_redistancing,phi_after_redistancing,levelset.phi],
                    PointFieldsNames=["phi","phiRedistanciated","phiConform"])
    return "ok"

def CheckIntegrity(GUI=False):
    if not Which(mshdistExec):
        return "Not Ok, mshdist not found!!"
    totest = [
    CheckIntegrity_meshdist2d,
    CheckIntegrity_mshdist,
    CheckIntegrity_GenerateDistToSubdomain,
              ]

    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res

    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
