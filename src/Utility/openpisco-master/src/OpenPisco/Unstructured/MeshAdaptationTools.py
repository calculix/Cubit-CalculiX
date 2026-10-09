# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import copy

import numpy as np
import sympy as sy

from Muscat.Types import MuscatFloat
import Muscat.MeshContainers.ElementsDescription as ED

from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags

from OpenPisco.PhysicalSolvers.PhysicalSolversTools import CellToPointData
from OpenPisco.ExternalTools.FreeFem.FreeFemInterface import FreeFemInterface
from OpenPisco.BaseElements import elementsTypeSupportedForDimension

metric   = 'metric'


def ComputeLevelSetMetric(levelset,opts,metricComputation="Freefem"):
    """
    Compute a metric field for level sets using FreeFem++

    Parameters
    ----------
    levelset     : level set
    hmin (float) : minimal edge size
    hmax (float) : maximal edge size

    Returns
    -------
    np.ndarray
        The metric field computed at mesh nodes
    """

    if metricComputation=="Freefem":
        obj = FreeFemLevelSetMetricComputation()
        obj.opts["hmin"]=opts["hmin"]
        obj.opts["hmax"]=opts["hmax"]
        obj.opts["dim"]=levelset.support.GetElementsDimensionality()
        metricfield = obj.ComputeLevelSetMetric(levelset)
    elif metricComputation=="Symbolic":
        symbolicExpression = opts["symbolicExpression"]
        assert "phi" in symbolicExpression, "Can not define metric expression without phi"
        expressionSympy = sy.sympify(symbolicExpression)
        phi = sy.symbols("phi")
        metricfield = sy.lambdify(phi, expressionSympy, 'numpy')(levelset.phi)
    else:
        raise ValueError("Only Freefem and symbolic computation are available")
    return metricfield

def RunIsotropicMeshAdaptation(mesh , error , tol):
    """
    Compute a nodal isotropic size field starting from an error field at mesh elements

    Parameters
    ----------
    mesh                : mesh
    error (np.ndarray)  : error field at mesh elements
    tol   (float)       : tolerance

    Returns
    -------
    np.ndarray
        The metric field computed at mesh nodes
    float
        The global error estimator
    """
    obj = IsotropicMeshAdaptation()
    obj.SetSupport(mesh)
    if tol is not None:
       obj.SetTolerance(tol)
    obj.SetLocalErrorEstimator(error)
    obj.ComputeMetricFromError()
    metric = obj.GetMetric()
    obj.ComputeGlobalErrorEstimator()
    globalErrorEstimator =obj.GetGlobalErrorEstimator()
    return metric ,globalErrorEstimator

class MeshAdaptationBase():
    """
    Base class to compute size fields used for mesh adaptation
    """
    def __init__(self  ):
        self.support = None
        self.tolerance =None
        self.metric =None
        self.localErrorEstimator = None
        self.globalErrorEstimator = None

    def SetSupport(self,support):
        self.support = support

    def SetLocalErrorEstimator(self, data):
        self.localErrorEstimator = data

    def SetTolerance(self,Tolerance):
        self.tolerance = Tolerance

    def GetGlobalErrorEstimator(self):
        return self.globalErrorEstimator

    def GetLocalErrorEstimator(self):
        return self.localErrorEstimator

    def GetMetric(self):
        return self.metric

class IsotropicMeshAdaptation(MeshAdaptationBase):
    """
    Class to compute isotropic size fields used for mesh adaptation
    """
    def __init__(self ):
        super(MeshAdaptationBase,self).__init__()
        self.tolerance = 1.

    def ComputeMetricFromError(self):
        error = copy.deepcopy(self.localErrorEstimator)
        h_k= ( 6*(self.tolerance)**2 /( (error.shape[0])*error) )**(1/3)
        dim = self.support.GetElementsDimensionality()
        self.metric = CellToPointData(self.support,{elementsTypeSupportedForDimension[dim]:h_k},dim)

    def ComputeGlobalErrorEstimator(self):
        self.globalErrorEstimator =  np.sqrt(np.sum(self.localErrorEstimator**2))


class FreeFemLevelSetMetricComputation():
    """
    Class for a file-exchange-based interface with Freefem for computing a level-set metric field for mesh adaptation.
    """
    def __init__(self):
        super(FreeFemLevelSetMetricComputation,self).__init__()
        self.support = None
        self.originalSupport = None
        self.codeInterface = FreeFemInterface()
        self.basename = "mshmetdata"
        self.opts={}

    def SetSupport(self, support):
        self.originalSupport = support
        self.support = ExtractElementsByTags(support, tagsToKeep =[], dimensionalityFilter=support.GetElementsDimensionality())

    def WriteSupport(self):
        self.codeInterface.writer.Open(self.codeInterface._filePath(self.basename + ".mesh"))
        self.codeInterface.writer.Write(self.support)
        self.codeInterface.writer.Close()

    def GetMetric(self):
        res = self.codeInterface.GetNodalField(metric)
        if len(res.shape)==1:
           metOnOriginalSupport = np.ones(self.originalSupport.GetNumberOfNodes(),dtype=MuscatFloat)
        elif len(res.shape)==2:
           metOnOriginalSupport = np.ones((self.originalSupport.GetNumberOfNodes(),res.shape[1]),dtype=MuscatFloat)
        metOnOriginalSupport[self.support.originalIDNodes] = res

        return metOnOriginalSupport

    def WriteLevelSet(self,field):
        ls = field[self.support.originalIDNodes]
        self.codeInterface.writer.SetFileName(self.codeInterface._filePath(self.basename+".chi.sol"))
        self.codeInterface.writer.OpenSolutionFile(self.support)
        self.codeInterface.writer.WriteSolutionsFields(self.support, [ls])
        self.codeInterface.writer.Close()

    def GenerateFreeFemScriptAndSetCodeInterface(self):
        from OpenPisco.ExternalTools.FreeFem.FreeFemProblemWriter import FreeFemLSMetricComputationWriter
        freefemWriter = FreeFemLSMetricComputationWriter(opts=self.opts)
        freefemWriter.WriteFreeFemProblem()
        self.codeInterface.SetFilename(freefemWriter.filename)

    def ComputeLevelSetMetric(self,levelset):
        self.SetSupport(levelset.support)
        self.WriteSupport()
        self.WriteLevelSet(levelset.phi)
        self.GenerateFreeFemScriptAndSetCodeInterface()
        self.codeInterface.ExecuteFreefem()
        return self.GetMetric()


from Muscat.Helpers.CheckTools import SkipTest
from Muscat.MeshTools.MeshCreationTools import CreateSquare,CreateCube

from OpenPisco.ExternalTools.FreeFem.FreeFemInterface import FREEFEM_AVAILABLE
from OpenPisco.Unstructured.Levelset import LevelSet

def CheckIntegrity_MeshAdaptation():
    UM =  CreateCube(dimensions=[5,5,5],spacing=[1./4,1./4,1./4],origin=[0., 0., 0.],ofTetras=True)
    isa=IsotropicMeshAdaptation()
    isa.SetSupport(UM)
    errorEstimator=np.ones(UM.GetElementsOfType(ED.Tetrahedron_4).GetNumberOfElements(),dtype=MuscatFloat)
    isa.SetLocalErrorEstimator(errorEstimator)
    isa.ComputeGlobalErrorEstimator()
    isa.ComputeMetricFromError()
    RunIsotropicMeshAdaptation(mesh=UM,error=errorEstimator,tol=1.)
    return "ok"

def CheckIntegrity_SymbolicMetric():
    mesh=  CreateCube(dimensions=[5,5,5],spacing=[2./4,1./4,1./4],origin=[0., 0., 0.],ofTetras=True)
    ls = LevelSet(support=mesh)
    ls.Initialize(lambda XYZs : (XYZs[:, 0]-0.5))
    initialLevelsetValue = ls.phi
    metrics = ComputeLevelSetMetric(ls,{"symbolicExpression":"phi**2+phi+1"},"Symbolic")
    assert np.linalg.norm(metrics - (initialLevelsetValue**2+initialLevelsetValue+1))<1e-6
    return "ok"

def CheckIntegrity_Freefem_3D():
    if SkipTest("FREEFEM_NO_FAIL"): return "skip"
    if not FREEFEM_AVAILABLE:
        return "skip Ok, FreeFem not found!!"
    mesh=  CreateCube(dimensions=[5,5,5],spacing=[2./4,1./4,1./4],origin=[0., 0., 0.],ofTetras=True)
    ls = LevelSet(support=mesh)
    ls.Initialize(lambda XYZs : (XYZs[:, 0]-0.5))
    opts={"hmin":0.5,"hmax":1.}
    ComputeLevelSetMetric(ls,opts)
    return "ok"

def CheckIntegrity_Freefem_2D():
    if SkipTest("FREEFEM_NO_FAIL"): return "skip"
    if not FREEFEM_AVAILABLE:
        return "skip Ok, FreeFem not found!!"
    mesh = CreateSquare(dimensions=[10,5],spacing=[4.,4.],ofTriangles=True)
    ls = LevelSet(support=mesh)
    ls.Initialize(lambda XYs : (XYs[:, 0]-0.5))
    opts={"hmin":0.5,"hmax":1.}
    ComputeLevelSetMetric(ls,opts)
    return "ok"

def CheckIntegrity(GUI=False):
    totest = [CheckIntegrity_SymbolicMetric,
        CheckIntegrity_MeshAdaptation,
        CheckIntegrity_Freefem_2D,
        CheckIntegrity_Freefem_3D]

    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
