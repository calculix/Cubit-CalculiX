# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations

from typing import Optional, Dict

import numpy as np

from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter, ZT_ALL_NODES
import Muscat.Helpers.ParserHelper as PH
from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometryWrapped
from Muscat.MeshContainers.Filters.FilterTools import ElementFilterToImplicitField
from Muscat.Helpers.Logger import Info, Debug, GetDiffTime
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags, ComputeMeshMinMaxLengthScale

import OpenPisco.ExtractConnexPart as ECP
from OpenPisco.Optim.Problems.OptimProblemConcrete import OptimProblemConcrete
from OpenPisco.LevelSetBase import LevelSetBase
import OpenPisco.TopoZones as TZ

class OptimTopoProblemBase(OptimProblemConcrete):
    def __init__(self,other:Optional[OptimTopoProblemBase] = None, dataDeepCopy:bool = True):
        """Class constructor

        Parameters
        ----------
        other : Optional[OptimTopoProblemBase], optional
            another instance of this very problem to copy attributes from, by default None.
        dataDeepCopy : bool, optional
            whether to use deepcopy, by default True
        """
        super(OptimTopoProblemBase,self).__init__(other=other)

        if  other is not None:
            self.point = type(other.point)(other.point)

            self.TakeValuesFrom(other)
            if hasattr(other.point, 'support'):
                import copy 
                self.point.support  = copy.deepcopy(other.point.support)
            if hasattr(other.point, 'originalSupport'):
                self.point.originalSupport = other.point.originalSupport
            else:
                Debug("originalSupport not found")
                


            self.e2 = other.e2
            self.OnZone = other.OnZone
            self.OnFilter = other.OnFilter
            self.OffZone = other.OffZone
            self.OffFilter = other.OffFilter
            self.OnETags = PH.ReadStrings(other.OnETags)
            self.OffETags = PH.ReadStrings(other.OffETags)
            self.GenerateImplicitZonesFromTags()
            self.writer = other.writer
            self.hj_iter = other.hj_iter
            self.outputEveryLevelsetUpdate = other.outputEveryLevelsetUpdate
            if other.connexPartOp is None :
                other.connexPartOp = ECP.ConnexPart(self.point.support)
            self.connexPartOp = other.connexPartOp
            self.maxChange = other.maxChange
            self.printMeshQualityInfo = other.printMeshQualityInfo
            self.adaptRegularizationToMeshSizeMin = other.adaptRegularizationToMeshSizeMin
            self.disableCFLTruncation = other.disableCFLTruncation
        else:
            ##Regularisation Parameter
            self.e2 = 2.
            self.maxChange = {}
            self.maxChange['Volume'] =  0.2 
            self.point = None
            self.OnZone = None
            self.OnFilter = None
            self.OffZone = None
            self.OffFilter = None
            self.OnETags = np.zeros(0,dtype=str)
            self.OffETags = np.zeros(0,dtype=str)
            self.writer = None
            self.hj_iter = 10
            self.outputEveryLevelsetUpdate = False
            self.connexPartOp = None
            self.adaptRegularizationToMeshSizeMin = False
            self.printMeshQualityInfo = False
            self.disableCFLTruncation = True

    def GetCurrentPoint(self)->LevelSetBase:
        """Retrieve current level set of the optimization problem

        Returns
        -------
        LevelSetBase
            levelset object
        """
        return self.point

    def TakeValuesFrom(self,other:OptimTopoProblemBase):
        """Retrieve values from another optimization problem

        Parameters
        ----------
        other : OptimTopoProblemBase
            Another instance of OptimTopoProblemBase
        """
        super(OptimTopoProblemBase,self).TakeValuesFrom(other)
        self.point.TakeValuesFrom(other.point)

    def UpdateValues(self)->int:
        """Store levelset function value, update the optimization problem, discard levelset function value"""
        self.point.StoreState()
        res = super().UpdateValues()
        self.point.DiscardState()
        return res

    def ApplyDomainConstraints(self,point:LevelSetBase)->np.ndarray:
        """Apply domain constraints to the level set function values

        Parameters
        ----------
        point : LevelSetBase
            levelset object

        Returns
        -------
        np.ndarray
            new levelset function value after applying domain constraints to it
        """
        if self.OnZone is not None:
            on = self.OnZone(point.support)
            res = np.minimum(point.phi,on)
        else:
            res = np.copy(point.phi)

        if self.OnFilter is not None:
            onphi = ElementFilterToImplicitField(self.OnFilter, mesh=point.support )
            np.minimum(res,onphi,out=res)

        if self.OffZone is not None:
            off = self.OffZone(point.support)
            np.maximum(res,-off,out=res)

        if self.OffFilter is not None:
            offphi = ElementFilterToImplicitField(self.OffFilter, mesh=point.support )
            np.maximum(res,-offphi,out=res)
        return  res

    def PrintHeader(self):
        """Print details about optimization problem constraints"""
        print("Laplace filter constant : " + str(self.e2))
        super().PrintHeader()

    def GenerateImplicitZonesFromTags(self):
        """Generate implicit zones based on element tags"""
        offset = 1e-3
        tags = [self.OnETags,self.OffETags]
        zones = ["OnZone","OffZone"]
        for tag,zone in zip(tags,zones):
            if len(tag):
                EF = ElementFilter(eTag=tag)
                implicitfield = ElementFilterToImplicitField(EF, self.point.support) - offset
                setattr(self,zone,ImplicitGeometryWrapped(implicitfield))

    def PreStartCheck(self)->bool:
        """Perform verifications before running the problem. 
        We make sure that all existing zones in the physical problems used for boundary conditions are not in the "off" region.
        If at least one boundary zone is in the "off" region, the test will fail.

        Returns
        -------
        bool
            whether verifications suceeded
        """
        boundaryFilters = self.RetrievePhysicalBoundaryFilters()
        for boundaryFilter in boundaryFilters:
            boundaryNodes = boundaryFilter.GetNodesIndices(self.point.support)
            if (self.OffZone is not None)  and np.any(self.OffZone(self.point.support)[boundaryNodes] <=(1.e-16)):# pragma: no cover
                print('Error : "Boundary conditions cannot be applied to off region')
                return False
        self.point.StoreState()
        self.RemoveIslands()
        self.point.AcceptChanges(self.ApplyDomainConstraints(self.point))
        return True

    def GetDirectionFromGradient(self,raw_v:np.ndarray)->np.ndarray:
        """Compute direction from gradient. 
        We perform a regularization based on the raw velocity and parameters depending on lenght scale caracteristics.

        Parameters
        ----------
        raw_v : np.ndarray
            raw velocity

        Returns
        -------
        np.ndarray
            regularized direction
        """
        if self.adaptRegularizationToMeshSizeMin and self.point.conform:
            surf = ExtractElementsByTags(self.point.support, [TZ.InterSurf])
            if surf.GetNumberOfElements() == 0 :
               print("Warning from GetDirectionFromGradient: iso not found")
            h =  ComputeMeshMinMaxLengthScale(surf)[0]
            alpha = np.maximum(2*h**2,self.e2)
        else:
            alpha = self.e2

        on=[]
        if self.OnZone is not None:
            ef = ElementFilter(dimensionality=self.point.support.GetElementsDimensionality()-1,zone=[self.OnZone],zonesTreatment=ZT_ALL_NODES)
            on=[ef]
 
        temp_velo = -self.point.Regularize(raw_v,lengthscaleParameter=alpha)
        

        return temp_velo.ravel()

    def DirectionMagnitude(self, direction:np.ndarray)->float:
        """Compute direction magnitude

        Parameters
        ----------
        direction : np.ndarray
            The (scalar) advection field

        Returns
        -------
        float
            The magnitude, more precisely the volume swept
        """
        return self.point.InterfaceIntegral(np.abs(direction))

    def RemoveIslands(self):
        phitemp = self.ApplyDomainConstraints(self.point)
        Debug("Apply ConnexPart")
        boundaryFilters = list(self.RetrievePhysicalBoundaryFilters())
        if boundaryFilters:
            boundaryNodes = boundaryFilters[0].GetNodesIndices(self.point.support)
            fix = np.ones_like(self.point.phi)
            fix[boundaryNodes] *=-1
            if  self.connexPartOp is None or self.point.support is not self.connexPartOp.support:
                self.connexPartOp = ECP.ConnexPart(self.point.support)
            self.point.phi = self.connexPartOp.Apply(phitemp, fix, self.point.phi)
            #phitemp = self.connexPartOp.Apply(phitemp, fix, phitemp)
            return

    def Advance(self, direction:np.ndarray, step_length:float):
        """
        Try to advance the levelset using an advection equation.

        Parameters
        ----------
        direction : np.ndarray
            The (scalar) advection velocity field.
        step_length : scalar
            The length of the pseudo-time integration interval.

        Returns
        -------
        bool
            Whether the advance was successful.

        Notes
        -----
        Two constraints are imposed on the advection process:
            1. The updated design must remain feasible, meaning that all optimization criteria must be computable at the new design point. 
            For example, when required by the underlying physics, regions subject to Neumann boundary conditions must remain connected to regions subject to Dirichlet boundary conditions. 
            Ensuring such problem-specific requirements, however, is the responsibility of each physical model.
            2. A change in the volume must not exceed a given threshold.

        """
        self.point.extraNodesFields['Direction'] = direction
        assert self.point is not None,'Please allocate the levelset first'

        # Make a copy of the level-set function in case the step fails
        self.point.StoreState()

        # Force recomputation of volume for current configuration
        # TODO Avoid a spurious volume evaluation
        phitemp = self.point.phi

        from OpenPisco.Optim.Criteria.GeoCriteria import TopoCriteriaVolume
        volumeCalculation = TopoCriteriaVolume()
        volumeCalculation.UpdateValuesUsingPhi(self.point,phitemp)
        vol_0 = volumeCalculation.GetValue()
        Debug(vol_0)

        # Transport parameters

        # Do the levelset update
        Debug("LevelSetUpdate")
        self.LevelSetUpdate(direction, step_length)

        Debug("Regularize Phi")
        if self.point.filterLevelSetField and self.point.conform:
           self.point.RegularizePhi()
        
        Debug("Apply Constraints")
        phitemp = self.ApplyDomainConstraints(self.point)

        Debug("Apply ConnexPart")
        boundaryFilters = list(self.RetrievePhysicalBoundaryFilters())
        if boundaryFilters:
            boundaryNodes = boundaryFilters[0].GetNodesIndices(self.point.support)
            fix = np.ones_like(self.point.phi)
            fix[boundaryNodes] *=-1
            if  self.connexPartOp is None or self.point.support is not self.connexPartOp.support:
                self.connexPartOp = ECP.ConnexPart(self.point.support)
            self.point.phi = self.connexPartOp.Apply(phitemp, fix, self.point.phi)
            #self.point.phi = self.connexPartOp.Apply(phitemp, fix)
            phitemp = self.connexPartOp.Apply(phitemp, fix, phitemp)
            #phitemp = self.connexPartOp.Apply(phitemp, fix)

        # Check both failure conditions
        # 1) Verify optim domain is valid
        ok = self.VerifyOptimDomainValidity(self.point.support,phitemp)
        if ok:
            # 2) Verify that volume variation is not too large (only if precedent test is ok )
            volumeCalculation.UpdateValuesUsingPhi(self.point,phitemp)
            vol_c = volumeCalculation.GetValue()
            Debug(vol_c)
            relative_volume_change = abs(vol_c - vol_0) / vol_0
            if relative_volume_change > self.maxChange['Volume']:
                ok = False
                Info(" ".join(("Volume change too large :",
                        str(relative_volume_change), ">",
                        str(self.maxChange['Volume']))))
        else:
            vol_c = -1

        if ok:
            self.point.AcceptChanges(phitemp)
            #self.point.AcceptChanges(self.ApplyDomainConstraints(self.point))
        else:
            Info(" Rejected Point ")
            # Reject step
            if self.outputEveryLevelsetUpdate:
                self.SaveData(data={"ControlVolume":vol_c,"OK":0.0}, point=self.point)
            self.point.DiscardState()

        return ok

    def LevelSetUpdate(self, direction:np.ndarray, step_length:float):
        """Update the levelset: transport and reinitialize in a given direction for a given step 

        Parameters
        ----------
        direction : np.ndarray
            direction for transport
        step_length : float
            step for transport
        """
        Debug("in LevelSetUpdate")
        self.point.TransportAndReinitialize(direction, step_length,disableCFLTruncation = self.disableCFLTruncation)
        Debug("in LevelSetUpdate Done")

    def ResetState(self):
        """Reset levelset state"""
        if self.point is not None:
            self.point.ResetState()

    def SaveData(self,
                 data:Dict,
                 point:Optional[LevelSetBase]=None, 
                 gradient:Optional[np.ndarray]= None,
                 direction:Optional[np.ndarray] = None,
                 onlyphi:Optional[np.ndarray]=None):
        """Save data on disk

        Parameters
        ----------
        data : Dict
            fields name/value to save
        point : Optional[LevelSetBase], optional
            levelset object, by default None
        gradient : Optional[np.ndarray], optional
            gradient, by default None
        direction : Optional[np.ndarray], optional
            direction, by default None
        onlyphi : Optional[np.ndarray], optional
            levelset function value, by default None

        """
        if self.writer is None: return

        Info("********************** output ***********************")
        
        if onlyphi is not None:
            self.writer.Write(self.point.support,
                            PointFields     = [onlyphi],
                            PointFieldsNames= ['phi'],
                            )
            return

        GridFieldsNames = list(data.keys())
        GridFields = list(data.values())
        
        PointFields = []
        PointFieldsNames = []

        def GetListOfCriteriaValues(c):
            res = []
            prefix = c.GetName()
            from OpenPisco.Optim.Criteria.ComposedCriteria import ComposedCriteria
            if issubclass(type(c), ComposedCriteria):
                for i in range(c.GetNumberOfInternalCriteria()):
                    ic = c.GetInternalCriteria(i)
                    res.extend([ (prefix+"_"+n,v) for n,v in GetListOfCriteriaValues(ic) ])
            res.append((prefix,c.GetValue()))
            return res
        
        def GetListOfCriteriaSensitivities(c):
            res = []
            prefix = c.GetName()+"_sensitivity"
            from OpenPisco.Optim.Criteria.ComposedCriteria import ComposedCriteria
            if issubclass(type(c), ComposedCriteria):
                for i in range(c.GetNumberOfInternalCriteria()):
                    ic = c.GetInternalCriteria(i)
                    res.extend([ (prefix+"_"+n,v) for n,v in GetListOfCriteriaValues(ic) ])
            res.append((prefix,c.GetSensitivity()))
            return res

        cvalues = GetListOfCriteriaValues(self.objectiveCriterion)
        GridFieldsNames.extend([n for n,v in cvalues])
        GridFields.extend([v for n,v in cvalues])
        
        csensitivities = GetListOfCriteriaSensitivities(self.objectiveCriterion)
        PointFieldsNames.extend([n for n,v in csensitivities])
        PointFields.extend([v for n,v in csensitivities])
        
        for c_i in range(self.GetNumberOfInequalityConstraints()):
            cvalues = GetListOfCriteriaValues(self.inequalityconstraintsCriteria[c_i])
            GridFieldsNames.extend([n for n,v in cvalues])
            GridFields.extend([v for n,v in cvalues])
            
            csensitivities = GetListOfCriteriaSensitivities(self.inequalityconstraintsCriteria[c_i])
            PointFieldsNames.extend([n for n,v in csensitivities])
            PointFields.extend([v for n,v in csensitivities])

        for c_i in range(self.GetNumberOfEqualityConstraints()):
            cvalues = GetListOfCriteriaValues(self.equalityConstraintsCriteria[c_i])
            GridFieldsNames.extend([n for n,_ in cvalues])
            GridFields.extend([v for _,v in cvalues])
            
            csensitivities = GetListOfCriteriaSensitivities(self.equalityConstraintsCriteria[c_i])
            PointFieldsNames.extend([n for n,v in csensitivities])
            PointFields.extend([v for n,v in csensitivities])

        GridFieldsNames.append("time")
        GridFields.append(GetDiffTime() )

        scalarOutputs = self.GetScalarsOutputs()

        if self.point.conform and self.printMeshQualityInfo:
            from OpenPisco.MuscatExtentions.MeshQualityControlTools import ComputeAspectRatioOnBodyElements
            qualityInfo = ComputeAspectRatioOnBodyElements(self.point.support)
            scalarOutputs["MeshQualityInfo"] = np.max(qualityInfo)

        GridFieldsNames.extend( scalarOutputs.keys() )
        GridFields.extend( scalarOutputs.values() )
        

        CellFields = []
        CellFieldsNames = []

        if self.point.IsNodal():
            Fields =  PointFields
            FieldsNames = PointFieldsNames
        else:
            Fields =  CellFields
            FieldsNames = CellFieldsNames

        if point is None:
            f = self.point.phi
        else:
            f = point.phi
        Fields.append(f)
        FieldsNames.append('phi')

        if direction is not None:
            Fields.append(direction)
            FieldsNames.append('direction')

        if gradient is not None:
            Fields.append(gradient)
            FieldsNames.append('gradient')

        if self.OnZone is not None:
            Fields.append(self.OnZone(self.point.support))
            FieldsNames.append('On')

        if self.GetNumberOfSolutions() == 1:
            PointFields.append(self.GetSolution(0))
            PointFieldsNames.append(self.GetSolutionName(0))
        else:
            # if we have a solution with the same number we add a '_' at the end
            almanac = {}
            for d in range(self.GetNumberOfSolutions()):
                PointFields.append(self.GetSolution(d))
                name = self.GetSolutionName(d)
                while name in almanac:
                    name += '_'
                PointFieldsNames.append(name)
                
        if self.point.conform and "remeshingMetric" in self.point.support.nodeFields: 
            PointFields.append(self.point.support.nodeFields["remeshingMetric"])
            PointFieldsNames.append("remeshingMetric")
            
        self.writer.Write(self.point.support,
                PointFields     = PointFields,
                PointFieldsNames= PointFieldsNames,
                CellFields      = CellFields,
                CellFieldsNames = CellFieldsNames,
                GridFields      = GridFields ,
                GridFieldsNames = GridFieldsNames
                )

def CheckIntegrity():
    return 'ok'

if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity())
