# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations

import numpy as np

import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Mesh import Mesh

def ComputeAspectRatioOnBodyElements(mesh:Mesh)->np.ndarray:
    """Compute mesh aspect ratio for body elements (triangles elements for 2D mesh, tetrahedron elements for 3D mesh)

    Parameters
    ----------
    mesh : Mesh
        Mesh

    Returns
    -------
    np.ndarray
        aspect ratio for all body elements
    """
    dimensionality=mesh.GetElementsDimensionality()
    assert dimensionality in [2,3],"Dim not supported "+str(dimensionality)
    if dimensionality==2:
        return ComputeAspectRatioOnTriangles(mesh)
    elif dimensionality==3:
        return ComputeAspectRatioOnTetrahedrons(mesh)

def ComputeAspectRatioOnTetrahedrons(mesh:Mesh):
    """Compute mesh aspect ratio for tetrahedron elements

    See, for more details
    https://docs.salome-platform.org/latest/gui/SMESH/aspect_ratio_3d.html
    
    Parameters
    ----------
    mesh : Mesh
        Mesh

    Returns
    -------
    np.ndarray
        aspect ratio for all body elements
    """
    elems = mesh.GetElementsOfType(ED.Tetrahedron_4)
    assert elems.GetNumberOfElements()!=0,"There is no Tetrahedron_4 in the mesh"
    nodes = mesh.nodes[elems.connectivity]
    arrpt = np.array([nodes[...,1,:]-nodes[...,0,:],nodes[...,2,:]-nodes[...,0,:],nodes[...,3,:]-nodes[...,0,:],nodes[...,2,:]-nodes[...,1,:],nodes[...,2,:]-nodes[...,3,:],nodes[...,3,:]-nodes[...,1,:]])
    crspt0 = np.array([nodes[...,1,:]-nodes[...,3,:],nodes[...,0,:]-nodes[...,3,:],nodes[...,1,:]-nodes[...,3,:],nodes[...,1,:]-nodes[...,0,:]])
    crspt1 = np.array([nodes[...,2,:]-nodes[...,3,:],nodes[...,2,:]-nodes[...,3,:],nodes[...,0,:]-nodes[...,3,:],nodes[...,2,:]-nodes[...,0,:]])
    crspt2 = np.array([nodes[...,0,:]-nodes[...,3,:],nodes[...,1,:]-nodes[...,3,:],nodes[...,2,:]-nodes[...,3,:],nodes[...,3,:]-nodes[...,0,:]])
    edges = np.linalg.norm(arrpt,axis=2)
    vnorms= np.cross(crspt0,crspt1)
    nnorms= np.linalg.norm(vnorms,axis=2)
    dedges= np.abs(np.sum(vnorms*crspt2,axis=2))
    radius= np.min(dedges,axis=0)/np.sum(nnorms,axis=0)
    return np.max(edges,axis=0)/(2*np.sqrt(6)*radius)

def ComputeAspectRatioOnTriangles(mesh:Mesh):
    """Compute mesh aspect ratio for triangle elements

    See, for more details
    https://docs.salome-platform.org/latest/gui/SMESH/aspect_ratio.html
    
    Parameters
    ----------
    mesh : Mesh
        Mesh

    Returns
    -------
    np.ndarray
        aspect ratio for all triangle elements
    """
    elems = mesh.GetElementsOfType(ED.Triangle_3)
    assert elems.GetNumberOfElements()!=0,"There is no Triangle_3 in the mesh"
    nodes = mesh.nodes[elems.connectivity]
    firstEdge = nodes[:,1,:]-nodes[:,0,:]
    secondEdge = nodes[:,2,:]-nodes[:,0,:]
    trianglesArea = 0.5 * np.abs(firstEdge[:,0] * secondEdge[:,1]- firstEdge[:,1] * secondEdge[:,0])
    arrpt = np.array([nodes[...,1,:]-nodes[...,0,:],
                      nodes[...,2,:]-nodes[...,0,:],
                      nodes[...,2,:]-nodes[...,1,:]])
    edges = np.linalg.norm(arrpt,axis=2)
    maxLenght=np.max(edges,axis=0)
    aspectRatio=maxLenght*np.sum(edges,axis=0)/(4*np.sqrt(3)*trianglesArea)
    return aspectRatio


def AcceptMeshQuality(mesh:Mesh,val:float=1.)->bool:
    """Accept mesh quality based on prescribed threshold

    Parameters
    ----------
    mesh : Mesh
        Mesh
    val : float, optional
        threshold, by default 1.

    Returns
    -------
    bool
        True if maximal aspect ratio is lower than threshold, else False
    """
    ar = ComputeAspectRatioOnBodyElements(mesh)
    if np.max(ar) <= val:return True
    else:return False

from Muscat.MeshTools.MeshCreationTools import CreateCube, CreateSquare, CreateMeshOf

def CheckIntegrity_AspectRatioTetra(GUI=False):

    points = [[-1,-1/np.sqrt(3),-1/np.sqrt(6)],[1,-1/np.sqrt(3),-1/np.sqrt(6)],[0,2/np.sqrt(3),-1/np.sqrt(6)],[0,0,3/np.sqrt(6)] ]
    tets = [[0,1,2,3],]
    mesh = CreateMeshOf(points,tets,ED.Tetrahedron_4)
    ar = ComputeAspectRatioOnBodyElements(mesh)
    np.testing.assert_almost_equal(ar[0],1.0)
    return "ok"

def CheckIntegrity_AspectRatioTria(GUI=False):
    mesh = CreateSquare(dimensions=[10,11],spacing=[4.,4.],ofTriangles=True)
    ComputeAspectRatioOnTriangles(mesh)
    points = [[3.0,4.0],[-2.0,3.0],[0.5+0.5*np.sqrt(3),3.5-2.5*np.sqrt(3)] ]
    tria = [[0,1,2],]
    mesh = CreateMeshOf(points,tria,ED.Triangle_3)
    ar = ComputeAspectRatioOnBodyElements(mesh)
    np.testing.assert_almost_equal(ar[0],1.0)
    return "ok"


def CheckIntegrity_QualityMesh(GUI=False):
    mesh = CreateCube(dimensions=[2,2,2],spacing=[1.,1.,1.],origin=[0, 0, 0],ofTetras=True)
    ComputeAspectRatioOnTetrahedrons(mesh)
    ac = AcceptMeshQuality(mesh,val=10.)
    print("Mesh quality accepted ",ac)
    return "ok"

def CheckIntegrity(GUI=False):
    totest= [
    CheckIntegrity_AspectRatioTetra,
    CheckIntegrity_AspectRatioTria,
    CheckIntegrity_QualityMesh,
    ]
    for f in totest:
        print("running test : " + str(f))
        res = f(GUI)
        if str(res).lower() != "ok":
            return "error in "+str(f) + " res"
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity(True))# pragma: no cover
