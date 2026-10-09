# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations
from typing import Dict

import numpy as np

from Muscat.Types import MuscatFloat, MuscatIndex
import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.Logger import GetDiffTime
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshContainers.Filters.FilterOperators import UnionFilter
from Muscat.MeshTools.MeshModificationTools import AddTagPerBody,CleanDoubleNodes, ComputeFeatures, CleanDoubleElements
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByElementFilter,CleanLonelyNodes,GetVolume,GetMeasure
from Muscat.Helpers.Logger import Error

from OpenPisco import TopoZones as TZ
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL

from OpenPisco.Actions.ActionFactory import RegisterClass
from OpenPisco.Actions.ActionBase import ActionBase
from OpenPisco.Actions.ActionTools import GetSupportFromLsOrSupport
from OpenPisco.Unstructured.MmgMesher import MmgMesherActionLevelset
from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco.CLApp.OpenPiscoCL import MainApp

class WritetoFileAction(ActionBase):
    """
    Write mesh in file

    Parameters
    ----------
        ls: levelset
        writer: Muscat.IO.WriterBase instance
        filename: string, name and format to use for file (format infered from extension)
        inTempDirectory: bool, whether to write file in temporary directory

    Example in XML:
    <Action type="WriteToFile" ls="1" filename="inputfile.xdmf" />

    Example in python:
    action = WritetoFileAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"filename":"inputfile.xdmf")
    """
    def __init__(self):
        super(WritetoFileAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        needToClose = False
        assert "writer" in ops or "filename" in ops,"Need an output or a filename"
        if "writer" in ops:
            writer = ops['writer']
            writer.Open()
        else:
            filename = ops['filename']
            from  Muscat.IO.IOFactory import InitAllWriters,CreateWriter
            InitAllWriters()
            writer = CreateWriter(filename)
            if "inTempDirectory" in ops and PH.ReadBool(ops['inTempDirectory']):
               filename =  TemporaryDirectory.GetFullFilenameOnTempDirectory(filename)
            if PH.ReadBool(ops.get("binary",'False')) :
                 writer.SetBinary("True")
            writer.Open(filename)
            needToClose = True

        ls = ops['ls']
        writer.Write(ls.support,PointFields= [ls.phi],PointFieldsNames= ["phi"],GridFieldsNames = ["time"],  GridFields = [ GetDiffTime() ] )
        if needToClose:
            writer.Close()
        return RETURN_SUCCESS

RegisterClass("WriteToFile",WritetoFileAction)

class ExtractShapeToFileAction(ActionBase):
    """Read a mesh and a field and export the iso the skin of the Inside3D if it exists if not
    remesh the iso 0 of the field to compute a surface. Save shape to file.

    Parameters
    ----------
        fieldName:"phi"*|string
        inputFilename:|string, mesh format infered from file extension
        outputFilename:|string, mesh format infered from file extension

    Example in XML:
    <Action type="ExtractShapeToFile" inputFilename="inputfile.xdmf" outputFilename="outputfile.xdmf" />

    Example in python:
    action = ExtractShapeToFileAction()
    action.Apply(app=None, ops={"inputFilename":"inputfile.xdmf","outputFilename":"outputfile.xdmf")

    """
    def __init__(self):
        super(ExtractShapeToFileAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        filename = PH.ReadString(ops['inputFilename'])
        phiName = PH.ReadString(ops.get('fieldName', 'phi') )
        from Muscat.IO.UniversalReader import ReadMesh
        support = ReadMesh(filename)
        phi=support.nodeFields[phiName]
        from OpenPisco.Unstructured.Levelset import LevelSet
        levelSet=LevelSet(support=support)
        levelSet.phi=phi

        opsII = { "ls":levelSet, "filename":PH.ReadString(ops['outputFilename']) }
        saveShapeAction=SaveShapeToFileAction()
        try:
            saveShapeAction.Apply(app=app, ops=opsII)
        except ValueError:
            opsII["useMmg"] = True
            saveShapeAction.Apply(app=app, ops=opsII)

        return RETURN_SUCCESS

RegisterClass("ExtractShapeToFile",ExtractShapeToFileAction)

class SaveShapeToFileAction(ActionBase):
    """Save shape to file

    Parameters
    ----------
        ls:levelset object
        useMmg: bool, whether to use mmg to remesh
        writer: Muscat.IO.WriterBase instance
        filename: string, name and format to use for file (format infered from extension)

    Example in XML:
    <Action type="SaveShapeToFile" ls="1" useMmg="True" filename="inputfile.xdmf"/>

    Example in python:
    action = SaveShapeToFileAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"useMmg":True,"filename":"inputfile.xdmf")

    """
    def __init__(self):
        super(SaveShapeToFileAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        useMmg = ops.get('useMmg',False)

        from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags
        from Muscat.MeshTools.MeshModificationTools import ComputeSkin
        from Muscat.MeshTools.MeshInspectionTools import ComputeMeshMinMaxLengthScale
        try:
            import vtk
        except ImportError:
            from paraview import vtk as vtk
        if ls.conform :
            inside = ExtractElementsByTags(ls.support,[TZ.Inside3D ])
            skin = ComputeSkin(inside)
            CleanLonelyNodes(skin)
        elif useMmg :
            lsConform = LevelSet(ls)
            lsConform.conform = True
            lsConformMesh = type(ls.support)()
            lsConformMesh.CopyProperties(ls.support)
            lsConformMesh.nodes = np.copy(ls.support.nodes)
            lsConformMesh.originalIDNodes = ls.support.originalIDNodes
            for name,data in ls.support.elements.items():
                lsConformMesh.elements.AddContainer(data)
            (edgemesh,_) = ComputeFeatures(lsConformMesh)
            for name,data in edgemesh.elements.items():
                lsConformMesh.GetElementsOfType(name).Merge(data)
            CleanDoubleElements(lsConformMesh)
            lsConform.TakePhiAndMesh(ls.phi,lsConformMesh)
            hmin,hmax = ComputeMeshMinMaxLengthScale(lsConformMesh)
            MmgMesherActionLevelset(lsConform, {"iso":0.0,"hmin":hmin,"hmax":hmax,"rmc":True,"nr":True})
            inside = ExtractElementsByTags(lsConform.support,[TZ.Inside3D ])
            skin = ComputeSkin(inside)
            CleanLonelyNodes(skin)
        else:
            import Muscat.Bridges.vtkBridge as vtkBridge
            saveOldNodeFields = ls.support.nodeFields
            ls.support.nodeFields = {"phi":ls.phi}
            vtkObject = vtkBridge.MeshToVtk(ls.support)
            vtkObject.GetPointData().SetActiveScalars("phi")

            ls.support.nodeFields = saveOldNodeFields

            # Exterior surface extraction
            extractSurface1 = vtk.vtkDataSetSurfaceFilter()
            extractSurface1.SetInputData(vtkObject)
            extractSurface1.SetInputArrayToProcess(0,0,0,0,'phi')
            extractSurface1.Update()

            # keep only in inside part
            clip = vtk.vtkClipPolyData()
            clip.SetInputConnection(extractSurface1.GetOutputPort())
            clip.SetInputArrayToProcess(0,0,0,0,'phi')
            clip.InsideOutOn()
            clip.SetValue(0.)

            #trianglulate the surface
            triangle = vtk.vtkTriangleFilter()
            triangle.SetInputConnection(clip.GetOutputPort())
            triangle.Update()

            iso = vtk.vtkContourFilter()
            iso.SetInputData(vtkObject)
            iso.ComputeGradientsOff()
            iso.ComputeNormalsOff()
            iso.SetNumberOfContours(1)
            iso.SetValue(0,0)
            iso.SetInputArrayToProcess(0,0,0,0,'phi')
            iso.SetComputeScalars(True)
            iso.SetGenerateTriangles(True)
            iso.Update()
            appendFilter = vtk.vtkAppendPolyData()

            appendFilter.AddInputConnection(iso.GetOutputPort())
            appendFilter.AddInputConnection(triangle.GetOutputPort())
            appendFilter.Update()

            # try to clean the mesh if the iso surface is tanget to the outher
            # surface of the mesh
            cleaner = vtk.vtkCleanPolyData()
            cleaner.SetInputConnection(appendFilter.GetOutputPort())
            cleaner.ConvertLinesToPointsOn()
            cleaner.ConvertPolysToLinesOn()
            cleaner.ConvertStripsToPolysOn()
            cleaner.Update()

            try:
                # this class is not present in the currect version of vtk
                cleaner2 = vtk.vtkRemoveDuplicatePolys()
                cleaner2.SetInputConnection(cleaner.GetOutputPort())
                cleaner2.Update()
            except Exception as e:
                Error(e)
                cleaner2 = cleaner

            data = cleaner.GetOutput(0)

            skin = vtkBridge.VtkToMesh(data)

        assert "writer" in ops or "filename" in ops,"Need an output or a filename"
        if "writer" in ops:
            writer = ops['writer']
            writer.Open()
        else:
            filename = ops['filename']
            from  Muscat.IO.IOFactory import InitAllWriters,CreateWriter
            InitAllWriters()
            writer = CreateWriter(filename)
            writer.Open(filename)

        writer.Write(skin )
        writer.Close()
        return RETURN_SUCCESS

RegisterClass("SaveShapeToFile",SaveShapeToFileAction)


class WriteFilterConnexBodiesFromFileAction(ActionBase):
    """    
    Filter simply connected bodies for tag within mesh.
        -Read mesh
        -Compute each connex bodies
        -Keep those whose measure (volume in 3D, surface en 2D) is within the interval [minRelativeMeasure*meas(support); maxRelativeMeasure*meas(support)],
        -Remove the others
        -Write result on disk

    Parameters
    ----------
        minRelativeMeasure: minimum body relative measure to keep, default value 0
        maxRelativeMeasure: maximum body relative measure to keep, default value 1
        inputFile: str, input path for mesh in the stl format to be filtered
        outputfile: str, output path for filtered mesh (format infered from file extension)

    Example in XML:
    <Action type="WriteFilterConnexBodiesFromFile" inputFile="inputfile.stl" outputfile="outputfile.xdmf"/>

    Example in python:
    action = WriteFilterConnexBodiesFromFileAction()
    action.Apply(app=None, ops={"ls":levelsetObject, "inputFile":"inputfile.stl","outputfile":"outputfile.xdmf")

    """
    def __init__(self):
        super(WriteFilterConnexBodiesFromFileAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        from Muscat.IO.StlReader import ReadStl
        inputFile = ops['inputFile']
        mesh=ReadStl(fileName=inputFile)
        CleanDoubleNodes(mesh)
        AddTagPerBody(mesh)
        elementsInNodalTag={nodalTag.name:ElementFilter(nTags=[nodalTag.name]) for nodalTag in mesh.nodesTags}

        nTagsToKeep=[]
        totalMeasure=GetVolume(mesh)
        minRelativeMeasure=PH.ReadFloat(ops.get("minRelativeMeasure",0.0))
        maxRelativeMeasure=PH.ReadFloat(ops.get("maxRelativeMeasure",1.0))
        for tagName,elementFilterInTag in elementsInNodalTag.items():
            measure=GetMeasure(inmesh=mesh,elementFilter=elementFilterInTag)
            relativeMeasure=measure/totalMeasure
            if minRelativeMeasure  <  relativeMeasure <  maxRelativeMeasure:
                nTagsToKeep.append(tagName)

        elementFiltersToKeep={tagName:elementsInTag
                              for tagName,elementsInTag in elementsInNodalTag.items()
                              if tagName in nTagsToKeep
                             }
        unionElementFilter=UnionFilter(mesh,elementFiltersToKeep.values())
        cleanMesh=ExtractElementsByElementFilter(mesh,unionElementFilter)
        bodyNodeTagNames=[nodalTagName for nodalTagName in cleanMesh.nodesTags.keys() if nodalTagName.startswith("Body")]
        cleanMesh.nodesTags.DeleteTags(bodyNodeTagNames)
        CleanLonelyNodes(cleanMesh)

        filename = ops['outputFile']
        from Muscat.IO.IOFactory import InitAllWriters,CreateWriter
        InitAllWriters()
        writer = CreateWriter(filename)
        writer.Open(filename)
        writer.Write(cleanMesh )
        writer.Close()
        return RETURN_SUCCESS

RegisterClass("WriteFilterConnexBodiesFromFileAction",WriteFilterConnexBodiesFromFileAction)

class SetAllToInsideAction(ActionBase):
    """    
    Set all element as Inside elements

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="SetAllToInside" ls="1"/>

    Example in python:
    action = SetAllToInsideAction()
    action.Apply(app=None, ops={"ls":levelsetObject)

    """
    def __init__(self):
        super(SetAllToInsideAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        dim = ls.support.GetElementsDimensionality()
        for name,data in ls.support.elements.items():
            if ED.dimensionality[name] == dim:
                data.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(data.GetNumberOfElements()))
                data.tags.CreateTag(TZ.Outside3D,False)
            if ED.dimensionality[name] == dim-1:
                data.tags.CreateTag(TZ.InterSurf,False).SetIds(np.arange(data.GetNumberOfElements()))
        return RETURN_SUCCESS

RegisterClass("SetAllToInside",SetAllToInsideAction)

class AddBulkAction(ActionBase):
    """    
    Add all elements in the Bulk tag

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="AddBulk" ls="1"/>

    Example in python:
    action = AddBulkAction()
    action.Apply(app=None, ops={"ls":levelsetObject)

    """
    def __init__(self):
        super(AddBulkAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        if 'ls' in ops:
            support = ops['ls'].support
        elif 'grid' in ops:
            support = ops['grid']
        dim = support.GetElementsDimensionality()
        for name,data in support.elements.items():
            if ED.dimensionality[name] == dim:
                data.tags.CreateTag(TZ.Bulk,False).SetIds(np.arange(data.GetNumberOfElements()))
        return RETURN_SUCCESS

RegisterClass("AddBulk",AddBulkAction)

class AddSkinAction(ActionBase):
    """    
    Add skin to the underlying support

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="AddSkin" ls="1"/>

    Example in python:
    action = AddSkinAction()
    action.Apply(app=None, ops={"ls":levelsetObject)

    """
    def __init__(self):
        super(AddSkinAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
      from OpenPisco.Actions.ActionTools import GetSupportFromLsOrSupport
      support = GetSupportFromLsOrSupport(ops)
      from Muscat.MeshTools.MeshModificationTools import ComputeSkin
      ComputeSkin(support, inPlace=True)
      return RETURN_SUCCESS

RegisterClass("AddSkin",AddSkinAction)

class PrintMeshAction(ActionBase):
    """    
    Print mesh

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="PrintMesh" ls="1"/>

    Example in python:
    action = PrintMeshAction()
    action.Apply(app=None, ops={"ls":levelsetObject)

    """
    def __init__(self) -> None:
        """Simple action to print the state of the
        """
        super(PrintMeshAction, self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        print(ops['ls'].support)
        return RETURN_SUCCESS

RegisterClass("PrintMesh", PrintMeshAction)

class CreateETagFromNTagAction(ActionBase):
    """
    Create an element tag from a nodal tag

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="CreateETagFromNTag" ls="1" dim="3" nTag="MyTag"/>

    Example in python:
    action = CreateETagFromNTagAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"dim":3 "nTag":"MyTag")
    """
    def __init__(self):
        super(CreateETagFromNTagAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        dim = int(ops["dim"])
        ls = ops['ls']

        ntag = PH.ReadString(ops['nTag'])

        for selection in ElementFilter(nTag=ntag,dimensionality=dim)(ls.support):
            selection.elements.tags.CreateTag("eTag_"+ntag).SetIds(selection.indices)

        return RETURN_SUCCESS

RegisterClass("CreateETagFromNTag",CreateETagFromNTagAction)


class AddEdgesAction(ActionBase):
    """
    Add edges to support

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="AddEdges" ls="1"/>

    Example in python:
    action = AddEdgesAction()
    action.Apply(app=None, ops={"ls":levelsetObject)
    """
    def __init__(self):
        super(AddEdgesAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):

      ls = ops['ls']

      from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags
      from Muscat.MeshTools.MeshModificationTools import ComputeSkin

      """
      We start by adding the edges of the extrasurfaces tags
      """
      if 'extraBorders' in ops:
          for tagname in PH.ReadStrings(ops['extraBorders']):
              submesh = ExtractElementsByTags(ls.support,[tagname] ,cleanLonelyNodes=False)
              elemndim = 0
              for elemname,data in submesh.elements.items():
                  if data.GetNumberOfElements() == 0:
                      continue
                  elemndim = max(elemndim, ED.dimensionality[elemname])

              barmesh = ComputeSkin(submesh,md=elemndim)
              barmesh.GetElementsOfType(ED.Bar_2).tags.CreateTag("Ridges").SetIds(np.arange(barmesh.GetElementsOfType(ED.Bar_2).GetNumberOfElements()))


              ls.support.GetElementsOfType(ED.Bar_2).Merge(barmesh.GetElementsOfType(ED.Bar_2))

          """
          merge of Corners
          """

      FeatureAngle= PH.ReadFloat(ops.get("ar",90+45))
      (edgemesh,skinmesh) = ComputeFeatures(ls.support,featureAngle=FeatureAngle)
      for name,data in edgemesh.elements.items():
          ls.support.GetElementsOfType(name).Merge(data)

      CleanDoubleElements(ls.support)
      if PH.ReadBool(ops.get("addskin",False)):
        for name,data in skinmesh.elements.items():
          ls.support.GetElementsOfType(name).Merge(data)

      if "Corners" in edgemesh.nodesTags and len(edgemesh.nodesTags["Corners"]):
         ls.support.nodesTags.CreateTag("Corners",errorIfAlreadyCreated=False).Merge(edgemesh.nodesTags["Corners"])

      if "Corners" in ls.support.nodesTags and len(ls.support.nodesTags["Corners"]):
        tag = ls.support.nodesTags.CreateTag(TZ.RequiredVertices,errorIfAlreadyCreated=False)
        tag.Merge(ls.support.nodesTags["Corners"])

      return RETURN_SUCCESS

RegisterClass("AddEdges",AddEdgesAction)


class SetAsRequiredAction(ActionBase):
    """
    Set tag(s) as required

    Parameters
    ----------
        ls: levelset object
        support: mesh object
        eTags: collection of element tags, optional
        nTags: collection of nodal tags, optional

    Example in XML:
    <Action type="SetAsRequired" ls="1" eTags="mytag1 mytag2"/>
    <Action type="SetAsRequired" support="1" eTags="mytag1 mytag2"/>

    Example in python:
    action = SetAsRequiredAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"eTags":[mytag1,mytag2])
    action.Apply(app=None, ops={"support":meshObject,"eTags":[mytag1,mytag2])
    """
    def __init__(self):
        super(SetAsRequiredAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        from OpenPisco.Actions.ActionTools import GetSupportFromLsOrSupport
        support = GetSupportFromLsOrSupport(ops)
        if 'eTags' in ops:
            for tag in PH.ReadStrings(ops['eTags']):
                for name,data in support.elements.items():
                    if tag in data.tags and len(data.tags[tag]):
                        errorMessage = "Can not set element "+str(name)\
                        +" as required (allowed :"+ED.Bar_2.name+" "+ED.Triangle_3.name+")"
                        assert name in [ED.Bar_2,ED.Triangle_3], errorMessage
                        if name == ED.Bar_2 :
                            requiredElement = TZ.RequiredEdges
                        elif name == ED.Triangle_3:
                            requiredElement = TZ.RequiredTriangles
                        data.tags.CreateTag(requiredElement,errorIfAlreadyCreated=False).Merge(other=data.tags[tag])

        if 'nTags' in ops:
            for tag in PH.ReadStrings(ops['nTags']):
                ntags = support.nodesTags
                if tag in ntags and len(ntags[tag]):
                    ntags.CreateTag(TZ.RequiredVertices,errorIfAlreadyCreated=False).Merge(other=ntags[tag])
        return RETURN_SUCCESS

RegisterClass("SetAsRequired",SetAsRequiredAction)

class SetAsCornersAction(ActionBase):
    """
    Set nodal tag as a corner

    Parameters
    ----------
        ls: levelset object
        support: mesh object
        nTags: collection of nodal tags, optional

    Example in XML:
    <Action type="SetAsCorners" ls="1" nTags="mytag1 mytag2"/>
    <Action type="SetAsCorners" support="1" nTags="mytag1 mytag2"/>

    Example in python:
    action = SetAsCorners()
    action.Apply(app=None, ops={"ls":levelsetObject,"eTags":[mytag1,mytag2])
    action.Apply(app=None, ops={"support":meshObject,"eTags":[mytag1,mytag2])
    """
    def __init__(self):
        super(SetAsCornersAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        from OpenPisco.Actions.ActionTools import GetSupportFromLsOrSupport
        support = GetSupportFromLsOrSupport(ops)
        if 'nTags' in ops:
            for tag in PH.ReadStrings(ops['nTags']):
                ntags = support.nodesTags
                if tag in ntags and len(ntags[tag]):
                    ntags.CreateTag("Corners",errorIfAlreadyCreated=False).Merge(other=ntags[tag])
        return RETURN_SUCCESS

RegisterClass("SetAsCorners",SetAsCornersAction)

class CleanElemTagsAction(ActionBase):
    """
    Clean element tags

    Parameters
    ----------
        ls: levelset object
        filter: filter specific element tags, optional

    Example in XML:
    <Action type="CleanElementTags" ls="1"/>

    Example in python:
    action = CleanElemTagsAction()
    action.Apply(app=None, ops={"ls":levelsetObject)
    """
    def __init__(self):
        super(CleanElemTagsAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        tags = ls.support.GetNamesOfElementTags()
        if 'filter' in ops:
            fil = ops['filter']
            res = [k for k in tags if fil in k]
            ls.support.DeleteElemTags(res)
        else:
            ls.support.DeleteElemTags(tags)
        return RETURN_SUCCESS

RegisterClass("CleanElementTags",CleanElemTagsAction)

class CleanNodesTagsAction(ActionBase):
    """
    Clean nodal tags

    Parameters
    ----------
        ls: levelset object
        filter: filter specific nodal tags, optional

    Example in XML:
    <Action type="CleanNodesTags" ls="1"/>

    Example in python:
    action = CleanNodesTagsAction()
    action.Apply(app=None, ops={"ls":levelsetObject)
    """
    def __init__(self):
        super(CleanNodesTagsAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        tags = ls.support.nodesTags.keys()
        if 'filter' in ops:
            fil = ops['filter']
            res = [k for k in tags if fil in k]
            ls.support.nodesTags.DeleteTags(res)
        else:
            ls.support.nodesTags.DeleteTags(tags)
        return RETURN_SUCCESS

RegisterClass("CleanNodesTags",CleanNodesTagsAction)

class RenameTagAction(ActionBase):
    """
    Rename tag

    Parameters
    ----------
        ls: levelset object
        support: mesh object
        eTag: element tag name
        nTag: nodal tag name, str
        name: new name for tag, str

    Example in XML:
    <Action type="RenameTag" ls="1" eTag=myTag name="myNewTag"/>
    <Action type="RenameTag" support="1" eTag="myTag" name="myNewTag"/>

    Example in python:
    action = RenameTagAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"eTag":myTag,"name":myNewTag)
    action.Apply(app=None, ops={"support":meshObject,"eTag":myTag,"name":myNewTag)
    """
    def __init__(self):
        super(RenameTagAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        support = GetSupportFromLsOrSupport(ops)
        name = PH.ReadString(ops['name'])

        if 'eTag' in ops and 'nTag' in ops:
            raise RuntimeError("cant provide eTag and nTag")
        elif 'eTag' in ops:
            eTag = PH.ReadString(ops['eTag'])

            for _,data in support.elements.items():
                if eTag in data.tags:
                    data.tags.RenameTag(eTag,name,noError=True)
        elif 'nTag' in ops:
            nTag = PH.ReadString(ops['nTag'])
            support.nodesTags.RenameTag(nTag,name,noError=True)

        return RETURN_SUCCESS

RegisterClass("RenameTag",RenameTagAction)

class CleanInternalFacesAction(ActionBase):
    """
    Clean internal faces

    Parameters
    ----------
        ls: levelset object

    Example in XML:
    <Action type="CleanInternalFaces" ls="1"/>

    Example in python:
    action = CleanInternalFacesAction()
    action.Apply(app=None, ops={"ls":levelsetObject)
    """
    def __init__(self):
        super(CleanInternalFacesAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        from Muscat.MeshTools.MeshModificationTools import DeleteInternalFaces
        ls = ops['ls']
        DeleteInternalFaces(ls.support)
        return RETURN_SUCCESS

RegisterClass("CleanInternalFaces", CleanInternalFacesAction)


class DeleteElementsAction(ActionBase):
    """
    Delete elements within zone or element filter in levelset support

    Parameters
    ----------
        ls: levelset object
        zone: ImplicitGeometry object
        ef: ElementFilter object

    Example in XML:
    <Action type="DeleteZone" ls="1" zone="1"/>
    <Action type="DeleteElements" ls="1" ef="1"/>

    Example in python:
    action = DeleteElementsAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"zone":implicitGeometryObject)
    action.Apply(app=None, ops={"ls":levelsetObject,"ef":elementFilterObject)
    """
    def __init__(self):
        super(DeleteElementsAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        from Muscat.MeshTools.MeshTools import  GetElementsCenters

        ls = ops['ls']
        pos = GetElementsCenters(ls.support)
        if "zone" in ops:
            mask = ops['zone'].ApplyVector(pos)
            from Muscat.MeshTools.MeshModificationTools import DeleteElements
            DeleteElements( ls.support, mask <= 0)
            return RETURN_SUCCESS
        elif "ef" in ops:
            from Muscat.MeshContainers.Filters.FilterTools import ReadElementFilter
            from Muscat.MeshContainers.Filters.FilterOperators import ComplementaryFilter
            ef = ReadElementFilter(ops["ef"])
            from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByElementFilter
            res = ExtractElementsByElementFilter(ls.support, ComplementaryFilter(ef), copy=False)

            res.originalIDNodes = np.arange(ls.support.GetNumberOfNodes(), dtype=MuscatIndex)
            CleanLonelyNodes(res)
            if ls.support.GetNumberOfNodes() != res.GetNumberOfNodes():
                phi = ls.phi[res.originalIDNodes]
                ls.TakePhiAndMesh(phi,res)
            else:
                ls.TakePhiAndMesh(ls.phi,res)
            return RETURN_SUCCESS
        return RETURN_FAIL

RegisterClass("DeleteZone", DeleteElementsAction)
RegisterClass("DeleteElements", DeleteElementsAction)

class MirrorLevelSetAction(ActionBase):
    """
    Mirror levelset from coordinates

    Parameters
    ----------
        ls: levelset object
        x : Optional[MuscatFloat], optional
        the position of the yz plane to build the symmetric part, if None, no symmetry to the plane yz
        , by default None
        y : Optional[MuscatFloat], optional
        the position of the xz plane to build the symmetric part, if None, no symmetry to the plane xz
        , by default None
        z : Optional[MuscatFloat], optional
        the position of the xy plane to build the symmetric part, if None, no symmetry to the plane xy
        _description_, by default None


    Example in XML:
    <Action type="MirrorLevelSet" ls="1" X="0.0"/>

    Example in python:
    action = MirrorLevelSetAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"X":0.0)
    """
    def __init__(self):
        super(MirrorLevelSetAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        x=None
        y=None
        z=None
        if "X" in ops:
            x= PH.ReadFloat(ops["X"])
        if "Y" in ops:
            y= PH.ReadFloat(ops["Y"])
        if "Z" in ops:
            z= PH.ReadFloat(ops["Z"])
        from Muscat.MeshTools.MeshCreationTools import MirrorMesh
        from Muscat.MeshTools.MeshModificationTools import CleanDoubleNodes, CleanDoubleElements, RigidBodyTransformation
        ls.support.nodeFields["phi"] = ls.phi 
        ls.support = MirrorMesh(ls.support,x=x,y=y,z=z,propagateToNodeFields=True)
        CleanDoubleNodes(ls.support,propagateToNodeFields=True)
        CleanDoubleElements(ls.support)
        ls.phi = ls.support.nodeFields["phi"] 

        return RETURN_SUCCESS

RegisterClass("MirrorLevelSet",MirrorLevelSetAction)

class SubDivideMeshAction(ActionBase):
    """
    Subdivide mesh

    Parameters
    ----------
        ls: levelset object
        level: int, level of mesh refinement, default to 1

    Example in XML:
    <Action type="SubDivideMesh" ls="1"/>

    Example in python:
    action = SubDivideMeshAction()
    action.Apply(app=None, ops={"ls":levelsetObject)
    """
    def __init__(self):
        super().__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        level = PH.ReadInt(ops.get("level",1))
        from Muscat.MeshTools.MeshCreationTools import SubDivideMesh
        newSupport = SubDivideMesh(mesh=ls.support, level=level)

        ls.TakePhiAndMesh(np.zeros(newSupport.GetNumberOfNodes() ,dtype=MuscatFloat), newSupport )

        return RETURN_SUCCESS

RegisterClass("SubDivideMesh",SubDivideMeshAction)

class WriteSurfaceToStlAction(ActionBase):
    """
    Write surface described by an element tag to Stl 

    Parameters
    ----------
        ls: levelset object
        eTag: str, tag to extract from support
        filename: str, file name

    Example in XML:
    <Action type="WriteSurfaceToStl" ls="1", eTag="myTag" filename="myFile.stl"/>

    Example in python:
    action = WriteSurfaceToStlAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"eTag":myTag,"filename":"myFile.stl")
    """
    def __init__(self):
        super().__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        tagname = PH.ReadString(ops['eTag'])

        from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags
        res = ExtractElementsByTags(ls.support,[tagname ])

        from Muscat.IO.StlWriter import WriteMeshToStl
        filename = ops.get('filename' ,tagname+".stl")
        print("Writing file :  {}".format(filename))
        WriteMeshToStl(filename,res)

        return RETURN_SUCCESS

RegisterClass("WriteSurfaceToStl", WriteSurfaceToStlAction)


def CheckIntegrity():
    return "ok"
if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity())
