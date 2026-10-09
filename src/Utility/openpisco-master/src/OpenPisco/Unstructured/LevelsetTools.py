# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

from Muscat.Types import MuscatFloat
import Muscat.MeshContainers.ElementsDescription as ED
import Muscat.MeshTools.MeshInspectionTools as UMIT
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
import Muscat.IO.MeshTools as MT

import OpenPisco.PhysicalSolvers.FieldsNames as FN
import OpenPisco.TopoZones as TZ

def AssertSupportValidity(on):
    assert on in [FN.Nodes,FN.Centroids]," Invalid support "+str(on)+" ."

def SnapLevelsetToPoint(ls, tol=1e-8, onlyOnRequiredEntities=True, iso = 0.0 ):
    modifiedPhi = np.copy(ls.phi)

    mesh = ls.support
    if onlyOnRequiredEntities:
        if MT.RequiredVertices  in mesh.nodesTags :
            ids = mesh.nodesTags[MT.RequiredVertices].GetIds()
            mask = abs(modifiedPhi[ids]-iso) <  tol
            modifiedPhi[ids[mask]] = iso

        toTreat = [(ED.Bar_2,MT.Ridges) ,
                   (ED.Bar_2, MT.RequiredEdges ),
                   (ED.Triangle_3, MT.RequiredTriangles) ]

        for elemtype,key in toTreat:
            elems = mesh.GetElementsOfType(elemtype)
            if key in elems.tags  and len(elems.tags[key]):
                elid = elems.tags[key].GetIds()
                ids = np.unique(elems.connectivity[elid,:])
                mask = abs(modifiedPhi[ids]-iso) <  tol
                modifiedPhi[ids[mask]] = iso
    else:
        mask = abs(modifiedPhi-iso) <  tol
        modifiedPhi[mask] = iso

    return modifiedPhi

def ComputeGradientOnBodyElements(support,phi,normalizeNodalField=True):
    dimensionality = support.GetElementsDimensionality()
    assert dimensionality in [2,3],"Can only handle dimension 2 and 3"
    if dimensionality==2:
        return ComputeGradientOnTriangles(support=support,phi=phi,on=FN.Nodes,normalizeNodalField=normalizeNodalField)
    elif dimensionality==3:
        return ComputeGradientOnTetrahedrons(support=support,phi=phi,on=FN.Nodes,normalizeNodalField=normalizeNodalField)


def ComputeGradientOnTetrahedrons(support,phi,on=FN.Nodes,normalizeNodalField=True):
    """
    Parameters
    ----------
    support : a tetrahedral mesh
    phi : a scalar field at nodes
    on : support of the output field
    Returns
    -------
    np.ndarry
        A (vector) field containing the gradient of phi evaluated at nodes or centroids
    """
    AssertSupportValidity(on)
    tetra = support.GetElementsOfType(ED.Tetrahedron_4)
    assert tetra.GetNumberOfElements()!=0,"No tetra_4 within the mesh, should fail here!"
    conn = tetra.connectivity
    coords = support.nodes[conn,:]
    mat = np.zeros((tetra.GetNumberOfElements(), 3,3 ), dtype = MuscatFloat)

    mat[...,0,:] = coords[..., 1, :] -  coords[..., 0, :]
    mat[...,1,:] = coords[..., 2, :] -  coords[..., 0, :]
    mat[...,2,:] = coords[..., 3, :] -  coords[..., 0, :]

    invmat =  np.linalg.inv(mat)

    data = phi[conn]
    phimat = np.transpose( ( data[..., 1] - data[..., 0],
                             data[..., 2] - data[..., 0],
                             data[..., 3] - data[..., 0], ) )

    grad = np.squeeze( np.matmul(invmat,phimat[:,:,np.newaxis]) )

    if on==FN.Centroids:
       return grad

    res = np.zeros((support.GetNumberOfNodes(), 3), dtype = MuscatFloat)
    weight = np.zeros( support.GetNumberOfNodes() , dtype = MuscatFloat)
    from  Muscat.MeshTools.MeshInspectionTools import GetVolumePerElement
    weightAtElement = GetVolumePerElement(support, ElementFilter(elementType=ED.Tetrahedron_4))
    
    for i in range(conn.shape[1]):
        np.add.at(weight, conn[:, i], weightAtElement)
        np.add.at(res, conn[:, i], grad * weightAtElement[:, None])

    ids = np.nonzero (weight > 0)[0]
    res[ids,:] = res[ids,:]/weight[ids,np.newaxis]
    
    if normalizeNodalField:
       eps = 1e-10
       norm = np.linalg.norm( res, axis=1 )
       ids = np.nonzero (norm > eps)[0]
       res[ids,:] = res[ids,:]/norm[ids,np.newaxis]

    return res


def ComputeGradientOnTriangles(support,phi,on=FN.Nodes,normalizeNodalField=True):
    """
    Parameters
    ----------
    support : a triangular mesh (2d)
    phi : a scalar field at nodes
    on : support of the output field
    Returns
    -------
    np.ndarry
        A (vector) field containing the gradient of phi evaluated at nodes or centroids
    """
    AssertSupportValidity(on)
    tris = support.GetElementsOfType(ED.Triangle_3)
    assert tris.GetNumberOfElements()!=0,"No Triangle_3 within the mesh, should fail here!"
    conn = tris.connectivity
    coords = support.nodes[conn,:]
    mat = np.zeros((tris.GetNumberOfElements(), 2,2 ), dtype = MuscatFloat)

    mat[...,0,:] = coords[..., 1, :] -  coords[..., 0, :]
    mat[...,1,:] = coords[..., 2, :] -  coords[..., 0, :]

    invmat =  np.linalg.inv(mat)

    data = phi[conn]
    phimat = np.transpose( ( data[..., 1] - data[..., 0],
                             data[..., 2] - data[..., 0], ))

    grad = np.squeeze( np.matmul(invmat,phimat[:,:,np.newaxis]) )

    if on==FN.Centroids:
       return grad

    res = np.zeros((support.GetNumberOfNodes(), 2), dtype = MuscatFloat)
    weight = np.zeros( support.GetNumberOfNodes() , dtype = MuscatFloat)
    from  Muscat.MeshTools.MeshInspectionTools import GetVolumePerElement
    weightAtElement = GetVolumePerElement(support, ElementFilter(elementType=ED.Triangle_3))
    
    for i in range(conn.shape[1]):
        np.add.at(weight, conn[:, i], weightAtElement)
        np.add.at(res, conn[:, i], grad * weightAtElement[:, None])

    ids = np.nonzero (weight > 0)[0]
    res[ids,:] = res[ids,:]/weight[ids,np.newaxis]
    if normalizeNodalField:
        eps = 1e-10
        norm = np.linalg.norm( res, axis=1 )
        ids = np.nonzero (norm > eps)[0]
        res[ids,:] = res[ids,:]/norm[ids,np.newaxis]

    return res

def RegularizeDistance(levelset,dmax, alpha_f):
    """
    Parameters
    ----------
    levelset : a level set object
    Returns
    -------
    np.ndarry
        A regularized version of the level set function
    """
    return 0.5*(1.+np.tanh( (np.abs(levelset.phi)-dmax) / (alpha_f*dmax) ))

def ComputeMeanCurvature(levelset):
    """
    Parameters
    ----------
    levelset : a level set object
    Returns
    -------
    Compute the mean curvature at level set support nodes
    """
    dimensionality = levelset.support.GetElementsDimensionality()
    assert dimensionality==3,"Curvature formula not coded in 2D"
    if levelset.conform:
       return ComputeMeanCurvatureCotangentOperator(levelset)
    else:
       return ComputeDivGradPhi(levelset.support,levelset.phi)

def ComputeMeanCurvatureCotangentOperator(levelset):
    """
    Parameters
    ----------
    levelset : a level set object
    Returns
    -------
    Compute the mean curvature at interface nodes using the cotangent operator
    The interface is located using the tag "InterfaceSurf"
    """
    errorMessage = 'Need a triangulation to compute mean curvature using this function'
    assert TZ.InterSurf in levelset.support.GetNamesOfElementTags(),errorMessage
    mean = np.zeros( levelset.support.GetNumberOfNodes(), dtype = MuscatFloat)
    levelset.support.ComputeGlobalOffset()
    from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags
    surf = ExtractElementsByTags(levelset.support, [TZ.InterSurf,TZ.InterBars])
    from Muscat.MeshTools.MeshModificationTools import CleanLonelyNodes
    CleanLonelyNodes(surf)
    surf.ComputeGlobalOffset()
    operator = np.zeros( (surf.GetNumberOfNodes(),3), dtype = MuscatFloat)
    normal = np.zeros( (surf.GetNumberOfNodes(),3), dtype = MuscatFloat)
    tris = surf.GetElementsOfType(ED.Triangle_3)
    mask = np.zeros((surf.GetNumberOfNodes()), dtype=bool )
    from OpenPisco.ExtractConnexPart import GetNodeToElementContainerConnectivity
    nodetoelementconn=GetNodeToElementContainerConnectivity(mesh=surf,elements=tris,maxNumConnections=15)
    nex = np.zeros(3, dtype=int)
    prev = np.zeros(3, dtype=int)
    nex[0] = 1
    nex[1] = 2
    nex[2] = 0
    prev[0] = 2
    prev[1] = 0
    prev[2] = 1
    for i in range(surf.GetNumberOfNodes()):
        boule = nodetoelementconn[i,:][nodetoelementconn[i,:] >=0]
        conn = tris.connectivity[boule,:]
        area = 0.
        for j in range(len(boule)):
            index = np.nonzero ( conn[j,:] == i )[0][0]
            edge0 = surf.nodes[conn[j,nex[index]]] - surf.nodes[conn[j,index]]
            edge1 = surf.nodes[conn[j,nex[index]]] - surf.nodes[conn[j,prev[index]]]
            edge2 = surf.nodes[conn[j,prev[index]]] - surf.nodes[conn[j,index]]
            cotalpha = np.dot(edge0, edge1)/ np.linalg.norm(np.cross(edge0, edge1))
            cotbeta = -np.dot(edge1, edge2)/ np.linalg.norm(np.cross(edge1, edge2))
            normal[i,:]+= np.cross(surf.nodes[conn[j,2]] - surf.nodes[conn[j,0]], surf.nodes[conn[j,1]] - surf.nodes[conn[j,0]])
            operator[ conn[j,index] ] += cotalpha*edge2+cotbeta*edge0
            area += 0.25*(cotalpha*np.linalg.norm(edge2)**2 + cotbeta*np.linalg.norm(edge0)**2 )
        operator[i,:] = operator[i,:]/area
        sign = np.sign(np.dot(normal[i,:],operator[i,:]))
    meansurf = 0.5*sign*np.linalg.norm(operator, axis = 1)

    #correction for points belonging to interface bars
    bars = surf.GetElementsOfType(ED.Bar_2)
    ids = bars.GetTag(TZ.InterBars).GetIds()
    if len(ids)==0:
        mean[surf.originalIDNodes] = meansurf
        return mean

    boundarynodes = bars.connectivity.flatten()
    ids = np.unique(boundarynodes)
    mask[ids] = True
    for index in ids:
        meansurf[index] = 0.
        boule = nodetoelementconn[index,:][nodetoelementconn[index,:] >=0]
        #conn: connectivity over the surface TZ.InterSurf
        conn =  tris.connectivity[boule,:]
        #indices: points over TZ.InterSurf but not over TZ.InterBars
        indices = conn[ mask[conn] == False]
        meansurf[index] = np.sum(meansurf[indices]) / len(boule)

    mean[surf.originalIDNodes] = meansurf

    return mean

def ComputeGaussianCurvature(levelset):
    """
    Parameters
    ----------
    levelset : a level set object
    Returns
    -------
    Compute the gaussian curvature at vertices of the interface using angle deficit approximation
    """
    errorMessage = 'Need a triangulation to compute mean curvature using this function'
    assert TZ.InterSurf in levelset.support.GetNamesOfElementTags(),errorMessage
    gaussian = np.zeros( levelset.support.GetNumberOfNodes(), dtype = MuscatFloat)
    from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByTags
    surf = ExtractElementsByTags(levelset.support, tagsToKeep= [TZ.InterSurf,TZ.InterBars])
    from Muscat.MeshTools.MeshModificationTools import CleanLonelyNodes
    CleanLonelyNodes(surf)
    operator = np.zeros( surf.GetNumberOfNodes(), dtype = MuscatFloat)
    tris = surf.GetElementsOfType(ED.Triangle_3)
    mask = np.zeros((surf.GetNumberOfNodes()), dtype=bool )
    from OpenPisco.ExtractConnexPart import GetNodeToElementContainerConnectivity
    nodetoelementconn=GetNodeToElementContainerConnectivity(levelset.support,tris,15)
    nex = np.zeros(3, dtype=int)
    prev = np.zeros(3, dtype=int)
    nex[0] = 1
    nex[1] = 2
    nex[2] = 0
    prev[0] = 2
    prev[1] = 0
    prev[2] = 1
    for i in range(surf.GetNumberOfNodes()):
        boule = nodetoelementconn[i,:][nodetoelementconn[i,:] >=0]
        conn = tris.connectivity[boule,:]
        angle = 0.
        area = 0.
        for j in range(len(boule)):
            index = np.nonzero( conn[j,:] == i )[0][0]
            edge0 = surf.nodes[conn[j,prev[index]]] - surf.nodes[conn[j,index]]
            edge1 = surf.nodes[conn[j,nex[index]]] - surf.nodes[conn[j,index]]
            area += np.linalg.norm(np.cross(edge0, edge1))
            angle += np.arccos(np.clip(np.dot(edge0,edge1), -1.0, 1.0)/(np.linalg.norm(edge0)*np.linalg.norm(edge1)))
        operator[i] = 6*(2*np.pi - angle)/area

    #correction for points belonging to interface bars

    bars = surf.GetElementsOfType(ED.Bar_2)
    boundarynodes = bars.connectivity.flatten()
    ids = np.unique(boundarynodes)
    mask[ids] = True
    for index in ids:
        operator[index] = 0.
        boule = nodetoelementconn[index,:][nodetoelementconn[index,:] >=0]
        conn =  tris.connectivity[boule,:]
        indices = conn[ mask[conn] == False]
        operator[index] = np.sum(operator[indices]) / len(boule)

    gaussian[surf.originalIDNodes] = operator
    return gaussian

def ComputePrincipalCurvatures(levelset):
    """
    Parameters
    ----------
    levelset : a level set object
    Returns
    -------
    Compute the principal curvatures at vertices of the interface using gaussian and mean
    """
    gaussian = ComputeGaussianCurvature(levelset)
    mean = ComputeMeanCurvature(levelset)
    # if delta < 0 we take the mean
    first = np.copy(mean)
    second = np.copy(mean)
    ids = np.nonzero(mean**2-gaussian >=0.)[0]
    first[ids] =  mean[ids]+ np.sqrt(mean[ids]**2- gaussian[ids])
    second[ids] = mean[ids] - np.sqrt(mean[ids]**2 - gaussian[ids])

    return first, second

def ComputeDivGradPhi(support,phi,on=FN.Nodes):
    """
    Parameters
    ----------
    levelset : a level set object
    Returns
    -------
    A (scalar) field containing the mean curvature at mesh vertices
    Notes
    -----
    The curvature is computed as div( nabla (phi) / ||nabla (phi)|| )
    """

    AssertSupportValidity(on)
    gradient = ComputeGradientOnTetrahedrons(support,phi)
    tetra = support.GetElementsOfType(ED.Tetrahedron_4)
    conn = tetra.connectivity
    coords = support.nodes[conn,:]
    mat = np.zeros((tetra.GetNumberOfElements(), 3,3 ), dtype = MuscatFloat)

    mat[...,0,:] = coords[..., 1, :] -  coords[..., 0, :]
    mat[...,1,:] = coords[..., 2, :] -  coords[..., 0, :]
    mat[...,2,:] = coords[..., 3, :] -  coords[..., 0, :]

    invmat =  np.linalg.inv(mat)
    data = gradient[conn,:]
    datamat =   np.transpose([ data[..., 1,:] - data[..., 0,:],
                               data[..., 2,:] - data[..., 0,:],
                               data[..., 3,:] - data[..., 0,:] ],
                               axes = (1,0,2) )
    grad = np.squeeze( np.matmul(invmat[:,:],datamat[:,:]) )
    gradel = grad[:,0,0] + grad[:,1,1] + grad[:,2,2]
    if on==FN.Centroids:
       return grad

    from  OpenPisco.PhysicalSolvers.PhysicalSolversTools import CellToPointData
    return CellToPointData(support,{ED.Tetrahedron_4:gradel},dim=3)

def UpdatePhiFromSupport(levelset):
    """
    Parameters
    ----------
    levelset : a level set object
    Returns
    -------
    Modify the level set function accordingly to the support mesh
    """
    assert levelset.conform,"Cannot use function UpdatePhiFromSupport in non conformal level set mode."
    from OpenPisco.Unstructured.MmgMesher import ComputeDistanceField
    surf = UMIT.ExtractElementsByTags(levelset.support, [TZ.InterSurf])
    phi = ComputeDistanceField(levelset.support,surf)
    levelset.TakePhiAndMesh(phi, levelset.support)

def TakeMeshFromXdmf(levelset,filename):
    #for use with an output xdmf file from topology optimization, already having its Tags
    
    from Muscat.IO.XdmfReader import XdmfReader
    reader = XdmfReader(filename = filename)
    reader.Read()
    grid = reader.xdmf.GetDomain(-1).GetGrid(-1)
    mesh = grid.GetSupport()
    levelset.support = mesh

    if 'phi' in grid.GetPointFieldsNames():
        levelset.phi = grid.GetFieldData('phi')
    else:
        UpdatePhiFromSupport(levelset)
    
def TakePhiandMeshFromXdmf(levelset,filename):

    def isBoolean(field):
        return np.all(np.logical_or(field==0,field==1))
    
    from Muscat.IO.XdmfReader import XdmfReader
    reader = XdmfReader(filename = filename)
    reader.Read()
    grid = reader.xdmf.GetDomain(-1).GetGrid(-1)
    mesh = grid.GetSupport()

    # convert boolean grid fields to elements tags
    offsets = mesh.ComputeGlobalOffset()
    for elemtype,data in mesh.elements.items():
        for name in grid.GetCellFieldsNames():
            field = grid.GetFieldData(name)
            if np.issubdtype(field.dtype,np.integer) and isBoolean(field):
                field = field[offsets[elemtype]:offsets[elemtype]+data.GetNumberOfElements()]
                ids = np.nonzero(field ==1 )[0]
                if len(ids):
                   data.GetTag(name).AddToTag(ids)

    # convert boolean node fields to node tags
    for name in grid.GetPointFieldsNames():
        field = grid.GetFieldData(name)
        if np.issubdtype(field.dtype,np.integer) and isBoolean(field):
            ids = np.nonzero(field ==1 )[0]
            if len(ids):
                mesh.nodesTags.CreateTag(name,errorIfAlreadyCreated=False).AddToTag(ids)
    levelset.support = mesh

    if 'phi' in grid.GetPointFieldsNames():
        levelset.phi = grid.GetFieldData('phi')
    else:
        UpdatePhiFromSupport(levelset)

from OpenPisco.Unstructured.Levelset import LevelSet as LevelSet
import  OpenPisco.Unstructured.MmgMesher as MmgMesher
from OpenPisco.TestData import  GetTestDataPath as GetTestDataPath


from Muscat.MeshTools.MeshCreationTools import CreateSquare, CreateCube
import Muscat.ImplicitGeometry.ImplicitGeometryObjects as ImplicitGeometry


def CheckIntegrity_Curvature():
    UM = CreateCube(dimensions=[5, 5, 5], spacing=[1./4, 1./4, 1./4], origin=[0., 0., 0], ofTetras=True)
    ls = LevelSet(support=UM)
    opts = {"iso":0.0,"hausd":0.09,"hmin":0.09,"hmax":0.7,"nr":True}
    ls.MesherInfo = opts
    ls.conform = True
    init = ImplicitGeometry.ImplicitGeometrySphere(center=[.5,.5,.5],radius=.25)
    ls.phi = init.ApplyVector(UM)
    MmgMesher.MmgMesherActionLevelset(ls,ls.MesherInfo)
    ff = ElementFilter(eTag=TZ.InterSurf)
    nIndices = ff.GetNodesIndices(ls.support)
    meanCurvatureAtSurface = ComputeMeanCurvature(ls)[nIndices]
    assert np.abs(np.min(meanCurvatureAtSurface)-4.)<0.4
    assert np.abs(np.max(meanCurvatureAtSurface)-4.)<0.4
    ComputeGaussianCurvature(ls)
    ComputePrincipalCurvatures(ls)
    
    return "ok"

def CheckIntegrity_Derivatives():
    UM =  CreateCube(dimensions=[10, 10, 10], spacing=[20./9, 20./9, 20./9], origin=[0., 0., 0], ofTetras=True)
    ls = LevelSet(support=UM)
    init = ImplicitGeometry.ImplicitGeometrySphere(center=[10.,10.,10.],radius=8.)
    init.insideOut = True
    ls.phi = init.ApplyVector(UM)
    ls.MesherInfo = {"iso":0.0,"hausd":1,"hmin":1,"hmax":2,"nr":True}
    ls.conform = True
    MmgMesher.MmgMesherActionLevelset(ls,ls.MesherInfo)
    res = ComputeGradientOnTetrahedrons(ls.support,ls.phi)
    ComputeDivGradPhi(ls.support,ls.phi)
    return "ok"

def CheckIntegrity_ComputeGradientOnTriangles():
    mesh = CreateSquare(origin=[0.,0.],dimensions=[40,40],spacing=[1./39,1./39],ofTriangles=True)
    levelset = LevelSet(support=mesh)
    levelset.phi = ImplicitGeometry.ImplicitGeometrySphere(center=[.5,.5,0.],radius=.25).ApplyVector(mesh)
    ComputeGradientOnTriangles(levelset.support,levelset.phi)
    return "ok"

def CheckIntegrity_TakePhiandMeshFromXdmf():
    ls = LevelSet()
    ls.conform=True
    FileName = GetTestDataPath()+"Beam.xmf"
    TakePhiandMeshFromXdmf(ls,FileName)
    return "ok"

def CheckIntegrity_SnapLevelsetToPoint():
    mesh = CreateSquare()
    ls = LevelSet(support=mesh)
    ls.phi = np.arange(mesh.GetNumberOfNodes() ) - mesh.GetNumberOfNodes()/2 + 0.0000000001
    print(ls.phi)
    newphi = SnapLevelsetToPoint(ls, onlyOnRequiredEntities=False)
    print(newphi)
    if np.abs(newphi[2]) > np.finfo(float).eps:
        return "not ok,SnapLevelsetToPoint(ls, onlyOnRequiredEntities=False)  "
    mesh.nodesTags.CreateTag(MT.RequiredVertices).SetIds([0])

    ls.phi[0] = 1e-12
    print(ls.phi)
    newphi = SnapLevelsetToPoint(ls)
    print(newphi)
    if np.abs(newphi[0]) > np.finfo(float).eps:
        return "not ok SnapLevelsetToPoint(ls)"

    return "ok"

def CheckIntegrity(GUI =  False):
    totest = [CheckIntegrity_ComputeGradientOnTriangles,
              CheckIntegrity_Curvature,
              CheckIntegrity_Derivatives,
              CheckIntegrity_TakePhiandMeshFromXdmf,
              CheckIntegrity_SnapLevelsetToPoint]

    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res

    return "ok"


if __name__ == '__main__': # pragma: no cover
    print(CheckIntegrity(True))
