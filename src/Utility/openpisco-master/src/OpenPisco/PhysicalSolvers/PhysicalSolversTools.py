# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

from Muscat.Types import MuscatIndex, MuscatFloat
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags,ExtractElementsByMask
from Muscat.MeshTools.MeshCreationTools import ToQuadraticMesh
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshTools.MeshCreationTools import ToQuadraticMesh
from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags,ExtractElementsByMask
from Muscat.MeshTools.MeshModificationTools import CleanLonelyNodes
from Muscat.Types import MuscatIndex, MuscatFloat

import OpenPisco.TopoZones as TZ
from OpenPisco.BaseElements import elementsTypeSupportedForDimension

QuadToLinElemNames = {}
#quadratic
QuadToLinElemNames[ED.Bar_3] = ED.Bar_2
QuadToLinElemNames[ED.Triangle_6] = ED.Triangle_3
QuadToLinElemNames[ED.Quadrangle_8] = ED.Quadrangle_4
QuadToLinElemNames[ED.Tetrahedron_10] = ED.Tetrahedron_4
QuadToLinElemNames[ED.Hexahedron_20] = ED.Hexahedron_8
QuadToLinElemNames[ED.Hexahedron_27] = ED.Hexahedron_8
#linear
QuadToLinElemNames[ED.Point_1] = ED.Point_1
QuadToLinElemNames[ED.Bar_2] = ED.Bar_2
QuadToLinElemNames[ED.Triangle_3] = ED.Triangle_3
QuadToLinElemNames[ED.Quadrangle_4] = ED.Quadrangle_4
QuadToLinElemNames[ED.Tetrahedron_4] = ED.Tetrahedron_4
QuadToLinElemNames[ED.Hexahedron_8] = ED.Hexahedron_8

def SymMatrixToMandelRepresentation(arg):
    if arg.shape[0]==6:
        return np.array([arg[0],arg[1],arg[2],np.sqrt(2)*arg[3],np.sqrt(2)*arg[4],np.sqrt(2)*arg[5]],dtype=MuscatFloat)
    if arg.shape[0]==3:
        return np.array([arg[0],arg[1],np.sqrt(2)*arg[2]],dtype=MuscatFloat)
    if arg.shape[0]==1:
        return np.array([arg[0]],dtype=MuscatFloat)
    raise ValueError('SymMatrixToMandelRepresentation: the input symetric matrix does not have the correct shape')

def ComputeSymMatrixProduct(m1,m2):
    if m1.shape[0]==6 and m2.shape[0]==6 or m1.shape[0]==4 and m2.shape[0]==4:
       return  np.dot(m1[0:3],m2[0:3])+2*np.dot(m1[3:],m2[3:,])
    if m1.shape[0]==3 and m2.shape[0]==3:
       return  np.dot(m1[0:2],m2[0:2])+2*m1[2]*m2[2]
    raise ValueError('ComputeSymMatrixProduct: the input symetric matrix does not have the correct shape')

def CellToPointData(support,field,dim=3,weightbyElementSize=True):
    """
    .. py:method:: CellToPointData(support,field,dim=3,weightbyElementSize=True)

    Transport data from elements of a given dimension to node. The data must be constant at elements.
    By default the routine computes a weighted mean of data at elements (the weights are the volumes of elements).
    Setting weightbyElementSize=False the routine computes an arithmetic mean.

    :param Mesh support: input mesh
    :param dict field: dictionary containing pairs (elementname, data)
    :param int dim: specify the dimension of elements to treat (optional, we treat volumic elements by default)
    :param bool weightbyElementSize: specify if the mean from elements to nodes should be weighted byelements size or arithmetic
    :return: data transported at mesh nodes
    :rtype np.ndarry
    """
    from Muscat.MeshTools.MeshInspectionTools import  GetVolumePerElement
    from Muscat.MeshContainers.Filters.FilterObjects import  ElementFilter

    res = np.zeros( support.GetNumberOfNodes(), dtype = MuscatFloat)
    weight = np.zeros_like(res)
    for name,data in field.items():
        if dim !=  ED.dimensionality[name]:
            continue
        elems = support.GetElementsOfType(name)
        if weightbyElementSize:
            weightAtElement = GetVolumePerElement(support, ElementFilter(elementType=name))
        else:
            weightAtElement = np.ones(elems.GetNumberOfElements())
        conn = elems.connectivity
        for el in range(elems.GetNumberOfElements()):
            res[conn[el,:]] += data[el]*weightAtElement[el]
            weight[ conn[el,:] ] += weightAtElement[el]
        ids = np.nonzero (weight > 0)[0]
        res[ids] = res[ids]/weight[ids]

    return res

def IpToNodesDataOnSurface(IPfield,surfacicTag):
    PByElementType={
        "tri3":lambda x : [1 - x[i,0] -x[i,1], x[i,0], x[i,1]]
    }

    res = np.zeros( IPfield.mesh.GetNumberOfNodes(), dtype = MuscatFloat)
    weight = np.zeros_like(res)
    for name,data in IPfield.data.items():
        if name not in PByElementType.keys():
            continue
        elemtype = IPfield.mesh.GetElementsOfType(name)
        elementaryQuadrature = IPfield.GetRuleFor(name)
        GaussPoints = elementaryQuadrature.points
        nbGaussPoints = GaussPoints.shape[0]
        conn = elemtype.connectivity[:,0:ED.numberOfNodes[name]]
        from Muscat.FE.Spaces.FESpaces import LagrangeSpaceP1
        space = LagrangeSpaceP1[name]
        nbShapeFunctions = space.GetNumberOfShapeFunctions()
        P = np.zeros((nbShapeFunctions,nbGaussPoints), dtype=MuscatFloat)
        for i in range(nbGaussPoints):
            P[:,i] = PByElementType[name](GaussPoints)
        M = np.einsum('ij,jk->ik',P,np.transpose(P))
        ExtMatrix = np.matmul(np.linalg.inv(M),P)
        for element in elemtype.tags[surfacicTag].GetIds():
            dataAtIP = data[element,:]
            dataExt = np.matmul(ExtMatrix,dataAtIP)
            res[conn[element,:]] += dataExt
            weight[ conn[element,:] ] += 1
        ids = np.nonzero (weight > 0)[0]
        res[ids] = res[ids]/weight[ids]
    return res


def IpToNodesData(IPfield,dim=3):
    """
    .. py:method:: IpToNodesData(IPfield,dim=3)

    Transport data from integration points to mesh nodes. The data to treat are filtered by the dimensionality of the mesh elements supporting the integration points. By default, we treat data at volumic elements (dim=3)

    :param IPField IPField : integration points field
    :param int dim: dimension of mesh elements supporting the integration points (optional)
    :return: integration points data transported to nodes
    :rtype np.ndarry
    """
    res = np.zeros( IPfield.mesh.GetNumberOfNodes(), dtype = MuscatFloat)
    weight = np.zeros_like(res)
    for name,data in IPfield.data.items():
         if dim !=  ED.dimensionality[name]:
            continue
         #https://www.code-aster.org/V2/doc/v14/fr/man_r/r3/r3.06.03.pdf
         elemtype = IPfield.mesh.GetElementsOfType(name)
         res = np.zeros( IPfield.mesh.GetNumberOfNodes(), dtype = MuscatFloat)
         elementaryQuadrature = IPfield.GetRuleFor(name)
         GaussPoints = elementaryQuadrature.points
         nbGaussPoints = GaussPoints.shape[0]
         conn = elemtype.connectivity[:,0:ED.numberOfNodes[QuadToLinElemNames[name]]]
         if nbGaussPoints == 1:
            localfield=CellToPointData(IPfield.mesh,IPfield.data,dim=dim)
            for el in range(elemtype.GetNumberOfElements()):
                res[conn[el,:]] = localfield[conn[el,:]]
            continue
         from Muscat.FE.Spaces.FESpaces import LagrangeSpaceP1
         if name==ED.Tetrahedron_10 or name==ED.Tetrahedron_4:
             space = LagrangeSpaceP1[QuadToLinElemNames[name]]
             nbShapeFunctions = space.GetNumberOfShapeFunctions()
             P = np.zeros((nbShapeFunctions,nbGaussPoints), dtype=MuscatFloat)
             for i in range(nbGaussPoints):
                 P[:,i] = [GaussPoints[i,1],1 - GaussPoints[i,0] -GaussPoints[i,1] - GaussPoints[i,2], GaussPoints[i,2], GaussPoints[i,0]]
             M = np.zeros((nbShapeFunctions,nbShapeFunctions), dtype=MuscatFloat)
             for i in range(nbShapeFunctions):
                 for j in range(nbShapeFunctions):
                     M[i,j] = np.dot(P[i,:],P[j,:])
             ExtMatrix = np.matmul(np.linalg.inv(M),P)
         elif name==ED.Triangle_6:
            ExtMatrix = (1./3)*np.array([[5,-1,-1],[-1,5,-1],[-1,-1,5] ],dtype=MuscatFloat)
         else:
            raise ValueError("Don't know transition matrix for element type "+name)
         for element in range(elemtype.GetNumberOfElements()):
             dataAtIP = data[element,:]
             dataExt = np.matmul(ExtMatrix,dataAtIP)
             res[conn[element,:]] += dataExt
             weight[ conn[element,:] ] += 1
         ids = np.nonzero (weight > 0)[0]
         res[ids] = res[ids]/weight[ids]

    return res

def CheckBorderElements(res):
    dimensionality = res.GetElementsDimensionality()
    skinElements = res.GetElementsOfType(elementsTypeSupportedForDimension[dimensionality-1])
    connSkin = skinElements.connectivity

    from OpenPisco.ExtractConnexPart import GetNodeToElementContainerConnectivity
    bodyElements = res.GetElementsOfType(elementsTypeSupportedForDimension[dimensionality])
    maxNumConnections={2:50,3:200}
    try:
        nodetoelementconn=GetNodeToElementContainerConnectivity(mesh=res,elements=bodyElements,maxNumConnections=maxNumConnections[dimensionality])
    except IndexError as e:
        raise Exception("Number of maximum connection allowed for mesh reached "+str(maxNumConnections[dimensionality])) from e
    
    goodSkinElements=[]
    nbNodesPerSkinElement=ED.numberOfNodes[elementsTypeSupportedForDimension[dimensionality-1]]
    for skinElemId in range(skinElements.GetNumberOfElements()):
        boules=[]
        for bouleIndex in range(nbNodesPerSkinElement):
            nodeConn=connSkin[skinElemId][bouleIndex]
            boule = nodetoelementconn[nodeConn,:][nodetoelementconn[nodeConn,:] >=0]
            boules.append(boule)
        intersectboules = np.array(list(set(boules[0]).intersection(*boules[1:])))
        if len(intersectboules):
            goodSkinElements.append(skinElemId)

    skinElements.tags.CreateTag("InteriorSkin").SetIds(goodSkinElements)
    return res

def ExtractInteriorMesh(levelset,userElemTagsToKeep = None):
    dimensionality = levelset.support.GetElementsDimensionality()
    levelset.support.ComputeGlobalOffset()
    # if specific user tags to keep are not specified, we keep only 3D elements and the skin connected to them
    userElemTags = []
    if userElemTagsToKeep is not None:
       userElemTags.extend(userElemTagsToKeep)

    surfaceElem = levelset.support.GetElementsOfType(elementsTypeSupportedForDimension[dimensionality-1])
    assert surfaceElem.GetNumberOfElements(), "No surfacic element, can not extract interior mesh"
    vals = levelset.phi[surfaceElem.connectivity]
    insideids = np.nonzero(np.all(vals<=0,axis=1))[0]
    assert len(insideids),"Need negative values of phi to extract InteriorSkin surface"

    ToKeep = [TZ.Inside3D]
    ToKeep.extend(userElemTags)
    assert len(levelset.support.GetElementsInTag(TZ.Inside3D)), "No tets in Inside3D tag: cannot extract computational mesh "

    outmeshtmp  = type(levelset.support)()
    outmeshtmp.CopyProperties(levelset.support)

    outmeshtmp.nodes = levelset.support.nodes
    outmeshtmp.originalIDNodes = levelset.support.originalIDNodes
    outmeshtmp.nodesTags = levelset.support.nodesTags

    for name,data in levelset.support.elements.items():
        mask = np.zeros(data.GetNumberOfElements(),dtype=bool)
        ids=np.array([],dtype=MuscatIndex)
        for tag in ToKeep:
            ids = np.concatenate((ids,data.GetTag(tag).GetIds()))
        if name == elementsTypeSupportedForDimension[dimensionality-1]:
            ids = np.concatenate((ids,insideids))
        mask[ids] = True
        outmeshtmp.elements.AddContainer(ExtractElementsByMask(data,mask))
    outmeshtmp.PrepareForOutput()

    outmeshtmp  = CheckBorderElements(outmeshtmp)

    ToKeep = [TZ.Inside3D, 'InteriorSkin']
    ToKeep.extend(userElemTags)
    outmesh = ExtractElementsByTags(outmeshtmp,ToKeep)

    CleanLonelyNodes(outmesh)

    #restore original ids
    for i in range(len(outmesh.originalIDNodes)):
        outmesh.originalIDNodes[i] = outmeshtmp.originalIDNodes[outmesh.originalIDNodes[i]]

    for name,data in outmesh.elements.items():
        if data.GetNumberOfElements():
            datatmp = outmeshtmp.GetElementsOfType(name)
            dataout = outmesh.GetElementsOfType(name)

            for i in range(len(dataout.originalIds)):
                dataout.originalIds[i] = datatmp.originalIds[dataout.originalIds[i]]
    return outmesh

def LinToQuadMesh(inputmesh):
    """
    .. py:method:: LinToQuadMesh(inputmesh)

    Convert a linear mesh into a quadratic one.

    :param Mesh inputmesh : input linear mesh
    :return: quadratic mesh built from the linear one
    :rtype Mesh
    """
    return ToQuadraticMesh(inputmesh)

def CheckIntegrity():
    from OpenPisco.Unstructured.LevelsetTools import TakePhiandMeshFromXdmf
    from OpenPisco.Unstructured.Levelset import LevelSet
    ls = LevelSet()
    from OpenPisco.TestData import  GetTestDataPath as GetTestDataPath
    FileName = GetTestDataPath()+"Beam.xmf"
    ls.conform = True
    TakePhiandMeshFromXdmf(ls,FileName)
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    cleanMesh = ExtractInteriorMesh(ls)
    tetra = cleanMesh.GetElementsOfType(ED.Tetrahedron_4)
    field = {}
    field[ED.Tetrahedron_4] = np.ones( tetra.GetNumberOfElements(), dtype = MuscatFloat)

    CellToPointData(cleanMesh,field)
    print(cleanMesh)
    quadMesh = LinToQuadMesh(cleanMesh)
    print(quadMesh)
    return 'ok'

if __name__ == '__main__':
    print(CheckIntegrity())# pragma: no cover
