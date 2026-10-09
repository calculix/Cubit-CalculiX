# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations

from typing import Dict,List,Union,Optional,Any

import numpy as np

import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.Logger import Info
from Muscat.Helpers.MPIInterface import MPIInterface

from Muscat.MeshTools.MeshCreationTools import CreateCube
from Muscat.MeshTools.MeshFieldOperations import GetFieldTransferOp

from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.Fields.FieldTools import CreateFieldFromDescription
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceGeo
from Muscat.FE.FETools import PrepareFEComputation
from Muscat.IO.IOFactory import CreateWriter

from Muscat.MeshContainers.Filters.FilterTools import  ReadElementFilter
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshContainers.Mesh import Mesh

from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco.CLApp.XmlToDic import XmlToDic


class ZoneMaterialData():
    def __init__(self):
        self.elementFilter = ElementFilter()
        self.coeffs = {}
        self.name = ""

    def __getitem__(self,key:str)->float:
        return self.coeffs[key]

    def GenerateFields(self, mesh:Mesh)->List:
        res = []
        self.elementFilter.mesh = mesh
        for fieldname in self.coeffs.keys():
            field = CreateFieldFromDescription(mesh,[(self.elementFilter,self.coeffs[fieldname])], fieldType="IP" )
            field.name = fieldname
            res.append(field)

        return res

def ReadZoneMaterialData(data:Union[Dict,str]) -> ZoneMaterialData | Any:
    if isinstance(data,dict):
        res = ZoneMaterialData()
        res.elementFilter = ReadElementFilter(data.get("ef",""))
        if "ef" in data:
            data.pop("ef")

        if "name" in data:
            res.name = ReadElementFilter(data["name"])
            data.pop("name")

        for prop in data:
            val = PH.ReadFloat(data[prop])
            print(f" Reading prop {prop} -> {val}")
            res.coeffs[prop] = val
        return res

    elif isinstance(data,str):
        return ReadZoneMaterialData(XmlToDic().ReadFromString(data)[1])


class AMProcessBase(SolverBase):
    def __init__(self):
        super().__init__()
        self.solveCpt= 0

        self.nx = 10
        self.ny = 10 
        self.nz = 10 
        self.useProjectionInitialization = True
        self.useLevelsetMesh = False
        self.customComputationalMesh = None
        self.boundaryConditions = [(['Z0'],[0,1,2])] # default

        self.debugFilename = ""
        self._debugWriter_ = None
        self.otherLevelSetMeshofTetra = True

    def SetBoundaryConditions(self, bc:any):
        self.boundaryConditions  = bc

    def PreStart(self):
        if  len(self.debugFilename) > 0:
            extra = ""
            mpiInterface = MPIInterface()
            extra = ("R"+str(mpiInterface.Rank())+"_")*mpiInterface.IsParallel()
            fname = self.debugFilename+"_"+extra+str(self.solveCpt) +"_debug.xdmf"
            writer = CreateWriter(fname)
            writer.SetFileName(fname)
            writer.SetTemporal()
            writer.SetBinary(False)
            writer.SetHdf5(True)
            writer.automaticOpen = True
            self._debugWriter_ = writer
        else:
            self._debugWriter_ = None

    def PostSolve(self):
        if self._debugWriter_ is not None:
            self._debugWriter_.Close()

    def WriteDebug(self, levelset:LevelSet, Time:Optional[float]= None):
        if self._debugWriter_ is not None:
            Info("Output")
            self._debugWriter_.Write(levelset.support,
                            PointFields=[levelset.phi]+list(levelset.support.nodeFields.values()), PointFieldsNames=["phi"]+list(levelset.support.nodeFields.keys()),
                            CellFields=list(levelset.support.elemFields.values()), CellFieldsNames=list(levelset.support.elemFields.keys()),
                            Time= Time)

    def GetComputationalLevelset(self, _levelSet:LevelSet, useSupport:bool=False):
        if self.useLevelsetMesh:
            computationLevelSet = LevelSet(_levelSet)
            computationLevelSet.phi = _levelSet.phi
        else:
            if self.customComputationalMesh is None:
                Info("Use generated cube mesh")
                dimensions = np.array([self.nx,self.ny,self.nz])
                meshBoundingMin, meshBoundingMax = _levelSet.support.ComputeBoundingBox()
                spacing = (meshBoundingMax-meshBoundingMin)/(dimensions-1)

                computationMesh = CreateCube(dimensions=dimensions,
                                            origin=meshBoundingMin,
                                            spacing=spacing,
                                            ofTetras=self.otherLevelSetMeshofTetra)
            else:
                Info("Use custom computational mesh")
                computationMesh = self.customComputationalMesh
            computationLevelSet = LevelSet(_levelSet)

            space, numberings, _, _ = PrepareFEComputation(_levelSet.support,numberOfComponents=1)
            tempfield = FEField("",mesh=_levelSet.support,space=space,numbering=numberings[0])
            op, _, _ = GetFieldTransferOp(tempfield,computationMesh.nodes,method="Interp/Clamp" )
            if not useSupport:
                computationLevelSet.TakePhiAndMesh(op.dot(_levelSet.phi), computationMesh)

        return computationLevelSet

    def GetZPosAtIntegrationPoints(self, mesh:Mesh):
        Info("Z at IP")
        res = CreateFieldFromDescription(mesh,[(ElementFilter(),lambda x : x[2])], fieldType="IP" )
        res.name = "CoordZ"
        Info("Z at IP DONE ")

        return res

    def GetProjectionInitialization(self, mesh:Mesh, deltaZ:float):
        Info("Creation of transfer operator")
        #This is a hack to accelerate the resolution by transporting the solution
        #by delta_z in the z direction at every layer step
        tNodes  = mesh.nodes.copy()
        tNodes[:,2] -= deltaZ
        numberingSpaceGeo = ComputeDofNumbering(mesh,LagrangeSpaceGeo,fromConnectivity=True)
        inputField= FEField(name="tempfield",mesh=mesh,numbering=numberingSpaceGeo, space=LagrangeSpaceGeo )
        # we use Nearest/Nearest because We know we are in structured meshes "Mode"
        transOP, _ , _ = GetFieldTransferOp(inputField,tNodes,method="Nearest/Nearest",verbose=False)
        Info("Creation of transfer operator Done")

        return transOP