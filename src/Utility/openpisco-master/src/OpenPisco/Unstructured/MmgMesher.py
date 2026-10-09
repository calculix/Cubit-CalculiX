# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import numpy as np

from Muscat.Helpers.IO.Which import Which
from Muscat.Helpers.Logger import Info, Debug
import Muscat.Helpers.ParserHelper as PH

from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.Types import MuscatIndex, MuscatFloat
import Muscat.MeshContainers.ElementsDescription as ED
from  Muscat.MeshContainers.Mesh import Mesh
import Muscat.MeshTools.MeshInspectionTools as UMIT
from Muscat.IO.MeshTools import ASCIIName
import Muscat.IO.MeshTools as MT

from Muscat.FE.FETools import PrepareFEComputation

from OpenPisco.ExternalTools.AppExecutableTools import AppExecutableBase, CommandLineBuilder
import OpenPisco.TopoZones as TZ
from OpenPisco.MuscatExtentions.PartitionRefining import PartitionRefining
from OpenPisco.BaseElements import elementsTypeSupportedForDimension

#-hausd: The -hausd option controls the boundary approximation:Thus, a low Hausdorff parameter leads to refine the high curvature areas.
#-hgrad: The hgrad option allows to set the gradation value. It controls the ratio between two adjacent edges.
# -hmin / -hmax: The -hmin and -hmax options allows to trun
# -m:  The -m option allows to impose (roughly) the maximal memory size used by the mmg applications (in MB). By default, our applications can takes 50% of the available memory. Use the -m option to increase or decrease this feature.
#      For example, to impose a maximal memory size of 2 GB, you just need to call the mmg application with the -m 2000 argument.
#-optim: The -optim option allows to preserve the initial sizes of the mesh edges in order to improve the mesh quality without modifying the edge lengths.
#-opnbdy: The -opnbdy option allow to preserve input triangles at the interface of two domains of the same reference
#-rmc [val]: The -rmc option enable the removal of componants whose volume fraction is less than val (1e-5 if val not given) of the mesh volume (ls mode).

def ComputeDistanceField(nmesh,surf,computeDistanceWith="muscat"):
        
    if computeDistanceWith.lower() =="muscat":
        Debug("computing distance using muscat")
        from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometryStl
        op = ImplicitGeometryStl()
        op.SetSurface(surf)
        newphi = abs(op(nmesh.nodes))
        Debug("computing distance using muscat Done")

    elif computeDistanceWith.lower() =="vtk":

        Debug("computing distance using vtk")

        from Muscat.Bridges.vtkBridge import MeshToVtk, NumpyFieldToVtkField, VtkFieldToNumpyFieldWithDims
        from vtkmodules.vtkCommonDataModel import vtkPolyData
        from vtkmodules.vtkFiltersCore import vtkImplicitPolyDataDistance
        from vtkmodules.vtkCommonCore import vtkDoubleArray

        surfVTK = MeshToVtk(surf, vtkobject=vtkPolyData())

        implicitFunction = vtkImplicitPolyDataDistance()
        implicitFunction.SetNoValue(-100.)
        implicitFunction.SetInput(surfVTK)

        ## we have some problem when a new point does not have a normal projection to the surface
        ## this is the case when the zero-iso is not close, so we compute the abs
        # and the we flip the sign of TZ.Inside3D
        a = NumpyFieldToVtkField(nmesh.nodes,"nodes")
        out = vtkDoubleArray()
        implicitFunction.EvaluateFunction(a, out)
        newphi = abs(VtkFieldToNumpyFieldWithDims(out)[1])
        Debug("computing distance using vtk Done")

    elif computeDistanceWith.lower() =="meshdist":
        Debug("computing distance using meshdist")
        from OpenPisco.Unstructured.Meshdist import Meshdist

        obj = Meshdist()
        obj.SetSupport(nmesh)
        obj.SetSurfaceSupport(surf)
        obj.WriteSupport()
        obj.WriteSurfaceSupport()
        obj.GenerateDist(ls=False, it=10)
        phi = obj.GetNewPhi()    
        newphi = abs(phi.ravel())
        Debug("computing distance using meshdist Done")
    else:
        raise ValueError("Unknown distance computation method "+str(computeDistanceWith)+".")
    
    insideIds = ElementFilter(eTag=[TZ.Inside3D]).GetNodesIndices(nmesh)
    newphi[insideIds] *= -1
        
    return newphi


def MmgMesherActionRemesh(support, child):

    obj = MmgMesher()
    obj.SetSupport(support)
    obj.keepGeneratedFiles = PH.ReadBool(child.get('keepGeneratedFiles',False) )
    obj.WriteSupport()
    if 'met' in child and PH.ReadBool(child["met"]):
        obj.WriteMetricField()

    obj.GenerateMesh(child)

    nmesh = obj.GetNewMesh()
    return nmesh


def MmgMesherActionAddRidges(levelset, options={}):
    ops = {"noinsert":True, "noswap":True, "nomove":True, "nosurf":True}
    if "ar" in options :
        ops["ar"] =  PH.ReadFloat(options["ar"])

    obj = MmgMesher()
    obj.SetSupport(levelset.support)
    obj.WriteSupport()
    obj.GenerateMesh(ops)
    obj.keepGeneratedFiles = PH.ReadBool(options.get('keepGeneratedFiles',False) )

    nmesh =  obj.GetNewMesh()
    newBars = nmesh.GetElementsOfType(ED.Bar_2)

    newBars.tags.CreateTag(TZ.Ridges,False).SetIds(np.arange(newBars.GetNumberOfElements()))
    levelset.TakePhiAndMesh(levelset.phi, nmesh)
    return 0

def GetIsoZeroSurface(nmesh,interfaceZoneName,skinTags=["Skin"],setPhiZeroOnBoundary=False):
    if setPhiZeroOnBoundary:
        inside3D =  UMIT.ExtractElementsByTags(nmesh, [TZ.Inside3D,TZ.ExterBars,interfaceZoneName]+skinTags)
        from OpenPisco.PhysicalSolvers.PhysicalSolversTools import  CheckBorderElements
        CheckBorderElements(inside3D)
        
        dimensionality = nmesh.GetElementsDimensionality()
        skinElemType = elementsTypeSupportedForDimension[dimensionality-1]
        ids =  inside3D.GetElementsOfType(skinElemType).tags["InteriorSkin"].GetIds() 
        ids = inside3D.GetElementsOfType(skinElemType).originalIds[ids]
        
        for _,data,_ in ElementFilter(dimensionality=dimensionality-1)(nmesh): 
            data.tags.CreateTag(interfaceZoneName,False).SetIds(ids)

    surf = UMIT.ExtractElementsByTags(nmesh, [interfaceZoneName])

    return surf 


def MmgMesherActionLevelset(levelset, child):
    """
    iso  : iso surface to mesh (0.0)
    hmin : Minimal edge size
    hmax : Maximal edge size
    hsiz : Constant edge size
    rmc  : Remove connected components smaller than a given threshold
    ar   : Value for angle detection
    hausd: Maximal Hausdorff distance for the boundaries approximation
    nr   : No angle detection
    computeDistanceWith (Muscat/meshdist/vtk) : update signed distance function using Muscat/meshdist/vtk
    keepGeneratedFiles                 : keep files generated during remeshing
    met  : use isotropic metric field 'met' to drive the remeshing
    The metric field has to be calculated before calling this function and stored in levelset.support.nodeFields["met"]
    """
    obj = MmgMesher()
    obj.SetBinaryFlag(True)
    obj.keepGeneratedFiles = PH.ReadBool(child.get('keepGeneratedFiles',False) )
    useOriginalSupport = PH.ReadBool(child.get('useOriginalSupport',False) )
    computeDistanceWith = PH.ReadString(child.get('computeDistanceWith','muscat'))
    setPhiZeroOnBoundary =  PH.ReadBool(child.get('setPhiZeroOnBoundary',False) )
    cleanNodalTagsOnIso =  PH.ReadBool(child.get('cleanNodalTagsOnIso',False) )

    if useOriginalSupport and (levelset.originalSupport is not None):
        obj.SetSupport(levelset.originalSupport)
        if 'iso' in child:
            # if we remesh in level set mode we need to transport to current phi to the original support
            spaces,numberings,_, _ = PrepareFEComputation(levelset.support,numberOfComponents=1)
            from Muscat.FE.Fields.FEField import FEField
            field = FEField(name = "",mesh=levelset.support,space=spaces,numbering=numberings[0])
            field.data = levelset.phi
            from Muscat.MeshTools.MeshFieldOperations import GetFieldTransferOp
            op,_,_ = GetFieldTransferOp(field,levelset.originalSupport.nodes)
            phi= op.dot(field.data)
            obj.SetRemeshWithIso()
    else:
        if 'iso' in child:
            # a new inside and outside will be created so no need to export
            # the topozones.
            obj.SetSupport(levelset.support)
            obj.SetRemeshWithIso()
        else:
            obj.SetSupport(levelset.support,writeTopoZones=True)

        phi = levelset.phi

    obj.WriteSupport()

    if 'met' in child and PH.ReadBool(child["met"]):
        obj.WriteMetricField()

    if 'iso' in child:
        obj.WriteLevelSet(phi)

    obj.WriteInputParametersFile(child)
    obj.GenerateMesh(child)

    nmesh = obj.GetNewMesh()
    dimensionality = nmesh.GetElementsDimensionality()
    interfaceZoneName= TZ.InterZoneByDim[dimensionality]
    surf = GetIsoZeroSurface(nmesh,interfaceZoneName,skinTags=["Skin"],setPhiZeroOnBoundary=setPhiZeroOnBoundary)

    if surf.GetNumberOfElements() == 0 :
        print("Warning from MMG interface: iso not found")
        newphi = np.zeros(nmesh.GetNumberOfNodes())
    else:
        newphi =  ComputeDistanceField(nmesh,surf,computeDistanceWith=computeDistanceWith)
    if "iso" in child:
        if obj.is3DTopo:
            #elimination of the bars in the InterBars tag and tag the rest of the bars as ridges
            elems = nmesh.GetElementsOfType(ED.Bar_2)
            mask = np.ones(elems.GetNumberOfElements(), dtype=bool)
            if TZ.InterBars in nmesh.GetElementsOfType(ED.Bar_2).tags :
                mask[ nmesh.GetElementsOfType(ED.Bar_2).tags[TZ.InterBars].GetIds() ] = False

            nbars = UMIT.ExtractElementsByMask(elems,mask)
            nbars.tags.CreateTag(TZ.Ridges,False).SetIds(np.arange(nbars.GetNumberOfElements()))

            nmesh.elements.AddContainer(nbars)
            
        if cleanNodalTagsOnIso:
           CleanNodalTagsOnIso(nmesh,newphi,PH.ReadFloat(child["iso"]))

    elif obj.is3DTopo:
        #tag the rest of the bars as ridges
        nbars = nmesh.GetElementsOfType(ED.Bar_2)
        nbars.tags.CreateTag(TZ.Ridges,False).SetIds(np.arange(nbars.GetNumberOfElements()))

    levelset.TakePhiAndMesh(newphi,nmesh)

    if levelset.support.GetElementsDimensionality()>dimensionality:
        from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter,UnionFilter
        elementsFilters=[None]*dimensionality
        for filterDimension in range(1,dimensionality+1):
           elementsFilters[filterDimension-1]=ElementFilter(dimensionality=filterDimension)
           elementFilterUnion=UnionFilter(mesh=levelset.support,filters=elementsFilters)
           levelset.support=UMIT.ExtractElementsByElementFilter(inmesh=levelset.support,elementFilter=elementFilterUnion)

    return obj.TRB.nodalRefNumber, obj.TRB.elemRefNumber

def CleanNodalTagsOnIso(mesh,phi,iso=0.0):
    #elimination of the Requiredvertices and Corners on the iso zeros
    tagsToFilter = [TZ.RequiredVertices,TZ.Corners]
    for tagToFilter in tagsToFilter:
        if tagToFilter in mesh.nodesTags:
            nodes = mesh.nodesTags[tagToFilter].GetIds()
            mask = np.abs(phi[nodes]-iso) >  1e-8
            newNodes = nodes[mask]
            mesh.nodesTags[tagToFilter].SetIds(newNodes)

class TagsRefsBridge():
    """
    Class to convert the tag names to reference for mmg3D
    """

    def __init__(self):
        super(TagsRefsBridge,self).__init__()
        self.nodalUserTagNames = None
        self.elementUserTagsNames = None
        self.nodalRefNumber = None
        self.elemRefNumber = None

        self.nodalGroupname_by_groupnumber = dict()
        self.elemGroupname_by_groupnumber = dict()

        self.exportInternalMMGZones = False
        self.importInternalMMGZones = True

        self.iso = False
        self.mapOldToNewRefNumbers = []

        self._offset = 5
        ##(support,name,number)
        self.elementsInternalZones = [(ED.Bar_2,TZ.ExterBars,0),
                   (ED.Triangle_3,TZ.ExterSurf,0),
                   (ED.Tetrahedron_4,TZ.Outside3D,TZ.MG_PLUS),
                   (ED.Tetrahedron_4,TZ.Inside3D,TZ.MG_MINUS),
                   (ED.Bar_2,TZ.InterBars,TZ.MMG_ISO),
                   (ED.Triangle_3,TZ.InterSurf,TZ.MMG_ISO),
                   ]

    def Reset(self):
        self.nodalUserTagNames = None
        self.elementUserTagsNames = None
        self.nodalRefNumber = None
        self.elemRefNumber = None

        self.nodalGroupname_by_groupnumber=dict()
        self.elemGroupname_by_groupnumber=dict()

        self.mapOldToNewRefNumbers = []

    def TagsToRefs(self, mesh:Mesh):
        mesh.ComputeGlobalOffset()

        self.nodalUserTagNames = [x for x in sorted(mesh.nodesTags.keys()) if x not in TZ.AllZones]
        self.nodalMmg3dTagNames = [x for x in sorted(mesh.nodesTags.keys()) if x in TZ.AllZones]

        self.elementUserTagsNames = [x for x in sorted(mesh.GetNamesOfElementTags()) if x not in TZ.AllZones]
        self.elementMmg3dTagsNames = [x for x in sorted(mesh.GetNamesOfElementTags()) if x in TZ.AllZones]

        self.nodalRefNumber = np.zeros(mesh.GetNumberOfNodes(), dtype=MuscatIndex)

        #Build list of reference numbers
        privateRefNumbers=np.unique([elementsInternalZone[2] for elementsInternalZone in self.elementsInternalZones])
        #In order to avoid the case where internal nodal tags collide with partition refining family number, we shift them
        self.shiftPartRefNumber=max(privateRefNumbers)+1

        #conversion of mmg3D nodes tags to refs numbers
        #Nothing to do for the moment

        # Conversion of user nodes tags to refs number
        cellsByGroupnumber=self.PrepareMeshTagsForPartitionRefining(mesh,"Nodal")
        PRef=PartitionRefining(cellsByGroupnumber,range(mesh.GetNumberOfNodes()))
        PRef.ExecutePartitioning()
        nodes_by_family=PRef.GetCellsByFamily()
        self.nodalgroupsByFamily=PRef.GetGroupsByFamily()

        #Encoding
        """
        Remark: From a rigorous point of view, there is nothing wrong with using some of the privateRefNumbers as refNumber in the nodal encoding below.
        In fact, currently, the only forbidden reference number is TZ.MMG_ISO,for the NEW nodes added on the level set zero
        0 for all the new nodes which are not labeled as required.

        As of now, the private ref numbers for nodal and element tags are merged into the same data structure. However, in the near future, it would make sense to have
        a nodalPrivateRefNumbers and a elementPrivateRefNumbers
        """
        cpt=self._offset+self.shiftPartRefNumber
        for family,nodes in nodes_by_family.items():
            refNumber=family+cpt
            errorMessage = "Incompatibility: collision between partition refining nodal reference numbers and internal reference number "+str(refNumber)
            assert refNumber not in privateRefNumbers,errorMessage
            self.nodalRefNumber[nodes]=refNumber

        self.elemRefNumber = np.zeros(mesh.GetNumberOfElements(), dtype=MuscatIndex)

        # Conversion of elem Mmg3D tags to refs number
        if self.exportInternalMMGZones:
            for elemname,tagname,number in self.elementsInternalZones:
                if tagname in self.elementMmg3dTagsNames:
                    elems = mesh.GetElementsOfType(elemname)
                    ids = elems.tags[tagname].GetIds()
                    if len(ids) > 0:
                        self.elemRefNumber[ids+elems.globaloffset] = number

        #Conversion of elem tags to refs number
        cellsByGroupnumber=self.PrepareMeshTagsForPartitionRefining(mesh,"Element")
        PRef=PartitionRefining(cellsByGroupnumber,range(mesh.GetNumberOfElements()))
        PRef.ExecutePartitioning()
        elems_by_family=PRef.GetCellsByFamily()
        self.groupsByFamily=PRef.GetGroupsByFamily()

        #Encoding
        #In order to avoid the case where elementsInternalZones tags collide with partition refining family number, we shift them
        cpt=self._offset+self.shiftPartRefNumber
        for family,elems in elems_by_family.items():
            refNumber=family+cpt
            errorMessage = "Incompatibility: collision between partition refining element reference numbers and internal reference number "+str(refNumber)
            assert refNumber not in privateRefNumbers,errorMessage
            self.elemRefNumber[elems]=refNumber
        ## treatment allowing to keep 3D tets tags (from mmg3d 5.5.1 version)
        dimensionality=mesh.GetElementsDimensionality()
        elements = mesh.GetElementsOfType(elementsTypeSupportedForDimension[dimensionality])
        globaloffset = mesh.ComputeGlobalOffset()
        tetrahedraRefNumber = self.elemRefNumber[globaloffset[elements.elementType]: globaloffset[elements.elementType] + elements.GetNumberOfElements()]
        regionsInPartition = np.unique(tetrahedraRefNumber)
        numberOfregionsInPartition = len(regionsInPartition)
        if len(self.elemRefNumber):
            cpt = np.max(self.elemRefNumber)
        else:
            cpt = 0
        # if we have only one region we do nothing
        #otherwise we assing to each reference number two integers (resp. for negative and positive values of the level set)
        if numberOfregionsInPartition <= 1 or not self.iso:
           return

        for reference in regionsInPartition:
            self.mapOldToNewRefNumbers.append((reference,cpt+1,cpt+2))
            cpt = cpt+2

    def PrepareMeshTagsForPartitionRefining(self,mesh,cellType):
        # Conversion of elem tags to refs number
        groupnumber = 1
        tags={"Nodal":self.nodalUserTagNames,"Element":self.elementUserTagsNames}
        groups_by_groupnum={"Nodal":self.nodalGroupname_by_groupnumber,"Element":self.elemGroupname_by_groupnumber}
        for tagname in tags[cellType]:
            groups_by_groupnum[cellType][groupnumber]=tagname
            groupnumber+=1

        groupnum_by_groupname= {v: k for k, v in groups_by_groupnum[cellType].items()}
        if cellType=="Nodal":
            cellsByGroupnumber={groupnum_by_groupname[tagName]:np.unique(mesh.nodesTags[tagName].GetIds()) for tagName in tags[cellType]}
        elif cellType=="Element":
            cellsByGroupnumber={groupnum_by_groupname[tagName]:np.unique(mesh.GetElementsInTag(tagName)) for tagName in tags[cellType]}
        else:
            raise ValueError("This type of cell is not available (Only Nodal and Element are allowed)")
        return cellsByGroupnumber


    def RebuildNodalTagsFromRef(self,mesh,newNodalRefNumber):
        cpt = self._offset+self.shiftPartRefNumber
        groupsByFamily=self.nodalgroupsByFamily
        for family,groups in groupsByFamily.items():
            for group in groups:
                groupname=self.nodalGroupname_by_groupnumber[group]
                idx = np.nonzero(newNodalRefNumber==family+cpt)[0]
                idx = np.intersect1d(idx, mesh.GetNodalTag(TZ.RequiredVertices).GetIds() )
                mesh.GetNodalTag(groupname).AddToTag(idx)

    def RebuildElemTagsFromRef(self,mesh,newElemRefNumber):
        mesh.ComputeGlobalOffset()
        cpt = self._offset+self.shiftPartRefNumber

        #retrieve volumic regions after level set split
        refstoRemove = []
        for originalRef,minus,plus in self.mapOldToNewRefNumbers:
            insideIds = np.nonzero( newElemRefNumber == minus )[0]
            mesh.AddElementsToTag(insideIds, TZ.Inside3D)
            newElemRefNumber[insideIds] = originalRef

            outsideIds = np.nonzero( newElemRefNumber == plus )[0]
            mesh.AddElementsToTag(outsideIds, TZ.Outside3D)
            newElemRefNumber[outsideIds] = originalRef

        # mmg3d compute the skin shared by tetrahedra with different reference
        # to each skin mmg3d assign the smallest reference of the adjacent tetrahedra
        # these extra references are eliminated
        tagstoRemove = []
        groupsByFamily=self.groupsByFamily

        for family,groups in groupsByFamily.items():
            for group in groups:
                groupname=self.elemGroupname_by_groupnumber[group]
                idx = np.nonzero(newElemRefNumber==family+cpt)[0]
                if len(idx):
                    mesh.AddElementsToTag(idx, groupname)
                for ref in refstoRemove:
                    if family+cpt == ref:
                       tagstoRemove.append(groupname)

        mesh.GetElementsOfType(ED.Triangle_3).tags.DeleteTags(tagstoRemove)

    def RefsToTags(self,mesh,keepRefsFields = False):
        newNodalRefNumber = mesh.nodeFields['refs']
        newElemRefNumber = mesh.elemFields['refs']

        elemsToTreat=elementsTypeSupportedForDimension
        dimensionality = mesh.GetElementsDimensionality()
        skinElemType = elemsToTreat[dimensionality-1]
        bodyElemType = elemsToTreat[dimensionality]

        if not keepRefsFields :
            del mesh.nodeFields['refs']
            del mesh.elemFields['refs']

        # Conversion of nodal refs to tags
        self.RebuildNodalTagsFromRef(mesh,newNodalRefNumber)
        # Build tag map
        Info("Adding Elements Tags")
        # Conversion of elem refs to tags
        self.RebuildElemTagsFromRef(mesh,newElemRefNumber)

        if self.importInternalMMGZones:
            Info("Adding topology tags")

            idx = np.nonzero(newElemRefNumber == 0)[0]
            mesh.AddElementsToTag(idx, "ExteriorTmp")

            elems = mesh.GetElementsOfType(ED.Bar_2)
            elems.tags.RenameTag('ExteriorTmp', TZ.ExterBars , noError=True)
            if TZ.ExterBars in elems.tags:
                Debug(TZ.ExterBars + " : " + str(0) + " (" + str(len(elems.tags[TZ.ExterBars])) + ")")

            elems = mesh.GetElementsOfType(skinElemType)

            if TZ.ExterSurf in elems.tags:
               elems.tags.RenameTag('ExteriorTmp', TZ.ExterSurf, noError=True)
               Debug(TZ.ExterSurf + " : " + str(0) + " (" + str(len(elems.tags[TZ.ExterSurf])) + ")")

            idx = np.nonzero(newElemRefNumber == TZ.MG_PLUS)[0]

            mesh.AddElementsToTag(idx, TZ.Outside3D)
            mesh.GetElementsOfType(skinElemType).tags.DeleteTags([TZ.Outside3D])
            Debug(TZ.Outside3D + " : " + str(TZ.MG_PLUS) + " (" + str(len(idx)) + ")")


            idx = np.nonzero(newElemRefNumber == TZ.MG_MINUS)[0]
            mesh.AddElementsToTag(idx, TZ.Inside3D)

            mesh.GetElementsOfType(skinElemType).tags.DeleteTags([TZ.Inside3D])
            Debug(TZ.Inside3D + " : " + str(TZ.MG_MINUS) + " (" + str(len(idx)) + ")")


            idx = np.nonzero(newElemRefNumber == TZ.MMG_ISO)[0]
            mesh.AddElementsToTag(idx, "InterfaceTmp")

            elems = mesh.GetElementsOfType(ED.Bar_2)
            elems.tags.RenameTag('InterfaceTmp', TZ.InterBars , noError=True)
            if TZ.InterBars in elems.tags:
                Debug(TZ.InterBars + " : " + str(TZ.MMG_ISO) + " (" + str(len(elems.tags[TZ.InterBars])) + ")")

            elems = mesh.GetElementsOfType(skinElemType)
            elems.tags.RenameTag('InterfaceTmp', TZ.InterSurf, noError=True)
            if TZ.InterSurf in elems.tags:
                Debug(TZ.InterSurf + " : " + str(TZ.MMG_ISO) + " (" + str(len(elems.tags[TZ.InterSurf])) + ")")


            # in the case of multiple volumic regions, mmg3d adds the label MG_ISO to all triangles shared by test of different reference.
            # This means that we have to filter the true isosurface 0 (i.e. triangles shared by Outside3D and Inside3D tags)
            if len(self.mapOldToNewRefNumbers):
               nodemask = np.zeros(mesh.GetNumberOfNodes(), dtype=bool)
               bodyElems = mesh.GetElementsOfType(bodyElemType)
               conn = bodyElems.connectivity
               nodemask[ np.intersect1d( conn[bodyElems.GetTag(TZ.Inside3D).GetIds(),:], conn[bodyElems.GetTag(TZ.Outside3D).GetIds(),:] ) ] = True
               ids = np.nonzero(np.all(nodemask[elems.connectivity] ==True,axis=1))[0]
               elems.GetTag(TZ.InterSurf).SetIds(ids)

            Info("Adding topology tags Done")

from Muscat.Helpers.IO.Which import Which
mmg3dExec = Which("mmg3d_O3")
mmg2dExec = Which("mmg2d_O3")
mmgsExec  = Which("mmgs_O3")

def MmgAvailable():
    if mmg3dExec is None:
       return False
    else:
        return True

class MmgMesher(AppExecutableBase):
    """
    File-exchange-based interface to the Mmg remeshing tool.
    """
    def __init__(self):
        super(MmgMesher, self).__init__("Mmg3d")
        self.SetBinaryFlag(True)
        self.support = None
        self.mmg_exec = mmg3dExec
        self.iso = False
        self.TRB = TagsRefsBridge()
        self.meshfilenameBase = "MmgMesher_meshfile"
        self.solutionfilenameBase = "MmgMesher_solution"
        self.metricfilenameBase = "MmgMesher_metric"
        self.keepGeneratedFiles = False

    def SetSupport(self, _support:Mesh, writeTopoZones = False):
        allTags = _support.GetNamesOfElementTags()

        tagsToKeep = [x for x in allTags if x not in TZ.AllZones]
        tagsToKeep.append(TZ.Corners)
        tagsToKeep.append(TZ.Ridges)
        tagsToKeep.append(TZ.ExterSurf)

        import copy
        #shallow copy mesh
        self.support = copy.copy(_support)
        #shallow copy elements
        self.support.elements = copy.copy(self.support.elements)
        for k in _support.elements.keys():
            #shallow copy of every elements containers
            self.support.elements.AddContainer(copy.copy(self.support.elements[k]))
            #shallow copy of tags
            self.support.elements[k].tags = copy.copy(self.support.elements[k].tags)
            #replicate the storage to eliminate some tags
            oldstorage = self.support.elements[k].tags.storage
            self.support.elements[k].tags.storage = [x for x in oldstorage if x.name in tagsToKeep ]

        #UMIT.ExtractElementsByTags(_support, tagsToKeep=tagsToKeep, dimensionalityFilter=3)
        self.support.ComputeGlobalOffset()

        self.TRB.exportInternalMMGZones = writeTopoZones

        self.is3DTopo = _support.GetNumberOfElements(dim=3) > 0

        self.is3DGeo = False
        if _support.nodes.shape[1] == 3 :
            mmax = np.max(_support.nodes[:,2])
            mmin = np.min(_support.nodes[:,2])
            if  mmax !=  mmin:
               self.is3DGeo = True

    def SetRemeshWithIso(self,val= True):
        self.iso = val
        self.TRB.iso = val
        self.TRB.Reset()

    def DeleteGeneratedFiles(self):
        filesToDelete = []
        filesToDelete.extend(  self.meshfilenameBase+ext for ext in  ['.mesh', '.meshb', '.solb','.sol'])
        filesToDelete.extend(  self.solutionfilenameBase+ext for ext in  ['.solb.meshb', '.solb.sol'])
        filesToDelete.extend(  self.meshfilenameBase+ext for ext in  ['.mmg3d','.mmg2d'])
        self.DeleteFiles(filesToDelete=filesToDelete)

    def WriteSupport(self):
        # First we clean the old files and reset internal iso state
        self.DeleteGeneratedFiles()

        # hack to preserve 0D elements
        # we add the ids to the RequiredVertices and add a nodaltag to
        #identified the 0D elements
        tagsToDelete = []
        if ED.Point_1 in self.support.elements.keys():
            tag = self.support.nodesTags.CreateTag("INTERNAL_0D_ELEMENTS")
            tagsToDelete.append("INTERNAL_0D_ELEMENTS")
            ids = self.support.elements[ED.Point_1].connectivity.flatten()
            tag.SetIds(ids)
            if MT.RequiredVertices not in self.support.nodesTags:
               self.support.nodesTags.CreateTag(MT.RequiredVertices)
            rvtag = self.support.nodesTags[MT.RequiredVertices]
            rvtag.AddToTag(ids)

            # we add extra nodal tags to identify the 0D elements tags
            for tagname in self.support.elements[ED.Point_1].tags.keys():
                ids = self.support.GetElementsOfType(ED.Point_1).tags[tagname].GetIds()
                nodalids = self.support.GetElementsOfType(ED.Point_1).connectivity[ids,:].flatten()
                self.support.nodesTags.CreateTag("INTERNAL_0D_ELEMENTS_"+tagname).SetIds(nodalids)
                tagsToDelete.append("INTERNAL_0D_ELEMENTS_"+tagname)
        # hack to preserve lonely edges (i.e. edges not attached to tetrahedral elements) when required
        bars = self.support.GetElementsOfType(ED.Bar_2)
        if TZ.RequiredEdges in bars.tags:
           usedNodes = np.zeros(self.support.GetNumberOfNodes(),dtype= bool)
           NodesInBars = bars.connectivity[bars.tags[TZ.RequiredEdges].GetIds(),:].ravel()
           NodesInTets = self.support.GetElementsOfType(elementsTypeSupportedForDimension[self.support.GetElementsDimensionality()] ).connectivity.ravel()
           nodalIds = np.setdiff1d(NodesInBars,NodesInTets)
           usedNodes[nodalIds] = True
           if MT.RequiredVertices not in self.support.nodesTags:
              self.support.nodesTags.CreateTag(MT.RequiredVertices)
           self.support.nodesTags[MT.RequiredVertices].AddToTag(nodalIds)
           usedNodes = usedNodes[bars.connectivity]
           bars.tags.CreateTag("INTERNAL_LONELY_EDGES").SetIds( np.nonzero(np.all(usedNodes==True,axis=1) )[0])
           tagsToDelete.append("INTERNAL_LONELY_EDGES")

           for idx in bars.tags["INTERNAL_LONELY_EDGES"].GetIds():
               self.support.nodesTags.CreateTag("INTERNALNODE_ON_LONELYEDGE_"+str(idx)+"_"+str(0)).SetIds(bars.connectivity[idx,0])
               self.support.nodesTags.CreateTag("INTERNALNODE_ON_LONELYEDGE_"+str(idx)+"_"+str(1)).SetIds(bars.connectivity[idx,1])
               tagsToDelete.append("INTERNALNODE_ON_LONELYEDGE_"+str(idx)+"_"+str(0))
               tagsToDelete.append("INTERNALNODE_ON_LONELYEDGE_"+str(idx)+"_"+str(1))

        self.TRB.TagsToRefs(self.support)
        self.writer.Open(self._filePath(self.meshfilenameBase+".mesh"))
        self.writer.Write(self.support, nodalRefNumber=self.TRB.nodalRefNumber, elemRefNumber=self.TRB.elemRefNumber)
        self.writer.Close()

        self.support.nodesTags.DeleteTags(tagsToDelete)
        for _, data in self.support.elements.items():
            data.tags.DeleteTags(tagsToDelete)

    def GetParamExtention(self):
        dimensionality=self.support.GetElementsDimensionality()
        return ".mmg"+str(dimensionality)+"d"

    def WriteInputParametersFile(self,opts):
        if self.iso:
           self.WriteLSReferencesMap()
        if "localParams" in opts:
           self.WriteLocalRemeshingParams(opts)

    def WriteLSReferencesMap(self):
        """
        LSReferences
        nregions

        ref  minus plus
        ...

        nregions (int) :            number of regions
        ref (int)     :             reference number
        minus,plus (int) :          reference numbers after level set split (negative and positive respectively)

        https://www.mmgtools.org/parameter-file
        """

        assert self.iso,"You cant use this function on SetRemeshWithIso(False) mode "
        #write mapping between old tets reference numbers and tets reference numbers after the split of the level set
        mapOldToNewRefNumbers = self.TRB.mapOldToNewRefNumbers
        if len(mapOldToNewRefNumbers):
           writeFile = open(self._filePath(self.meshfilenameBase+self.GetParamExtention(),binary=False),"a")
           writeFile.write("LSReferences \n")
           writeFile.write(str(len(mapOldToNewRefNumbers))+"\n")
           for originalRef,minus,plus in mapOldToNewRefNumbers:
               writeFile.write(str(originalRef)+" "+str(minus)+" "+str(plus)+" "+"\n")
           writeFile.close()

    def WriteLocalRemeshingParams(self,opts):
        """
        Parameters
        nregions
        ref keyword hmin hmax hauds
        ...

        nregions (int) :            number of regions
        ref (int)     :             reference number
        keyword (str) :             Triangles/Tetrahedra
        hmin, hmax, hausd (floats): local remeshing parameters in rgion labeled with ref

        NB When local parameters are assignes in level set mode we need to write the references numbers after split

        https://www.mmgtools.org/parameter-file
        """
        assert "localParams" in opts,"MmgMesher : Cannot write local parameters "
        groupNumbers = [ group for _,groups in self.TRB.groupsByFamily.items() for group in groups]
        groupNames = [ self.TRB.elemGroupname_by_groupnumber[num] for num in groupNumbers ]

        familiesByGroupNames={}
        for fam,groups in self.TRB.groupsByFamily.items():
            for group in groups:
                familiesByGroupNames.setdefault(self.TRB.elemGroupname_by_groupnumber[group],[]).append(fam)

        localParamsbyRefs={}
        localParams = opts["localParams"]
        cpt = self.TRB._offset+self.TRB.shiftPartRefNumber
        for tagname in localParams:
            if tagname in groupNames:
               for ref in familiesByGroupNames[tagname]:
                   localParamsbyRefs[str(ref+cpt)] = localParams[tagname]

        mapOldToNewRefNumbers = self.TRB.mapOldToNewRefNumbers
        if len(mapOldToNewRefNumbers):
           for originalRef,minus,plus in mapOldToNewRefNumbers:
               for key in list(localParamsbyRefs):
                   if key ==str(originalRef):
                      localParamsbyRefs[str(minus)]=localParamsbyRefs[key]
                      localParamsbyRefs[str(plus)]=localParamsbyRefs[key]
                      localParamsbyRefs.pop(str(originalRef))

        keywordByDim = {2:ASCIIName[ED.Triangle_3],3:ASCIIName[ED.Tetrahedron_4]}
        writeFile = open(self._filePath(self.meshfilenameBase+self.GetParamExtention(),binary=False),"a")
        writeFile.write("Parameters \n")
        writeFile.write(str(len(localParamsbyRefs))+"\n")
        for ref,params in localParamsbyRefs.items():
            keyword = keywordByDim[params["dim"]]
            writeFile.write(str(ref)+" "+keyword+" "+str(params["hmin"])+" "+str(params["hmax"])+" "+str(params["hausd"])+"\n")
        writeFile.close()

    def WriteLevelSet(self, levelset):
        assert self.iso,"You must call SetRemeshWithIso before WriteSupport and before WriteLevelSet"
        self.writer.SetFileName( \
        self._filePath(".".join((self.meshfilenameBase,"sol")),binary=True))
        self.writer.OpenSolutionFile(self.support)
        self.writer.WriteSolutionsFields(self.support, [levelset])
        self.writer.Close()

    def WriteMetricField(self):
        assert 'met' in self.support.nodeFields.keys(),"Metric field not found"
        metric = self.support.nodeFields["met"]
        self.writer.SetFileName( \
        self._filePath(".".join((self.metricfilenameBase,"sol")),binary=True))
        self.writer.OpenSolutionFile(self.support)
        self.writer.WriteSolutionsFields(self.support, [metric])
        self.writer.Close()

    def GenerateMesh(self, ops):
        if self.is3DTopo:
            cmdBuilder = CommandLineBuilder(self.mmg_exec)
        elif self.is3DGeo:
            cmdBuilder = CommandLineBuilder(mmgsExec)
        else:
            cmdBuilder = CommandLineBuilder(mmg2dExec)

        # No edge detection wen using Levelset. We keep the edges already in the mesh
        if "iso" in ops:
            cmdBuilder.optionAdd("ls", arg=str(PH.ReadFloat(ops["iso"])))
            cmdBuilder.optionAdd("sol", arg=self.meshfilenameBase)

            if "rmc" in ops:
               cmdBuilder.optionAdd("rmc", arg=str(PH.ReadFloat(ops["rmc"])))

            if "nr" in ops:
                if PH.ReadBool(ops["nr"]):
                    cmdBuilder.optionAdd("nr")
            else:
                cmdBuilder.optionAdd("nr")
        else:
            if "nr" in ops and PH.ReadBool(ops["nr"]):
                cmdBuilder.optionAdd("nr")

        for kw in ("hmin", "hmax", "ar", "hsiz", "hgrad","hausd"):
            if kw in ops:
                cmdBuilder.optionAdd(kw, arg=str(PH.ReadFloat(ops[kw])))

        if "lag" in ops:
            lagVal = PH.ReadInt(ops["lag"])
            assert lagVal in [0, 1, 2],"Option not valid for lag"
            cmdBuilder.optionAdd(kw, arg=str(lagVal))

        for kw in ("A", "optim", "optimLES", "noinsert", "noswap", "nomove", "nosurf","opnbdy","xreg"):
            if kw in ops and PH.ReadBool(ops[kw]):
                cmdBuilder.optionAdd(kw)

        cmdBuilder.argumentAdd(self.meshfilenameBase)
        cmdBuilder.optionAdd("out", arg=self.solutionfilenameBase+".meshb")

        if "met" in ops and PH.ReadBool(ops["met"]):
            if "iso" in ops:
               cmdBuilder.optionAdd("met", arg=self.metricfilenameBase)
            else:
               cmdBuilder.optionAdd("sol", arg=self.metricfilenameBase)

        cmd = cmdBuilder.result()
        self._executeCommand(cmd)

    def GetNewMesh(self,keepRefsFields=False):
        self.reader.SetFileName( \
                self._filePath(self.solutionfilenameBase+".mesh", binary=True))
        self.reader.SetReadRefsAsField(True)
        res = self.reader.Read(out=type(self.support)())

        data =  self.reader.ReadExtraFields(self._filePath(self.solutionfilenameBase+".sol",binary=False))
        res.nodeFields['remeshingMetric'] = data['SolAtVertices0']

        bars = res.GetElementsOfType(ED.Bar_2)
        if TZ.Ridges in bars.tags:
            res.GetElementsOfType(ED.Bar_2).tags.DeleteTags([TZ.Ridges])

        # Conversion of nodes tags to refs number
        nodalTagNames = self.support.nodesTags.keys()
        for tagname in TZ.AllZones:
            if nodalTagNames.count(tagname):
                nodalTagNames.remove(tagname)
        Info("Adding Nodal Tags")
        res.ComputeGlobalOffset()

        self.TRB.RefsToTags(res,keepRefsFields=keepRefsFields)

        res.CopyProperties(self.support)

        if not self.keepGeneratedFiles:
            self.DeleteGeneratedFiles()

        if "INTERNAL_0D_ELEMENTS" in res.nodesTags:
            self.Recover0DElements(res)

        supportBars = self.support.GetElementsOfType(ED.Bar_2)
        if "INTERNAL_LONELY_EDGES" in supportBars.tags:
            self.RecoverLonelyEdges(res,supportBars)
        res.PrepareForOutput()
        
        
        

        return res

    def Recover0DElements(self,mesh):
        ids = mesh.nodesTags["INTERNAL_0D_ELEMENTS"].GetIds()
        #retrieve coordinates
        data = mesh.elements.GetElementsOfType(ED.Point_1)
        data.Allocate(len(ids))
        data.connectivity[:] = ids[:,np.newaxis]
        mesh.PrepareForOutput()
        mesh.nodesTags.DeleteTags(["INTERNAL_0D_ELEMENTS"])
        #retrieve tags
        for tagname in self.support.elements[ED.Point_1].tags.keys():
            if "INTERNAL_0D_ELEMENTS_"+tagname in mesh.nodesTags:
                idsinTag = []
                for idx in mesh.nodesTags["INTERNAL_0D_ELEMENTS_"+tagname].GetIds():
                    idsinTag.extend(np.nonzero (data.connectivity.flatten() == idx) [0])
                data.tags.CreateTag(tagname).SetIds(idsinTag)
                mesh.nodesTags.DeleteTags(["INTERNAL_0D_ELEMENTS_"+tagname])

    def RecoverLonelyEdges(self,mesh,supportBars):
        newBars = mesh.GetElementsOfType(ED.Bar_2)
        nBars = newBars.GetNumberOfElements()
        ids = supportBars.tags["INTERNAL_LONELY_EDGES"].GetIds()
        supportBars.tags.DeleteTags(["INTERNAL_LONELY_EDGES"])
        cpt = 0
        for idx in ids:
            first  = "INTERNALNODE_ON_LONELYEDGE_"+str(idx)+"_"+str(0)
            second = "INTERNALNODE_ON_LONELYEDGE_"+str(idx)+"_"+str(1)
            newBars.AddNewElement( [mesh.nodesTags[first].GetIds(), mesh.nodesTags[second].GetIds()] ,nBars+cpt)
            mesh.nodesTags.DeleteTags([first,second])
            for tagname in supportBars.tags.keys():
                barinTag = np.intersect1d(idx, supportBars.tags[tagname].GetIds() )
                if len(barinTag):
                    newBars.GetTag(tagname).AddToTag(nBars+cpt)
            cpt+=1
        newBars.originalIds = np.arange(nBars+cpt,dtype=MuscatIndex)

from copy import deepcopy
import os
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
from Muscat.IO.XdmfWriter import WriteMeshToXdmf as WriteMeshToXdmf
import Muscat.ImplicitGeometry.ImplicitGeometryObjects as ImplicitGeometry
from Muscat.MeshTools.MeshCreationTools import CreateSquare, CreateCube
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags
from OpenPisco.Unstructured.Levelset import LevelSet


def CheckIntegrity_mmg():
    obj = MmgMesher()

    if not Which(obj.mmg_exec):
        return "Not Ok, " + str(obj.mmg_exec) + " not found!!"

    res = CreateCube(dimensions=[20, 20, 20], spacing=[1., 1., 1.], origin=[-10, -10, -10],ofTetras=True)

    myObj1 = ImplicitGeometry.ImplicitGeometryCylinder()
    myObj1.center1 = np.array([-5,-5,-5])
    myObj1.center2 = np.array([ 5, 5, 5])
    myObj1.radius = 2.5
    myObj1.wcups = True
    myObj1.infinit = True
    dataO1 = myObj1.ApplyVector(res)

    obj.SetBinaryFlag(True)
    obj.SetSupport(res)
    obj.SetRemeshWithIso()
    obj.WriteSupport()
    obj.WriteLevelSet(dataO1 )
    obj.GenerateMesh({"iso":0.0, "hausd":1, "ar":20, "hmin":1, "nr":True})
    nmesh = obj.GetNewMesh(keepRefsFields=True)
    Nfield = nmesh.nodeFields['refs']
    Efield = nmesh.elemFields['refs']

    WriteMeshToXdmf(TemporaryDirectory.GetTempPath()+os.sep+ "sol_FromBinary.xdmf", nmesh, PointFields=[Nfield], PointFieldsNames=['NRefs'], CellFields=[Efield], CellFieldsNames=['ERefs'])

    obj.SetBinaryFlag(False)
    obj.SetSupport(res)
    obj.WriteSupport()
    obj.WriteLevelSet(dataO1 )
    obj.GenerateMesh({"iso":0.0, "hausd":1, "ar":20, "hmin":1, "nr":True})
    nmesh = obj.GetNewMesh(keepRefsFields=True)
    Nfield = nmesh.nodeFields['refs']
    Efield = nmesh.elemFields['refs']

    WriteMeshToXdmf(TemporaryDirectory.GetTempPath()+os.sep+"sol_FromAscii.xdmf", nmesh, PointFields=[Nfield], PointFieldsNames=['NRefs'], CellFields=[Efield], CellFieldsNames=['ERefs'])


    nmesh = ExtractElementsByTags(nmesh, [TZ.Inside3D])
    WriteMeshToXdmf(TemporaryDirectory.GetTempPath()+os.sep+"sol2.xdmf", nmesh)

    return 'OK'

def Define_3DTestMesh():
    import Muscat.MeshContainers.Mesh as UM
    mymesh = UM.Mesh()
    mymesh.nodes = np.array([[0,0,0],[1,0,0],[0,1,0],[0,0,1],[0,0,-1],[-1,0,0],[0,-1,0],],dtype=MuscatFloat)
    mymesh.originalIDNodes = np.arange(7,dtype=MuscatIndex)

    bar = mymesh.GetElementsOfType(ED.Bar_2)
    bar.AddNewElement([0,1],0)
    bar.AddNewElement([1,2],1)
    bar.AddNewElement([2,3],2)
    bar.AddNewElement([3,1],3)
    bar.AddNewElement([3,5],4)
    bar.AddNewElement([5,0],5)
    bar.AddNewElement([0,2],6)

    tris = mymesh.GetElementsOfType(ED.Triangle_3)
    tris.AddNewElement([0,5,2],0)
    tris.AddNewElement([5,2,3],1)
    tris.AddNewElement([2,3,6],2)
    tris.AddNewElement([2,6,1],3)
    tris.AddNewElement([4,1,2],4)
    tris.AddNewElement([4,2,0],5)
    tris.AddNewElement([0,3,6],6)
    tris.AddNewElement([1,6,0],7)
    tris.AddNewElement([4,1,0],8)
    tris.originalIds = np.array(range(9),dtype=MuscatIndex)

    tets = mymesh.GetElementsOfType(ED.Tetrahedron_4)
    tets.AddNewElement([0,1,2,3],0)
    tets.AddNewElement([0,1,4,2],1)
    tets.AddNewElement([0,2,5,3],2)
    tets.AddNewElement([0,3,6,1],3)
    tets.originalIds = np.array(range(9,13),dtype=MuscatIndex)
    return mymesh

def Add_NodalTagsToMesh(mesh):
    mesh.nodesTags.CreateTag("NodalTag1").AddToTag(np.array([0,1,3]))
    mesh.nodesTags.CreateTag("NodalTag2").AddToTag(np.array([1,5,6]))
    mesh.nodesTags.CreateTag("NodalTag3").AddToTag(2)
    mesh.nodesTags.CreateTag("NodalTag4").AddToTag(np.array([3,4,6]))
    mesh.nodesTags.CreateTag("NodalTag5").AddToTag(np.array([4,0,2]))

def Add_ElemTagsToMesh(mesh):
    bars = mesh.GetElementsOfType(ED.Bar_2)
    bars.tags.CreateTag("Bar1").AddToTag(np.array(range(1)))
    bars.tags.CreateTag("SeveralBars").AddToTag(np.array(range(2)))
    bars.tags.CreateTag("AllBar").AddToTag(np.array(range(7)))

    tets = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tets.tags.CreateTag("AllTetra").AddToTag(np.array(range(4)))
    tets.tags.CreateTag("TagTetra1").AddToTag(np.array([0,1,2]))
    tets.tags.CreateTag("TagTetra2").AddToTag(np.array([0,1,3]))
    tets.tags.CreateTag("TagTetra3").AddToTag(np.array([2,1,3]))

    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag("AllTria").AddToTag(np.arange(9))
    tris.tags.CreateTag("TagTria1").AddToTag(np.array([0,2,4]))
    tris.tags.CreateTag("TagTria2").AddToTag(np.array([1,3,5]))
    tris.tags.CreateTag("TagTria3").AddToTag(np.array([2,6,7]))
    tris.tags.CreateTag("TagTria4").AddToTag(np.array([8,0,3]))


def CheckIntegrity_3DTagsToRefToTagsNoRemesh():
    #Test to check transfer of elem tags from a mesh to the same "vanilla" mesh (mesh with no tags)
    mymesh=Define_3DTestMesh()

    #Add required nodal tag
    mymesh.nodesTags.CreateTag(MT.RequiredVertices)
    mymesh.nodesTags[MT.RequiredVertices].SetIds(range(5))
    myVanillaMesh=deepcopy(mymesh)
    Add_NodalTagsToMesh(mymesh)
    Add_ElemTagsToMesh(mymesh)
    testPRef=TagsRefsBridge()
    testPRef.TagsToRefs(mymesh)

    AssertNoRegressionRefToTags(vanillaMesh=myVanillaMesh,originalMesh=mymesh,refBridge=testPRef)
    return "OK"

def AssertNoRegressionRefToTags(vanillaMesh,originalMesh,refBridge):
    #Regression observed for element partition refining in Mmg
    elemsByTagsInit={tagName:sorted(originalMesh.GetElementsInTag(tagName)) for tagName in refBridge.elementUserTagsNames}
    refBridge.RebuildElemTagsFromRef(vanillaMesh,refBridge.elemRefNumber)
    elemsByTagsRebuilt={tagName:sorted(vanillaMesh.GetElementsInTag(tagName)) for tagName in refBridge.elementUserTagsNames}
    np.testing.assert_equal(elemsByTagsInit,elemsByTagsRebuilt)

    refBridge.RebuildNodalTagsFromRef(vanillaMesh,refBridge.nodalRefNumber)
    #Test to check RequiredVertices tag survives correctly
    RequiredOriginalMeshNodes=originalMesh.nodes[originalMesh.nodesTags[MT.RequiredVertices].GetIds()]
    RequiredNewMeshNodes=vanillaMesh.nodes[np.unique(vanillaMesh.nodesTags[MT.RequiredVertices].GetIds())]
    np.testing.assert_array_equal(np.sort(RequiredOriginalMeshNodes.flat),np.sort(RequiredNewMeshNodes.flat))

    #Regression observed for nodal partition refining in Mmg writer
    nodesByTagsInit={tagName:sorted(originalMesh.nodesTags[tagName].GetIds()) for tagName in refBridge.nodalUserTagNames}
    nodesByTagsRebuilt={tagName:sorted(np.unique(vanillaMesh.nodesTags[tagName].GetIds())) for tagName in refBridge.nodalUserTagNames}
    #Compute intersection of required vertices nodal tag and other tags
    nodesByTagsRef={k:list(set(v) & set(nodesByTagsInit[MT.RequiredVertices])) for k,v in nodesByTagsInit.items()}
    np.testing.assert_equal(nodesByTagsRef,nodesByTagsRebuilt)


def Define_2DTestMesh():
    mesh = CreateSquare(dimensions=[10,11],spacing=[4.,4.],ofTriangles=True)
    bars = mesh.GetElementsOfType(ED.Bar_2)
    barTags=["Skin","X0","X1","Y0","Y1"]
    for barTag in barTags:
        bars.tags.DeleteTags([barTag])
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.DeleteTags("2D")
    mesh.nodesTags.DeleteTags(mesh.nodesTags)
    return mesh

def Add_ElemTagsTo2DMesh(mesh):
    bars = mesh.GetElementsOfType(ED.Bar_2)
    nbBars=bars.GetNumberOfElements()
    bars.tags.CreateTag("Bar1").AddToTag(np.array(range(1)))
    bars.tags.CreateTag("SeveralBars").AddToTag(np.array(range(2)))
    bars.tags.CreateTag("AllBar").AddToTag(np.arange(nbBars))

    tris = mesh.GetElementsOfType(ED.Triangle_3)
    nbTris=tris.GetNumberOfElements()
    tris.tags.CreateTag("AllTria").AddToTag(np.arange(nbTris))
    tris.tags.CreateTag("TagTria1").AddToTag(np.array([0,2,4]))
    tris.tags.CreateTag("TagTria2").AddToTag(np.array([1,3,5]))
    tris.tags.CreateTag("TagTria3").AddToTag(np.array([2,6,7]))
    tris.tags.CreateTag("TagTria4").AddToTag(np.array([8,0,3]))

def CheckIntegrity_2DTagsToRefToTagsNoRemesh():
    #Test to check transfer of elem tags from a mesh to the same "vanilla" mesh (mesh with no tags)
    mymesh=Define_2DTestMesh()
    #Add required nodal tag
    mymesh.nodesTags.CreateTag(MT.RequiredVertices)
    mymesh.nodesTags[MT.RequiredVertices].SetIds(range(5))
    myVanillaMesh=deepcopy(mymesh)
    Add_NodalTagsToMesh(mymesh)
    Add_ElemTagsTo2DMesh(mymesh)
    testPRef=TagsRefsBridge()
    testPRef.TagsToRefs(mymesh)

    AssertNoRegressionRefToTags(vanillaMesh=myVanillaMesh,originalMesh=mymesh,refBridge=testPRef)
    return "OK"

def CheckIntegrity_NodalTagsToRefToTagsWithRemesh():

    def Check_NodalTags(originalMesh,newMesh,levelset=None):
        if levelset is not None:
            CleanNodalTagsOnIso(originalMesh,levelset.phi)

        nodalUserTagNames = [x for x in originalMesh.nodesTags.keys() if x not in TZ.AllZones]
        #Check: Regression observed for nodal partition refining in Mmg
        #Mmg add its own required vertices, it should be taken into account when rebuilding nodal tags
        nodesByTagsMymesh={tagName:sorted(np.unique(originalMesh.nodesTags[tagName].GetIds())) for tagName in nodalUserTagNames if tagName != MT.RequiredVertices}
        nodesByTagsMymesh={k:originalMesh.nodes[v] for k,v in nodesByTagsMymesh.items()}
        nodesByTagsNew={tagName:sorted(np.unique(newMesh.nodesTags[tagName].GetIds())) for tagName in nodalUserTagNames if tagName != MT.RequiredVertices}
        nodesByTagsNew={k:newMesh.nodes[v] for k,v in nodesByTagsNew.items()}

        for tagName in nodesByTagsNew.keys():
            #Check no extra node was added to tag rebuilt (i.e. all nodes in rebuilt tag are part of tag in original mesh)
            newNodesInTag=set(tuple(x) for x in nodesByTagsNew[tagName])
            originalNodesInTag=set(tuple(x) for x in nodesByTagsMymesh[tagName])
            nodesInTagIntersect=set([x for x in originalNodesInTag & newNodesInTag])
            noNodesAdded=nodesInTagIntersect==newNodesInTag
            assert noNodesAdded

            #Check the other nodes were lost because not on required in newmesh
            nodesInTagDif=set([x for x in originalNodesInTag if x not in newNodesInTag])
            requiredVertRemesh=newMesh.nodes[newMesh.nodesTags[MT.RequiredVertices].GetIds()]
            requiredVertRemesh=set(tuple(x) for x in requiredVertRemesh)
            lostNodesNotRequired=all(node not in requiredVertRemesh for node in nodesInTagDif)
            assert lostNodesNotRequired

    def Check_ReqVertices(originalMesh,newMesh,levelset=None):
        if levelset is not None:
            CleanNodalTagsOnIso(originalMesh,levelset.phi)

        #We may have extra required vertices in remesh created by mmg but it does not matter
        requiredVertOriginal=originalMesh.nodes[originalMesh.nodesTags[MT.RequiredVertices].GetIds()]
        requiredVertRemesh=newMesh.nodes[newMesh.nodesTags[MT.RequiredVertices].GetIds()]

        #make sure all the required vertices in the original mesh are kept
        requiredVertOriginal=set(tuple(x) for x in requiredVertOriginal)
        requiredVertRemesh=set(tuple(x) for x in requiredVertRemesh)
        intersect2DrequiredVert=set([x for x in requiredVertOriginal & requiredVertRemesh])
        requiVertOrigiSaved=intersect2DrequiredVert==requiredVertOriginal
        assert requiVertOrigiSaved

    #Create mesh with tags
    mymesh=Define_3DTestMesh()
    Add_NodalTagsToMesh(mymesh)
    mymesh.nodesTags.CreateTag(MT.RequiredVertices)
    mymesh.nodesTags[MT.RequiredVertices].SetIds(range(3))

    for name, elements in mymesh.elements.items():
        if elements.GetNumberOfElements() == 0:
            continue
        elements.Tighten()

    #Remeshing with default metric
    mymeshInfo={"hausd":1, "ar":20, "hmin":1, "nr":True}
    nmesh=MmgMesherActionRemesh(mymesh,mymeshInfo)
    Check_ReqVertices(mymesh,nmesh)
    Check_NodalTags(mymesh,nmesh)

    #Remeshing with levelset metric
    res = CreateCube(dimensions=[5,5,5],spacing=[1, 1, 1], origin=[-10, -10, -10],ofTetras=True)

    #Set Nodal tags with several RequiredVertices on iso-zero
    Add_NodalTagsToMesh(res)
    res.nodesTags.CreateTag(MT.RequiredVertices)
    res.nodesTags[MT.RequiredVertices].SetIds(range(100))
    lsTmp = LevelSet(support=res)
    lsTmp.Initialize(lambda XYZs : (XYZs[:, 0]+9.))
    MmgMesherActionLevelset(lsTmp, {"iso":0.0, "hausd":1, "ar":20, "hmin":1, "nr":True})
    nmesh=lsTmp.support

    Check_ReqVertices(res,nmesh,lsTmp)
    Check_NodalTags(res,nmesh,lsTmp)
    return "OK"


def CheckIntegrity_LevelSetRemeshingWithTwoMaterials():

    UM = CreateCube(dimensions=[5,5,5],spacing=[1./4, 1./4, 1./4],origin=[0, 0, 0],ofTetras=True)
    tetra = UM.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag('all',False).SetIds(np.arange(tetra.GetNumberOfElements()))
    X = UM.nodes[tetra.connectivity,0]
    left = np.nonzero( np.all(X<=.5,axis=1) )[0]
    tetra.tags.CreateTag('Left',False).SetIds(left)
    ls = LevelSet(support=UM)
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.01,"hmax":0.7,"nr":True,"keepGeneratedFiles":True}
    ls.conform = True
    ls.phi = ImplicitGeometry.ImplicitGeometrySphere(center=[.5,.5,.5],radius=.3).ApplyVector(UM)
    ls.originalSupport = ls.support
    MmgMesherActionLevelset(ls,opts)
    opts['useOriginalSupport'] = True
    MmgMesherActionLevelset(ls,opts)

    mesh = CreateSquare(dimensions=[5,5],spacing=[1./4, 1./4],origin=[0, 0],ofTriangles=True)
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag('all',False).SetIds(np.arange(tris.GetNumberOfElements()))
    X = mesh.nodes[tris.connectivity,0]
    left = np.nonzero( np.all(X<=.5,axis=1) )[0]
    tris.tags.CreateTag('Left',False).SetIds(left)
    ls = LevelSet(support=mesh)
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.01,"hmax":0.7,"nr":True}
    ls.conform = True
    ls.phi = ls.support.nodes[:,1] - 0.5
    MmgMesherActionLevelset(ls,opts)

    return "ok"

def CheckIntegrity_LevelSetRemeshingWithLocalParams():
    UM = CreateCube(dimensions=[5,5,5],spacing=[1./4, 1./4, 1./4],origin=[0, 0, 0],ofTetras=True)
    tetra = UM.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag('all',False).SetIds(np.arange(tetra.GetNumberOfElements()))
    X = UM.nodes[tetra.connectivity,0]
    left = np.nonzero( np.all(X<=.5,axis=1) )[0]
    tetra.tags.CreateTag('Left',False).SetIds(left)
    ls = LevelSet(support=UM)
    localParams={"Left":{"hmin":0.1,"hmax":2.,"hausd":0.1,"dim":3}}
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.01,"hmax":0.1,"nr":True,"keepGeneratedFiles":True,"localParams":localParams}
    ls.conform = True
    ls.phi = ImplicitGeometry.ImplicitGeometrySphere(center=[.5,.5,.5],radius=.3).ApplyVector(UM)
    MmgMesherActionLevelset(ls,opts)
    return "ok"

def CheckIntegrity_LevelSetRemeshingWithMetric():
    from Muscat.Helpers.CheckTools import SkipTest
    from OpenPisco.ExternalTools.FreeFem.FreeFemInterface import FREEFEM_AVAILABLE
    from OpenPisco.Unstructured.Meshdist import MSHDIST_AVAILABLE

    if SkipTest("FREEFEM_NO_FAIL"): return "skip"

    if not FREEFEM_AVAILABLE:
        return "skip Ok, FreeFem not found!!"
    
    if not MSHDIST_AVAILABLE:
        return "skip Ok, mshdist not found!!"

    mesh = CreateCube(dimensions=[5,5,5],spacing=[2., 1., 1.],origin=[0, 0, 0],ofTetras=True)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    X = mesh.nodes[tetra.connectivity,0]
    left = np.nonzero( np.all(X<=1.,axis=1) )[0]
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(left)

    from OpenPisco.Unstructured.MeshAdaptationTools import ComputeLevelSetMetric
    print(TemporaryDirectory.GetTempPath())

    ls = LevelSet(support=mesh)
    ls.Initialize(lambda XYZs : (XYZs[:, 0]-0.5))
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.01,"hmax":0.7,"nr":True,"keepGeneratedFiles":True,"met":True}
    metricfield = ComputeLevelSetMetric(ls,opts)
    ls.support.nodeFields["met"] = metricfield
    ls.conform = True
    MmgMesherActionLevelset(ls,opts)
    WriteMeshToXdmf(filename=TemporaryDirectory.GetTempPath()+"CheckIntegrity_mmg2d_levelsetInit_adaptation.xdmf",
                    baseMeshObject=ls.support,
                    PointFields=[ls.phi],
                    PointFieldsNames = ["phi"])
    
    mesh = CreateSquare(origin=[0.,0.],dimensions=[10,10],spacing=[1./9,1./9],ofTriangles=True)
    levelset = LevelSet(support=mesh)
    implicitSphere =  ImplicitGeometry.ImplicitGeometrySphere(center=[.5,.5,0.],radius=.25)
    implicitSphere.insideOut = True
    levelset.phi = implicitSphere.ApplyVector(mesh)

    opts = {"hmin":0.01,"hmax":0.7}
    metricfield = ComputeLevelSetMetric(levelset,opts)
    levelset.support.nodeFields["met"] = metricfield
    levelset.conform = True
    remeshingInfo={"hgrad":-1,"met":True,"iso":0.0,'computeDistanceWith':'meshdist'}
    MmgMesherActionLevelset(levelset, remeshingInfo)
    WriteMeshToXdmf(filename=TemporaryDirectory.GetTempPath()+"CheckIntegrity_mmg2d_levelset_adaptation.xdmf",
                    baseMeshObject=levelset.support,
                    PointFields=[levelset.phi],
                    PointFieldsNames = ["phi"])

    return "ok"

def CheckIntegrity_Adaptation():
    mesh = CreateSquare(dimensions=[10,5],spacing=[4.,4.],ofTriangles=True)
    remeshingInfo={"hgrad":-1,"met":True}
    metricIso = (mesh.nodes[:,0]+1.1)/20
    mesh.nodeFields["met"] = metricIso
    meshIso=MmgMesherActionRemesh(mesh,remeshingInfo)

    WriteMeshToXdmf(TemporaryDirectory.GetTempPath()+"CheckIntegrity_mmg2d_isotropic_adaptation.xdmf", meshIso)

    mesh = CreateCube(dimensions=[10,5,5],spacing=[9., 4., 4.],origin=[0, 0, 0],ofTetras=True)
    metricIso = (mesh.nodes[:,0]+1.1)
    mesh.nodeFields["met"] = metricIso
    meshIso=MmgMesherActionRemesh(mesh,remeshingInfo)

    WriteMeshToXdmf(TemporaryDirectory.GetTempPath()+"CheckIntegrity_mmg3d_isotropic_adaptation.xdmf", meshIso)
    return "ok"

def CheckIntegrity_SymbolicAdaptation():
    length = np.array([1,1])
    refinementFactor = 3
    dimensions = refinementFactor*np.array([10, 10])
    spacing = length/(dimensions-1)
    mesh = CreateSquare(dimensions=dimensions,spacing=spacing,origin=[0.,0.],ofTriangles=True)

    levelset = LevelSet(support=mesh)
    implicitSphere =  ImplicitGeometry.ImplicitGeometrySphere(center=[.5,.5,0.],radius=.25)
    implicitSphere.insideOut = True
    levelset.phi = implicitSphere.ApplyVector(mesh)

    from OpenPisco.Actions.MeshActions import SetAsRequiredAction
    action = SetAsRequiredAction()
    action.Apply(None,{"ls":levelset,"eTags":"Skin"})
    print(levelset.support)
    opts = {"symbolicExpression":"(phi**2+0.00001)**0.5"}
    from OpenPisco.Unstructured.MeshAdaptationTools import ComputeLevelSetMetric
    metricfield = ComputeLevelSetMetric(levelset,opts,"Symbolic")
    levelset.support.nodeFields["met"] = metricfield
    levelset.conform = True
    remeshingInfo={"hgrad":-1,"met":True,"iso":0.0,'computeDistanceWith':'meshdist'}
    MmgMesherActionLevelset(levelset, remeshingInfo)
    WriteMeshToXdmf(filename=TemporaryDirectory.GetTempPath()+"CheckIntegrity_mmg2d_levelset_Symbolicadaptation.xdmf",
                    baseMeshObject=levelset.support,
                    PointFields=[levelset.phi],
                    PointFieldsNames = ["phi"])
    return "ok"

def CheckIntegrity_SetPhiZeroOnBoundary():

    mesh = CreateSquare(dimensions=[5,5],spacing=[1./4, 1./4],origin=[0, 0],ofTriangles=True)
    ls = LevelSet(support=mesh)
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.01,"hmax":0.7,"nr":True,"keepGeneratedFiles":True,"setPhiZeroOnBoundary":True}
    ls.conform = True
    ls.phi = ls.support.nodes[:,1] - 0.5
    MmgMesherActionLevelset(ls,opts)
    return "ok"

def CheckIntegrity_ReproductibilityMMg():
    from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometrySphere
    length = np.array([8,4,4])
    refinementFactor = 1
    dimensions = refinementFactor*np.array([20, 10, 10])
    spacing = length/(dimensions-1)
    
    nbNodes = []
    nodalRefNumbers = []
    elemRefNumbers = []
    mesh = CreateCube(dimensions=dimensions,spacing=spacing,origin=[0.,0.,0.],ofTetras=True)
    opts = {"iso":0.0,"hausd":0.01,"hmin":0.5,"hmax":1.2,"nr":True,"keepGeneratedFiles":True}
    implicitSphere = ImplicitGeometrySphere(center=[2.5, 2., 2.], radius=1)
    implicitSphere.insideOut = True
    for _ in range(2):
        ls = LevelSet(support=mesh)
        ls.conform = True
        ls.phi = implicitSphere.ApplyVector(mesh)
        ls.originalSupport = ls.support
        nodalRefNumber, elemRefNumber = MmgMesherActionLevelset(ls,opts)
        nodalRefNumbers.append(nodalRefNumber)
        elemRefNumbers.append(elemRefNumber)
        nbNodes.append(ls.support.GetNumberOfNodes())
    assert np.linalg.norm(nodalRefNumbers[0]-nodalRefNumbers[1])<1e-6
    assert np.linalg.norm(elemRefNumbers[0]-elemRefNumbers[1])<1e-6
    assert len(set(nbNodes))==1, "remeshing reproductibility issue"
    return "ok"

def CheckIntegrity(GUI=False):
    totest = [
    CheckIntegrity_SetPhiZeroOnBoundary,
    CheckIntegrity_mmg,
    CheckIntegrity_LevelSetRemeshingWithTwoMaterials,
    CheckIntegrity_3DTagsToRefToTagsNoRemesh,
    CheckIntegrity_2DTagsToRefToTagsNoRemesh,
    CheckIntegrity_NodalTagsToRefToTagsWithRemesh,
    CheckIntegrity_LevelSetRemeshingWithLocalParams,
    CheckIntegrity_Adaptation,
    CheckIntegrity_SymbolicAdaptation,
    CheckIntegrity_LevelSetRemeshingWithMetric,
    ]

    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res

    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
