# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

# -*- coding: utf-8 -*-
from __future__ import annotations
from typing import Dict

import numpy as np

import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.Logger import Info, Debug, Error
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter, NodeFilter
from Muscat.MeshContainers.Filters.FilterOperators import ComplementaryFilter, DifferenceFilter, IntersectionFilter, UnionFilter
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshTools.MeshModificationTools import AddTagPerBody
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByElementFilter,CleanLonelyNodes,ExtractElementsByTags,GetVolume,GetMeasure
from Muscat.ImplicitGeometry.ImplicitGeometryTools import CutMeshIsoZero
from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometryStl

import OpenPisco.TopoZones as TZ
from OpenPisco.Actions.ActionFactory import RegisterClass
from OpenPisco.Actions.ActionBase import ActionBase
from OpenPisco.Unstructured.MeshAdaptationTools import ComputeLevelSetMetric
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL
from OpenPisco.CLApp.OpenPiscoCL import MainApp

class CreateTagsAction(ActionBase):
    """
    Create tags within support of levelset and add them to the levelset support.

    Parameters
    ----------
        ls: levelset object
        elementTags:ints
        elemenprefix:string
        nodalTags:int
        nodalprefix:string
        dim:int

    Example in XML:
    <Action type="CreateTagsFromZones" ls="1" elementTags="1 3" dim="2" elemenprefix="eTag"/>

    Example in python:
    action = CreateTagsAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"elementTags":[1,3],"dim":2,"elemenprefix":"eTag"})
    """
    def __init__(self):
        super(CreateTagsAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        del ops['ls']

        optionsToRead = list(ops.keys())
        optionsToRead.remove("type")
        options = {'nodalprefix' : "NT",
                   'nodalTags' : np.empty(0,dtype=int),
                   'elemenprefix' :"ET",
                   'elementTags' : np.empty(0,dtype=int),
                   'dim': ls.support.GetElementsDimensionality()}

        PH.ReadProperties(ops,optionsToRead,options)

        for tag in options['nodalTags']:
            val = app.zones[tag](ls.support)
            w = np.nonzero(val<=0)[0]
            ls.support.nodesTags.CreateTag(options["nodalprefix"]+str(tag)).SetIds(w)

        dim = options['dim']

        for tag in options['elementTags']:
            ff = ElementFilter()
            zone = app.zones[tag]
            ff.AddZone(zone)
            ff.SetDimensionality(dim)
            for name,data,ids in ff(ls.support):
                data.tags.CreateTag(options["elemenprefix"]+str(tag)).SetIds(ids)
                if not len(ids):
                   Info("Warning : creating empty tag "+options["elemenprefix"]+str(tag)+" for elements of type "+name)

        return RETURN_SUCCESS

RegisterClass("CreateTagsFromZones",CreateTagsAction)

class CreateElementTagAction(ActionBase):
    """
    Create element tag based on an element-level operation on several elements tags and add it to the levelset support.

    Parameters
    ----------
        ls: levelset object
        elementTags:ints
        tagname:string
        operator:string (available: intersection,difference,union,complementary, default intersection)

    Example in XML:
    <Action type="CreateElementTag" ls="1" elementTags="1 3" tagname="MyTag" operator="intersection"/>

    Example in python:
    action = CreateElementTagAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"elementTags":[1,3],"tagname":"MyTag","operator":"intersection"})
    """
    def __init__(self):
        super(CreateElementTagAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):

        ls = ops['ls']
        del ops['ls']

        FilterOperatorOps = {'intersection'  : IntersectionFilter,
                             'difference'    : DifferenceFilter,
                             'union'         : UnionFilter,
                             'complementary' : ComplementaryFilter }

        filters = []
        for tagname in PH.ReadStrings(ops["elementTags"]):
            filters.append( ElementFilter(eTag=tagname), )

        operatortype = ops.get('operator','intersection')
        FilterOperator = FilterOperatorOps[operatortype](filters)

        for name,data,ids in FilterOperator(ls.support):
            data.tags.CreateTag(ops["tagname"]).SetIds(ids)
            if not len(ids):
               Info("Warning : creating empty tag "+str(ops["tagname"])+" for elements of type "+name)

        return RETURN_SUCCESS

RegisterClass("CreateElementTag", CreateElementTagAction)

class CreateNodeTagAction(ActionBase):
    """
    Create node tag based on an node-level operation on several node tags and add it to the levelset support.

    Parameters
    ----------
        ls: levelset object
        nodeTags:strings
        tagname:string
        operator:string (available: intersection,difference,union,complementary, default intersection)

    Example in XML:
    <Action type="CreateNodeTag"  ls="1" tagname="Corners"  operator="union" nodeTags="x0y0 x1y0 x0y1 x1y1"/>

    Example in python:
    action = CreateNodeTagAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"tagname":"Corners","operator":"union","nodeTags":"x0y0 x1y0 x0y1 x1y1"})
    """
    def __init__(self):
        super(CreateNodeTagAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):

        ls = ops['ls']
        del ops['ls']

        FilterOperatorOps = {'intersection'  : IntersectionFilter,
                             'difference'    : DifferenceFilter,
                             'union'         : UnionFilter,
                             'complementary' : ComplementaryFilter }
        filters = []
        for tagname in PH.ReadStrings(ops["nodeTags"]):
            filters.append( NodeFilter(nTag=tagname), )
        operatortype = ops.get('operator','intersection')
        FilterOperator = FilterOperatorOps[operatortype](filters)
        ids=FilterOperator.GetNodesIndices(ls.support) 
        ls.support.nodesTags.CreateTag(ops["tagname"]).SetIds(ids)
        if not len(ids):
            Info("Warning : creating empty node tag "+str(ops["tagname"]))

        return RETURN_SUCCESS

RegisterClass("CreateNodeTag", CreateNodeTagAction)

class CreateZonesFromETagsAction(ActionBase):
    """
    Create zones from an element tag.

    Parameters
    ----------
        ls: levelset object
        etags: strings, existing element tagname name
        zones: ints,zone ids

    Example in XML:
    <Action type="CreateZonesFromETags" ls="1" etags="X0 X1" zones="7 8"/>

    Example in python:
    action = CreateZonesFromETagsAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"etags":"X0 X1","zones":"7 8"})
    """
    def __init__(self):
        super().__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        del ops['ls']

        tagnames = PH.ReadString(ops['etags']).split(" ")
        zones = PH.ReadInts(ops["zones"])
        assert len(tagnames) == len(zones),"Number of zones and tagnames provided not consistent"

        if any(zone in app.zones.keys() for zone in zones):
          existingZone=np.fromiter(app.zones.keys(), dtype=int)
          Info("Existing zone IDs: "+str(existingZone))
          Info("To be created zone IDs: "+str(zones))
          Info("Common zone IDs: "+str(np.intersect1d(existingZone, zones)))
          raise ValueError("At least one existing zone will be overwritten, not allowed")

        from Muscat.ImplicitGeometry.ImplicitGeometryObjects import CreateImplicitGeometryByETag
        for tagname,zone in zip(tagnames,zones):
            IGByETag = CreateImplicitGeometryByETag({"support":ls.support, "eTags":[tagname]})
            app.zones[zone]=IGByETag

        return RETURN_SUCCESS

RegisterClass("CreateZonesFromETags",CreateZonesFromETagsAction)


class CreateTagsFromElemFieldAction(ActionBase):
    """
    Create tag from a boolean field and add it to the levelset support.

    Parameters
    ----------
        ls: levelset object
        fieldname: string, boolean element field name 
        prefix: string, tag prefix

    Example in XML:
    <Action type="CreateTagsFromElemField" ls="1" fieldname="MyField" prefix="Prefix"/>

    Example in python:
    action = CreateTagsFromElemFieldAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"fieldname":"MyField","prefix":"Prefix"})
    """
    def __init__(self):
        super(CreateTagsFromElemFieldAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']

        fieldname = ops.get('fieldname' ,'Label')
        prefix = ops.get('prefix' ,'eTag')
        field = ls.support.elemFields[fieldname]
        tagnumbers = np.unique(field)

        for _,data in ls.support.elements.items():
            for integer in tagnumbers:
                elemfield = field[data.globaloffset:data.globaloffset+data.GetNumberOfElements()]
                ids = np.nonzero(elemfield ==integer )[0]
                if len(ids):
                   data.GetTag(prefix+str(integer)).SetIds(ids)

        return RETURN_SUCCESS

RegisterClass("CreateTagsFromElemField",CreateTagsFromElemFieldAction)


class AddETagFromNTagAction(ActionBase):
    """
    Create element tags from nodal tags and add them to the levelset support.

    Parameters
    ----------
        ls: levelset object
        etagNames: string, element tag names
        ntagNames: string, nodal tag names
        elemenprefix : string, element tag prefix (default: ET)
        dim: int, dimension

    Example in XML:
    <Action type="AddETagFromNTag" ls="1" etagNames="ET1 ET2" ntagNames="NT1 NT2"/>

    Example in python:
    action = AddETagFromNTagAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"etagNames":"ET1 ET2","ntagNames":"NT1 NT2"})
    """
    def __init__(self):
        super(AddETagFromNTagAction,self).__init__()

    def Apply(self,app:MainApp, ops:Dict):

        ls = ops['ls']
        nodaltagNames = PH.ReadStrings(ops.get('ntagNames'))
        etagNames= PH.ReadStrings(ops.get('etagNames',[]))

        if not len(etagNames):
            etagNames = [""]*len(nodaltagNames)
        elif len(etagNames) != len(nodaltagNames):
            raise ValueError("The number of nodaltagNames and the etagNames must be the same")

        elementprefix = ops.get("elemenprefix","ET")
        ef = ElementFilter()

        if 'dim' in ops:
            ef.SetDimensionality(PH.ReadInt(ops['dim']) )

        nodalMask = np.zeros(ls.support.GetNumberOfNodes(),dtype = bool)
        for etagName,name in zip(etagNames,nodaltagNames):
            nodalMask.fill(False)
            tag = ls.support.GetNodalTag(name)
            nodalMask[tag.GetIds()] = True
            for name, data, _ in ef(ls.support):
                vals = np.sum(nodalMask[data.connectivity],axis=0)
                idd = np.nonzero( vals == data.GetNumberOfPointsPerElement())[0]
                if len(idd):
                    data.GetTag(elementprefix+str(etagName)).AddToTag(idd)

        return RETURN_SUCCESS

RegisterClass("AddETagFromNTag",AddETagFromNTagAction)

class AddNTagFromETagAction(ActionBase):
    """
    Create nodal tags from element tags and add it to the levelset support.

    Parameters
    ----------
        ls: levelset object
        etagNames: string, element tag names
        ntagNames: string, nodal tag names

    Example in XML:
    <Action type="AddNTagFromETag" ls="1" fieldname="MyField" prefix="Prefix"/>

    Example in python:
    action = AddNTagFromETagAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"fieldname":"MyField","prefix":"Prefix"})
    """
    def __init__(self):
        super(AddNTagFromETagAction,self).__init__()

    def Apply(self,app:MainApp, ops:Dict):

        ls = ops['ls']
        etagNames=PH.ReadStrings(ops.get('etagNames'))
        nodaltagNames=PH.ReadStrings(ops.get('ntagNames',[]))

        assert len(etagNames) == len(nodaltagNames),"The number of nodaltagNames and the etagNames must be the same"
        for etagName,ntagName in zip(etagNames,nodaltagNames):
            for _, data in ls.support.elements.items():
                if etagName in data.tags:
                    nodalids = data.GetNodesIdFor(data.tags[etagName].GetIds())
                    if len(nodalids):
                        ls.support.GetNodalTag(ntagName).AddToTag(nodalids)

        return RETURN_SUCCESS

RegisterClass("AddNTagFromETag",AddNTagFromETagAction)


class AddNodalTagUsingOriginalIdAction(ActionBase):
    """
    Create nodal tags using original ids and add them to the levelset support.

    Parameters
    ----------
        ls: levelset object
        name: string, tag name


    Example in XML:
    <Action type="AddNTagFromETag" ls="1" name="Mytag"/>

    Example in python:
    action = AddNTagFromETagAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"name":"Mytag"})
    """
    def __init__(self):
        super(AddNodalTagUsingOriginalIdAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        del ops['ls']

        optionsToRead = list(ops.keys())
        optionsToRead.remove("type")
        options = {'name' : "",
                   'ids' : np.empty(0,dtype=int),
                   'tag' : ""}

        PH.ReadProperties(ops,optionsToRead,options)

        assert options["name"] != "","The name '"+options["name"]+"' is not valid"

        if options['tag'] != "":
            ls.support.nodesTags.CreateTag(options['name'],False).AddToTag(ls.support.nodesTags[options['tag']].GetIds())

        for ID in options['ids']:
            ls.support.AddNodeToTagUsingOriginalId(ID,options['name'])

        return RETURN_SUCCESS

RegisterClass("AddNodalTag", AddNodalTagUsingOriginalIdAction)


class CutMeshIsoZeroAction(ActionBase):
    """
    Cut mesh iso zero surface

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="AddNTagFromETag" ls="1" name="Mytag"/>

    Example in python:
    action = CutMeshIsoZeroAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"name":"Mytag"})
    """
    def __init__(self):
        super(CutMeshIsoZeroAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        if ls.support.props.get("IsConstantRectilinear",False):
            Error("Cant remesh this type of mesh")
            return RETURN_FAIL

        ls.support = CutMeshIsoZero(ls.support, ls.phi ) [0]

        for _,data in ls.support.elements.items():
            data.tags.RenameTag("isoNegative",TZ.Inside3D,noError=True)
            data.tags.RenameTag("isoPositive",TZ.Outside3D,noError=True)
            data.tags.RenameTag("isoZero",TZ.InterSurf,noError=True)

        surf = ExtractElementsByTags(ls.support, [TZ.InterSurf])
        
        Info("computing distance ")
        op = ImplicitGeometryStl()
        op.SetSurface(surf)
        newphi = abs(op(ls.support.nodes))
        interior = ExtractElementsByTags(ls.support, [TZ.Inside3D])
        newphi[interior.originalIDNodes] *= -1
        Info("end")
        ls.phi = newphi

        return RETURN_SUCCESS

RegisterClass("CutMeshIsoZero",CutMeshIsoZeroAction)


class RemeshAction(ActionBase):
    """
    Remesh levelset support

    Parameters
    ----------
        ls: levelset object
        mmg parameters: collection of parameters compatible with MmgMesher.py module

    Example in XML:
    <Action type="Remesh" ls="1" hmax=".1" hmin="0.05" hausd="0.05" iso="0.0" rmc="1e-5" nr="True"/>

    Example in python:
    action = RemeshAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"hmax":".1","hmin":"0.05","hausd":"0.05","iso":"0.0","rmc":"1e-5","nr":"True"})
    """
    def __init__(self):
       super(RemeshAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        if ls.support.props.get("IsConstantRectilinear",False):
            Error("Cant remesh this type of mesh")
            return RETURN_FAIL

        force = PH.ReadBool(ops.get("force",False))

        if not ls.conform and force == False:
            Error("Cant remesh levelset, not conform")
            return RETURN_FAIL

        import  OpenPisco.Unstructured.MmgMesher as MmgMesher
        #copy specs form the levelset
        remeshOptions = ls.MesherInfo.copy()
        # we delete the iso becase the user just want to do a remesh with no iso
        remeshOptions.pop("iso",None)
        # push action options and override defaut if necesary
        remeshOptions.update(ops)
        for key in ("hmin", "hmax", "hausd", "hgrad","hsiz","iso","ar"):
            if key in ops:
               remeshOptions[key] = PH.ReadFloat(ops[key])

        for key in ("rmc","met","keepGeneratedFiles"):
            if key in ops:
               remeshOptions[key] = PH.ReadBool(ops[key])
        if "met" in remeshOptions.keys() and remeshOptions["met"]:
            #metricfield = ComputeLevelSetMetric(ls,remeshOptions,"Freefem") #behavior inconsistent with met option in MmgMesher
            #ls.support.nodeFields["met"] = metricfield
            if "symbolicMetricExpression" in remeshOptions.keys():
                #remeshOptions["met"] = True
                remeshOptions["symbolicExpression"] = remeshOptions.pop("symbolicMetricExpression")
                metricfield = ComputeLevelSetMetric(ls,remeshOptions,"Symbolic")
                ls.support.nodeFields["met"] = metricfield
        

        MmgMesher.MmgMesherActionLevelset(ls,remeshOptions)

        return RETURN_SUCCESS

RegisterClass("Remesh", RemeshAction)

class CleanLonelyNodesAction(ActionBase):
    """
    Remove nodes not part of any element

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="CleanLonelyNodes" ls="1" />

    Example in python:
    action = CleanLonelyNodesAction()
    action.Apply(app=None, ops={"ls":levelsetObject)
    """
    def __init__(self):
        super(CleanLonelyNodesAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']

        if not ls.support.props.get("IsConstantRectilinear",False):
            Info("Cant CleanLonelyNodes on this type of mesh")
            return 0

        CleanLonelyNodes(ls.support)
        ls.phi = ls.phi[ls.support.originalIDNodes ]
        return RETURN_SUCCESS

RegisterClass("CleanLonelyNodes", CleanLonelyNodesAction)

class Create0DelementAction(ActionBase):
    """
    Create 0d element

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="Create0DElements" ls="1" />

    Example in python:
    action = Create0DelementAction()
    action.Apply(app=None, ops={"ls":levelsetObject)
    """
    def __init__(self):
        super(Create0DelementAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']

        if not ls.support.props.get("IsConstantRectilinear",False):
            Info("Cant call Create0DelementAction on this type of mesh")
            return 0

        elements = ls.support.GetElementsOfType(ED.Point_1)
        zone = ops.get('zone',None)

        if zone is not None:
            val = zone(ls.support)
            w = np.nonzero(val<=0)[0]
            for idd in w:
                elements.AddNewElement([idd],0)

        nTag = ops.get('nTag',None)
        if nTag is not None:
            w = ls.support.GetNodalTag(nTag).GetIds()
            tag = elements.CreateTag(nTag)
            for idd in w:
                cpt = elements.AddNewElement([idd],0)
                tag.AddId(cpt-1)

        ls.support.PrepareForOutput()
        return RETURN_SUCCESS

RegisterClass("Create0DElements", Create0DelementAction)

class FilterSimplyConnectedBodiesInTagAction(ActionBase):
    """
    Filter simply connected bodies for tag within mesh.
        -Compute each connex bodies
        -Keep those whose measure (volume in 3D, surface en 2D) is within the interval [minRelativeMeasure*meas(support); maxRelativeMeasure*meas(support)],
        -Remove the others

    Parameters
    ----------
        ls: levelset
        minRelativeMeasure: minimum body relative measure to keep, default value 0
        maxRelativeMeasure: maximum body relative measure to keep, default value 1

    The relative measure of a body is the ratio between the measure of a simply connected body and the measure of the whole support

    Example in XML:
    <Action type="FilterSimplyConnectedBodiesInTag" ls="1" maxRelativeMeasure="1.0" minRelativeMeasure="0.0625"  eTag="overhangSurface" />

    Example in python:
    action = FilterSimplyConnectedBodiesInTagAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"maxRelativeMeasure":"1.0","minRelativeMeasure":"0.0625","eTag":"overhangSurface")
    """
    def __init__(self):
        super(FilterSimplyConnectedBodiesInTagAction,self).__init__()

    def Apply(self,app:MainApp, ops:Dict):
        ls = ops['ls']
        originalMesh=ls.support

        tagToClean=PH.ReadString(ops.get("eTag"))
        mesh = ExtractElementsByTags(originalMesh,[tagToClean])

        AddTagPerBody(mesh)
        elementsInNodalTag={nodalTag.name:ElementFilter(nTag=[nodalTag.name]) for nodalTag in mesh.nodesTags if nodalTag.name.startswith("Body_")}

        nTagsToKeep=[]
        totalMeasure=GetVolume(mesh)
        minRelativeMeasure=PH.ReadFloat(ops.get("minRelativeMeasure",0.0))
        maxRelativeMeasure=PH.ReadFloat(ops.get("maxRelativeMeasure",1.0))
        for tagName,elementFilterInTag in elementsInNodalTag.items():
            measure=GetMeasure(inmesh=mesh,elementFilter=elementFilterInTag)
            relativeMeasure=measure/totalMeasure
            Debug(tagName+": "+str(relativeMeasure))
            if minRelativeMeasure  <  relativeMeasure <  maxRelativeMeasure:
                nTagsToKeep.append(tagName)

        Debug(nTagsToKeep)
        if len(nTagsToKeep):
            elementFiltersToKeep=[elementsInTag
                                  for tagName,elementsInTag in elementsInNodalTag.items()
                                  if tagName in nTagsToKeep]
            unionElementFilter=UnionFilter(filters=elementFiltersToKeep)
            cleanMesh=ExtractElementsByElementFilter(mesh,unionElementFilter)
        else:
            Debug("No nTagsToKeep to keep: all of the "+str(tagToClean)+" portions are outside limits.")

        #Rebuild cleaned tag
        for name,_ in originalMesh.elements.items():
            originalMeshElemType = originalMesh.GetElementsOfType(name)
            if tagToClean not in originalMeshElemType.tags.keys():
                continue

            if len(nTagsToKeep):
                cleanElemType = cleanMesh.GetElementsOfType(name)
                cleanExtractedMeshIds = cleanElemType.originalIds

                extractedMeshElemType = mesh.GetElementsOfType(name)
                cleanTagIdsInOriginalMesh= extractedMeshElemType.originalIds[cleanExtractedMeshIds]

                originalMeshElemType.tags[tagToClean].SetIds(cleanTagIdsInOriginalMesh)
            else:
                originalMeshElemType.tags[tagToClean].SetIds([])

        return RETURN_SUCCESS

RegisterClass("FilterSimplyConnectedBodiesInTag",FilterSimplyConnectedBodiesInTagAction)

def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
