# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations
from typing import Dict

import numpy as np
import Muscat.Helpers.ParserHelper as PH

from OpenPisco.Actions.ActionFactory import RegisterClass
from OpenPisco.Actions.ActionBase import ActionBase
from OpenPisco.BaseElements import elementsTypeSupportedForDimension
from OpenPisco.Unstructured.LevelsetTools import TakePhiandMeshFromXdmf

from OpenPisco import RETURN_SUCCESS
from OpenPisco.CLApp.OpenPiscoCL import MainApp

class UpdateDistanceAction(ActionBase):
    """
    Reinitialize level set distance

    Parameters
    ----------
        ls: levelset object
        lenght:"float"

    Example in XML:
    <Action type="UpdateDistance" ls="1"/>

    Example in python:
    action = UpdateDistanceAction()
    action.Apply(app=None, ops={"ls":levelSet})
    """
    def __init__(self):
       super(UpdateDistanceAction,self).__init__()

    def Apply(self,app:MainApp, ops:Dict):
        ls = ops['ls']
        length = ops.get('length' ,None)
        ls.Reinitialize(length=length)
        return RETURN_SUCCESS

RegisterClass("UpdateDistance",UpdateDistanceAction)

class FieldTransfertAction(ActionBase):
    """
    Tranfer a field from a level set to another level set.

    Parameters
    ----------
        from: levelset object
        to: levelset object

    Example in XML:
    <Action type="FieldTransfert" from="1" to="2"/>

    """
    def __init__(self):
       super(FieldTransfertAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
          from Muscat.MeshTools.ConstantRectilinearMeshTools import GetNodeTransferMatrix, GetElementTransferMatrix

          fieldFrom = app.levelSets[int(ops['from'])]
          gridFrom = fieldFrom.support

          fieldTo = app.levelSets[int(ops['to'])]
          gridTo = fieldTo.support

          if fieldFrom.IsNodal():
              if gridFrom.props.get("IsConstantRectilinear",False):
                  tMatrix = GetNodeTransferMatrix(gridFrom,gridTo)
              else:
                  from Muscat.FE.Fields.FEField import FEField
                  from Muscat.FE.FETools import PrepareFEComputation
                  space, numberings, _, _ = PrepareFEComputation(gridFrom)
                  phiFEField = FEField(name="phi",mesh=gridFrom,space=space,numbering=numberings[0])
                  from Muscat.MeshTools.MeshFieldOperations import GetFieldTransferOp
                  tMatrix,_, _ = GetFieldTransferOp(phiFEField,gridTo.nodes,method = "Interp/Clamp",verbose=False)
          else:
              assert gridFrom.props.get("IsConstantRectilinear",False),"Not coded jet"
              tMatrix = GetElementTransferMatrix(gridFrom,gridTo)

          fieldTo.phi = tMatrix*fieldFrom.phi

          return RETURN_SUCCESS

RegisterClass("LevelSetTransfert",FieldTransfertAction)
RegisterClass("FieldTransfert",FieldTransfertAction)


class SetOriginalSupportAction(ActionBase):
    """
    Set original support

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="SetOriginalSupport" ls="1"/>

    Example in python:
    action = SetOriginalSupportAction()
    action.Apply(app=None, ops={"ls":levelSetObject})

    """
    def __init__(self):
        super(SetOriginalSupportAction,self).__init__()

    def Apply(self, app:MainApp, ops):
        ls = ops['ls']
        ls.originalSupport = ls.support
        return RETURN_SUCCESS

RegisterClass("SetOriginalSupport",SetOriginalSupportAction)

class InitLevelsetAction(ActionBase):
    """
    Initialize level set on zone

    Parameters
    ----------
        ls: levelset object
        zone: zone

    Example in XML:
    <Action type="InitLevelset" ls="1" zone=1 UpdateDistance="True"/>

    Example in python:
    action = InitLevelsetAction()
    action.Apply(app=None, ops={"ls":levelSetObject,"zone":zoneObject,"UpdateDistance":True})

    """
    def __init__(self):
        super(InitLevelsetAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        if ops['zone'] == "support":
            from Muscat.ImplicitGeometry.ImplicitGeometryBase import ImplicitGeometryDelayedInit
            zone = ImplicitGeometryDelayedInit("All",{"support":ls.support}).ApplyVector
        else:
            zone = ops['zone']
        ls.Initialize(zone)

        if "UpdateDistance" in ops and PH.ReadBool(ops["UpdateDistance"]):
            return UpdateDistanceAction().Apply(app, ops)
        return RETURN_SUCCESS

RegisterClass("InitLevelset",InitLevelsetAction)

class LevelSetOffsetAction(ActionBase):
    """
    Add an offset to the level set value overall

    Parameters
    ----------
        ls: levelset object
        val: float

    Example in XML:
    <Action type="LevelSetOffset" val="0.1"/>

    Example in python:
    action = LevelSetOffsetAction()
    action.Apply(app=None, ops={"ls":levelSetObject,"val":0.1})

    """
    def __init__(self):
        super(LevelSetOffsetAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        val = PH.ReadFloat(ops['val'])
        ls.phi += val

        return RETURN_SUCCESS

RegisterClass("LevelSetOffset",LevelSetOffsetAction)

class LevelSetChangePhiByZonesAction(ActionBase):
    """
    Change level set value based on zone

    Parameters
    ----------
        ls: levelset object
        sign: float
        zone: zone object

    Example in XML:
    <Action type="ChangePhiByZones" ls="1" zone="1"/>

    Example in python:
    action = LevelSetChangePhiByZonesAction()
    action.Apply(app=None, ops={"ls":levelSetObject,"zone":zoneObject})

    """
    def __init__(self):
        super(LevelSetChangePhiByZonesAction,self).__init__()

    def Apply(self,app:MainApp, ops:Dict):
        ls = ops['ls']
        sgn = PH.ReadFloat(ops['sign']) if 'sign' in ops else 1.
        on = ops['zone'].ApplyVector(ls.support)
        ls.phi[:] = sgn*on

        return RETURN_SUCCESS
RegisterClass("ChangePhiByZones",LevelSetChangePhiByZonesAction)

class InitFromAction(ActionBase):
    """
    Initialize level set from the content of a .xmf file.

    Parameters
    ----------
        ls: levelset object
        file: file path

    Example in XML:
    <Action type="InitFrom" ls="1" file="/path/to/file.xmf" />

    Example in python:
    action = InitFromAction()
    action.Apply(app=None, ops={"ls":levelSetObject,"file":"/path/to/file.xmf"})

    """
    def __init__(self):
        super(InitFromAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        fname = ops['file']
        ls = ops['ls']
        TakePhiandMeshFromXdmf(ls,fname)

        return RETURN_SUCCESS

RegisterClass("InitFrom",InitFromAction)

class InitWithAction(ActionBase):
    """
    Initialize level set with the content of a .xmf file.
    TODO: clarify difference with InitFrom.

    Parameters
    ----------
        ls: levelset object
        file: file path

    Example in XML:
    <Action type="InitWith" ls="1" file="/path/to/file.xmf" />

    Example in python:
    action = InitWithAction()
    action.Apply(app=None, ops={"ls":levelSetObject,"file":"/path/to/file.xmf"})

    """
    def __init__(self):
        super(InitWithAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        fname = ops['file']
        ls = ops['ls']
        iter = int(ops['iter'])

        from Muscat.IO.XdmfReader import XdmfReader
        f = XdmfReader(fname)
        f.lazy = True
        f.Read()
        gridcpt = iter #-1
        def GetGrid():
            print(f"using grid {gridcpt}")
            return f.xdmf.GetDomain(0).GetGrid(gridcpt)
        lastgrid = GetGrid()

        # verification of the number of nodes
        if lastgrid.GetSupport().GetNumberOfNodes() != ls.support.GetNumberOfNodes():
            gridcpt -= 1
            lastgrid = GetGrid()

        fieldToRead = ops["field"] if "field" in ops else "phi"
        if fieldToRead not in lastgrid.GetPointFieldsNames():
            gridcpt -= 1
            print(f"Field not found in the last grid. trying in grind {gridcpt}")
            lastgrid = GetGrid()

        ls.phi = lastgrid.GetFieldData(fieldToRead,noTranspose=True)

        if lastgrid.geometry.Type == "ORIGIN_DXDYDZ":
            #dont know why I have to do this
            s = ls.phi.shape
            ls.phi.shape = np.fliplr([s])[0]
            ls.phi = ls.phi.T.flatten()

        return RETURN_SUCCESS

RegisterClass("InitWith",InitWithAction)

class ComputeLevelsetStatisticsAction(ActionBase):
    """
    Compute and print statistics of a levelset

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="LsStats" ls="1"/>

    Example in python:
    action = ComputeLevelsetStatisticsAction()
    action.Apply(app=None, ops={"ls":levelSetObject})

    """
    def __init__(self):
        super(ComputeLevelsetStatisticsAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        label = ops.get('label', id(ls))

        lsmin = np.min(ls.phi)
        lsmax = np.max(ls.phi)
        from OpenPisco.Optim.Criteria.GeoCriteria import TopoCriteriaVolume
        tc = TopoCriteriaVolume()
        tc.UpdateValues(ls)
        lsVolumeInside3D = tc.GetValue()
        tc.UpdateValuesUsingPhi(ls,ls.phi)
        lsVolume = tc.GetValue()
        tc.UpdateValuesUsingPhi(ls,np.zeros_like(ls.phi) - 1 )
        totalVolume = tc.GetValue()
        percentageVolume = lsVolume/totalVolume
        boundingMin, boundingMax = ls.support.ComputeBoundingBox()

        res  = " *** LS ({}) Statistics *** \n".format(label)
        res += " Mesh Min Bounding: {} \n".format(" ".join([str(x) for x in boundingMin]))
        res += " Mesh Max Bounding: {} \n".format(" ".join([str(x) for x in boundingMax]))
        res += " Ls Minimum       : {} \n".format(lsmin)
        res += " Ls Maximum       : {} \n".format(lsmax)
        res += " Ls Volume        : {} \n".format(lsVolume)
        res += " Inside3D Volume  : {} \n".format(lsVolumeInside3D)
        res += " Mesh Total Volume: {} \n".format(totalVolume)
        res += " Ls Percent Volume: {:.2%}%\n".format(percentageVolume)

        print(res)

        return RETURN_SUCCESS

RegisterClass("LsStats", ComputeLevelsetStatisticsAction)

class ComputeLevelSetSupportLengthScaleAction(ActionBase):
    """
    Compute and print level set support min/max lengthscale

    Parameters
    ----------
        ls: levelset object
        label:int

    Example in XML:
    <Action type="LsSupportLengthScale" ls="1"/>

    Example in python:
    action = ComputeLevelSetSupportLengthScaleAction()
    action.Apply(app=None, ops={"ls":levelSetObject})

    """
    def __init__(self):
        super(ComputeLevelSetSupportLengthScaleAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        label = ops.get('label', id(ls))

        from  Muscat.MeshTools.MeshInspectionTools import ComputeMeshMinMaxLengthScale
        hmin,hmax =  ComputeMeshMinMaxLengthScale(ls.support)

        res  = " *** LS ({}) Support LengthScale *** \n".format(label)
        res += " Mesh HMin : {} \n".format(hmin)
        res += " Mesh HMax : {} \n".format(hmax)

        print(res)

        return RETURN_SUCCESS

RegisterClass("LsSupportLengthScale", ComputeLevelSetSupportLengthScaleAction)

class ComputeLevelsetQualityInfoAction(ActionBase):
    """
    Compute and print mesh quality info of a levelset

    Parameters
    ----------
        ls: levelset object
        label:int

    Example in XML:
    <Action type="LsQualityInfo" ls="1"/>

    Example in python:
    action = ComputeLevelsetQualityInfoAction()
    action.Apply(app=None, ops={"ls":levelSetObject})

    """
    def __init__(self):
        super(ComputeLevelsetQualityInfoAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']

        from OpenPisco.MuscatExtentions.MeshQualityControlTools import ComputeAspectRatioOnBodyElements
        qualityInfo = ComputeAspectRatioOnBodyElements(ls.support)

        dimensionality = ls.support.GetElementsDimensionality()
        elementType=elementsTypeSupportedForDimension[dimensionality]

        res  = " *** LS Mesh Quality Info *** \n"
        typeOfQuantity = "body element Aspect Ratio       : {} \n"
        res += " Best "+str(elementType)+" "+typeOfQuantity.format(np.min(qualityInfo))
        res += " Worst "+str(elementType)+" "+typeOfQuantity.format(np.max(qualityInfo))
        res += " Mean  "+str(elementType)+" "+typeOfQuantity.format(np.mean(qualityInfo))
        print(res)
        return RETURN_SUCCESS

RegisterClass("LsQualityInfo", ComputeLevelsetQualityInfoAction)

class AddExteriorsZerosAction(ActionBase):
    """
    Add zeros to exterior surface and additional tags

    Parameters
    ----------
        ls: levelset object
        offset: float
        nTags: string, nodal tags
        eTags: string, element tags

    Example in XML:
    <Action type="AddExteriorsZeros" ls="1"/>

    Example in python:
    action = AddExteriorsZerosAction()
    action.Apply(app=None, ops={"ls":levelSetObject})

    """
    def __init__(self):
        super(AddExteriorsZerosAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        import Muscat.ImplicitGeometry.ImplicitGeometryObjects as IGObs
        import Muscat.ImplicitGeometry.ImplicitGeometryOperators as IGOps
        ls = ops['ls']

        if ls.support.props.get("IsConstantRectilinear",False) :
            exterieur = IGObs.ImplicitGeometryAxisAlignBox(origin=ls.support.GetOrigin(),size=(ls.support.props.get("dimensions")-1)*ls.support.props.get("spacing"))

        else:
            from Muscat.MeshTools.MeshModificationTools import ComputeSkin
            skinmesh = ComputeSkin(ls.support )
            exterieur = IGObs.ImplicitGeometryStl()
            exterieur.SetSurface(skinmesh )

        offset = float(ops.get('offset',-1))
        if 'nTags' in ops:
            offset = float(ops.get('offset',-1))
            tagnames = PH.ReadStrings(ops['nTags'])
            ll = abs(np.copy(ls.phi))
            for tn in tagnames :
                ids = ls.support.nodesTags[tn].GetIds()
                ll[ids] = -abs(ls.phi[ids]) + offset

            on = IGObs.ImplicitGeometryWrapped(field=ll)
            exterieur = IGOps.ImplicitGeometryUnion([exterieur,on])

        if 'eTags' in ops:
            tagnames = PH.ReadStrings(ops['eTags'])
            ll = abs(np.copy(ls.phi))
            from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
            ff = ElementFilter(tags=tagnames)
            for _, data, ids in ff(ls.support):
                ids = data.GetNodesIdFor(ids)
                ll[ids] = -abs(ls.phi[ids]) + offset

            on = IGObs.ImplicitGeometryWrapped(field=ll)
            exterieur = IGOps.ImplicitGeometryUnion([exterieur,on])

            # intersection of the exterior surface
        ls.phi[:] = np.maximum(exterieur.ApplyVector(ls.support),ls.phi)

        return RETURN_SUCCESS

RegisterClass("AddExteriorsZeros",AddExteriorsZerosAction)


def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
