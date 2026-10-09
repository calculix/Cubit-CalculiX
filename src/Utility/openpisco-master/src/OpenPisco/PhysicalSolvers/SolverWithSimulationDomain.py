# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import xml.etree.ElementTree as ET
import numpy as np

from Muscat.FE.FETools import PrepareFEComputation
from Muscat.FE.Fields.FEField import FEField
from Muscat.MeshTools.MeshFieldOperations import GetFieldTransferOp
from Muscat.MeshTools.MeshCreationTools import CreateCube

import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.UnstructuredFEASolverBase import UnstructuredFEASolverBase
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import RegisterClass
from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco import RETURN_SUCCESS
from OpenPisco.PhysicalSolvers.UnstructuredFEAGenericSolver import CreateUnstructuredFEAGenericProblem
from OpenPisco.CLApp.XmlToDic import XmlToDic

def CreateSolverWithSimulationDomain(ops):
    res = SolverWithSimulationDomain(problem=ops["problem"],grid=ops["grid"])
    return res

class SolverWithSimulationDomain(UnstructuredFEASolverBase):
    def __init__(self, problem=None, grid=None):
        super().__init__()
        self.internalSolver = problem
        self.simulationLevelSet= LevelSet(support=grid)

    def SolveByLevelSet(self,levelSet):
        self.levelSet = levelSet
        self.TransferLSFieldToSimulationDomain()
        self.internalSolver.SolveByLevelSet(self.simulationLevelSet)
        return RETURN_SUCCESS

    def SetAuxiliaryScalarGeneration(self,name,flag=True):
        return self.internalSolver.SetAuxiliaryScalarGeneration(name,flag)

    def SetAuxiliaryFieldGeneration(self,name,on=FN.Nodes,flag=True):
        return self.internalSolver.SetAuxiliaryFieldGeneration(name,on,flag)

    def GetAuxiliaryScalar(self,name):
        return self.internalSolver.GetAuxiliaryScalar(name)

    def GetTransferedNodalField(self,inField,inMesh,outMesh):
        space, numberings, _, _ = PrepareFEComputation(inMesh,numberOfComponents=1)
        tempfield = FEField("",mesh=inMesh,space=space,numbering=numberings[0])
        op, _ , _ = GetFieldTransferOp(tempfield,outMesh.nodes,method="Interp/Clamp" )
        return op.dot(inField)

    def GetAuxiliaryField(self,name,on=FN.Nodes,index=None):
        assert on is FN.Nodes,"SolverWithSimulationDomain can return only nodal quantities"
        res = self.internalSolver.GetAuxiliaryField(name,on,index)
        return self.GetTransferedNodalField(inField=res,
                                           inMesh=self.simulationLevelSet.support,
                                           outMesh=self.levelSet.support)

    def TransferLSFieldToSimulationDomain(self):
        field = self.GetTransferedNodalField(inField=self.levelSet.phi,inMesh=self.levelSet.support,
                                           outMesh=self.simulationLevelSet.support)
        self.simulationLevelSet.TakePhiAndMesh(field,self.simulationLevelSet.support)

    def GetNodalSolution(self,i=0):
        res = self.internalSolver.GetNodalSolution()
        return  self.GetTransferedNodalField(inField=res,
                inMesh=self.simulationLevelSet.support,
                outMesh=self.levelSet.support)

RegisterClass("SolverWithSimulationDomain",SolverWithSimulationDomain,CreateSolverWithSimulationDomain)


def CheckIntegrity(GUI=False):
    xmlInputString = """<InternalSolver type="static_elastic"  >
    <Material    young="1" poisson="0.3"/>
    <Dirichlet   eTag="X0" dofs="0 1 2" value="0" />
    <Force eTag="X1" dir="1 0 0" value="0.002" />
    </InternalSolver>"""
    root = ET.fromstring(xmlInputString)
    obj = XmlToDic()
    _,data = obj.XmlToDic(root)
    internalSolver = CreateUnstructuredFEAGenericProblem(data)
    simulationGrid = CreateCube(ofTetras=True)
    ls = LevelSet(support = CreateCube(ofTetras=True))
    solver = CreateSolverWithSimulationDomain({"problem":internalSolver,"grid":simulationGrid})
    solver.SolveByLevelSet(ls)
    internalSolver.SolveByLevelSet(ls)
    assert np.max(internalSolver.GetNodalSolution() - solver.GetNodalSolution()) < 1e-20
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity(GUI=True))
