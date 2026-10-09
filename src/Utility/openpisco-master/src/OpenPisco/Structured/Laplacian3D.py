# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np
import scipy.sparse as sps

import Muscat.LinAlg.LinearSolver as LS
from Muscat.FE.ConstantRectilinearFea import ElementaryMatrix

from OpenPisco.BaseElements import complexElementsTypesSupportedForDimension

def AssembledOperator(support, elementary_matrix):
    ndof = support.GetNumberOfNodes()
    nnodes = support.props.get("dimensions")
    nelems = np.prod(nnodes - 1)
    ndims = support.GetElementsDimensionality()
    nodesPerElement = 2**ndims

    edofMat = support.GetElementsOfType(complexElementsTypesSupportedForDimension[ndims]).connectivity

    v = np.tile(elementary_matrix, (nelems, 1, 1)).ravel()
    i = np.repeat(edofMat, nodesPerElement)
    j = np.tile(edofMat, nodesPerElement).ravel()
    shape = (ndof, ndof)
    return sps.coo_matrix((v, (i, j)), shape=shape).tocsc()

def ConstantMassMatrix(support, rho=1.0):
    elem = ElementaryMatrix(dim = support.GetElementsDimensionality(),physics="thermal")
    elem.geoFactor = support.props.get("spacing")
    elem.rho = rho
    eM =  elem.GetMassMatrix()

    return AssembledOperator(support, eM)

def LaplacianOperator(support, kx=1.0):
    elem = ElementaryMatrix(dim = support.GetElementsDimensionality(),physics="thermal")
    elem.geoFactor = support.props.get("spacing")
    elem.thermalK = kx
    eK =  elem.GetTangentMatrix()

    return AssembledOperator(support, eK)

def RegularizationOperator(support, kx=1.0, blockDofs= None):
    __regularization_operator_cache = LS.LinearProblem()
    LapOP = LaplacianOperator(support, kx)

    if blockDofs is  None:
        dv = np.prod(support.props.get("spacing"))
        factor = sps.identity(support.GetNumberOfNodes(), format='csc')*dv
    else:
        factor = sps.dia_matrix((blockDofs,0), shape=LapOP.shape)

    __regularization_operator_cache.SetOp(LapOP + factor)
    return __regularization_operator_cache

def RegularizationOperatorM(support, blockDofs = None):
    dv = np.prod(support.props.get("spacing"))
    if blockDofs is None:
        return sps.identity(support.GetNumberOfNodes(), format='csc')*dv
    else:
        return sps.dia_matrix((blockDofs,0), (support.GetNumberOfNodes(),support.GetNumberOfNodes()) )*dv


def CheckIntegrity():
    import Muscat.MeshTools.ConstantRectilinearMeshTools as CRM

    print('--------- 3D --------- ')
    support = CRM.CreateConstantRectilinearMesh(dimensions=[2,2,2], spacing=[1,1,1], origin=[0,0,0])

    RegularizationOperator(support,blockDofs=[0])
    RegularizationOperatorM(support,blockDofs=[0])

    RegularizationOperator(support)
    RegularizationOperatorM(support)

    print('--------- 2D --------- ')
    support = CRM.CreateConstantRectilinearMesh(dimensions=[2,2], spacing=[1,1], origin=[0,0])

    RegularizationOperator(support)
    RegularizationOperatorM(support)

    RegularizationOperator(support)
    RegularizationOperatorM(support)

    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
