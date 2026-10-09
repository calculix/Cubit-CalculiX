# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from collections import defaultdict

import numpy as np

from Muscat.Types import MuscatFloat
import Muscat.Helpers.ParserHelper as PH

from Muscat.FE.UnstructuredFeaSym import UnstructuredFeaSym
from Muscat.FE.SymPhysics import ThermalPhysics
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.KR.KRBlock import KRBlockVector,KRBlockScalar
from Muscat.FE.Fields.FieldTools import FieldsMeshTransportation

from OpenPisco.PhysicalSolvers.UnstructuredFEASolverBase import UnstructuredFEASolverBase
import OpenPisco.TopoZones as TZ
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS

def CreateUnstructuredFEAThermalProblem(ops):
    """
    Constructor for a UnstructuredFEAThermalProblem

    Parameters
    ----------
    ops
        A dictionary-like object

    Returns
    -------
    UnstructuredFEAThermalProblem
        A populated `UnstructuredFEAThermalProblem` instance

    Notes
    -----
    The `ops` can be generated from a string like this::

        a = <XXXXXXXX id="X" type="thermal"  p="*1|2"  >

    0.*<Material    eTag="*everyelement|eTag" lambda="float" rho="float" cp="float" />
    0.*<Dirichlet   eTag="eTag" value="[float]" />
    0.*<Source      eTag="eTag"  value="[float]"/>
    0.*<Convection  eTag="eTag" h="float" temp_ext="float"/>
    0.*<TimeParameters start="float" end="float" nsteps="float" starttemperature="float"/>

         </GXXXXXXXX>
    """

    spaceDim = PH.ReadInt(ops.get('dimensionality',"3"))
    if 'dimensionality' in ops:
        del(ops['dimensionality'])
    res = UnstructuredFEAThermalProblem(dim=spaceDim)
    if ops is None:
        return res
    if 'computeSteadyStateSolution' in ops:
        res.SetSteadyStateComputation(ops['computeSteadyStateSolution'])
    assert ops["type"] == "thermal","for the moment only thermal are allowed: '{}' ".format(ops["type"])
    P = PH.ReadInt(ops.get("p",1))
    ops.pop("p",None)
    thermalPhysics = ThermalPhysics(spaceDimension=spaceDim)
    thermalPhysics.SetSpaceToLagrange(P=P)
    res.internalSolveur.physics.append(thermalPhysics)
    tagsToKeep=[]

    for tag,child in ops["children"]:
        if tag.lower() == "material":
            eTag = child.get('eTag','AllZones')
            material = {"lambda":PH.ReadFloat(child["lambda"]),"rho":PH.ReadFloat(child.get("rho",1.)),"cp":PH.ReadFloat(child.get("cp",1.)) }
            res.materials[eTag] = material
            continue
        if tag.lower() == "dirichlet":
            eTag = PH.ReadString(child["eTag"])
            dirichlet = KRBlockScalar()
            dirichlet.AddArg("t").On(eTag)
            dirichlet.SetValue(PH.ReadFloat(child["value"]))
            res.internalSolveur.solver.constraints.AddConstraint(dirichlet)
            tagsToKeep.append(eTag)
            continue
        if tag.lower() == "source":
            eTag = child.get('eTag','AllZones')
            res.source[eTag]=PH.ReadFloat(child["value"])

    ops.pop("type")
    res.tagsToKeep.extend(tagsToKeep)
    res.tagsToKeep.extend(list(PH.ReadStrings(ops.get("tagsToKeep",""))))
    #ps.pop("tagsToKeep",None)
    del ops["children"]
    PH.ReadProperties(ops,ops.keys(), res, typeConversion=True)
    return res

class UnstructuredFEAThermalProblem(UnstructuredFEASolverBase):
    def __init__(self,dim=3):
       super().__init__()
       self.SetSteadyStateComputation()
       self.internalSolveur = UnstructuredFeaSym(spaceDim=dim)
       self.auxiliaryFieldGeneration[FN.temperature][FN.Nodes] = False
       self.propsFields = defaultdict(list)
       self.transportFieldOP = FieldsMeshTransportation(oldMesh=None, newMesh=None)
       self.__numberingCache__ = {}
       self.dim = dim
       self.delayedInitialization = None
       self.extraRHSVector = dict()
       self.primalUnknownNames = ["t"]
       self.materials = {}
       self.source= {}

    def SetSteadyStateComputation(self,flag=False):
        self.computeSteadyStateSolution = flag

    def PreSolver(self,levelSet):
        self.conform = levelSet.conform
        self.ComputeDomainToTreatLs(levelSet)
        self.transportFieldOP.ResetCacheData()
        self.numbering = None

        self.internalSolveur.Reset()
        assert self.conform, "Not conformal not coded yet"
        tokeep = [ self.DomainToTreat ]
        tokeep.extend(self.tagsToKeep)
        thermalPhysics = self.internalSolveur.physics[0]
        thermalPhysics.bilinearWeakFormulations = []
        material = self.materials["AllZones"]
        if not self.computeSteadyStateSolution:
           thermalPhysics.AddBFormulation(self.DomainToTreat,thermalPhysics.GetMassOperator(rho=material["rho"],cp=material["cp"]))
        thermalPhysics.AddBFormulation(self.DomainToTreat, thermalPhysics.GetBulkFormulation(alpha=material["lambda"]) )
        thermalPhysics.AddLFormulation(self.DomainToTreat,thermalPhysics.GetNormalFlux("source"))
        self.internalSolveur.SetMesh(self.cleanMesh)
        self.internalSolveur.ComputeDofNumbering(tokeep)
        source = FEField(name="source",mesh=self.cleanMesh,space=thermalPhysics.spaces[0],numbering=self.internalSolveur.numberings[0])
        source.data = np.ones(self.cleanMesh.GetNumberOfNodes(),dtype=MuscatFloat)
        self.internalSolveur.fields["source"] = source


#RegisterClass("UnstructuredThermal",None,CreateUnstructuredFEAThermalProblem)

import xml.etree.ElementTree as ET

from Muscat.MeshTools.MeshCreationTools import CreateSquare
import Muscat.MeshContainers.ElementsDescription as ED

from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco.CLApp.XmlToDic import XmlToDic

def CheckIntegrityThermal2D(GUI=False):
    teststringField=u"""
    <Unstructured type="thermal" >
        <Material  lambda="50." rho="7850.0" cp="1046.0"/>
        <Dirichlet eTag="X0" value="50.0"/>
        <Source value="1.0"/>
    </Unstructured >
    """
    teststringField = PH.ApplyGlobalDictionary(teststringField)
    reader = XmlToDic()
    _, ops = reader.ReadFromString(teststringField)
    ops["dimensionality"]=2
    phyProblem=CreateUnstructuredFEAThermalProblem(ops)
    mesh = CreateSquare(dimensions=[2,3],spacing=[.2,.2],ofTriangles=True)
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet( support=mesh)
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    ls.conform=True
    phyProblem.SolveByLevelSet(ls)
    phyProblem.GetNodalSolution()
    return "ok"

def CheckIntegrity(GUI=False):
    totest = [ CheckIntegrityThermal2D ]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity(GUI=True))
