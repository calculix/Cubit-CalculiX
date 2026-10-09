# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
"""For more details about OpenFOAM format, we refer to section 4.1.2 the polymesh description in
https://www.openfoam.com/documentation/user-guide/4-mesh-generation-and-conversion/4.1-mesh-description

see also
https://openfoamwiki.net/index.php/Write_OpenFOAM_meshes

see also
https://www.cfd-online.com/Forums/openfoam-meshing/61656-creating-your-own-mesh-files.html
"""
import os
import itertools
import time
from typing import Iterable,Optional
import subprocess

#from bs4 import ElementFilter
import numpy as np

import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Mesh import Mesh, ElementsContainer, AllElements
from Muscat.IO.WriterBase import WriterBase
from Muscat.Helpers.Logger import Info
from Muscat.MeshTools.MeshModificationTools import MeshExtrusion, ComputeSkin
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter

from OpenPisco.Actions.UnstructuredActions import FilterSimplyConnectedBodiesInTagAction
from OpenPisco.ExternalTools.AppExecutableTools import AppExecutableBase,CommandLineBuilder
from OpenPisco import RETURN_SUCCESS

class GzipConverter(AppExecutableBase):
    def __init__(self):
        super(GzipConverter, self).__init__("gzip")

    def _executeSubprocess(self, cmd):
        return subprocess.check_output(cmd, shell=True,stderr=subprocess.STDOUT)

    def ExecuteConversion(self,filename):
        cmdBuilder = CommandLineBuilder(self.GetAppName())
        cmdBuilder.argumentAdd(filename)
        cmd = " ".join(cmdBuilder.result())
        self._runCommand(cmd)
        self.DeleteFiles(filename)
        return RETURN_SUCCESS

def CompressAndRemoveFile(filePath):
    compressor = GzipConverter()
    compressor.ExecuteConversion(filePath)

def CreateHeader(filePath:str,keyWordClass:str,keyWordObject:str,note:Optional[str]=None)->str:
    """Create header following OpenFOAM style

    Parameters
    ----------
    filePath : str
        file path
    keyWordClass : str
        type of data in file
    keyWordObject : str
        type of main physical/geometrical quantity in file
    note: Optional[str], optional
        extra note, by default None
    Returns
    -------
    str
        OpenFOAM header formated as a string
    """
    header = r"""/*--------------------------------*- C++ -*----------------------------------*\
| =========                 |                                                 |
| \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox           |
|  \\    /   O peration     | Version:  v2412                                 |
|   \\  /    A nd           | Website:  www.openfoam.com                      |
|    \\/     M anipulation  |                                                 |
\*---------------------------------------------------------------------------*/
FoamFile
{
    format      ascii;"""
    header += "\n    class    "+keyWordClass+";"
    if note is not None:
        header +="\n\t"+note
    header += "\n    location    \""+filePath+"\";"
    header += "\n    object    "+keyWordObject+";"
    header +="""
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //"""
    return header

ENDLINE = r"""// ************************************************************************* //"""


class FoamWriter(WriterBase):
    def __init__(self,location:Optional[str]=None,compressMeshFiles:bool=False):
        """Constructor (can only handle ASCII output for now, binary to implement)

        Parameters
        ----------
        location : str
            main directory output path
        """
        super(FoamWriter,self).__init__()
        self._isBinary = False
        self.compressMeshFiles = compressMeshFiles
        self.location = location

    def Write(self,
              meshObject:Mesh,
              physicalTagNames:Iterable,
              tagTypeByTagName:dict,
              PointFields:Optional[Iterable]=None,
              PointFieldsNames:Optional[str]=None,
              CellFields:Optional[Iterable]=None,
              CellFieldsNames:Optional[str]=None
              ):
        """Write mesh in FOAM format

        Parameters
        ----------
        meshObject : Mesh
            Original mesh to be written
        physicalTagNames : Iterable
            collection of physical tag names to be used in a FOAM analysis
        tagTypeByTagName : Dict
            A dictionary of tags and their associated types
        PointFields : Optional[Iterable], optional
            Values of nodal fields to be written if required, by default None
        PointFieldsNames : Optional[str], optional
            Names of nodal fields to be written if required, by default None
        CellFields : Optional[Iterable], optional
            Values of cell fields to be written if required, by default None
        CellFieldsNames : Optional[str], optional
            Names of cell fields to be written if required, by default None
        """
        if PointFields is not None or PointFieldsNames is not None:
            raise NotImplementedError("Can not write nodal fields for now")
        start = time.time()
        os.makedirs(self.location, exist_ok=True)
        meshToWrite, tagTypeByTagName = self.Convert2DMesh(meshObject, tagTypeByTagName)
        Info("Write Points")
        self.WritePoints(filename="points",mesh=meshToWrite)
        Info("Write Faces")
        self.WriteFaceCaracteristics(mesh=meshToWrite,physicalTagNames=physicalTagNames,tagTypeByTagName=tagTypeByTagName)
        Info("Write Fields")
        if CellFields is not None:
            assert len(CellFields)==len(CellFieldsNames),"Number of cell fields names should match number of cell fields values provided"
            numTimeStepsForFields = set(len(fieldValue) for fieldValue in CellFields)
            assert len(numTimeStepsForFields)==1, "All fields do not have the same number of time step! Not allowed"
            numTimeSteps = list(numTimeStepsForFields)[0]
            for stepNum in range(numTimeSteps):
                stepPath = os.path.join(self.location,str(stepNum))
                os.mkdir(stepPath)
            for CellFieldsName,CellField in zip(CellFieldsNames,CellFields):
                self.WriteSingleNodalField(CellFieldsName,CellField)
        Info("Time to write mesh: "+str(time.time() - start))

    @staticmethod
    def Convert2DMesh(meshObject:Mesh, tagTypeByTagName:dict) -> tuple[Mesh, dict]:
        if meshObject.GetElementsDimensionality() == 3:
            return meshObject, tagTypeByTagName
        elif meshObject.GetElementsDimensionality() == 2:
            extrudedMesh = MeshExtrusion(meshObject, depth = 1.0, propagateTagsToExtrudedElements = True)
            tagTypeByTagName["Original"] = "empty"
            tagTypeByTagName["Projected"] = "empty"
            return extrudedMesh[0], tagTypeByTagName
        else:
            raise ValueError(f"Mesh is of dimensions {meshObject.GetElementsDimensionality()}, but only 2 and 3 dimensional mesh are handled")

    @staticmethod
    def Count2DSkinElements(meshObject:Mesh):
        counter = 0
        ef = ElementFilter(dimensionality = meshObject.GetElementsDimensionality() - 1, eTag = "Skin")
        for selection in ef(meshObject):
            counter += selection.Size()
        return counter
        
    def WriteSingleNodalField(self,fieldName:str,fieldValue:Iterable)->str:
        """Write single nodal field in file

        Parameters
        ----------
        fieldName : str
            field name to be written
        fieldValue : Iterable
            field value
        """
        location = self.location
        for singleFieldValueStepId,singleFieldValue in enumerate(fieldValue):
            fieldLocation = os.path.join(location,str(singleFieldValueStepId),fieldName)
            with open(fieldLocation, "w+") as fieldFile:
                header = CreateHeader(filePath=str(singleFieldValueStepId),
                                    keyWordClass="volVectorField",
                                    keyWordObject=fieldName)
                fieldFile.write(header)
                fieldFile.write("\ndimensions      [0 1 -1 0 0 0 0];\n")
                fieldFile.write("internalField   nonuniform List<vector>\n")
                fieldFile.write(str(len(singleFieldValue))+"\n")
                fieldFile.write("(\n")
                for nodalValue in singleFieldValue:
                    fieldFile.write("("+" ".join(map(str,nodalValue))+")\n")
                fieldFile.write(")\n;\n")

    def WritePoints(self,filename:str,mesh:Mesh):
        """Write nodes coordinates in FOAM format

        Parameters
        ----------
        filename : str
            Output file name
        meshObject : Mesh
            Original mesh to be written
        """
        filePath = os.path.join(self.location,filename)
        header = CreateHeader(filePath="polyMesh",keyWordClass="vectorField",keyWordObject="points")
        with open(filePath, "w+") as pointFile:
            pointFile.write(header)
            pointFile.write("\n\n"+str(mesh.GetNumberOfNodes()))
            nodes = mesh.nodes
            pointFile.write("\n(\n")
            for node in nodes:
                pointFile.write("("+str(node[0])+" "+str(node[1])+" "+str(node[2])+")\n")
            pointFile.write(")\n")
            pointFile.write("\n\n"+ENDLINE)
        if self.compressMeshFiles:
            CompressAndRemoveFile(filePath)

    def WriteFaceCaracteristics(self,mesh:Mesh,physicalTagNames:Iterable,tagTypeByTagName:dict):
        """Write face caracteristics, including:
            - boundary file (related to physical tag names in physical problem)
            - faces file
            - owner file
            - neighbour file

        Parameters
        ----------
        mesh : Mesh
            Mesh to be written
        physicalTagNames : Iterable
            collection of physical tag names used in physical problem setting
        tagTypeByTagName : Dict
            A dictionary of tags and their associated types used to create boundary file
        """
        Info("Compute Face Characteristics")
        internalFacesCarac, externalFacesCarac = self.ComputeFaceCaracteristics(mesh)
        nbInternalFace = len(internalFacesCarac[0])
        Info("Renumber Internal Faces")
        internalFacesCarac = self.RenumberInternalFaces(internalFacesCarac)
        Info("Renumber External Faces")
        externalFacesCarac, faceIdsByTagName = self.RenumberExternalFacesForBoundary(externalFacesCarac,mesh,physicalTagNames)
        self.WriteFaceBoundary(filename="boundary",nbInternalFace=nbInternalFace,faceIdsByTagName=faceIdsByTagName,tagTypeByTagName=tagTypeByTagName)

        internalTriaFaces, externalTriaFaces = internalFacesCarac[0], externalFacesCarac[0]
        triaFaces = internalTriaFaces + externalTriaFaces
        #triaFaces = np.concatenate([internalTriaFaces, externalTriaFaces])
        internalFaceTetra, externalFaceTetra = internalFacesCarac[1], externalFacesCarac[1]
        facesTetra = internalFaceTetra + externalFaceTetra
        #facesTetra = np.concatenate([internalFaceTetra, externalFaceTetra])
        #easier to create faces with the correct connectivity from the start
        #no need to reorder the connectivity, and cumbersome for polygons larger than triangles
        #triaFaces = self.RenumerateFaceConnectivity(triaFaces,facesTetra,mesh)

        self.WriteFaceConnectivity(filename="faces",triaFaces=triaFaces)

        nbPoints = mesh.GetNumberOfNodes()
        nbCells = mesh.GetElementsOfType(ED.Tetrahedron_4).GetNumberOfElements()
        nInternalFaces = len(internalTriaFaces)
        nExternalFaces = len(externalTriaFaces)
        nAllFaces = nInternalFaces+nExternalFaces
        note = "note        \"nPoints:"+str(nbPoints)+"  nCells:"+str(nbCells)+"  nFaces:"+str(nAllFaces)+"  nInternalFaces:"+str(nInternalFaces)+"\";"
        self.WriteFaceNeighbour(filename="neighbour",internalFacesNeighbours=internalFacesCarac[2],note=note)

        internalFaceOwner,externalFaceOwner = internalFacesCarac[1],externalFacesCarac[1]
        facesOwner= np.concatenate([internalFaceOwner,externalFaceOwner])
        self.WriteFaceOwner(filename="owner",facesOwner=facesOwner,note=note)

    def ComputeFaceCaracteristics(self,mesh:Mesh)->tuple:
        """Compute internal/external triangular face caracteristics:
            - internal (2 tetra faces in common): 
                triangular faces connectivity
                faces tetra (owners) indices (lowest tetra indices linked to internal faces)
                face tetra (neighbours) indices (other tetra indices linked to internal faces)
            - external: 
                triangular faces connectivity
                faces tetra indices (tetra indices linked to external faces)

        Parameters
        ----------
        mesh : Mesh
            mesh to be written
        Returns
        -------
        tuple
            internal/external face caracteristics
        """
        Info("Compute Face Numbering")
        faceNumbers, tetraCorresponding, triaFacesUnsorted, triaFacesUnsorted = self.ComputeFaceNumbering(mesh) 
        #internalFaces = np.array(list(filter(lambda x: x.size > 1, faceNumbers)))
        #internalFacesTetra = tetraCorresponding[internalFaces] #pair of elements sharing internal faces
        #internalTriaFaces = np.array(triaFaces)[internalFaces]
        #np.testing.assert_almost_equal(internalTriaFaces[:,0,:],internalTriaFaces[:,1,:])
        #assert internalFacesTetra.shape[-1]==2,"Only for acceptable FE mesh (faces can only belong to 2 volumic elements at most)"
        #internalFacesOwner = np.min(internalFacesTetra,axis=1)
        #internalFacesNeighbours = np.max(internalFacesTetra,axis=1)
#
        #renumbering = np.argsort(internalFacesOwner)
        #internalFacesOwner = internalFacesOwner[renumbering]
        #internalFacesNeighbours = internalFacesNeighbours[renumbering]
        #internalTriaFaces = internalTriaFaces[renumbering][:,0,:]
#
        #externalFaces = np.array(list(filter(lambda x: x.size == 1, faceNumbers)))
        #externalTriaFaces = np.array(triaFaces)[externalFaces][:,0,:]
        #externalFacesTetra = tetraCorresponding[externalFaces][:,0]
        #renumbering = np.argsort(externalFacesTetra)
        #externalTriaFaces = externalTriaFaces[renumbering]
        #externalFacesTetra = externalFacesTetra[renumbering]
        #return (internalTriaFaces,internalFacesOwner,internalFacesNeighbours),(externalTriaFaces,externalFacesTetra)

        # Internal faces: those shared by two elements
        Info("Internal Faces")
        internalFaces = [face for face in faceNumbers if len(face) > 1] # List of face index of internal faces [face index 1, face index 2]
        internalFacesTetra = [tetraCorresponding[i] for i in internalFaces] # List of [owner element, neighbour element]
        internalTriaFaces = [[triaFacesUnsorted[i[0]],triaFacesUnsorted[i[1]]] for i in internalFaces] # List of connectivities of internal faces [connectivity 1, connectivity 2]

        # Assert the faces match for internal faces
        for f in internalTriaFaces:
            assert set(f[0]) == set(f[1]), "Internal faces connectivity doesn't match"

        # Owners & neighbours as lowest & highest indices
        internalFacesOwner = [min(tetraIndex) for tetraIndex in internalFacesTetra]
        ownerIndex = [min(range(len(tetraIndex)), key=lambda i: tetraIndex[i]) for tetraIndex in internalFacesTetra]
        internalFacesNeighbours = [max(tetraIndex) for tetraIndex in internalFacesTetra]

        # Renumbering by owner index
        renumbering = sorted(range(len(internalFacesOwner)), key=lambda k: internalFacesOwner[k])
        internalFacesOwner = [internalFacesOwner[k] for k in renumbering]
        internalFacesNeighbours = [internalFacesNeighbours[k] for k in renumbering]
        ownerIndex = [ownerIndex[k] for k in renumbering]
        internalTriaFaces = [internalTriaFaces[k][index] for k,index in zip(renumbering,ownerIndex)] # Take only the owner side

        # External faces: those shared by 1 element only
        externalFaces = [face for face in faceNumbers if len(face) == 1] # List of face index of external faces
        externalFacesTetra = [tetraCorresponding[i][0] for i in externalFaces]# List of owner element 
        externalTriaFaces = [triaFacesUnsorted[i[0]] for i in externalFaces] # List of connectivities of external faces

        # Renumbering external faces by tetra index
        renumbering = sorted(range(len(externalFacesTetra)), key=lambda k: externalFacesTetra[k])
        externalTriaFaces = [externalTriaFaces[k] for k in renumbering]
        externalFacesTetra = [externalFacesTetra[k] for k in renumbering]

        return (internalTriaFaces, internalFacesOwner, internalFacesNeighbours), (externalTriaFaces, externalFacesTetra)

    def ComputeFaceNumbering(self,mesh:Mesh)->tuple:
        """Compute face numbering from tetrahedron:
            - Extract all faces associated to each individual tetrahedron and map it to associated tetrahedron
            - Collect unique triangle faces, defined unique integer for each, map all triangles faces to such integer
            - Build unique face numbering mapping to corresponding triangle faces numbering
            For instance, in faceNumbers = [array([8]), array([13]), array([10, 14])...] 
            it means unique triangular face 2 can be found in triangular face 10 and 14

            Note that, for tetrahedron, a triangular face can belong to 1 (external face) or 2 tetrahedrons (internal face)

        Parameters
        ----------
        mesh : Mesh
            mesh to be written
        Returns
        -------
        tuple
            unique face number mapping to all triangular faces,tetrahedron indices associated to each face,triangle face connectivity
        """
        #bodyElements = mesh.GetElementsOfType(ED.Tetrahedron_4)
        ef = ElementFilter(dimensionality = mesh.GetElementsDimensionality())
        res = Mesh()

        for selection in ef(mesh):
            faces = ED.faces1[selection.elementType]
            for faceType, localFaceConnectivity in faces:
                elements = res.elements.GetElementsOfType(faceType)
                globalFaceConnectivity = selection.elements.connectivity[selection.indices, :][:, localFaceConnectivity]
                globalIndex = selection.indices
                elements.AddNewElements(globalFaceConnectivity,globalIndex)
            tetraCorresponding2 = np.tile(np.arange(selection.elements.GetNumberOfElements()),len(faces))
            
            #bodyElements = selection.elements
            #faceConnectivities = [faceConnectivity for faceType, faceConnectivity in ED.faces1[selection.elementType]] #list of the connectivity of the faces of a "template" element
            #triaFaces = [sorted(element[localConnectivity].tolist()) for element in bodyElements.connectivity for localConnectivity in faceConnectivities] #list of all faces, sorted connectivities
            #triaFacesUnsorted = [(element[localConnectivity].tolist()) for element in bodyElements.connectivity for localConnectivity in faceConnectivities] #list of all faces, unsorted connectivities
            #tetraCorresponding = np.repeat(np.arange(bodyElements.GetNumberOfElements()),len(faceConnectivities)) #id of tetra corresponding to triaFaces
            ##uniqueTriaFaces = np.unique(triaFaces,axis=0).tolist()
            #uniqueTriaFaces = []
            #for face in triaFaces: #extremement lent !!
            #    if face not in uniqueTriaFaces:
            #        uniqueTriaFaces.append(face)
            #triaToInt = dict(zip(list(map(tuple,uniqueTriaFaces)),range(len(uniqueTriaFaces))))
            #triaFacesAsInt = np.fromiter(map(triaToInt.get, map(tuple,triaFaces)),dtype=int)
#
            #idxSort = np.argsort(triaFacesAsInt)
            #sortedTriaFacesAsInt = triaFacesAsInt[idxSort]
            #_, idxStart = np.unique(sortedTriaFacesAsInt, return_index=True)
            #faceNumbers = np.split(idxSort, idxStart[1:])

        allEf = ElementFilter()
        triaFaces2 = []
        triaFacesUnsorted2 = []
        uniqueTriaFaces2 = []
        faceNumbers2 = []
        for selection in allEf(res):
            sortedConnectivities = np.sort(selection.elements.connectivity)
            triaFaces2.extend(sortedConnectivities)
            triaFacesUnsorted2.extend((selection.elements.connectivity))
            uniqueTriaFaces2.extend(np.unique(sortedConnectivities,axis=0)) 
        triaToInt2 = dict(zip(list(map(tuple,uniqueTriaFaces2)),range(len(uniqueTriaFaces2))))
        triaFacesAsInt2 = np.fromiter(map(triaToInt2.get, map(tuple,triaFaces2)),dtype=int)

        idxSort2 = np.argsort(triaFacesAsInt2)
        sortedTriaFacesAsInt2 = triaFacesAsInt2[idxSort2]
        _, idxStart2 = np.unique(sortedTriaFacesAsInt2, return_index=True)
        faceNumbers2 = np.split(idxSort2, idxStart2[1:])
        return faceNumbers2, tetraCorresponding2, triaFaces2, triaFacesUnsorted2

    def RenumberInternalFaces(self,internalFacesCarac:tuple)->tuple:
        """Renumber internal faces: OpenFOAM is slightly picky about face ordering.
         Internal faces get ordered such that when stepping through the higher numbered neighbouring cells in 
         incremental order one also steps through the corresponding faces in incremental order (upper-triangular ordering) 

         The idea is to build a face numbering mapping to modify the faces caracteristics order for each tetra owner 
         according to the neighbour faces. To do so, the faces are splitted based on their respective tetra "owner" and ordered 
         based on their associated "neighbours" in the increasing order.

        Parameters
        ----------
        internalFacesCarac : tuple
            unique face number mapping to all triangular faces,tetrahedron indices associated to each face,triangle face connectivity
        Returns
        -------
        tuple
            renumerated internal face caracteristics
        """
        internalTriaFaces, internalFacesOwner, internalFacesNeighbours = internalFacesCarac
        _, idxStart = np.unique(internalFacesOwner, return_index=True)
        faceIndicesPerTetraOwner = np.split(np.arange(len(internalFacesOwner)), idxStart[1:])

        internalFaceMapping = np.arange(len(internalFacesOwner))
        startingIndex = 0
        for faceIndicesInTetra in faceIndicesPerTetraOwner:
            #internalFacesNeighboursInTetra = internalFacesNeighbours[faceIndicesInTetra]
            internalFacesNeighboursInTetra = [internalFacesNeighbours[i] for i in faceIndicesInTetra]
            indicesSort = np.argsort(internalFacesNeighboursInTetra)
            internalFaceMapping[faceIndicesInTetra] = startingIndex+indicesSort
            startingIndex += len(faceIndicesInTetra)

        #renumerotedInternalFaces = internalTriaFaces[internalFaceMapping]
        renumerotedInternalFaces = [internalTriaFaces[i] for i in internalFaceMapping]
        #renumerotedInternalFacesOwner = internalFacesOwner[internalFaceMapping]
        renumerotedInternalFacesOwner = [internalFacesOwner[i] for i in internalFaceMapping]
        #renumerotedinternalFacesNeighbours = internalFacesNeighbours[internalFaceMapping]
        renumerotedinternalFacesNeighbours = [internalFacesNeighbours[i] for i in internalFaceMapping]

        renumerotedInternalFacesCarac = renumerotedInternalFaces,renumerotedInternalFacesOwner,renumerotedinternalFacesNeighbours
        return renumerotedInternalFacesCarac

    def RenumberExternalFacesForBoundary(self,externalFacesCarac:tuple,mesh:Mesh,physicalTagNames:Iterable):
        """Renumber external faces: OpenFOAM is slightly picky about face ordering.
         Boundary faces are supposed to be bunched per patch (contiguous numerotation for elem ids in tag). 
         Case where intersection between tags is not empty not handled. The physical tag names should be a partition of the external surface

        Parameters
        ----------
        externalFacesCarac : tuple
            triangular faces connectivity,faces tetra indices (tetra indices linked to external faces)
        mesh : Mesh
            mesh to be written
        physicalTagNames : Iterable
            physical tag names
        Returns
        -------
        tuple
            renumerated external face caracteristics
        """
        #triaElements = mesh.GetElementsOfType(ED.Triangle_3)
        ef = ElementFilter(dimensionality = mesh.GetElementsDimensionality()-1)
        renumerotedExternalFaces = []
        renumerotedExternalFacesTetra = []
        faceIdsByTagName = dict()
        tagNames = [tagName for tagName in physicalTagNames]
        if "Projected" in mesh.GetNamesOfElementTags():
            tagNames.extend(["Original", "Projected"])
        currentGlobalElementId = 0
        #ComputeSkin(mesh, inPlace = True)
        #mappingContiguousBoundaryTags = np.zeros(self.Count2DSkinElements(mesh)).astype(int)-1

        for selection in ef(mesh):
            triaElements = selection.elements
            sortedTria = np.array([sorted(triaElement) for triaElement in triaElements.connectivity])
            externalFacesCaracSelection_0 = [i for i in externalFacesCarac[0] if len(i) == ED.numberOfNodes[selection.elementType]]
            sortedExternalFacesCaracSelection_0 = [sorted(i) for i in externalFacesCaracSelection_0]
            externalFacesCaracSelection_1 = [v for i,v in enumerate(externalFacesCarac[1]) if len(externalFacesCarac[0][i]) == ED.numberOfNodes[selection.elementType]]
            externalFacesCaracSelection = (externalFacesCaracSelection_0, externalFacesCaracSelection_1)
            assert sortedTria.shape[0] == len(externalFacesCaracSelection[0]), "Number of extracted external faces and number of surfacic elements should be the same"
            if np.linalg.norm(sortedTria-externalFacesCaracSelection[0])>1e-8:
                #Face/nodes ordering not the same for original mesh and extracted external surface
                from scipy.spatial import cKDTree
                myTree = cKDTree(sortedExternalFacesCaracSelection_0)
                _, mapping=myTree.query(sortedTria)
                #externalFacesCarac= (externalFacesCarac[0][mapping],externalFacesCarac[1][mapping])
                externalFacesCaracSelection = ([externalFacesCaracSelection[0][i] for i in mapping], [externalFacesCaracSelection[1][i] for i in mapping])

            #allTagNames = set([tag.name for tag in triaElements.tags])
            #physicalTagNotInMesh = set(physicalTagNames).difference(allTagNames)
            #assert len(physicalTagNotInMesh)==0,"Physical tag(s) "+str(list(physicalTagNotInMesh))+" not part of the mesh"
            allIds = [triaElements.GetTag(tagName).GetIds() for tagName in tagNames]
            #for combo in itertools.combinations(range(len(allIds)),2):
                #intersection = set(allIds[combo[0]]) & set(allIds[combo[1]])
                #assert len(intersection)==0, "Intersection between physical tags should be empty"
            mappingContiguousBoundaryTags = np.zeros(sortedTria.shape[0]).astype(int)-1
            currentElementId = 0

            for tagId,tagName in enumerate(tagNames):
                tagElementsIds = allIds[tagId]
                newGlobalOrder = np.arange(currentGlobalElementId,currentGlobalElementId+len(tagElementsIds))
                newOrder = np.arange(currentElementId,currentElementId+len(tagElementsIds))
                mappingContiguousBoundaryTags[newOrder] = tagElementsIds
                if (tagName not in faceIdsByTagName) and (tagElementsIds.size > 0):
                    faceIdsByTagName[tagName] = newGlobalOrder
                currentElementId += len(tagElementsIds)
                currentGlobalElementId += len(tagElementsIds)

            #renumerotedExternalFaces = externalFacesCarac[0][mappingContiguousBoundaryTags]
            renumerotedExternalFaces.extend([externalFacesCaracSelection[0][i] for i in mappingContiguousBoundaryTags])
            #renumerotedExternalFacesTetra = externalFacesCarac[1][mappingContiguousBoundaryTags]
            renumerotedExternalFacesTetra.extend([externalFacesCaracSelection[1][i] for i in mappingContiguousBoundaryTags])

        #assert np.min(mappingContiguousBoundaryTags)>-1, "Mapping not done for all faces! Tagnames should be a partition"
        externalFacesCaracRenumeroted = renumerotedExternalFaces, renumerotedExternalFacesTetra
        return externalFacesCaracRenumeroted, faceIdsByTagName

    def RenumerateFaceConnectivity(self,triaFaces:np.ndarray,facesTetra:np.ndarray,mesh:Mesh):
        """Renumerate face connectivity: OpenFOAM is slightly picky about normal orientation. 
        The normal (righthand rule) should point away from the owner cell (so the boundary faces point out of the domain). 
        If this condition is not respected for specific faces, the faces connectivity is changed

        Parameters
        ----------
        triaFaces : np.ndarray
            triangular faces connectivity
        facesTetra : np.ndarray
            corresponding tetra
        mesh : Mesh
            mesh to be written
        Returns
        -------
        tuple
            renumerated face connectivity
        """
        bodyElements = mesh.GetElementsOfType(ED.Tetrahedron_4).connectivity
        bodyElementExtended = bodyElements[facesTetra][:, :, np.newaxis]
        triaFacesExpanded = triaFaces[:, np.newaxis, :]
        comparison = (bodyElementExtended != triaFacesExpanded)
        notInTriaFace = np.all(comparison, axis=2)
        complementaryNodes = np.where(notInTriaFace[:, :, np.newaxis], bodyElementExtended, np.nan).flatten()
        complementaryNodes = complementaryNodes[~np.isnan(complementaryNodes)].astype(int)
        nodesCoordinates = mesh.nodes
        complementaryNodeCoords = nodesCoordinates[complementaryNodes]

        wrongOrientationFaces = self.RetrieveWrongOrientedFaces(nodesCoordinates,triaFaces,complementaryNodeCoords)
        Info("There are "+str(len(wrongOrientationFaces))+" faces not correctly oriented. Correcting orientation")
        triaFaces[wrongOrientationFaces,0],triaFaces[wrongOrientationFaces,1] = triaFaces[wrongOrientationFaces,1],triaFaces[wrongOrientationFaces,0].copy()
        wrongOrientationFaces = self.RetrieveWrongOrientedFaces(nodesCoordinates,triaFaces,complementaryNodeCoords)
        Info("Remaining wrong oriented faces: "+str(len(wrongOrientationFaces)))
        return triaFaces

    def RetrieveWrongOrientedFaces(self,nodesCoordinates:np.ndarray,triaFaces:np.ndarray,complementaryNodeCoords:np.ndarray)->np.ndarray:
        """Retrieve wrong oriented faces 
        Find which faces do not have an acceptable orientation. For a given face, if we move in the direction of the normal, 
        the distance between the triangle center and the complementary node should increase. Else, the orientation is wrong.

        Parameters
        ----------
        nodesCoordinates : np.ndarray
            nodes coordinates
        triaFaces : np.ndarray
            triangular faces connectivity
        complementaryNodeCoords : np.ndarray
            each node coordinates belonging to the tetra attached to the face but not belonging to this very face
        Returns
        -------
        tuple
            renumerated face connectivity
        """
        triaFaceNodes = nodesCoordinates[triaFaces]
        triaFacesCenter = np.mean(triaFaceNodes,axis=1)
        normalTria =  np.cross(triaFaceNodes[:,1,:] - triaFaceNodes[:,0,:],triaFaceNodes[:,2,:] - triaFaceNodes[:,1,:])
        distCenterToCompementaryNodes = np.linalg.norm(triaFacesCenter-complementaryNodeCoords,axis=1)
        distMoveAlongNormalToCompementaryNodes = np.linalg.norm(triaFacesCenter+1e-4*normalTria-complementaryNodeCoords,axis=1)
        closerToComplementaryNodes = distMoveAlongNormalToCompementaryNodes<distCenterToCompementaryNodes
        wrongOrientationFaces = np.nonzero(closerToComplementaryNodes==True)[0]
        return wrongOrientationFaces

    def WriteFaceBoundary(self,filename:str,nbInternalFace:int,faceIdsByTagName:dict,tagTypeByTagName:dict):
        """Write face boundary
         Write face boundary in file

        Parameters
        ----------
        filename : str
            output file name
        nbInternalFace : int
            number of internal face
        physicalTagNames : dict
            mapping between tag name and associated face ids (assume contiguous numerotation for boundary)
        tagTypeByTagName: dict
            mapping between tag name and associated tag type
        """
        filePath = os.path.join(self.location,filename)
        header = CreateHeader(filePath="polyMesh",keyWordClass="polyBoundaryMesh",keyWordObject="boundary")
        with open(filePath, "w+") as faceFile:
            faceFile.write(header)
            faceFile.write("\n\n"+str(len(faceIdsByTagName.keys()))+"\n(\n")
            for tagName,tagIds  in faceIdsByTagName.items():
                    faceFile.write("    "+tagName+"\n    {\n") 
                    faceFile.write("        type"+"\t"+tagTypeByTagName[tagName]+";\n")
                    faceFile.write("        inGroups        List<word> 1("+tagTypeByTagName[tagName]+");\n")
                    faceFile.write("        nFaces          "+str(len(tagIds))+";\n")
                    faceFile.write("        startFace       "+str(min(tagIds)+nbInternalFace)+";\n    }\n")
            faceFile.write(")")
            faceFile.write("\n\n"+ENDLINE)
        if self.compressMeshFiles:
            CompressAndRemoveFile(filePath)


    def WriteFaceConnectivity(self,filename:str,triaFaces:np.ndarray):
        """Write face connectivity
         Write face connectivity in file

        Parameters
        ----------
        filename : str
            output file name
        triaFaces : np.ndarray
            number of internal face
        """
        filePath = os.path.join(self.location,filename)
        header = CreateHeader(filePath="polyMesh",keyWordClass="faceList",keyWordObject="faces")
        with open(filePath, "w+") as faceFile:
            faceFile.write(header)
            faceFile.write("\n\n"+str(len(triaFaces))+"\n(\n")
            for triaFace in triaFaces:
                faceFile.write(f"{len(triaFace)}({' '.join(map(str,triaFace))})\n")
            faceFile.write(")\n")
            faceFile.write("\n\n"+ENDLINE)
        if self.compressMeshFiles:
            CompressAndRemoveFile(filePath)

    def WriteFaceNeighbour(self,filename:str,internalFacesNeighbours:np.ndarray,note:str):
        """Write face neighbours
         Write face neighbours (for each tetra: tetra index attached to each face except minimal index amongst tetra attached to face) in file

        Parameters
        ----------
        filename : str
            output file name
        internalFacesNeighbours : np.ndarray
            internal face neighbours
        note : str
            extra note
        """
        filePath = os.path.join(self.location,filename)
        header = CreateHeader(filePath="polyMesh",keyWordClass="labelList",keyWordObject="neighbour",note=note)
        with open(filePath, "w+") as faceFile:
            faceFile.write(header)
            faceFile.write("\n\n"+str(len(internalFacesNeighbours))+"\n(\n")
            for internalFaceNeighbour in internalFacesNeighbours:
                faceFile.write(str(internalFaceNeighbour)+"\n")
            faceFile.write(")\n")
            faceFile.write("\n\n"+ENDLINE)
        if self.compressMeshFiles:
            CompressAndRemoveFile(filePath)

    def WriteFaceOwner(self,filename:str,facesOwner:np.ndarray,note:str):
        """Write face owners
         Write face owner (for each tetra: tetra indices attached to each face whose index is minimal amongst tetra attached to face) in file

        Parameters
        ----------
        filename : str
            output file name
        internalFacesNeighbours : np.ndarray
            internal face neighbours
        note : str
            extra note
        """
        filePath = os.path.join(self.location,filename)
        header = CreateHeader(filePath="polyMesh",keyWordClass="labelList",keyWordObject="owner",note=note)
        with open(filePath, "w+") as faceFile:
            faceFile.write(header)
            faceFile.write("\n\n"+str(facesOwner.shape[0])+"\n(\n")
            for faceOwner in facesOwner:
                faceFile.write(str(faceOwner)+"\n")
            faceFile.write(")\n")
            faceFile.write("\n\n"+ENDLINE)
        if self.compressMeshFiles:
            CompressAndRemoveFile(filePath)

def CheckIntegrity():
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    from Muscat.MeshTools.MeshCreationTools import CreateCube

    from OpenPisco.MuscatExtentions.FoamReader import FoamReader

    mesh = CreateCube(dimensions=[2,2,2],spacing=[2., 2., 2.],origin=[0, 0, 0],ofTetras=True)
    physicalTagNames = ["X0","X1","Y0","Y1","Z0","Z1"]

    tempdir = TemporaryDirectory.GetTempPath()
    path = os.path.join(tempdir,"FoamMesh")

    print(f"Write OpenFOAM mesh in {path}")
    writer = FoamWriter(location=path)
    tagTypeByTagName=dict(zip(physicalTagNames,["symmetry"]*len(physicalTagNames)))
    writer.Write(mesh,physicalTagNames,tagTypeByTagName)

    reader = FoamReader()
    meshRead = reader.Read(path)
    print(mesh)

    assert np.linalg.norm(meshRead.nodes -mesh.nodes)<1e-8,"Nodes coordinates should be the same"
    #for elementType in [ED.Triangle_3,ED.Tetrahedron_4]:
    #    elementsRead = set(tuple(sorted(connectiv)) for connectiv in meshRead.GetElementsOfType(elementType).connectivity)
    #    elementsOrigin = set(tuple(sorted(connectiv)) for connectiv in mesh.GetElementsOfType(elementType).connectivity)
    #    assert len(elementsRead.difference(elementsOrigin))==0, "Connectivity should be the same"
    return "OK"

def CheckIntegrity_2D():
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    from Muscat.MeshTools.MeshCreationTools import CreateSquare

    from OpenPisco.MuscatExtentions.FoamReader import FoamReader

    mesh = CreateSquare(dimensions=[2,2],spacing=[2., 2.],origin=[0, 0],ofTriangles=True)
    physicalTagNames = ["X0","X1","Y0","Y1"]

    tempdir = TemporaryDirectory.GetTempPath()
    path = os.path.join(tempdir,"FoamMesh")

    print(f"Write OpenFOAM mesh in {path}")
    writer = FoamWriter(location=path)
    tagTypeByTagName=dict(zip(physicalTagNames,["symmetry"]*len(physicalTagNames)))
    writer.Write(mesh,physicalTagNames,tagTypeByTagName)

    reader = FoamReader()
    meshRead = reader.Read(path)
    print(mesh)

    assert np.linalg.norm(meshRead.nodes -mesh.nodes)<1e-8,"Nodes coordinates should be the same"
    #for elementType in [ED.Triangle_3,ED.Tetrahedron_4]:
    #    elementsRead = set(tuple(sorted(connectiv)) for connectiv in meshRead.GetElementsOfType(elementType).connectivity)
    #    elementsOrigin = set(tuple(sorted(connectiv)) for connectiv in mesh.GetElementsOfType(elementType).connectivity)
    #    assert len(elementsRead.difference(elementsOrigin))==0, "Connectivity should be the same"
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
    print(CheckIntegrity_2D())
