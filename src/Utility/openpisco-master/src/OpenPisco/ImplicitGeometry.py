# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import numpy as np

from Muscat.Types import  MuscatFloat

from Muscat.ImplicitGeometry.ImplicitGeometryBase import ImplicitGeometryBase
from Muscat.ImplicitGeometry.ImplicitGeometryFactory import RegisterClass
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter

class ImplicitGeometryLevelset(ImplicitGeometryBase):
    """ImplicitGeometry Wrapper around a levelset
        ls : a levelset
    """

    def __init__(self, ls = None ):
        super().__init__()
        self.SetLevelset(ls)

    def __str__(self):
        res = "ImplicitGeometryLevelset \n"
        res += "  ls: " + str(self.ls)+"\n"
        return res

    def SetLevelset(self, levelset):
        self.ls = levelset
        if self.ls is None:
            return
        from Muscat.MeshTools.TransferBackEnds.NativeTransfer import NativeTransfer
        self.nt = NativeTransfer()
        self.nt.SetVerbose(False)
        self.nt.SetTransferMethod("Interp/Extrap")
        from Muscat.FE.FETools import PrepareFEComputation
        space, numberings, _, _ = PrepareFEComputation(mesh=levelset.support)
        from Muscat.FE.Fields.FEField import FEField
        inputFEField = FEField(name="", mesh=levelset.support, space=space, numbering=numberings[0])
        self.nt.SetSourceFEField(inputFEField, ElementFilter(dimensionality=levelset.support.GetElementsDimensionality() ))

    def ApplyVector(self, support, cellCenter=False):
        if support is self.ls.support and cellCenter == False:
           return self.ApplyInsideOut(self.ls.phi)
        return super().ApplyVector(support, cellCenter= cellCenter)

    def GetDistanceToPoint(self,_pos):
        if len(_pos.shape) == 1:
            pos = np.array([_pos], dtype=pos.dtype)
        else:
            pos = _pos

        self.nt.SetTargetPoints(pos)
        self.nt.Compute()
        op = self.nt.GetOperator()
        res =  op.dot(self.ls.phi)

        return self.ApplyInsideOut(res)

RegisterClass("FromLevelset", ImplicitGeometryLevelset)

# For backward compatibility
RegisterClass("ls", ImplicitGeometryLevelset)

#########################operators ##########################

def CheckIntegrity(GUI=False):

    from Muscat.MeshTools.ConstantRectilinearMeshTools import CreateConstantRectilinearMesh
    from Muscat.MeshTools.MeshCreationTools import CreateCube

    myMesh = CreateConstantRectilinearMesh(dimensions=[30,30,30],spacing=[2/29,]*3,origin=[-1, -1, -1])
    myMesh2 = CreateCube(dimensions=[31,32,33],spacing=[2/29,]*3,origin=[-1, -1, -1],ofTetras=True)

    import OpenPisco.Structured.Levelset3D as Levelset3D

    ls = Levelset3D.LevelSet3D(support=myMesh)

    TwoPoints = np.array([[0.,0.,0.],[1.,2.,3.]], dtype=MuscatFloat)

    myObj9 = ImplicitGeometryLevelset(ls)
    myObj9(TwoPoints)
    myObj9(myMesh2)
    return 'ok'

if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity(GUI=False))
