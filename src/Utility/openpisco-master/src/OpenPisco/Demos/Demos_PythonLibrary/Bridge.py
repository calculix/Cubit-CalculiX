# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

from Muscat.MeshTools.MeshCreationTools import CreateSquare
from Muscat.ImplicitGeometry.ImplicitGeometryObjects import CreateImplicitGeometryPlane, ImplicitGeometryHoles,CreateImplicitGeometryAnalytical
from Muscat.ImplicitGeometry.ImplicitGeometryOperators import ImplicitGeometryUnion, ImplicitGeometryIntersection
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.IO.XdmfWriter import XdmfWriter

from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco.PhysicalSolvers.UnstructuredFEAGenericSolver import CreateUnstructuredFEAGenericProblem
from OpenPisco.Optim.Problems.OptimProblemTopoGeneric import OptimProblemTopoGeneric
from OpenPisco.Optim.Criteria.GeoCriteria import TopoCriteriaVolume
from OpenPisco.Optim.Criteria.PhyCriteria import TopoCriteriaCompliance
from OpenPisco.Optim.Algorithms.OptimAlgoNullSpace import OptimAlgoNullSpace

def RunTopologyOptimizationBridgeTestCase():
    IGHoles = ImplicitGeometryHoles()
    IGHoles.r = 0.2
    IGHoles.n = np.array([6., 4., 2.])
    IGHoles.type = "Original"

    IGGeometryPlane = CreateImplicitGeometryPlane({"point": [0, 1.8, 0], "normal": [0, -1, 0]})

    IGAnalytical1 = CreateImplicitGeometryAnalytical({"expr": "x-1 "})
    IGAnalytical2 = CreateImplicitGeometryAnalytical({"expr": "x-7 "})
    IGAnalytical2.insideOut = True
    IGAnalytical3 = CreateImplicitGeometryAnalytical({"expr": "y-0.01 "})

    IGUnion = ImplicitGeometryUnion([IGAnalytical1,IGAnalytical2])
    IGIntersection = ImplicitGeometryIntersection([IGUnion,IGAnalytical3])

    dimensions = np.array([25,20])
    length = np.array([8,2])
    mesh= CreateSquare(dimensions=dimensions, spacing=length/(dimensions-1), origin=[0,0], ofTriangles=True)

    for _,data,_ in ElementFilter(dimensionality=1)(mesh):
        data.tags.CreateTag("Ridges",False).SetIds(np.arange(data.GetNumberOfElements()))

    mesh.nodesTags.CreateTag("Corners",errorIfAlreadyCreated=False).Merge(other=mesh.nodesTags[ "x0y0"])
    mesh.nodesTags.CreateTag("Corners",errorIfAlreadyCreated=False).Merge(other=mesh.nodesTags[ "x1y0"])
    mesh.nodesTags.CreateTag("Corners",errorIfAlreadyCreated=False).Merge(other=mesh.nodesTags[ "x0y1" ])
    mesh.nodesTags.CreateTag("Corners",errorIfAlreadyCreated=False).Merge(other=mesh.nodesTags[  "x1y1" ])

    ff = ElementFilter()
    ff.SetZones([IGIntersection])
    ff.SetDimensionality(1)
    for _,data,ids in ff(mesh):
        data.tags.CreateTag("Dirichlet").SetIds(ids)

    print(mesh)

    levelset = LevelSet(support=mesh)
    levelset.conform = True
    levelset.MesherInfo = {"gradientMethod":"gradPhi","hmax":0.1,"hmin":0.01,"rmc":1e-5,"hausd":0.01,"nr":True,"iso":0.0}

    IGHoles.SetSupport(levelset.support)
    levelset.phi = IGHoles(levelset.support)



    material = ('Material', {'young': '1', 'poisson': '0.3'})
    dirichlet = ('Dirichlet', {'eTag': 'Dirichlet', 'dofs': '0 1', 'value': '0'})
    load = ('Force', {'eTag': 'Y1', 'dir': '0 1', 'value': '-0.002'})
    data = {'type': 'static_elastic', 'p': '1','dimensionality':'2',"linearSolverName":"Mumps",'children':[material,dirichlet,load]}
    physicalSolver = CreateUnstructuredFEAGenericProblem(data)

    res = OptimProblemTopoGeneric()
    volCrit = TopoCriteriaVolume()
    comCrit = TopoCriteriaCompliance()
    res.SetObjectiveCriterion(comCrit)
    res.AddInequalityConstraint(volCrit)
    volCrit.SetUpperBound(0.4*1.55651243e+01)
    comCrit.problem = physicalSolver
    res.point = levelset
    res.ResetState()
    res.OnZone = IGGeometryPlane

    writer = XdmfWriter('Bridge2DHistory.xmf')
    print("Opening file :"+writer.fileName)
    writer.SetTemporal()
    writer.SetBinary()
    writer.SetXmlSizeLimit(0)
    writer.Open()
    res.writer = writer

    OA = OptimAlgoNullSpace()
    OA.ResetState()
    OA.alphaC = 0.5
    OA.tol_merit =  0.001
    OA.optimProblem =res
    OA.Start()

    return "Optimization executed successfully"

if __name__ == '__main__':
    print(RunTopologyOptimizationBridgeTestCase())






