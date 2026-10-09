# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

from __future__ import division
import numpy as np

from OpenPisco.Structured.Laplacian3D import ConstantMassMatrix
from OpenPisco.Structured.StructuredLevelsetBase import StructuredLevelsetBase
from OpenPisco.ConceptionFieldFactory import RegisterClass

try:
    from numba import jit
except ImportError:
    print("numba not installed, things are going to be very slow")
    def jit(nopython=None):
        return lambda x: x

def CreateLevelSet3D(ops):
    """ 3D structured Levelset
        support : Grid (mesh)
    """
    assert "support" in ops,"Need a support to init a LevelSet2D object"
    ls = LevelSet3D()
    ls.SetSupport(ops["support"])
    return ls

class LevelSet3D(StructuredLevelsetBase):
    def __init__(self, other=None, support=None):
        super().__init__(other=other,support=support)
        self.__massMatrix__ = None

        assert (self.support is None or self.support.props.get("IsConstantRectilinear",False)),"Need a ConstantRectilinear mesh"

    def GetMassMatrix(self):
        if self.__massMatrix__ is None:
            self.__massMatrix__ =  ConstantMassMatrix(self.support)

        return self.__massMatrix__

    def GetVolumeFractionOperator(self):
        import OpenPisco.Structured.Geometry3D as Geometry3D
        return Geometry3D.cuboid_volumic_fractions

    def RegularizationOperator(self):
        from OpenPisco.Structured.Laplacian3D import RegularizationOperator
        return RegularizationOperator

    def LevelsetUpdate(self, velocity_field, phi, extension_func, support):
        ext_phi = extension_func(phi,support)

        diff = Order1DirectionalDifferences(ext_phi[1:-1, 1:-1,1:-1])
        diff2 = Order2DirectionalDifferences(ext_phi)

        diff[:, 0] += (Minmod(diff2[:, 0], diff2[:, 2])) / 2.0
        diff[:, 1] -= (Minmod(diff2[:, 1], diff2[:, 2])) / 2.0

        dxyz = support.props.get("spacing")
        diff[0, :] *= 1./dxyz[0]
        diff[1, :] *= 1./dxyz[1]
        diff[2, :] *= 1./dxyz[2]
        flux_p = NumericalFlux(diff[0, 0], diff[0, 1], diff[1, 0], diff[1, 1],diff[2, 0], diff[2, 1])
        flux_m = NumericalFlux(diff[0, 1], diff[0, 0], diff[1, 1], diff[1, 0],diff[2, 1], diff[2, 0])
        velocity_field_p = np.maximum(velocity_field, 0.0)
        velocity_field_m = np.minimum(velocity_field, 0.0)
        return velocity_field_p.flatten() * flux_p.flatten() + velocity_field_m.flatten() * flux_m.flatten()


    def NeumannOrder2Extension(self,field,support):
        field2 = field.view()
        field2.shape = tuple(x for x in support.props.get("dimensions"))
        ext_field = np.zeros(tuple(n + 4 for n in field2.shape))

        np.copyto(ext_field[2:-2, 2:-2,2:-2], field2)
        np.copyto(ext_field[  :2, 2:-2, 2:-2], field2[0:1, :,:])
        np.copyto(ext_field[-2: , 2:-2, 2:-2], field2[-1:, :,:])

        np.copyto(ext_field[2:-2,  :2, 2:-2], field2[:, 0:1,:])
        np.copyto(ext_field[2:-2,-2: , 2:-2], field2[:, -1:,:])

        np.copyto(ext_field[2:-2, 2:-2,   :2 ], field2[:, :, 0:1])
        np.copyto(ext_field[2:-2, 2:-2, -2:  ], field2[:, :,-1:])
        #
        return ext_field

    def GradientNeumannOrder2Extension(self,field,support):

        field2 = field.view()
        field2.shape = tuple(x for x in support.props.get("dimensions"))
        ext_field = np.zeros(tuple(n + 4 for n in field2.shape))

        np.copyto(ext_field[2:-2, 2:-2,2:-2], field2)

        np.copyto(ext_field[0   , 2:-2,2:-2], 3 * field2[ 0, :, :] - 2 * field2[ 1, :, :])
        np.copyto(ext_field[1   , 2:-2,2:-2], 2 * field2[ 0, :, :] -     field2[ 1, :, :])
        np.copyto(ext_field[-2  , 2:-2,2:-2], 2 * field2[-1, :, :] -     field2[-2, :, :])
        np.copyto(ext_field[-1  , 2:-2,2:-2], 3 * field2[-1, :, :] - 2 * field2[-2, :, :])

        np.copyto(ext_field[2:-2, 0   ,2:-2], 3 * field2[ :, 0, :] - 2 * field2[:, 1, :])
        np.copyto(ext_field[2:-2, 1   ,2:-2], 2 * field2[ :, 0, :] -     field2[:, 1, :])
        np.copyto(ext_field[2:-2, -2  ,2:-2], 2 * field2[ :,-1, :] -     field2[:,-2, :])
        np.copyto(ext_field[2:-2, -1  ,2:-2], 3 * field2[ :,-1, :] - 2 * field2[:,-2, :])

        np.copyto(ext_field[2:-2,2:-2, 0   ], 3 * field2[ :, :, 0] - 2 * field2[:, :, 1])
        np.copyto(ext_field[2:-2,2:-2, 1   ], 2 * field2[ :, :, 0] -     field2[:, :, 1])
        np.copyto(ext_field[2:-2,2:-2, -2  ], 2 * field2[ :, :,-1] -     field2[:, :,-2])
        np.copyto(ext_field[2:-2,2:-2, -1  ], 3 * field2[ :, :,-1] - 2 * field2[:, :,-2])


        return ext_field


RegisterClass("LevelSet3DS",LevelSet3D,CreateLevelSet3D )

def Order1CentralDifferences(ext_field):
    result = np.empty((3, 1, ext_field.shape[0] - 2, ext_field.shape[1] - 2, ext_field.shape[2] - 2))
    result[0, 0] = ext_field[2:  , 1:-1,1:-1] - ext_field[0:-2, 1:-1,1:-1]
    result[1, 0] = ext_field[1:-1, 2:  ,1:-1] - ext_field[1:-1, 0:-2,1:-1]
    result[2, 0] = ext_field[1:-1, 1:-1,2:  ] - ext_field[1:-1, 1:-1,0:-2]
    return result

#OK
def Order1DirectionalDifferences(ext_field):
    result = np.empty((3, 2, ext_field.shape[0] - 2, ext_field.shape[1] - 2, ext_field.shape[2] - 2))
    #the backward
    result[0, 0] = ext_field[1:-1, 1:-1, 1:-1] - ext_field[0:-2, 1:-1, 1:-1]
    #the forward
    result[0, 1] = ext_field[2:  , 1:-1, 1:-1] - ext_field[1:-1, 1:-1, 1:-1]

    result[1, 0] = ext_field[1:-1, 1:-1, 1:-1] - ext_field[1:-1, 0:-2, 1:-1]
    result[1, 1] = ext_field[1:-1, 2:  , 1:-1] - ext_field[1:-1, 1:-1, 1:-1]

    result[2, 0] = ext_field[1:-1, 1:-1, 1:-1] - ext_field[1:-1, 1:-1, 0:-2]
    result[2, 1] = ext_field[1:-1, 1:-1, 2:  ] - ext_field[1:-1, 1:-1, 1:-1]
    return result


#OK
@jit(nopython=True)
def Order2DirectionalDifferences(ext_field):
    result = np.empty((3, 3, ext_field.shape[0] - 4, ext_field.shape[1] - 4, ext_field.shape[2] - 4))
    ## center about 1
    result[0, 0] = ext_field[2:-2, 2:-2, 2:-2] - 2 * ext_field[1:-3, 2:-2, 2:-2] + ext_field[0:-4, 2:-2, 2:-2]
    ## center about 3
    result[0, 1] = ext_field[4:  , 2:-2, 2:-2] - 2 * ext_field[3:-1, 2:-2, 2:-2] + ext_field[2:-2, 2:-2, 2:-2]
    ## center about 2
    result[0, 2] = ext_field[3:-1, 2:-2, 2:-2] - 2 * ext_field[2:-2, 2:-2, 2:-2] + ext_field[1:-3, 2:-2, 2:-2]

    result[1, 0] = ext_field[2:-2, 2:-2, 2:-2] - 2 * ext_field[2:-2, 1:-3, 2:-2] + ext_field[2:-2, 0:-4, 2:-2]
    result[1, 1] = ext_field[2:-2, 4:  , 2:-2] - 2 * ext_field[2:-2, 3:-1, 2:-2] + ext_field[2:-2, 2:-2, 2:-2]
    result[1, 2] = ext_field[2:-2, 3:-1, 2:-2] - 2 * ext_field[2:-2, 2:-2, 2:-2] + ext_field[2:-2, 1:-3, 2:-2]

    result[2, 0] = ext_field[2:-2, 2:-2, 2:-2] - 2 * ext_field[2:-2, 2:-2, 1:-3] + ext_field[2:-2, 2:-2, 0:-4]
    result[2, 1] = ext_field[2:-2, 2:-2, 4:  ] - 2 * ext_field[2:-2, 2:-2, 3:-1] + ext_field[2:-2, 2:-2, 2:-2]
    result[2, 2] = ext_field[2:-2, 2:-2, 3:-1] - 2 * ext_field[2:-2, 2:-2, 2:-2] + ext_field[2:-2, 2:-2, 1:-3]

    return result


@jit(nopython=True)
def Minmod(a, b):
    res = np.empty_like(a)
    M, N,O,P  = a.shape
    for i in range(M):
        for j in range(N):
            for k in range(O):
                for l in range(P):
                    va = a[i,j,k,l]
                    vb = b[i,j,k,l]
                    sa =np.sign(va)
                    res[i,j,k,l] = max(0.0, sa * np.sign(vb) )* sa * min(abs(va), abs(vb))
    return res

def NumericalFlux1(u1, u2, v1, v2, w1, w2):
    # need dx, dy
    return np.sqrt(u1**2 + u2**2 + \
                   v1**2 + v2**2 + \
                   w1**2 + w2**2)

@jit(nopython=True)
def NumericalFlux(u1, u2, v1, v2, w1, w2):
    # need dx, dy
    return np.sqrt(np.maximum(u1, 0.0)**2 + np.minimum(u2, 0.0)**2 + \
                   np.maximum(v1, 0.0)**2 + np.minimum(v2, 0.0)**2 + \
                   np.maximum(w1, 0.0)**2 + np.minimum(w2, 0.0)**2)


################## test functions ###########################################

def CheckIntegrity_SetUpAndReinitialize(writer,myMesh,ls,function):

    ls.InitializePointByPoint(function)

    writer.Write(myMesh,
                PointFields     = [ls.phi],
                PointFieldsNames= ['phi'],
                TimeStep = 1
                )
    for _ in range(10):
         ls.Reinitialize(0.25)
         writer.Write(myMesh,
                PointFields     = [ls.phi],
                PointFieldsNames= ['phi'],
                TimeStep = 1
                )

def CheckIntegrity():
    from Muscat.MeshTools.ConstantRectilinearMeshTools import CreateConstantRectilinearMesh
    from Muscat.IO.XdmfWriter import XdmfWriter
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory

    import math
    n = np.array([21,22,23],dtype=int)*3
    myMesh = CreateConstantRectilinearMesh(dimensions=n,spacing=20./(n-1), origin=[-10.,-10.,-10])

    ls = LevelSet3D(support=myMesh)
    print(ls)
    print("A) ls.phi       " + str(type(ls.phi)))

    ls.Initialize(lambda XYZs : XYZs[:,0] )

    writer = XdmfWriter(TemporaryDirectory.GetTempPath()+'LevelSetTest3D.xmf')
    writer.SetTemporal()
    writer.SetBinary()
    writer.SetXmlSizeLimit(0)
    writer.Open()
    print("B) ls.phi       " + str(type(ls.phi)))
    fieldToTreat = ls.phi*0+1
    print("C) fieldToTreat " + str(type(fieldToTreat)))
    ir= ls.InterfaceRestriction(fieldToTreat)
    print("D) ir           " + str(type(ir)))
    r1 = ls.Regularize(fieldToTreat,lengthscaleParameter=0.1)
    print("E) r1           " + str(type(r1)))

    r2 =  ls.Regularize(fieldToTreat,lengthscaleParameter = 1,extra = ls.phi)
    print("F) r2           " + str(type(r2)))

    phi = ls.phi

    writer.Write(myMesh,
                PointFields     = [phi,ir,r1,r2],
                PointFieldsNames= ['phi','ir',"r1",'r2'],
                TimeStep = 1
                )

    ls.TransportOnce(ir, cfl=0.5)

    factor = 10
    def f(xyz):
        return xyz[0]

    CheckIntegrity_SetUpAndReinitialize(writer,myMesh,ls,f )
    CheckIntegrity_SetUpAndReinitialize(writer,myMesh,ls,lambda xyz:  factor*(xyz[0]-0.001) )
    CheckIntegrity_SetUpAndReinitialize(writer,myMesh,ls,lambda xyz:  factor*((xyz[0]+xyz[1] - 0.001)/math.sqrt(2)) )
    CheckIntegrity_SetUpAndReinitialize(writer,myMesh,ls,lambda xyz:  ((xyz[0]+xyz[1] - 0.001)/math.sqrt(2)) )
    CheckIntegrity_SetUpAndReinitialize(writer,myMesh,ls,lambda xyz:  factor*((xyz[0]+xyz[1]+xyz[2] - 0.001)/math.sqrt(3))  )
    CheckIntegrity_SetUpAndReinitialize(writer,myMesh,ls,lambda xyz:  ((xyz[0]+xyz[1]+xyz[2] - 0.001)/math.sqrt(3))  )
    CheckIntegrity_SetUpAndReinitialize(writer,myMesh,ls,lambda xyz:  factor*(math.sqrt(xyz[0]**2+xyz[1]**2+xyz[2]**2) - 5) )
    CheckIntegrity_SetUpAndReinitialize(writer,myMesh,ls,lambda xyz:  (math.sqrt(xyz[0]**2+xyz[1]**2+xyz[2]**2) - 5) )
    writer.Close()
    return 'OK'

if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity())
