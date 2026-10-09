# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

import Muscat.MeshContainers.ElementsDescription as ED

import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.AsterSolverBase import AsterSolverBase
from OpenPisco.PhysicalSolvers.AsterStatic import GetMaterialFields
from OpenPisco.PhysicalSolvers.AsterStatic import PrepareStaticInstance
import OpenPisco.TopoZones as TZ
from OpenPisco.PhysicalSolvers.AsterAnalysisSolverFactory import RegisterAsterClass
import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
from OpenPisco.Unstructured.Levelset import triangle_3d_areas

def CreateAsterStaticBucklingPhysicalProblem(ops):
     res = AsterStaticBuckling()
     return PrepareStaticInstance(res=res,ops=ops)

class AsterStaticBuckling(AsterSolverBase):
    def __init__(self,loadcritParams={"NumbermaxLoadCrit":1}):
        super(AsterStaticBuckling,self).__init__()
        self.name = 'AsterStaticBuckling'
        FieldsICanSupply = [FN.potential_energy,FN.von_mises, FN.stress, FN.strain,FN.elastic_energy_buckling,
                            FN.strain_buckling,FN.strain_green_buckling,FN.mode_buckling]
        for fieldname in FieldsICanSupply:
            self.auxiliaryFieldGeneration[fieldname][FN.Centroids] = False
            self.auxiliaryFieldGeneration[fieldname][FN.Nodes] = False
        self.auxiliaryScalarGeneration = {FN.int_potential_energy:False}
        self.auxiliaryScalarsbyOrderGeneration = {FN.load_crit:False}
        self.loadcritParams= loadcritParams
        self.residual = 1e-5

    def WriteComputationalSupport(self, levelset):
        self.PrepareComputationalSupport(levelset)
        materialFieldsNames,materialFields = GetMaterialFields(support=self.cleanMesh,
                                                                phi=levelset.phi,
                                                                materials=self.materials,
                                                                levelset=levelset,
                                                                eVoid=self.eVoid)
        self.WriteSupport(self.cleanMesh,cellFieldsNames=materialFieldsNames,cellFields=materialFields)

    def SolveStandard(self):
        solverState = self.SolveWithRelaxedResidual()
        return solverState

    def WriteParametersInput(self,writeFile):
        assert len(self.problems) <= 1, "GeneralAster: Sorry multi loads problem not implemented yet "
        dimensionality = self.originalSupport.GetElementsDimensionality()
        AsterCommonWriter.WriteDirichletParametersInput(writeFile,self.dirichlet,dimensionality)
        AsterCommonWriter.WriteNeumannParametersInput(writeFile,self.neumann,dimensionality,self.problems)
        AsterCommonWriter.WriteNodalForcesParametersInput(writeFile,self.forcenodale,dimensionality,self.problems)
        AsterCommonWriter.WriteBodyForcesParametersInput(writeFile,self.bodyforce,dimensionality,self.problems)
        AsterCommonWriter.WriteVariable(writeFile=writeFile,
                                        variableName="NMAX_CHAR_CRIT",
                                        variableValue="{'NMAX_CHAR_CRIT' : "+str(self.loadcritParams['NumbermaxLoadCrit'])+"}")
        AsterCommonWriter.WriteVariable(writeFile=writeFile,variableName="resi_rela",variableValue=self.residual)


    def ComputeTriangleAreaByTag(self,mesh,tag):
        elems = mesh.GetElementsOfType(ED.Triangle_3)
        ids = elems.tags[tag].GetIds()
        return np.sum(triangle_3d_areas(mesh.nodes[elems.connectivity[ids]]))

    def ComputeMagnitudeLoad(self):
        assert len(self.problems) <= 1,"GeneralAster: multi loads problem is not implemented "
        load_sum = np.zeros(3)
        if self.forcenodale:
            loads = self.forcenodale[self.problems[0]]
            assert len(loads)>0,"nodal force is not given"
            for load in loads:
                load_sum += np.array(load[1])
        if self.neumann:
            loads = self.neumann[self.problems[0]]
            assert len(loads)>0,"surface force is not given"
            for tag,load in loads:
                load_sum += np.array(load)*self.ComputeTriangleAreaByTag(self.originalSupport,tag)
        norm_load = np.linalg.norm(load_sum)
        return norm_load

    def GetBucklingMode(self,onCleanMesh=False):
        solution = self.GetField('mode_buckling',"nodes")
        self.u1 = self.ReorderFieldToComputationalSupport(field=solution,onCleanMesh=onCleanMesh)
        return self.u1

    def GetNodalSolution(self,onCleanMesh=False):
        self.u = self.GetSolution(name='DEPL',onCleanMesh=onCleanMesh)
        return self.u

    def GetArrayofLoadMultiplier(self):
        return np.array(self.GetAuxiliaryScalarsbyOrders(FN.load_crit))

    def GetArrayofCriticalLoad(self):
        return self.GetArrayofLoadMultiplier()*self.ComputeMagnitudeLoad()
RegisterAsterClass("buckling", AsterStaticBuckling,CreateAsterStaticBucklingPhysicalProblem)

from Muscat.MeshTools.MeshCreationTools import CreateCube
from OpenPisco.Unstructured.Levelset import LevelSet

def CheckIntegrityBuckling(GUI=False):
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    print("Using temporary directory: " +str(TemporaryDirectory.GetTempPath()))
    mesh = CreateCube(dimensions=[5,5,5],spacing=[1./4, 1./2, 1./2],origin=[0, 0, 0],ofTetras=True)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.ExterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))

    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    loadcritParams={"NumbermaxLoadCrit":10}
    nb_crit = loadcritParams["NumbermaxLoadCrit"]
    PPM = AsterStaticBuckling(loadcritParams=loadcritParams)
    PPM.SetAuxiliaryScalarsbyOrderGeneration(FN.load_crit,listSize=nb_crit)
    PPM.SetAuxiliaryFieldGeneration(FN.elastic_energy_buckling,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.potential_energy,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.strain_buckling,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.strain_green_buckling,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.mode_buckling,on=FN.Nodes)
    material1={"young":1.0,"poisson":0.3}
    PPM.materials = [['AllZones',material1]]
    PPM.dirichlet= [['X0',(0.,0.,0.)]]
    PPM.problems = ['idx1']
    PPM.neumann= {"idx1": [['X1',(0.,0.,-0.1)]]}

    print("Computing buckling mode conform ...")
    PPM.SolveByLevelSet(ls)

    PPM.GetAuxiliaryField(FN.strain_buckling,on=FN.Centroids)
    PPM.GetAuxiliaryField(FN.strain_green_buckling,on=FN.Centroids)
    PPM.GetAuxiliaryField(FN.elastic_energy_buckling,on=FN.Centroids)
    PPM.GetAuxiliaryField(FN.potential_energy,on=FN.Centroids)
    PPM.GetBucklingMode()
    PPM.GetNodalSolution()
    print("list of load muliplier : ",PPM.GetArrayofLoadMultiplier())
    return "ok"

def CheckIntegrity(GUI=False):
    from OpenPisco.ExternalTools.Aster.AsterInterface import SkipAsterTest
    skipTestPhase,message = SkipAsterTest()
    if skipTestPhase:
        return message

    totest = [
        CheckIntegrityBuckling
        ]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
