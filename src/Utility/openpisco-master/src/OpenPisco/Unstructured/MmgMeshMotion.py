# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import numpy as np

from Muscat.Types import MuscatIndex

import Muscat.MeshContainers.ElementsDescription as ED
import Muscat.MeshTools.MeshInspectionTools as UMIT
import Muscat.IO.MeshTools as MT

from OpenPisco.ExternalTools.AppExecutableTools import AppExecutableBase, CommandLineBuilder
from OpenPisco.Unstructured.MmgMesher import MmgAvailable, mmg3dExec, ComputeDistanceField
import OpenPisco.TopoZones as TZ

meshExtension = ".mesh"

def MmgMeshMotionActionMoveMesh(support,vectorfield):
    assert len(vectorfield.shape) == 2 and vectorfield.shape[1] == 3,"Field argument does not have the correct shape"
    obj = MmgMeshMotion()
    obj.SetSupport(support)
    obj.WriteSupport()
    obj.WriteVectorField(vectorfield)
    obj.GenerateMesh()
    return obj.GetNewMesh()

def MmgMeshMotionActionMoveConformalLevelSet(levelset,vectorfield):
    assert levelset.conform,"Need a conformal level set to use this routine "
    levelset.support = MmgMeshMotionActionMoveMesh(levelset.support,vectorfield)
    surf = UMIT.ExtractElementsByTags(levelset.support, [TZ.InterSurf])
    phi = ComputeDistanceField(levelset.support,surf)
    levelset.TakePhiAndMesh(phi, levelset.support)

class MmgMeshMotion(AppExecutableBase):
    """
    File-exchange-based interface to the Mmg mesh motion tool (no remeshing).
    """
    def __init__(self):
        super(MmgMeshMotion, self).__init__(mmg3dExec)
        self.SetBinaryFlag(False)
        self.support = None
        self.baseFilename = "MmgMeshMotion"
        self.sTag = TZ.InterSurf # surface tag for the triangulation to move
        self.keepGeneratedFiles = False
        self.has_rn_option = None

    def DeleteGeneratedFiles(self):
        filesToDelete = []
        filesToDelete.extend(  self.baseFilename+ext for ext in  ['.mesh', '.sol'])
        filesToDelete.extend(  self.baseFilename+ext for ext in  ['.o.mesh', '.o.sol'])
        self.DeleteFiles(filesToDelete=filesToDelete)

    def SetSupport(self, _support):
        self.support = _support

    def WriteSupport(self):
        if not self.keepGeneratedFiles:
            self.DeleteGeneratedFiles()

        if ED.Point_1 in self.support.elements.keys():
            tag = self.support.nodesTags.CreateTag("INTERNAL_0D_ELEMENTS")
            ids = self.support.elements[ED.Point_1].connectivity.flatten()
            tag.SetIds(ids)
            if MT.RequiredVertices not in self.support.nodesTags:
                self.support.nodesTags.CreateTag(MT.RequiredVertices)
            rvtag = self.support.nodesTags[MT.RequiredVertices]
            rvtag.AddToTag(ids)


        self.writer.SetFileName(self._filePath(self.baseFilename +meshExtension))
        self.writer.Open(self.writer.fileName)

        elemRefNumber = np.zeros(self.support.GetNumberOfElements(), dtype=MuscatIndex)

        # we label with the reference 10 the interface which will drive the displacement

        errorMessage = "The given support does not contain the suface tag "+str(self.sTag)+". Please provide one "
        assert self.sTag in self.support.GetElementsOfType(ED.Triangle_3).tags,errorMessage
        elements = self.support.GetElementsInTag(self.sTag)
        assert len(elements)> 0,"Surface tag "+str(self.sTag)+" is empty. Don't have an interface to move "
        elemRefNumber[elements] = TZ.MMG_ISO
        self.writer.Write(self.support, elemRefNumber=elemRefNumber)
        self.writer.Close()

    def WriteVectorField( self,vectorfield):
        self.writer.SetFileName(self._filePath(self.baseFilename +meshExtension))
        self.writer.OpenSolutionFile(self.support)
        self.writer.WriteSolutionsFields(self.support, [vectorfield])
        self.writer.Close()

    def GenerateMesh(self):
        if self.has_rn_option == None:
            cmdBuilder = CommandLineBuilder(mmg3dExec)
            cmdBuilder.optionAdd("h")
            try:
                import subprocess
                out = self._executeSubprocess(cmdBuilder.result())
            except subprocess.CalledProcessError  as e:
                out = e.output.decode("utf-8",errors="ignore")
            else:
                out = ""
            if "-rn" in out:
                self.has_rn_option = True
            else:
                self.has_rn_option = False

        cmdBuilder = CommandLineBuilder(mmg3dExec)
        cmdBuilder.optionAdd("lag", arg=str((0)))
        if self.has_rn_option:
            cmdBuilder.optionAdd("rn", arg="0")
        cmdBuilder.optionAdd("opnbdy")
        cmdBuilder.argumentAdd(self.baseFilename+meshExtension)
        cmd = cmdBuilder.result()
        self._executeCommand(cmd)

    def GetNewMesh(self):
        import copy
        self.reader.SetFileName( \
                self._filePath(self.baseFilename+".o.mesh", binary=False))
        res = self.reader.Read(out=type(self.support)())

        errorMessage = "The number of nodes has changed during mesh motion. Unable to continue"
        assert res.GetNumberOfNodes() == self.support.GetNumberOfNodes(),errorMessage
        outputMesh = copy.copy(self.support)
        outputMesh.nodes = res.nodes

        if "INTERNAL_0D_ELEMENTS" in self.support.nodesTags:
            ids = self.support.nodesTags["INTERNAL_0D_ELEMENTS"].GetIds()
            nodalIds = np.setdiff1d(self.support.nodesTags["RequiredVertices"].GetIds(),\
                                    ids)
            self.support.nodesTags.DeleteTags(["INTERNAL_0D_ELEMENTS"])
            self.support.nodesTags.DeleteTags(["RequiredVertices"])
            if len(nodalIds):
               self.support.nodesTags.CreateTag("RequiredVertices").SetIds(nodalIds)

        return outputMesh

def CheckIntegrity():
    if not MmgAvailable:
        return "Not Ok, mmg3d_O3 not found!!"
    import subprocess
    output = subprocess.run([mmg3dExec,"-h"], timeout =30, capture_output=True).stdout.decode("utf-8",errors="ignore")

    if "-lag [n]" not in output:
        return "skip! mmg lagrangian mesh displacement not available !!!"

    from Muscat.MeshTools.MeshCreationTools import CreateCube
    from OpenPisco.Unstructured.Levelset import LevelSet

    length = np.array([1,1,1])
    dimensions = np.array([25, 25, 25])
    spacing = length/(dimensions-1)
    UM = CreateCube(dimensions=dimensions,spacing=spacing,origin=[0., 0., 0.],ofTetras=True)
    ls = LevelSet(support=UM)
    import Muscat.ImplicitGeometry.ImplicitGeometryObjects as ImplicitGeometry
    ls.phi = ImplicitGeometry.ImplicitGeometrySphere(center=[.5,.5,.5],radius=.3).ApplyVector(UM)
    ls.conform = True
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.01,"hmax":0.7,"nr":True}
    import  OpenPisco.Unstructured.MmgMesher as MmgMesher
    MmgMesher.MmgMesherActionLevelset(ls,opts)

    from OpenPisco.Unstructured.LevelsetTools import ComputeGradientOnTetrahedrons
    field = np.ones_like(ls.phi)
    data = ComputeGradientOnTetrahedrons(ls.support,ls.phi)
    data *= field[:,np.newaxis]

    from Muscat.IO.XdmfWriter import XdmfWriter
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    writer = XdmfWriter(TemporaryDirectory.GetTempPath()+ 'test_MmgMeshMotion' + '.xmf')
    writer.SetTemporal()
    writer.SetBinary(True)
    writer.SetXmlSizeLimit(0)
    writer.Open()
    writer.Write(ls.support,TimeStep = 1)
    MmgMeshMotionActionMoveConformalLevelSet(ls, data)
    writer.Write(ls.support, TimeStep = 1)
    writer.Close()
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
