# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module is dedicated to thickness criteria in the context of topology optimisation
"""
from __future__ import annotations

from typing import Optional,Iterable

from attr import field
import numpy as np

from Muscat.Types import MuscatFloat
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Mesh import Mesh
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshTools.MeshInspectionTools import GetVolumePerElement, ComputeMeshMinMaxLengthScale
from Muscat.MeshTools.MeshFieldOperations import PointToCellData
from Muscat.MeshTools.TransferBackEnds.NativeTransfer import NativeTransfer

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import CriteriaBase
import OpenPisco.TopoZones as TZ
from OpenPisco.Unstructured.LevelsetTools import ComputeGradientOnTetrahedrons, ComputeMeanCurvature, ComputeGaussianCurvature
from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco import RETURN_SUCCESS


def GetInterpolationOperator(mesh: Mesh) -> NativeTransfer:
    """Create a interpolator for point data

    Parameters
    ----------
    inputsuppmeshort : Mesh
        input mesh

    Returns
    -------
    NativeTransfer
        a NativeTransfer class with the following API

        nt.SetTargetPoints(interpolationPoints)
        nt.Compute()
        interpolationOperator = nt.GetOperator()
        interpolationStatus = nt.GetStatus()
        phiAtIntPoints = interpolationOperator.dot(PointField)
    """

    from Muscat.FE.FETools import PrepareFEComputation
    from Muscat.FE.Fields.FEField import FEField

    nt = NativeTransfer()
    nt.SetVerbose(False)
    nt.SetTransferMethod("Interp/Clamp")
    spaces, numberings, _, _ = PrepareFEComputation(mesh, numberOfComponents=1)
    field = FEField(name="", mesh=mesh, space=spaces, numbering=numberings[0])
    nt.SetSourceFEField(field, None)
    return nt


class TopoCriteriaMaxThickness(CriteriaBase):
    def __init__(self, other:Optional[TopoCriteriaMaxThickness]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaMaxThickness], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaMaxThickness, self).__init__(other)
        self.update = True
        self.f_val = 0.0
        self.fsensitivity_val = 0.0
        self.dmax = 0.0
        self.jacobianfree = True
        self.mean = None
        self.gaussian = None

        if other is None:
            self.SetName("MaxThickness")

    def SetMaxThick(self, maxthick:float):
        """Set maximum thickness value

        Parameters
        ----------
        maxthick : float
            maximum thickness value
        """
        self.dmax = maxthick

    def ComputeObjective(self, levelset:LevelSet)->float:
        r"""Compute criterion objective value

        J(\Omega) = \int_{\Omega}{\left[ \left( d_{\Omega} (x) + \frac{d_{max}}{2} \right)^{-}\right]^2 \:dx}

        Parameters
        ----------
        levelset : LevelSet
            levelset object

        Returns
        -------
        float
            criterion objective value
        """
        vol = GetVolumePerElement(levelset.support, ElementFilter(elementType=ED.Tetrahedron_4))
        fieldAtCentroids = PointToCellData(levelset.support, levelset.phi, dim=3)
        fieldAtCentroids = np.minimum(fieldAtCentroids + 0.5 * self.dmax, 0.0)
        return np.sum(fieldAtCentroids**2 * vol, axis=0)

    def ComputeLayerContributionToRayIntegral(self, data:np.ndarray, ids:Iterable[int])->np.ndarray:
        """The original idea consists in decomposing a volume integral into a surface integral 
        along the boundary of the shape and an integral along the rays emerging from it.
        This method computes the integrand of the integral over the surface

        Parameters
        ----------
        data : np.ndarray
            levelset function at integration points
        ids : Iterable[int]
            integration points id on surface

        Returns
        -------
        np.ndarray
            surface integral integrand
        """
        toIntegrate = 2 * np.fmin(data + 0.5 * self.dmax, 0.0)
        if not self.jacobianfree:
            surfJacobian = 1 + 2 * data * self.mean[ids] + data**2 * self.gaussian[ids]
            toIntegrate *= surfJacobian
        return toIntegrate

    def ComputeSensitivity(self, levelset:LevelSet)->np.ndarray:
        """Compute criterion sensitivity

        Parameters
        ----------
        levelset : LevelSet
            levelset object

        Returns
        -------
        np.ndarray
            criterion sensitivity
        """
        assert levelset.conform,"MaxThickness criterion : unable to compute sensitivity in non conformal setting "
        res = np.zeros(levelset.support.GetNumberOfNodes(), dtype=MuscatFloat)

        if not self.jacobianfree:
            self.mean = ComputeMeanCurvature(levelset)
            self.gaussian = ComputeGaussianCurvature(levelset)

        tris = levelset.support.GetElementsOfType(ED.Triangle_3)
        surfconn = tris.connectivity[tris.tags[TZ.InterSurf].GetIds(), :]
        surfids = np.unique(surfconn.flatten())
        surfPoints = levelset.support.nodes[surfids]
        delta = ComputeMeshMinMaxLengthScale(levelset.support)[0]
        normal = ComputeGradientOnTetrahedrons(levelset.support, levelset.phi)
        intPoints = surfPoints - 0.5 * delta * normal[surfids]

        nt = GetInterpolationOperator(levelset.support)
        nt.SetTargetPoints(intPoints)
        nt.Compute()
        phiAtIntPoints = nt.GetOperator().dot(levelset.phi)

        toIntegrate = self.ComputeLayerContributionToRayIntegral(phiAtIntPoints, surfids)
        res[surfids] += delta * toIntegrate

        maxcpt = 500
        cpt = 1
        coef = np.ones_like(phiAtIntPoints)

        while True:
            coef.fill(1.0)
            intPoints = surfPoints - (cpt + 0.5) * delta * normal[surfids]
            nt.SetTargetPoints(intPoints)
            nt.Compute()
            newphiAtIntPoints = nt.GetOperator().dot(levelset.phi)

            if (newphiAtIntPoints >= phiAtIntPoints).all() or cpt == maxcpt:
                break
            coef[newphiAtIntPoints >= phiAtIntPoints] = 0.0

            toIntegrate = self.ComputeLayerContributionToRayIntegral(newphiAtIntPoints, surfids)
            res[surfids] += coef * delta * toIntegrate

            phiAtIntPoints = np.copy(newphiAtIntPoints)
            cpt += 1

        return -res

    def UpdateValues(self, point:LevelSet)->int:
        """Update value and sensitivity of the criterion for the current levelset

        Parameters
        ----------
        point : LevelSetBase
            current level set

        Returns
        -------
        int
            parameter indicating if the criterion update was successfull (typically 0)
        """
        if self.update:
            self.f_val = self.ComputeObjective(point)
            self.fsensitivity_val = self.ComputeSensitivity(point)
        return RETURN_SUCCESS

    def GetNumberOfSolutions(self)->int:
        """Get number of solutions

        Returns
        -------
        int
            number of solutions, here 0 as there is no extra field of interest
        """
        return 0


RegisterCriteriaClass("MaxThickness", TopoCriteriaMaxThickness)


class TopoCriteriaMinThickness(CriteriaBase):
    """Minimal thickness criterion"""
    def __init__(self, other=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaMinThickness], optional
             another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaMinThickness, self).__init__(other)
        self.update = True
        self.f_val = 0.0
        self.fsensitivity_val = 0.0
        self.method = "EnergyFormulation" # default was "EnergyFormulation", other option is "Original"
        self.jacobianfree = True  # only used with method energy formulation
        self.mean = None
        self.gaussian = None
        self.dmin = 0.005 #must be implemented in XML
        if other is None:
            self.SetName("MinThickness")

    def SetMinThick(self, minthick:float):
        """Set minimum thickness value

        Parameters
        ----------
        minthick : float
            minimum thickness value
        """
        self.dmin = minthick

    def UpdateValues(self, point:LevelSet)->int:
        """Update value and sensitivity of the criterion for the current levelset

        There are 2 approaches available. The classical one is classic. 
        There is also an alternative based on the minimization of energy functional.
        The underlying intuition is that it will hinder the creation of thin features in the structure by adding a penalization term within the objective function.
        For more details, we refer to
        https://hal.archives-ouvertes.fr/hal-00985000v2/document
        Sect. 9.2.2
        
        Parameters
        ----------
        point : LevelSetBase
            current level set

        Returns
        -------
        int
            parameter indicating if the criterion update was successfull (typically 0)

        Raises
        ------
        ValueError
            if criteria method chosen is neither the original one ("Original") ore the energy formulation ("EnergyFormulation")
        """
        if self.update:
            if self.method == "EnergyFormulation":
                self.f_val = self.ComputeObjectiveEnergyFormulation(point)
                self.fsensitivity_val = self.ComputeSensitivityEnergyFormulation(point)
            elif self.method == "Original":
                self.f_val = self.ComputeObjective(point)
                self.fsensitivity_val = self.ComputeSensitivity(point)
            else:
                raise ValueError("Unknown method " + str(self.method)+", only choices are EnergyFormulation and Original")
        return RETURN_SUCCESS

    def ComputeObjective(self, levelset:LevelSet)->float:
        r"""Compute criterion objective

        J(\Omega) = \int_{ \partial \Omega}{ \int_{0}^{d_{min}}{ \left[ ( d_{\Omega}(s - z n(s)))^+ \right]^2 } \:dz \:dx}

        Parameters
        ----------
        levelset : LevelSet
            levelset object

        Returns
        -------
        float
            criterion objective value
        """
        tris = levelset.support.GetElementsOfType(ED.Triangle_3)
        tag = tris.tags[TZ.InterSurf]
        surfconn = tris.connectivity[tag.GetIds(), :]
        surfids = np.unique(surfconn.flatten())
        surfPoints = levelset.support.nodes[surfids]
        delta = ComputeMeshMinMaxLengthScale(levelset.support)[0]
        N = int(self.dmin / delta)
        field = np.zeros(levelset.support.GetNumberOfNodes(), dtype=MuscatFloat)
        normal = ComputeGradientOnTetrahedrons(levelset.support, levelset.phi)
        nt = GetInterpolationOperator(levelset.support)

        for i in range(N):
            intPoints = surfPoints - (0.5 + i) * delta * normal[surfids]
            nt.SetTargetPoints(intPoints)
            nt.Compute()
            phiAtIntPoints = nt.GetOperator().dot(levelset.phi)
            field[surfids] += delta * np.maximum(phiAtIntPoints, 0.0) ** 2
        return levelset.InterfaceIntegral(field)

    def ComputeSensitivity(self, levelset:LevelSet)->np.ndarray:
        """Compute criterion sensitivity (only works in conformal setting)

        Parameters
        ----------
        levelset : LevelSet
            levelset object

        Returns
        -------
        np.ndarray
            criterion sensitivity value
        """
        assert levelset.conform,"MinThickness criterion : unable to compute sensitivity in non conformal setting "
        tris = levelset.support.GetElementsOfType(ED.Triangle_3)
        tag = tris.tags[TZ.InterSurf]
        surfconn = tris.connectivity[tag.GetIds(), :]
        surfids = np.unique(surfconn.flatten())
        surfPoints = levelset.support.nodes[surfids]
        delta = ComputeMeshMinMaxLengthScale(levelset.support)[0]
        N = int(self.dmin / delta)
        field = np.zeros(levelset.support.GetNumberOfNodes(), dtype=MuscatFloat)
        normal = ComputeGradientOnTetrahedrons(levelset.support, levelset.phi)
        kappa = ComputeMeanCurvature(levelset)
        nt = GetInterpolationOperator(levelset.support)

        for i in range(N):
            intPoints = surfPoints - (0.5 + i) * delta * normal[surfids]
            nt.SetTargetPoints(intPoints)
            nt.Compute()
            phiAtIntPoints = nt.GetOperator().dot(levelset.phi)
            gradPhiAtIntPoints = nt.GetOperator().dot(normal)
            gradProduct = np.sum(normal[surfids] * gradPhiAtIntPoints, axis=1)
            field[surfids] += np.maximum(phiAtIntPoints, 0.0) * (kappa[surfids] * np.maximum(phiAtIntPoints, 0.0) + 2 * gradProduct)

        return -field


    def ComputeLayerContributionToRayIntegral(self, data:np.ndarray, ids:Iterable[int])->np.ndarray:
        """The original idea consists in decomposing a volume integral into a surface integral 
        along the boundary of the shape and an integral along the rays emerging from it.
        This method computes the integrand of the integral over the surface

        Parameters
        ----------
        data : np.ndarray
            levelset function at integration points
        ids : Iterable[int]
            integration points id on surface

        Returns
        -------
        np.ndarray
            surface integral integrand
        """

        #toIntegrate = 2 * data * (1 + data) * np.fmax(data + 0.5 * self.dmin, 0.0)
        #wrong ? see eq. (3.31) Michailidis
        toIntegrate = 2 * data * (np.fmax(data + 0.5 * self.dmin, 0.0) + data) * np.fmax(data + 0.5 * self.dmin, 0.0)
        if not self.jacobianfree:
            surfJacobian = 1 + 2 * data * self.mean[ids] + data**2 * self.gaussian[ids]
            toIntegrate *= surfJacobian
        return toIntegrate

    def ComputeObjectiveEnergyFormulation(self, levelset:LevelSet)->float:
        """Compute criterion objective for the energy formulation method

        Parameters
        ----------
        levelset : LevelSet
            levelset object

        Returns
        -------
        float
            criterion objective value
        """
        vol = GetVolumePerElement(levelset.support, ElementFilter(elementType=ED.Tetrahedron_4))
        #integrate over the domain omega where d<0 only !  
        indicator = (PointToCellData(levelset.support, levelset.phi, dim=3) < 0).astype(int) #1 if phi<0, else 0
        fieldAtCentroids = PointToCellData(levelset.support, levelset.phi, dim=3)
        fieldAtCentroids = np.fmax(fieldAtCentroids + 0.5 * self.dmin, 0.0) * fieldAtCentroids
        return -np.sum(fieldAtCentroids**2 * vol * indicator, axis=0)

    def ComputeSensitivityEnergyFormulation(self, levelset:LevelSet)->np.ndarray:
        """Compute criterion sensitivity for the energy formulation method (only works in conformal setting)

        Parameters
        ----------
        levelset : LevelSet
            levelset object

        Returns
        -------
        np.ndarray
            criterion sensitivity value
        """
        assert levelset.conform,"MinThickness criterion : unable to compute sensitivity in non conformal setting "
        res = np.zeros(levelset.support.GetNumberOfNodes(), dtype=MuscatFloat)

        if not self.jacobianfree:
            self.mean = ComputeMeanCurvature(levelset)
            self.gaussian = ComputeGaussianCurvature(levelset)

        tris = levelset.support.GetElementsOfType(ED.Triangle_3)
        surfconn = tris.connectivity[tris.tags[TZ.InterSurf].GetIds(), :]
        surfids = np.unique(surfconn.flatten())
        surfPoints = levelset.support.nodes[surfids]
        delta = ComputeMeshMinMaxLengthScale(levelset.support)[0]
        normal = ComputeGradientOnTetrahedrons(levelset.support, levelset.phi)
        intPoints = surfPoints - 0.5 * delta * normal[surfids]

        nt = GetInterpolationOperator(levelset.support)
        nt.SetTargetPoints(intPoints)
        nt.Compute()
        phiAtIntPoints = nt.GetOperator().dot(levelset.phi)
        print("phiAtInt=",phiAtIntPoints)

        toIntegrate = self.ComputeLayerContributionToRayIntegral(phiAtIntPoints, surfids)
        res[surfids] += delta * toIntegrate

        maxcpt = 500
        #cpt reaches its max value 500 in the distributor test case !
        cpt = 1
        coef = np.ones_like(phiAtIntPoints)

        while True:

            #coef.fill(1)
            intPoints = surfPoints - (cpt + 0.5) * delta * normal[surfids]
            nt.SetTargetPoints(intPoints)
            nt.Compute()
            newphiAtIntPoints = nt.GetOperator().dot(levelset.phi)

            if (coef == 0).all() or cpt == maxcpt:
                break
            coef[newphiAtIntPoints >= phiAtIntPoints] = 0.0

            toIntegrate = self.ComputeLayerContributionToRayIntegral(phiAtIntPoints, surfids)
            res[surfids] += coef * delta * toIntegrate
            phiAtIntPoints = np.copy(newphiAtIntPoints)
            cpt += 1
            
        #positifs = np.sum(res[surfids] > 0)
        #negatifs = np.sum(res[surfids] < 0)
        #zeros = np.sum(res[surfids] == 0)

        #print(f"Nombre d'éléments positifs : {positifs}")
        #print(f"Nombre d'éléments négatifs : {negatifs}")
        #print(f"Nombre d'éléments nuls     : {zeros}")
        
        # Set all positive sensitivities to zero (discretization error, mathematically it cannot be positive)
        res[surfids][res[surfids] > 0] = 0

        return res

    def ComputeSensitivityEnergyFormulationCorr(self, levelset):
        # https://hal.archives-ouvertes.fr/hal-00985000v2/document
        # Sect. 9.2.2

        assert levelset.conform,"MinThickness criterion : unable to compute sensitivity in non conformal setting "
        res = np.zeros(levelset.support.GetNumberOfNodes(), dtype=MuscatFloat)

        vol = GetVolumePerElement(levelset.support, ElementFilter(elementType=ED.Tetrahedron_4))
        distanceAtCentroids = PointToCellData(levelset.support, levelset.phi, dim=3)
        term1AtCentroids = distanceAtCentroids * np.fmax(distanceAtCentroids + 0.5 * self.dmin, 0.0)**2
        term2AtCentroids = distanceAtCentroids**2 * np.fmax(distanceAtCentroids + 0.5 * self.dmin, 0.0)
        fieldAtCentroids = term1AtCentroids+term2AtCentroids 
        return -np.sum(2*fieldAtCentroids*vol, axis=0)
    
    def UpdateValues(self, point):
        if self.update:
            if self.method == "EnergyFormulation":
                self.f_val = self.ComputeObjectiveEnergyFormulation(point)
                self.fsensitivity_val = self.ComputeSensitivityEnergyFormulation(point)
            elif self.method == "Original":
                self.f_val = self.ComputeObjective(point)
                self.fsensitivity_val = self.ComputeSensitivity(point)
            else:
                raise ValueError("Unknown method " + str(self.method))
        return RETURN_SUCCESS

    def GetValue(self):
        print("f_val=",self.f_val)
        return self.f_val

    def GetSensitivity(self):
        return self.fsensitivity_val

    def GetNumberOfSolutions(self):
        return 0


RegisterCriteriaClass("MinThickness", TopoCriteriaMinThickness)

from Muscat.MeshTools.MeshCreationTools import CreateCube
import Muscat.ImplicitGeometry.ImplicitGeometryObjects as ImplicitGeometry
from OpenPisco.Unstructured.MmgMesher import MmgAvailable
from OpenPisco.Unstructured.Levelset import LevelSet
import OpenPisco.Unstructured.MmgMesher as MmgMesher

def CheckIntegrity(GUI=False):
    if not MmgAvailable:
        return "Not Ok, mmg3d_O3 not found!!"

    UM = CreateCube(dimensions=[10,10,10],spacing=[1./9, 1./9, 1./9],origin=[0, 0, 0],ofTetras=True)
    ls = LevelSet(support=UM)
    init = ImplicitGeometry.ImplicitGeometrySphere(center=[0.5, 0.5, 0.5], radius=0.3)
    ls.phi = init.ApplyVector(UM)
    ls.conform = True
    MmgMesher.MmgMesherActionLevelset(ls, {"iso": 0.0, "hausd": 0.01, "hmin": 0.01, "hmax": 0.7, "nr": True})
    toypoints = np.zeros((5, 3), dtype=MuscatFloat)
    for i in range(5):
        toypoints[i, :] = [1.0 / (i + 2), 1.0 / (i + 2), 1.0 / (i + 2)]

    c1 = TopoCriteriaMaxThickness()
    c1.SetMaxThick(0.1)
    c1.UpdateValues(ls)

    c2 = TopoCriteriaMinThickness()
    c2.SetMinThick(0.1)
    c2.UpdateValues(ls)
    c2.method = "Original"
    c2.UpdateValues(ls)

    return "OK"


if __name__ == "__main__":  # pragma: no cover
    print(CheckIntegrity())
