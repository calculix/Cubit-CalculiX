# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

from collections.abc import Iterable
from typing import Union,Tuple,Optional
import tempfile, os

from collections import defaultdict
import itertools
import numpy as np

from Muscat.Types import ArrayLike,MuscatFloat
from Muscat.MeshContainers.Mesh import Mesh
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags
from Muscat.MeshTools.MeshFieldOperations import PointToCellData
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter,NodeFilter
from Muscat.Helpers.Logger import Info
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory

import OpenPisco.PhysicalSolvers.FieldsNames as FN
import OpenPisco.TopoZones as TZ
from OpenPisco.ExtractConnexPart import ConnexPart
from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco.PhysicalSolvers.PhysicalSolversTools import ExtractInteriorMesh

def OptionObjGenerator():
    res = dict()
    for on in FN.AllSupports:
        res[on]  = None
    return res

def AssessConnectedZones(support:Mesh,
                         phi:ArrayLike,
                         rootZones:Iterable[Union[str,NodeFilter]],
                         connectedZones:Iterable[Union[str,NodeFilter]])->bool:
    """Assess whether all pair of zones are connected

    Parameters
    ----------
    levelset : LevelSet
        level set to check
    rootZones : Iterable[Union[str,NodeFilter]]
        collection of zones
    connectedZones : Iterable[Union[str,NodeFilter]]
        collection of zones

    Returns
    -------
    bool
        True if all pair of zones are connected, else False
    """
    rootZonesCombinations = list(itertools.combinations(rootZones,2))
    connectedZonesCombinations = list(itertools.combinations(connectedZones,2))
    rootConnectedCombinations = list(itertools.product(rootZones,connectedZones))
    allCombinations = rootZonesCombinations+connectedZonesCombinations+rootConnectedCombinations
    for zoneCouple in allCombinations:
        rootZoneFilter,connectedZoneFilter = zoneCouple
        fix = rootZoneFilter.GetNodesIndices(mesh=support)
        mask = np.zeros(support.GetNumberOfNodes())
        mask[fix] = 1
        connexPartOp = ConnexPart(support)
        phiTemp = connexPartOp.Apply(phi, mask)
        fix = connectedZoneFilter.GetNodesIndices(mesh=support)
        connectedCouple = np.any(phiTemp[fix]  <= 0)
        if not connectedCouple:
            Info("At least one couple of disconnected zones")
            return False
    return True

def AssessZonesAttachedToLevelset(support:Mesh,
                                  phi:ArrayLike,
                                  rootZones:Iterable[Union[str,NodeFilter]],
                                  connectedZones:Iterable[Union[str,NodeFilter]])->bool:
    """Assess each zone belong to the support (negative part of the level set)

    Parameters
    ----------
    levelset : LevelSet
        level set to check
    rootZones : Iterable[Union[str,NodeFilter]]
        collection of zones
    connectedZones : Iterable[Union[str,NodeFilter]]
        collection of zones

    Returns
    -------
    bool
        True if root zones intersect the support and connected zones belong to support 
    """
    for zoneFilter in rootZones:
        fix= zoneFilter.GetNodesIndices(mesh=support)
        zoneInBody = np.any(phi[fix] <= 0)
        if not zoneInBody:
            try:
                zoneFilterName = str(zoneFilter)
            except TypeError:
                zoneFilterName = str(type(zoneFilter))
            Info("Zones for "+zoneFilterName+" does not intersect the body.")
            return False
    for zoneFilter in connectedZones:
        fix= zoneFilter.GetNodesIndices(mesh=support)
        zoneInBody = np.all(phi[fix] <= 0)
        if not zoneInBody:
            try:
                zoneFilterName = str(zoneFilter)
            except TypeError:
                zoneFilterName = str(type(zoneFilter))
                Info("Zones for "+zoneFilterName+" is not contained in the body."\
                "Level set maximum value in zone is "+str(np.max(phi[fix])))
            return False
    return True

class SolverBase():
    """Base class for physical solvers"""
    def __init__(self):
        super(SolverBase,self).__init__()
        self.DomainToTreat = TZ.Inside3D
        self.auxiliaryFieldGeneration = defaultdict(OptionObjGenerator)
        self.auxiliaryScalarGeneration = dict()
        self.auxiliaryScalarsbyOrderGeneration=dict()

        self.tagsToKeep = []
        self.numberOfNodesOfTheOriginalSupport = 0

        #in the case of levelset with no conformal
        #not aplicable for conform
        self.narrow = 0.
        self.originalSupport = None
        self.cleanMesh = None
        self.minthreshold = -1.
        self.eVoid = 1.0e-3

        self.tagForPostProcess = ""
        self.workingDirectory = None
        
        self.useProblem = -1 

    def GetUniqueWorkingDirectory(self):
        if self.workingDirectory is None:
            tempPath=TemporaryDirectory.GetTempPath()
            self.workingDirectory = tempfile.mkdtemp(prefix="OpenPiscoPhysicalSolver",dir=tempPath,suffix="_safe_to_delete") + os.sep
        return self.workingDirectory

    def SetSupport(self,support):
        assert support is not None,"Please provide a computational mesh !"
        self.originalSupport = support

    def GetSupport(self):
        return self.originalSupport

    def PrepareComputationalSupport(self,levelset):
        self.conform = levelset.conform
        self.SetSupport(levelset.support)
        if self.conform:
            self.cleanMesh = ExtractInteriorMesh(levelset,self.tagsToKeep)
        else:
            self.cleanMesh = levelset.support
        self.cleanMeshToOriginalNodal = self.cleanMesh.originalIDNodes

    def PrepareComputationalSupportWithoutTags(self,levelset):
        self.conform = levelset.conform
        self.SetSupport(levelset.support)
        if self.conform:
            self.cleanMesh = ExtractInteriorMesh(levelset)
        else:
            self.cleanMesh = levelset.support
        self.cleanMeshToOriginalNodal = self.cleanMesh.originalIDNodes
        
    def ComputeDomainToTreatLs(self,levelSet:LevelSet):
        self.originalSupport = levelSet.support
        dimensionality = self.originalSupport.GetElementsDimensionality()

        if levelSet.conform :
            self.ApplyConformalTreatment(dimensionality,levelSet)
        else:
            self.ApplyNonConformalTreatment(dimensionality,levelSet)
        self.cleanMesh.ConvertDataForNativeTreatment()

    def ApplyConformalTreatment(self,dimensionality,levelSet):
        tokeep = [ self.DomainToTreat ]
        tokeep.extend(self.tagsToKeep)
        self.filterToCleanMesh = ElementFilter(eTag=tokeep)
        self.cleanMesh = ExtractInteriorMesh(levelSet,self.tagsToKeep)
        self.cleanMeshDensities = np.ones(self.cleanMesh.GetNumberOfElements(dim=dimensionality) )

        cpt = 0
        self.cleanMeshToOriginalElementary = np.zeros(self.cleanMesh.GetNumberOfElements(dim=dimensionality),dtype=int )
        for selection in ElementFilter(dimensionality=dimensionality)(self.cleanMesh):
            nbel = len(selection.elements.originalIds)
            self.cleanMeshToOriginalElementary[cpt:cpt+nbel] = selection.elements.originalIds+selection.meshOffset
            cpt += nbel

        self.cleanMeshToOriginalNodal = self.cleanMesh.originalIDNodes
        self.tagForPostProcess = self.DomainToTreat

    def ApplyNonConformalTreatment(self,dimensionality,levelSet):
        self.tagForPostProcess = TZ.Bulk
        if self.DomainToTreat == TZ.Inside3D:
            self.densities = levelSet.GetElementsVolumicFractions(levelSet.phi)
        elif self.DomainToTreat == TZ.Outside3D:
            self.densities = 1-levelSet.GetElementsVolumicFractions(levelSet.phi)
        else:
            raise ValueError("Dont know how to treat "+str(self.DomainToTreat))

        self.densities *= (1.-self.eVoid)
        self.densities += self.eVoid

        if self.narrow > 0 :
            phiAtCenter = PointToCellData(self.originalSupport, levelSet.phi, dim=dimensionality)
            if self.DomainToTreat == TZ.Outside3D:
                phiAtCenter *= -1.
            self.densities[phiAtCenter > self.narrow] = 0

        totalNumberOfElementInTargetMesh = 0
        totalNumberOfElement = 0
        for selection in ElementFilter(dimensionality=dimensionality)(self.originalSupport):

            mask = np.nonzero(self.densities[totalNumberOfElement:totalNumberOfElement+selection.elements.GetNumberOfElements()] > self.minthreshold  )[0]
            selection.elements.GetTag(TZ.Bulk).SetIds(mask)
            totalNumberOfElement += selection.elements.GetNumberOfElements()
            totalNumberOfElementInTargetMesh += len(mask)

        ttk = set(self.tagsToKeep)
        ttk.add(TZ.Bulk)
        ttk = list(ttk)

        if totalNumberOfElementInTargetMesh == totalNumberOfElement:
            self.filterToCleanMesh = ElementFilter()

            #no need to duplicate the mesh
            self.cleanMesh = self.originalSupport
            self.cleanMeshDensities = self.densities
            self.cleanMeshToOriginalElementary = np.arange(totalNumberOfElement)
            self.cleanMeshToOriginalNodal = np.arange(self.originalSupport.GetNumberOfNodes() )
        else:
            self.filterToCleanMesh = ElementFilter(eTag=ttk)
            self.cleanMesh = ExtractElementsByTags(self.originalSupport,ttk)
            self.cleanMeshDensities = np.zeros(self.cleanMesh.GetNumberOfElements(dim=dimensionality) )

            cpt = 0
            self.cleanMeshToOriginalElementary = np.zeros(self.cleanMesh.GetNumberOfElements(dim=dimensionality),dtype=int )
            for selection in ElementFilter(dimensionality=dimensionality)(self.cleanMesh):
                nbel = len(selection.indices)
                self.cleanMeshDensities[cpt:cpt+nbel] = self.densities[selection.elements.originalIds[selection.indices]]
                self.cleanMeshToOriginalElementary[cpt:cpt+nbel] = selection.elements.originalIds+selection.meshOffset
                cpt += nbel

            self.cleanMeshToOriginalNodal = self.cleanMesh.originalIDNodes

    def GetNodalSolution(self,i:int=0):
        """Retrieve the PDE solution at mesh nodes

        Parameters
        ----------
        i : int, optional
            solution index, by default 0

        Raises
        ------
        NotImplementedError
            to be redefined in derived class
        """
        raise NotImplementedError()

    def VerifyPhysicalDomainValidity(self,support:Mesh,phi:ArrayLike)->bool:
        """Verify physical domain validity:
            - boundary zone should be in body
            - boundary zones should be connected

        Parameters
        ----------
        levelSet : LevelSet
            problem level set

        Returns
        -------
        bool
            True if both conditions are fullfilled, else False
        """
        includeState = self.VerifyBoundaryZonesInBody(support,phi)
        if not includeState:
            Info("Boundary zones not all in body")
            return False
        boundaryConnected = self.VerifyBoundaryZonesConnected(support,phi)
        if not boundaryConnected:
            Info("Boundary zones in the void (not all connected)")
            return False
        return True

    def VerifyBoundaryZonesInBody(self,support:Mesh,phi:ArrayLike)->bool:
        """Verify boundary zones are fully contained in body

        Parameters
        ----------
        levelset : LevelSet
            problem level set

        Returns
        -------
        bool
            True if boundary zones in body, else False
        """
        rootZones,connectedZones = self.GetBoundaryConditionsFilters()
        includeState = AssessZonesAttachedToLevelset(support=support,phi=phi,rootZones=rootZones,connectedZones=connectedZones)
        return includeState

    def VerifyBoundaryZonesConnected(self,support:Mesh,phi:ArrayLike)->bool:
        """Verify boundary zones are connected

        Parameters
        ----------
        levelset : LevelSet
            problem level set

        Returns
        -------
        bool
            True if boundary zones are connected, else False
        """
        rootZones,connectedZones = self.GetBoundaryConditionsFilters()
        connectionState = AssessConnectedZones(support=support,phi=phi,rootZones=rootZones,connectedZones=connectedZones)
        return connectionState

    def GetBoundaryConditionsFilters(self)->Tuple[Iterable[NodeFilter],Iterable[NodeFilter]]:
        """Specify which pair of zones should be connected. If this method is not redefined in derived class, there is no check

        Returns
        -------
        Tuple[Iterable,Iterable]
            pair of zones to be checked
        """
        return [],[]
    
    def CheckComputationalSupportValidity(self):
        for BCFilter in self.GetBoundaryConditionsFilters():
            for zone in BCFilter:
                ids = zone.GetNodesIndices(mesh=self.cleanMesh)
                if not len(ids):
                   return False 
        return True 

    def GetAvailableIndices(self):
        return 1

    def GetAuxiliaryFieldNames(self):
        # this is the list of standart names
        return self.auxiliaryFieldGeneration.keys()

    def CanSupplyAuxiliaryField(self, name:str, on:FN=FN.Nodes)->bool:
        """Check whether solver can supply auxiliary field

        Parameters
        ----------
        name : str
            auxiliary field name
        on : FN, optional
            computational support of the field, by default FN.Nodes

        Returns
        -------
        bool
            True if solver can supply auxiliary field on location, else False
        """
        if name in self.auxiliaryFieldGeneration:
            if self.auxiliaryFieldGeneration[name][on] is None:
                return False
            else:
                return True
        else:
            return False

    def SetAuxiliaryFieldGeneration(self, name:str, on:FN=FN.Nodes, flag:bool=True):
        errorMessage = "This solver"+str(type(self))+" can not generate field: " +str(name) + " on " + str(on)
        assert self.CanSupplyAuxiliaryField(name,on),errorMessage
        self.auxiliaryFieldGeneration[name][on] = flag

    def GetAuxiliaryFieldGeneration(self, name:str, on:FN)->bool:
        """Check whether an auxiliary field is to be generated on computational support

        Parameters
        ----------
        name : str
            name of the field to be generated
        on : FN
            computational support of the field

        Returns
        -------
        bool
            True if the field is ti be generated on specified computational support, else False
        """
        if name in self.auxiliaryFieldGeneration:
            if self.auxiliaryFieldGeneration.get(name)[on]:
                return True
            else:
                return False
        else:
            return False

    def GetAuxiliaryField(self, name:str, on:FN = FN.Nodes,index:Optional[int]=None):
        """Retrieve an auxiliary field on the computational support specified by the keyword "on"

        Parameters
        ----------
        name : name of the field to be retrieved
            _description_
        on : FN, optional
            computational support of the field, by default FN.Nodes
        index : Optional[int], optional
            index of the field to be retrieved (if needed), by default None

        Raises
        ------
        NotImplementedError
            To be redefined in derived class if needed
        """
        raise NotImplementedError()

    def GetAuxiliaryScalarNames(self):
        """ this is the list of standard names. the prefix "int_" means the integral
        over the domain of a Auxiliary Field
        """
        res = {FN.volume,FN.mass,FN.int_strain_energy}
        res.update(self.auxiliaryScalarGeneration.keys())
        return res

    def CanSupplyAuxiliaryScalar(self, name):
        return name in self.auxiliaryScalarGeneration

    def SetAuxiliaryScalarGeneration(self, name, flag=True):
        assert self.CanSupplyAuxiliaryScalar(name),"This solver cant generate scalar " +str(name)
        self.auxiliaryScalarGeneration[name] = flag

    def GetAuxiliaryScalarGeneration(self, name):
        return self.auxiliaryScalarGeneration.get(name, False)

    def GetAuxiliaryScalar(self, name:str):
        """Retrieve an auxiliary scalar quantity

        Parameters
        ----------
        name : str
            name of the scalar to be retrieved

        Raises
        ------
        NotImplementedError
            To be redefined in the derived class
        """
        raise NotImplementedError()

    def ExtendingNodalFieldToOriginalSupport(self,field):
        supportOriginalNumNodes = self.originalSupport.GetNumberOfNodes()
        extendedSize = (supportOriginalNumNodes,field.shape[1]) if len(field.shape)==2 else (supportOriginalNumNodes)
        fieldextended = np.zeros( extendedSize,dtype=MuscatFloat)
        fieldextended[self.cleanMesh.originalIDNodes] = field
        return fieldextended

from Muscat.MeshTools.ConstantRectilinearMeshTools import CreateConstantRectilinearMesh
from Muscat.MeshTools.MeshCreationTools import CreateCube, CreateMeshOf
from Muscat.MeshTools.MeshTetrahedrization import Tetrahedrization
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshTools.MeshModificationTools import ComputeSkin
from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometryExternalSurface

def CheckIntegrity_SolverBase():
    obj = SolverBase()

    # THIS TYPE OF LINE MUST BE INSIDE THE __init__ of the derived class
    obj.auxiliaryFieldGeneration[FN.stress][FN.Nodes] = False

    print("obj.GetAuxiliaryFieldNames()")
    print(obj.GetAuxiliaryFieldNames())

    print("obj.CanSupplyAuxiliaryField(FN.stress,on=FN.Nodes)")
    print(obj.CanSupplyAuxiliaryField(FN.stress,on=FN.Nodes))

    print("obj.CanSupplyAuxiliaryField(FN.stress,on=FN.Centroids)")
    print(obj.CanSupplyAuxiliaryField(FN.stress,on=FN.Centroids))


    print("obj.GetAuxiliaryFieldGeneration(FN.stress,on=FN.Nodes)")
    print(obj.GetAuxiliaryFieldGeneration(FN.stress,on=FN.Nodes))

    obj.SetAuxiliaryFieldGeneration(FN.stress,on=FN.Nodes)

    print("obj.GetAuxiliaryFieldGeneration(FN.stress,on=FN.Nodes)")
    print(obj.GetAuxiliaryFieldGeneration(FN.stress,on=FN.Nodes))

    print("obj.GetAuxiliaryFieldGeneration(FN.stress,on=FN.Centroids)")
    print(obj.GetAuxiliaryFieldGeneration(FN.stress,on=FN.Centroids))

    print("obj.GetAuxiliaryFieldGeneration('toto',on=FN.Centroids)")
    print(obj.GetAuxiliaryFieldGeneration("toto",on=FN.Centroids))

    return 'ok'

def CheckIntegrity_ConnectedZonesConsistency():
    _,disconnectedMeshUnion,meshBox = GenerateTestMeshes()
    disconnectedZone = ImplicitGeometryExternalSurface(disconnectedMeshUnion)
    phi = disconnectedZone.ApplyVector(meshBox)
    connectedZones = AssessConnectedZones(support=meshBox,
                                          phi=phi,
                                          rootZones=[NodeFilter(eTag="X0")],
                                          connectedZones=[NodeFilter(eTag="X1")])
    assert not connectedZones, "Should be disconnected"
    connectedZone = ImplicitGeometryExternalSurface(meshBox)
    phi = connectedZone.ApplyVector(meshBox) - 1e-3
    connectedZones = AssessConnectedZones(support=meshBox,
                                          phi=phi,
                                          rootZones=[NodeFilter(eTag="X0")],
                                          connectedZones=[NodeFilter(eTag="X1")])
    assert connectedZones, "Should be connected"
    return "ok"

def CheckIntegrity_ZoneAttachedTobody():
    mesh1,_,meshBox = GenerateTestMeshes()
    attachedZone = ImplicitGeometryExternalSurface(meshBox)
    phi = attachedZone.ApplyVector(meshBox) - 1e-3
    for zoneFilter in [NodeFilter(eTag="X0"),NodeFilter(eTag="X1")]:
        zoneIndices = zoneFilter.GetNodesIndices(meshBox)
        assert np.all(phi[zoneIndices]<=0),"Zone should be in the body"
    attachedStatus = AssessZonesAttachedToLevelset(support=meshBox,
                                                   phi=phi,
                                                   rootZones=[NodeFilter(eTag="X0")],
                                                   connectedZones=[NodeFilter(eTag="X1")])
    assert attachedStatus, "Should be attached to body"

    attachedZone = ImplicitGeometryExternalSurface(mesh1)
    phi = attachedZone.ApplyVector(meshBox)
    zoneIndices = NodeFilter(eTag="X1").GetNodesIndices(meshBox)
    assert not np.all(phi[zoneIndices]<=0), "X1 tag should be in the body"
    attachedStatus = AssessZonesAttachedToLevelset(support=meshBox, 
                                                   phi=phi,
                                                   rootZones=[NodeFilter(eTag="X0")],
                                                   connectedZones=[NodeFilter(eTag="X1")])
    assert not attachedStatus, "Should not be attached to body"

    return "ok"

def GenerateTestMeshes():
    mesh1 = Tetrahedrization(CreateConstantRectilinearMesh(dimensions=[10,10,10],spacing=[1./8, 1./8, 1./8],origin=[0, 0, 0]))
    mesh2 = Tetrahedrization(CreateConstantRectilinearMesh(dimensions=[10,10,10],spacing=[1./8, 1./8, 1./8],origin=[2, 0, 0]))
    points = np.vstack((mesh1.nodes,mesh2.nodes))
    connectivityMesh1 = mesh1.GetElementsOfType(ED.Tetrahedron_4).connectivity
    connectivityMesh2 = mesh2.GetElementsOfType(ED.Tetrahedron_4).connectivity+mesh1.GetNumberOfNodes()
    connectivityMesh = np.vstack((connectivityMesh1,connectivityMesh2))
    disconnectedMeshUnion = CreateMeshOf(points,connectivityMesh,ED.Tetrahedron_4)
    meshBox = CreateCube(dimensions=[26,10,10],spacing=[1./8, 1./8, 1./8],origin=[0, 0, 0],ofTetras=True)

    return mesh1,disconnectedMeshUnion,meshBox

def CheckIntegrity():
    totest = [
        CheckIntegrity_SolverBase,
        CheckIntegrity_ConnectedZonesConsistency,
        CheckIntegrity_ZoneAttachedTobody
              ]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity())
