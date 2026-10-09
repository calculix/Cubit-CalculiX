# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module provides classical geometric criteria in the context of topology optimisation
"""
from __future__ import annotations
from typing import Optional

import numpy as np

import Muscat.Helpers.ParserHelper as PH
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags, ExtractElementsByElementFilter
from Muscat.MeshTools.MeshInspectionTools import GetVolume
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter as ElementFilter
from Muscat.MeshContainers.Filters.FilterOperators import DifferenceFilter
from Muscat.MeshTools.MeshModificationTools import CleanLonelyNodes
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.FE.IntegrationRules import GetIntegrationRuleByName
from Muscat.FE.Fields.IPField import IPField
from Muscat.ImplicitGeometry.ImplicitGeometryTools import DistanceToSurface
from Muscat.Types import MuscatFloat

from OpenPisco.BaseElements import elementsTypeSupportedForDimension
from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import CriteriaBase
from OpenPisco.LevelSetBase import LevelSetBase
from OpenPisco.Unstructured.LevelsetTools import ComputeMeanCurvature
from OpenPisco.MuscatExtentions.IPFieldIntegration import IPFieldIntegration
from OpenPisco.PhysicalSolvers.PhysicalSolversTools import IpToNodesData
import OpenPisco.TopoZones as TZ

from OpenPisco import RETURN_SUCCESS

class TopoCriteriaVolume(CriteriaBase):
    """Volume criterion"""
    def __init__(self, other:Optional[TopoCriteriaVolume]=None):
        super(TopoCriteriaVolume, self).__init__(other)

        self.volume = 0.0
        self.npoints = 0
        self.tagsToKeep = [""]
        if other is None:
            self.SetName("Volume")
            self.dVolumedOmega = np.array(0)
            self.point = None
        else:
            self.volume = other.volume
            self.npoints = other.npoints
            self.point = other.point
            self.dVolumedOmega = other.dVolumedOmega.copy()

    def GetNumberOfSolutions(self)->int:
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions, here 0 as there is no extra field of interest
        """
        return 0

    def UpdateValuesUsingPhi(self,levelSet:LevelSetBase,phi:np.ndarray)->int:
        """Update criterion based on the levelset and the levelset value (phi)

        Parameters
        ----------
        levelSet : LevelSetBase
            levelset object, only the underlying support of the levelset is used.
        phi : np.ndarray
            levelset function value

        Returns
        -------
        int
            parameter indicating if the criterion update was successfull (typically 0)
        """
        from OpenPisco.Unstructured.Levelset import LevelSet
        ls = LevelSet()
        ls.conform = False
        ls.phi = phi
        ls.support = levelSet.support
        return self.UpdateValues(ls)

    def UpdateValues(self,levelSet:LevelSetBase)->int:
        r"""Update value and sensitivity of the criterion for the current levelset

        J(\Omega) = \int_{\Omega}{ dx}

        Parameters
        ----------
        levelSet : LevelSetBase
            levelset object

        Returns
        -------
        int
            parameter indicating if the criterion update was successfull (typically 0)
        """
        self.npoints = levelSet.phi.size
        #if the mesh changes then we recompute the mass matrix
        if self.point is not None:
            if levelSet.support is not self.point.support:
                self.M = None

        self.point = levelSet

        if levelSet.conform :
            tokeep = [ TZ.Inside3D ]
            tokeep.extend(self.tagsToKeep)
            self.cleanmesh = ExtractElementsByTags(levelSet.support,tokeep)
            self.volume = GetVolume(self.cleanmesh)
        else:
            self.volume = levelSet.GetVolumeOfNegativePartbyElement(levelSet.phi).sum()

        return RETURN_SUCCESS

    def GetValue(self)->float:
        """Get the current value of the criterion

        Returns
        -------
        float
            current value of the criterion
        """
        return self.volume

    def GetSensitivity(self)->np.ndarray:
        """Get the current value of the sensitivity
        The sensitivity is straightforward to establish. 
        In order to increase the volume, one simply have to move the interface homogeneously in the outward direction

        Returns
        -------
        np.ndarray
            current value of the sensitivity
        """
        return np.ones(self.npoints)

RegisterCriteriaClass("Volume", TopoCriteriaVolume)

class TopoCriteriaSurface(CriteriaBase):
    def __init__(self, other=None):
        super(TopoCriteriaSurface, self).__init__(other)

    def GetNumberOfSolutions(self):
        return 0

    def UpdateValuesUsingPhi(self,levelSet,phi):
        from OpenPisco.Unstructured.Levelset import LevelSet
        ls = LevelSet()
        ls.conform = False
        ls.phi = phi
        ls.support = levelSet.support
        return self.UpdateValues(ls)

    def UpdateValues(self,levelSet):
        self.point = levelSet

        surfaceMesh = ExtractElementsByTags(levelSet.support,[TZ.InterSurf])
        assert surfaceMesh.GetNumberOfElements()!=0, "Extracted surface is empty"
        self.f_val = GetVolume(surfaceMesh)
        self.fSensitivity_val = ComputeMeanCurvature(levelSet)
        
        return RETURN_SUCCESS

RegisterCriteriaClass("Surface", TopoCriteriaSurface)

def CreateTopoCriteriaCenterOfMassDeviation(ops):
    res=TopoCriteriaCenterOfMassDeviation()
    res.centerOfMassReference = PH.ReadFloats(ops["centerOfMassReference"])
    res.computeOnInsideDomain = PH.ReadBool(ops["computeOnInsideDomain"])
    return res

class TopoCriteriaCenterOfMassDeviation(CriteriaBase):
    def __init__(self, other=None):
        super(TopoCriteriaCenterOfMassDeviation, self).__init__(other)
        self.volume = 0

        if other is None:
            self.SetName("CenterOfMassDeviation")
            self.centerOfMassReference = None
            self.centerOfMass = None
            self.computeOnInsideDomain = True 
        else:
            self.centerOfMassReference = other.centerOfMassReference
            self.centerOfMass = other.centerOfMass
            self.computeOnInsideDomain = other.computeOnInsideDomain

    def UpdateValues(self,levelSet):
        self.f_val = self.ComputeObjective(levelSet)
    
        self.fSensitivity_val = self.ComputeSensitivity(levelSet)
        return RETURN_SUCCESS
    
    def ComputeObjective(self,levelSet):
        """ComputeObjective

        J(\\Omega) = \\| (\\int_\\Omega x dx) / |\\Omega| - centerOfMassReference\\|^2
        """
        volCriteria=TopoCriteriaVolume()
        if self.computeOnInsideDomain:
            phi = levelSet.phi
        else:
            phi = -levelSet.phi
        volCriteria.UpdateValuesUsingPhi(levelSet=levelSet,phi=phi)
        self.volume = volCriteria.GetValue()
        assert self.volume>0, "Volume should be greater than 0"

        mesh = levelSet.support
        dimensionality = mesh.GetPointsDimensionality()
        bodyElements = mesh.GetElementsOfType(elementsTypeSupportedForDimension[dimensionality])
        centroidsVol = np.mean(mesh.nodes[bodyElements.connectivity],axis=1)
        if self.computeOnInsideDomain:
            eTag = TZ.Inside3D
        else:
            eTag = TZ.Outside3D
        ff = ElementFilter(dimensionality=dimensionality,eTag=eTag)
        nIndices = ff.GetNodesIndices(mesh).shape[0]
        assert nIndices>0, eTag+" is empty"
        centerOfMass = np.zeros(dimensionality)
        coordinatesByDimensionality={2:["x","y"],3:["x","y","z"]}
        for axisId,axisName in enumerate(coordinatesByDimensionality[dimensionality]):
            centroidsVolAlongAxis = centroidsVol[:,axisId]
            tointegrate= IPField("CenterOfMass_Component"+axisName,mesh=mesh,rule=GetIntegrationRuleByName("ElementCenter"))
            tointegrate.Allocate()
            tointegrate.SetDataFromNumpy(centroidsVolAlongAxis.flatten(),ff)
            centerOfMass[axisId] = IPFieldIntegration(tointegrate, ff)
            centerOfMass[axisId]/=self.volume
        self.centerOfMass = centerOfMass
        return np.linalg.norm(self.centerOfMass - self.centerOfMassReference)**2

    def ComputeSensitivity(self,levelSet):
        """ComputeSensitivity

        centerOfMass = (\\int_\\Omega x dx) / |\\Omega|
        J'(\\Omega) = 2 (\\int_{\\partial Omega} (s - centerOfMass) ds) \\cdot (centerOfMass - self.centerOfMassReference) /|\\Omega|
        """
        mesh = levelSet.support
        dimensionality = mesh.GetPointsDimensionality()
        assert dimensionality in [2,3],"Can only handle dimension 2 or 3"
        skinElements = mesh.GetElementsOfType(elementsTypeSupportedForDimension[dimensionality-1])
        centroidsSurf = np.mean(mesh.nodes[skinElements.connectivity],axis=1)
        centerOfMass = self.GetCenterOfMass()
        centerOfMassDerivative = (centroidsSurf - centerOfMass)/self.volume
        sensitivityAtTria = 2*centerOfMassDerivative.dot(centerOfMass - self.centerOfMassReference)
        sensitivityIp= IPField("sensitivity",mesh=mesh,rule=GetIntegrationRuleByName("ElementCenter"))
        sensitivityIp.Allocate()
        interfaceTagName= TZ.InterZoneByDim[dimensionality]
        ff = ElementFilter(eTag=interfaceTagName)
        nIndices = ff.GetNodesIndices(mesh).shape[0]
        assert nIndices>0, "InterSurf is empty"
        sensitivityIp.SetDataFromNumpy(sensitivityAtTria.flatten(),elementFilter=ff)
        sensitivityAtNodes = IpToNodesData(IPfield=sensitivityIp,dim=dimensionality-1)
        return sensitivityAtNodes

    def GetCenterOfMass(self):
        return self.centerOfMass

RegisterCriteriaClass("CenterOfMassDeviation", TopoCriteriaCenterOfMassDeviation,CreateTopoCriteriaCenterOfMassDeviation)

class TopoCriteriaMass(TopoCriteriaVolume):
    def __init__(self, other:Optional[TopoCriteriaMass]=None):
        super(TopoCriteriaMass, self).__init__(other)
        #Replacing composition by inheritance or define criterion as weighted TopoCriteriaVolume sounds like a better idea here.

        if other is None:
            self.density = 1.
            self.SetName("Mass")
        else:
            self.density = other.density

    def GetValue(self):
        """Get the current value of the criterion (almost equivalent to volume criterion)

        Returns
        -------
        float
            current value of the criterion
        """
        return super(TopoCriteriaMass, self).GetValue()*self.density

    def GetSensitivity(self):
        return super(TopoCriteriaMass, self).GetSensitivity()*self.density

RegisterCriteriaClass("Mass", TopoCriteriaMass)

class TopoCriteriaContactSurfacePenalization(CriteriaBase):
    def __init__(self, other:Optional[TopoCriteriaContactSurfacePenalization]=None):
        super(TopoCriteriaContactSurfacePenalization, self).__init__(other)
        if other is None:
            self.SetName("ContactSurfacePenalization")
            self.eTags = np.zeros(0,dtype=str)
            self.length = 1e-3
            self.overhangTag = None
            self.surfMesh = None
            self.dist = None
        else:
            self.eTags = PH.ReadStrings(other.eTags)
            self.length = other.length
            self.overhangTag = other.overhangTag
            self.dist = other.dist

    def GetNumberOfSolutions(self)->int:
        """Get number of solutions

        Returns
        -------
        int
            number of solutions, here 0 as there is no extra field of interest
        """
        return 1

    def GetSolution(self,i)->np.ndarray:
        """Get criteria solution, here the contact area

        Parameters
        ----------
        i : int
            solution to retrieve index, single solution here

        Returns
        -------
        np.ndarray
            solution value
        """
        self.AssertSuitableSolutionId(i)
        return self.contactarea

    def GetSolutionName(self,i:int)->str:
        """Get criteria solution name, here "ContactArea"

        Parameters
        ----------
        i : int
            solution index, single solution here

        Returns
        -------
        str
            solution name
        """
        self.AssertSuitableSolutionId(i)
        return "ContactArea"

    def ComputeDistanceSurface(self,levelSet:LevelSetBase):
        """Compute distance to surface

        Parameters
        ----------
        levelSet : LevelSetBase
            Surface description provided by the levelset
        """
        fil = ElementFilter(dimensionality=[levelSet.support.GetElementsDimensionality()-1],eTag=self.eTags)
        if self.overhangTag is not None:
            oTfil = ElementFilter(dimensionality=[levelSet.support.GetElementsDimensionality()-1],eTag=self.overhangTag)
            fil = DifferenceFilter(filters=[fil,oTfil])

        surfMesh = ExtractElementsByElementFilter(levelSet.support, fil, copy=False)
        CleanLonelyNodes(surfMesh)
        surfMesh.PrepareForOutput()
        self.dist = DistanceToSurface(levelSet.support, surfMesh )

    def UpdateValues(self,levelSet:LevelSetBase)->int:
        """Update value and sensitivity of the criterion for the current levelset

        Parameters
        ----------
        point : LevelSetBase
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        self.fSensitivity_val = np.zeros(levelSet.support.GetNumberOfNodes() ,dtype=MuscatFloat)
        nodeMask = np.zeros(levelSet.support.GetNumberOfNodes(),dtype=bool)

        if self.dist is None:
            self.ComputeDistanceSurface(levelSet)

        nodeMask[self.dist<self.length] = True
        nodeMask[levelSet.phi >= self.length] = False

        self.contactarea = np.abs(self.dist - self.length)
        self.contactarea[nodeMask] = -self.dist[nodeMask]

        self.f_val = levelSet.GetVolumeOfNegativePartbyElement(self.contactarea).sum()/(self.length)
        self.fSensitivity_val[nodeMask] = 1./(self.length)

        return RETURN_SUCCESS

RegisterCriteriaClass("ContactSurfacePenalization", TopoCriteriaContactSurfacePenalization)


def CheckIntegrityVolume(GUI=False):
    criterion=TopoCriteriaVolume()

    from OpenPisco.Structured.Levelset3D  import LevelSet3D
    from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometrySphere
    from Muscat.IO.XdmfWriter import XdmfWriter
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    from Muscat.MeshTools.ConstantRectilinearMeshTools import CreateConstantRectilinearMesh

    writer = XdmfWriter(TemporaryDirectory.GetTempPath()+'Test_GeoCriteria.xmf')
    writer.SetTemporal()
    writer.SetBinary()
    writer.SetXmlSizeLimit(0)
    writer.Open()

    n = 11
    myMesh = CreateConstantRectilinearMesh(dimensions=[n,n,n],spacing=[20./(n-1), 20./(n-1), 20./(n-1)], origin=[-10.,-10.,-10] )
    ls = LevelSet3D(support=myMesh)

    r = 7.
    sp = ImplicitGeometrySphere()
    sp.SetCenter([0.,0.,0.])
    sp.SetRadius(r)
    ls.Initialize(sp )
    criterion.UpdateValues(ls)

    #theorical volume, surface
    tv = 4./3.*np.pi*r**3
    ts = 4.*np.pi*r**2
    volume = criterion.GetValue()
    print("Volume : {0:.3f},  {1:.3f}, error: {2:.3f}%".format(tv,volume,(tv-volume)/tv*100) )

    one = np.ones(len(ls.phi))
    dvdphi = criterion.GetSensitivity()
    surface =  ls.InterfaceIntegral(one)
    print("surface  : {0:.3f},  {1:.3f}, error: {2:.3f}% ".format(ts,surface,(ts-surface)/ts*100) )
    from OpenPisco.Structured.Laplacian3D import ConstantMassMatrix
    M = ConstantMassMatrix(myMesh)
    dirac = ls.InterfaceRestriction(ls.phi).flatten()
    surface2 = np.dot(dvdphi,M*dirac)
    print("surfac 2 : {0}".format(surface2) )
    surface2 = np.dot(dvdphi,np.sum(M,axis=1))
    print("surfac 2 : {0}".format(surface2))

    speed = ls.Regularize(dvdphi,lengthscaleParameter=1)
    writer.Write(myMesh,
            PointFields     = [ls.phi,sp(myMesh.nodes),dirac,dvdphi,speed],
            PointFieldsNames= ['phi','phiExact',"dirac",'dvdphi',"speed"],
            TimeStep = 1
            )

    print("After transport")

    dt = 0.24
    tv = 4./3.*np.pi*(r+dt)**3
    ts = 4.*np.pi*(r+dt)**2

    tvOrdre1 = 4./3.*np.pi*(r)**3 + dt * (4.*np.pi*(r)**2)
    tsOrdre1 = 4.*np.pi*(r)**2 + dt*(8*np.pi*r)
    ff = ls.phi.ravel() - dt * (speed.ravel())

    tvOrdre1Numeric  = ls.GetVolumeOfNegativePartbyElement(ff).sum()
    print("tvOrdre1Numeric  : ", tvOrdre1Numeric)
    tvOrdre1Numberic2 = tv+ls.InterfaceIntegral(speed)*dt
    print("tvOrdre1Numberic2 : ", tvOrdre1Numberic2)

    ls.TransportAndReinitialize(speed,dt)
    criterion.UpdateValues(ls)

    volume = criterion.GetValue()
    dvdphi = criterion.GetSensitivity()
    speed = ls.Regularize(dvdphi,lengthscaleParameter=1.)

    surface =  ls.InterfaceIntegral(one)

    print("volume : {0:.3f}, {1:.3f}, {2:.3f}".format(tv,tvOrdre1,volume))
    print("vError : {0}%, {1:.3f}%, {2:.3f}%".format(0.000,(tv-tvOrdre1)/tv*100,(tv-volume)/tv*100))
    print("surface  : {0:.3f}, {1:.3f}, {2:.3f} ".format(ts,tsOrdre1,surface))
    print("surface  : {0:.3f}, error:  {1:.3f}, {2:.3f}% ".format(ts,(ts-tsOrdre1)/ts*100,(ts-surface)/ts*100))

    sp.SetRadius(r+dt)

    dirac = ls.InterfaceRestriction(ls.phi).flatten()
    writer.Write(myMesh,
            PointFields     = [ls.phi,sp(myMesh.nodes),dirac,dvdphi,speed],
            PointFieldsNames= ['phi','phiExact','dirac','dvdphi',"speed"],
            TimeStep = 1
            )
    writer.Close()

    return "OK"

def CheckIntegritySurface(GUI=False):
    ls = PrepareConformLs()
    c=TopoCriteriaSurface()
    c.UpdateValues(ls)
    surface = c.GetValue()
    assert (6.-surface)/6*100<1, "Difference between true area and computed area should be lower than 1%"
    return "OK"

def CheckIntegrityUpdateValuesUsingPhi(GUI=False):
    ls = PrepareConformLs()
    volCriteria=TopoCriteriaVolume()
    volCriteria.UpdateValuesUsingPhi(levelSet=ls,phi=ls.phi)
    computedVolume = volCriteria.GetValue()
    assert 100*(1.-computedVolume)<1, "Difference between true volume and computed volume should be lower than 1%"
    return "ok"

def PrepareConformLs():
    from Muscat.MeshTools.MeshCreationTools import CreateCube
    from OpenPisco.Unstructured.Levelset import LevelSet
    mesh = CreateCube(dimensions=[2,2,2],spacing=[1., 1., 1.],origin=[0, 0, 0],ofTetras=True)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.InterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    return ls 

def CheckIntegrityCenterOfMass(GUI=False):
    ls = PrepareConformLs()
    criterion = TopoCriteriaCenterOfMassDeviation()
    criterion.centerOfMassReference = 0.5*np.ones(3)
    criterion.UpdateValues(ls)
    criterionValue = criterion.GetValue()
    assert criterionValue<1e-6, "center of mass not consistent, expected almost 0, got "+str(criterionValue)
    criterionSensitivity = criterion.GetSensitivity()
    assert np.abs(np.sum(criterionSensitivity))<1e-6, "Sensitivity should only exist at InterfaceSurf"
    return "OK"

def CheckIntegrity(GUI=False):
    from Muscat.Helpers.CheckTools import RunListOfCheckIntegrities
    return RunListOfCheckIntegrities([CheckIntegrityVolume,CheckIntegritySurface,CheckIntegrityUpdateValuesUsingPhi,CheckIntegrityCenterOfMass])

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
