# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import itertools
import os
from struct import unpack,calcsize
from typing import Optional,Iterable

import numpy as np
from scipy.datasets import face

from Muscat.MeshContainers.Mesh import Mesh
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.IO.ReaderBase import ReaderBase
from Muscat.Types import MuscatIndex, MuscatFloat
from Muscat.Helpers.Logger import Info


class FieldNotFoundInLastStepDirectoryError(Exception):
    pass

class FieldNotFoundError(Exception):
    pass

def FilterRelevantContent(fileContent:Iterable):
    startIndex = None
    for lineId,line in enumerate(fileContent):
        if line and line[0].isdigit():
            startIndex = lineId+1
            break

    assert startIndex is not None,"Could not find relevant content in file"
    filteredFileContent = fileContent[startIndex:]
    return filteredFileContent

def ReadSingleColumnFile(file:str)->np.ndarray:
    """Read data within single column file

    Parameters
    ----------
    file : str
        file path
    Returns
    -------
    np.ndarray
        data
    """
    with open(file, "r") as fp:
        fileContent = fp.read().splitlines()

    fileContent = FilterRelevantContent(fileContent)
    column = np.array([int(singleLine) for singleLine in fileContent if any(char.isdigit() for char in singleLine)])
    return column

def ReadFoam(inputFile:str)->Mesh:
    """Read mesh written using FOAM format

    Parameters
    ----------
    file : str
        file path
    Returns
    -------
    Mesh
        Mesh object
    """
    reader = FoamReader()
    reader.fileName = inputFile
    return reader.Read(inputFile=inputFile)

def GetLastStepDirectoryInPath(path):
    lastStepDirectory = str(max(
                [float(directory) 
                    for directory in os.listdir(path) 
                    if directory.replace('.','',1).isdigit()]
                ))
    lastStepDirectoryPath = os.path.join(path,lastStepDirectory)
    if not os.path.exists(lastStepDirectoryPath):
        lastStepDirectory = str(int(float(lastStepDirectory)))
        lastStepDirectoryPath = os.path.join(path,lastStepDirectory)
    return lastStepDirectoryPath

class FoamReader(ReaderBase):
    def __init__(self):
        super(FoamReader,self).__init__()

    def Read(self,inputFile:Optional[str]=None)->Mesh:
        """Read mesh written using FOAM format from file

        Parameters
        ----------
        inputFile : str
            input file
        Returns
        -------
        Mesh
            Mesh object
        """
        return self.ReadFoam(directoryPath=inputFile)

    def ReadFoam(self,directoryPath:str)->Mesh:
        """Read mesh written using FOAM format from directoryPath

        Parameters
        ----------
        inputFile : str
            input file
        Returns
        -------
        Mesh
            Mesh object
        """
        nodesCoordinates = self.ReadCoordinates(directoryPath=directoryPath)
        ownerFile=os.path.join(directoryPath,"owner")
        faceTags = ReadSingleColumnFile(file=ownerFile)
        neighbourFile=os.path.join(directoryPath,"neighbour")
        neighbours = ReadSingleColumnFile(file=neighbourFile)
        faceConnectivitiesDict, globalToLocalFaceIndex = self.ReadFaceConnectivity(directoryPath=directoryPath)
        faceConnectivitiesList = list(faceConnectivitiesDict.values())
        faceConnectivityFlat = [connectivity for sublist in faceConnectivitiesList for connectivity in sublist]

        tetraNumber = max(max(faceTags),max(neighbours))+1
        tetraFaces = [ [] for _ in range(tetraNumber) ]
        for faceIndex,faceOwner in enumerate(faceTags):
            tetraFaces[faceOwner].append(faceConnectivityFlat[faceIndex])

        for faceIndex,faceNeighbour in enumerate(neighbours):
            tetraFaces[faceNeighbour].append(faceConnectivityFlat[faceIndex])

        tetraConnectivities = [ [] for _ in range(tetraNumber) ]
        for tetraIndex,tetraFaceConnectivity in enumerate(tetraFaces):
            tetraConnectivities[tetraIndex] = list(set([nodeIndex for face in tetraFaceConnectivity for nodeIndex in face ]))
        tetraConnectivities = np.array(tetraConnectivities)

        if len(faceConnectivitiesList) > 1: #2D extruded with quadrangle and triangle faces, and wedge elements
            connectivities = {ED.Triangle_3:faceConnectivitiesDict[3], ED.Quadrangle_4:faceConnectivitiesDict[4], ED.Wedge_6:tetraConnectivities}
        else: #3D with triangle faces, and tetra elements
            connectivities = {ED.Triangle_3:faceConnectivitiesDict[3],ED.Tetrahedron_4:tetraConnectivities}

        boundaryTags = self.ReadBoundaryTags(directoryPath=directoryPath)
        boundaryTagsWithLocalIndex = dict()
        for tagName, globalIndices in boundaryTags.items():
            numberOfNodeFirst = globalToLocalFaceIndex[globalIndices[0]][0]
            numberOfNodeLast = globalToLocalFaceIndex[globalIndices[-1]][0]
            localIndexFirst = globalToLocalFaceIndex[globalIndices[0]][-1]
            localIndexLast = globalToLocalFaceIndex[globalIndices[-1]][-1]
            assert numberOfNodeFirst == numberOfNodeLast, f"{tagName} contains faces with {numberOfNodeFirst} and {numberOfNodeLast} nodes. Not handled"
            if numberOfNodeFirst == 3:
                boundaryTagsWithLocalIndex[tagName] = {"faceType":ED.Triangle_3, "indices":[localIndexFirst, localIndexLast]}
            elif numberOfNodeFirst == 4:
                boundaryTagsWithLocalIndex[tagName] = {"faceType":ED.Quadrangle_4, "indices":[localIndexFirst, localIndexLast]}
            else:
                raise ValueError("Faces with more than 4 nodes are not handled")
        #externalFacesConnectivity = np.array(list(itertools.chain.from_iterable((externalFacesConnectivityByTagName.values()))))

        return self.BuildMeshFromInformation(nodesCoordinates,connectivities,boundaryTagsWithLocalIndex)

    def BuildMeshFromInformation(self,nodesCoordinates:np.ndarray,connectivities:dict,boundaryTagsWithLocalIndex:dict)->Mesh:
        """Build mesh based on data parsed from FOAM format

        Parameters
        ----------
        nodesCoordinates : np.ndarray
            nodes coordinates
        connectivities : dict
            mapping between element type and element connectivities
        boundaryTags : dict
            mapping between boundary tag names and element ids within tags
        Returns
        -------
        Mesh
            Mesh object
        """
        res = Mesh()
        dataType = MuscatFloat
        res.nodes = np.array(nodesCoordinates, dtype = dataType)
        res.originalIDNodes= np.empty((len(nodesCoordinates),),dtype=MuscatIndex)
        for cpt in range(len(nodesCoordinates)):
            res.originalIDNodes[cpt] = cpt

        #create Muscat Mesh
        globalElementCounter = 0
        for elementName, connectivity in connectivities.items():
            elements = res.GetElementsOfType(elementName)
            nbElements = len(connectivity)
            elements.Reserve(nbElements)
            elements.originalIds = np.arange(globalElementCounter,globalElementCounter+nbElements)
            elements.cpt = nbElements
            globalElementCounter +=nbElements
            elements.connectivity = np.array(connectivity)

        #create Tags
        for tagName, faceTypeAndLocalIndices in boundaryTagsWithLocalIndex.items():
            elements = res.GetElementsOfType(faceTypeAndLocalIndices["faceType"])
            elementIndices = range(faceTypeAndLocalIndices["indices"][0],faceTypeAndLocalIndices["indices"][-1])
            elements.GetTag(str(tagName)).AddToTag(elementIndices)

        res.GenerateManufacturedOriginalIDs()
        res.PrepareForOutput()
        return res

    def ReadCoordinates(self,directoryPath:str)->np.ndarray:
        """Read nodes coordinates from file

        Parameters
        ----------
        directoryPath : str
            OpenFOAM directory path
        Returns
        -------
        np.ndarray
            nodes coordinates
        """
        file=os.path.join(directoryPath,"points")
        with open(file, "r") as fp:
            fileContent = fp.read().splitlines()

        fileContent = FilterRelevantContent(fileContent)
        fileContent = [singleLine.replace("(","").replace(")","") for singleLine in fileContent if any(char.isdigit() for char in singleLine)]
        nodesCoordinates = np.array([[float(coord) for coord in nodeCoordinates.split(" ")] for nodeCoordinates in fileContent])
        return nodesCoordinates

    def ReadFaceConnectivity(self,directoryPath:str)->tuple[dict, list]:
        """Read faces connectivities from file

        Parameters
        ----------
        directoryPath : str
            OpenFOAM directory path
        Returns
        -------
        dict
            key: number of node, value: face connectivity
        list of list
            index: global face index, value: list of [number of node, local face index]
        """
        file=os.path.join(directoryPath,"faces")
        with open(file, "r") as fp:
            fileContent = fp.read().splitlines()

        fileContent = FilterRelevantContent(fileContent)
        fileContent = [singleLine.replace("("," ").replace(")","") for singleLine in fileContent if any(char.isdigit() for char in singleLine)]

        facesConnectivity = [[int(digit) for digit in digits.split(" ")] for digits in fileContent]
        nbNodesPerFace = []
        faceIndicesByNbNodes = dict()
        globalToLocalFaceIndex = []
        cpt = dict()
        for faceConnectivity in facesConnectivity:
            nbNodesInFace = faceConnectivity[0]
            if nbNodesInFace not in nbNodesPerFace:
                nbNodesPerFace.append(nbNodesInFace)
                faceIndicesByNbNodes[nbNodesInFace] = []
                cpt[nbNodesInFace] = 0
            faceIndicesByNbNodes[nbNodesInFace].append(faceConnectivity[1:])
            globalToLocalFaceIndex.append([nbNodesInFace,cpt[nbNodesInFace]])
            cpt[nbNodesInFace] += 1
        return faceIndicesByNbNodes, globalToLocalFaceIndex

    def ReadBoundaryTags(self,directoryPath:str)->dict:
        """Read boundary tags from file

        Parameters
        ----------
        directoryPath : str
            directory path
        Returns
        -------
        np.ndarray
            mapping between boundary tags and element ids is tags
        """
        file=os.path.join(directoryPath,"boundary")
        with open(file, "r") as fp:
            fileContent = fp.read().splitlines()
        fileContent = FilterRelevantContent(fileContent)
        tagNameIndices = [lineIndex-1 for lineIndex,line in enumerate(fileContent) if "{" in line]
        tagNames = [fileContent[tagNameIndex].split()[0] for tagNameIndex in tagNameIndices]
        tagsCaracteristics = [int(fileLine.replace(";","").split(" ")[-1]) for fileLine in fileContent if "nFaces" in fileLine or "startFace" in fileLine]
        elementsInTags = np.array_split(tagsCaracteristics,len(tagsCaracteristics)//2)
        #elementsInTags = [range(elementsInTag[1],elementsInTag[1]+elementsInTag[0]) for elementsInTag in elementsInTags]
        elementsInTags = [[elementsInTag[1],elementsInTag[1]+elementsInTag[0]-1] for elementsInTag in elementsInTags]
        tagMapping = dict(zip(tagNames,elementsInTags))
        return tagMapping

def ReadExtraField(path:str,
                   fieldNames:Iterable[str],
                   fieldSupports:Iterable["ElementType"],
                   binary:Optional[bool]=True,
                   fileDirectory:Optional[str]=None)->np.ndarray:
    """Read field produced by OpenFOAM

    Parameters
    ----------
    path : str
        input file location
    fieldNames : Iterable[str]
        fields name to be read
    fieldSupports : Iterable["ElementType"]
        Field support associated to fields to be read
    binary : bool
        read from binary or ASCII, default is binary
    Returns
    -------
    np.ndarray
        field array
    """
    assert len(fieldNames)==len(fieldSupports),"Number of fields should match number of field supports associated to them"
    if fileDirectory is None:
        try:
            lastStepDirectory = GetLastStepDirectoryInPath(path)
        except ValueError as e:
            raise Exception('Directory '+path+' may not contain any field') from e
    else:
        lastStepDirectory = fileDirectory
    fieldsValues = []
    for fieldName in fieldNames:
        Info("Read field "+fieldName+" in "+str(path))
        directoryPath = os.path.join(path,lastStepDirectory,fieldName)
        if not os.path.exists(directoryPath):
            raise FieldNotFoundInLastStepDirectoryError("Field "+fieldName+" not found in path "+directoryPath)

        if binary:
            fieldValues = ReadBinaryFile(directoryPath,fieldName)
        else:
            fieldValues = ReadASCIIFile(directoryPath,fieldName)

        #if 2D, point sensitivities are doubled (original and projected face)
        #keep only one face
        try: #2D or 3D
            reshapedFieldValues = fieldValues.reshape(2, fieldValues.size //2)
            if np.allclose(reshapedFieldValues[0], reshapedFieldValues[1]): #2D
                finalFieldValues = reshapedFieldValues[0][:, np.newaxis]
            else: #3D
                finalFieldValues = fieldValues
        except ValueError: #3D
            finalFieldValues = fieldValues
        fieldsValues.append(finalFieldValues)
    return fieldsValues

def ReadBinaryFile(directoryPath,fieldName):
    with open(directoryPath, "rb") as fp:
        fileContent = fp.readlines()
        fieldFound = False
        for lineId, line in enumerate(fileContent): 
            if line.startswith(b'internalField') and b'nonuniform' in line: 
                num = int(fileContent[lineId + 1]) 
                if b'vector' in line: 
                    dimensions = 3 
                elif b'symmTensor' in line: 
                    dimensions = 6 
                elif b'tensor' in line: 
                    dimensions = 9 
                else:
                    dimensions = 1
                    
                dataBytes = b''.join(fileContent[lineId + 2:len(fileContent)+1]) 
                fieldValues = unpack(f'{num * dimensions}d', dataBytes[calcsize('c'):num*dimensions*calcsize('d')+calcsize('c')] ) 
                fieldValues = np.array(fieldValues).reshape((num, dimensions)) 
                fieldFound = True
                break

        if not fieldFound:
            raise FieldNotFoundError("InternalField "+fieldName+" not found in path "+str(directoryPath))
    return fieldValues

def ReadASCIIFile(directoryPath,fieldName):
    with open(directoryPath, "r") as fp:
        fileContent = fp.read().splitlines()

        try:
            startIndex = fileContent.index('(')
            endtIndex = fileContent.index(')')
        except ValueError:
            raise FieldNotFoundError("Suspicious, the file "+fieldName+" may contain boundary conditions instead of a proper field")
        fileContent = fileContent[startIndex-1:endtIndex]
        fileContent = [singleLine.replace("(","").replace(")","") for singleLine in fileContent if any(char.isdigit() for char in singleLine)]
        fileContent = [singleLine for singleLine in fileContent][1:]
        fieldValues = np.array([[float(coord) for coord in fieldValue.split(" ")] for fieldValue in fileContent])
    return fieldValues
        
def CheckIntegrity():
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    from Muscat.MeshTools.MeshCreationTools import CreateCube

    from OpenPisco.MuscatExtentions.FoamWriter import FoamWriter

    mesh = CreateCube(dimensions=[2,2,2],spacing=[2., 2., 2.],origin=[0, 0, 0],ofTetras=True)
    print(mesh)
    numTetra = mesh.GetElementsOfType(ED.Tetrahedron_4).GetNumberOfElements()

    physicalTagNames = ["X0","X1","Y0","Y1","Z0","Z1"]

    tempdir = TemporaryDirectory.GetTempPath()
    path = os.path.join(tempdir,"FoamMesh")

    dummyArray = np.arange(3*numTetra).reshape(numTetra,3)
    fieldsToExport = {"DummyField":[dummyArray,2*dummyArray,3*dummyArray],
                      "DummyFieldAgain":[18*dummyArray,10*dummyArray,5*dummyArray]
                      }
    print(f"Write OpenFOAM mesh in {path}")
    writer = FoamWriter(location=path)
    tagTypeByTagName=dict(zip(physicalTagNames,["symmetry"]*len(physicalTagNames)))
    writer.Write(meshObject=mesh,
                 physicalTagNames=physicalTagNames,
                 tagTypeByTagName=tagTypeByTagName,
                 CellFieldsNames=["DummyField","DummyFieldAgain"],
                 CellFields=[fieldsToExport["DummyField"],fieldsToExport["DummyFieldAgain"]])
    mesh = ReadFoam(inputFile=path)

    tet = mesh.GetElementsOfType(ED.Tetrahedron_4)
    assert np.min(tet.connectivity)==0
    assert np.max(tet.connectivity)==mesh.GetNumberOfNodes()-1

    coordinatesField = ReadExtraField(path=path,fieldNames=["DummyField"],fieldSupports=[ED.Tetrahedron_4],binary=False)[0]
    np.testing.assert_almost_equal(fieldsToExport["DummyField"][-1],coordinatesField)

    coordinatesField = ReadExtraField(path=path,fieldNames=["DummyFieldAgain"],fieldSupports=[ED.Tetrahedron_4],binary=False)[0]
    np.testing.assert_almost_equal(fieldsToExport["DummyFieldAgain"][-1],coordinatesField)
    return "OK"

def CheckIntegrity_2D():
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    from Muscat.MeshTools.MeshCreationTools import CreateSquare

    from OpenPisco.MuscatExtentions.FoamWriter import FoamWriter

    mesh = CreateSquare(dimensions=[2,2],spacing=[2., 2.],origin=[0, 0],ofTriangles=True)
    print(mesh)
    numTri = mesh.GetElementsOfType(ED.Triangle_3).GetNumberOfElements()

    physicalTagNames = ["X0","X1","Y0","Y1"]

    tempdir = TemporaryDirectory.GetTempPath()
    path = os.path.join(tempdir,"FoamMesh2D")

    dummyArray = np.arange(3*numTri).reshape(numTri,3)
    fieldsToExport = {"DummyField":[dummyArray,2*dummyArray,3*dummyArray],
                      "DummyFieldAgain":[18*dummyArray,10*dummyArray,5*dummyArray]
                      }
    writer = FoamWriter(location=path)
    tagTypeByTagName=dict(zip(physicalTagNames,["symmetry"]*len(physicalTagNames)))
    writer.Write(meshObject=mesh,
                 physicalTagNames=physicalTagNames,
                 tagTypeByTagName=tagTypeByTagName,
                 CellFieldsNames=["DummyField","DummyFieldAgain"],
                 CellFields=[fieldsToExport["DummyField"],fieldsToExport["DummyFieldAgain"]])
    mesh = ReadFoam(inputFile=path)

    tet = mesh.GetElementsOfType(ED.Tetrahedron_4)
    assert np.min(tet.connectivity)==0
    assert np.max(tet.connectivity)==mesh.GetNumberOfNodes()-1

    coordinatesField = ReadExtraField(path=path,fieldNames=["DummyField"],fieldSupports=[ED.Tetrahedron_4],binary=False)[0]
    np.testing.assert_almost_equal(fieldsToExport["DummyField"][-1],coordinatesField)

    coordinatesField = ReadExtraField(path=path,fieldNames=["DummyFieldAgain"],fieldSupports=[ED.Tetrahedron_4],binary=False)[0]
    np.testing.assert_almost_equal(fieldsToExport["DummyFieldAgain"][-1],coordinatesField)
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
    print(CheckIntegrity_2D()) # pragma: no cover
