# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np
from typing import Optional

import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.Types import MuscatIndex
from Muscat.Helpers.Logger import Debug
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter


def GetDualGraphElementContainer(elementcontainer,output):
    setoutput = [set() for _ in range(output.shape[0]) ]
    nbpointsperelement = elementcontainer.GetNumberOfNodesPerElement()

    for i in range(elementcontainer.GetNumberOfElements()):
        coon = elementcontainer.connectivity[i,:]
        for j in range(nbpointsperelement):
           myIndex = coon[j]
           setoutput[myIndex].update(coon)

    for cpt,s in enumerate(setoutput):
        if len(s)==0: continue
        s.discard(cpt)
        s.update(output[cpt,:])
        s.discard(-1)
        output[cpt,0:len(s)] = list(s)

def GetNodeToElementContainerConnectivity(mesh,elements,maxNumConnections:Optional[int]=None):
    if maxNumConnections is None:
        maxNumConnections = GetMaxNumConnectionsRecommanded(mesh)
    dualGraph  = np.zeros((mesh.GetNumberOfNodes(), maxNumConnections), dtype=int) - 1
    usedPoints = np.zeros(mesh.GetNumberOfNodes(), dtype=int)

    cpt =0
    for i in range(elements.GetNumberOfElements()):
        coon = elements.connectivity[i,:]
        for j in coon:
            try:
                dualGraph[j,usedPoints[j]] =  cpt
            except IndexError:
                raise IndexError("Limit reached, only "+str(maxNumConnections)+" allowed ("+str(usedPoints[j])+" found)")
            usedPoints[j] += 1
        cpt+=1

    return dualGraph

def GetMaxNumConnectionsRecommanded(support):
    if support.GetElementsDimensionality() == 2:
        maxNumConnectionsRecommanded = 20
    else:
        maxNumConnectionsRecommanded = 200
    return maxNumConnectionsRecommanded



class ConnexPart():
    def __init__(self, support):
        super(ConnexPart,self).__init__()

        Debug("Init ConnexPart")
        self.support = support

        maxNumConnections = GetMaxNumConnectionsRecommanded(support)
        self.dualGraph = np.zeros((support.GetNumberOfNodes(),maxNumConnections), dtype=int )-1

        if support.props.get("IsConstantRectilinear",False):
            for selection in ElementFilter(dimensionality=support.GetElementsDimensionality())(support):

              for i in selection.indices:
                coon = selection.elements.connectivity[i,:]
                for j in range(len(coon)):
                    myIndex = coon[j]
                    c = np.union1d(self.dualGraph[myIndex,:],np.hstack((coon[0:j],coon[j+1:])))
                    self.dualGraph[myIndex,0:(len(c)-1)] = c[1:]
        else:
            for _, data in support.elements.items():
                GetDualGraphElementContainer(data,self.dualGraph)

        self.NumberOfNeighbours = np.sum((self.dualGraph>=0),axis=1)
        Debug("Init ConnexPart Done")



    def Apply(self, phi, fix, fillField=None):
        """
        Get the maximal connected subset from a subdomain.

        Parameters
        ----------
        phi : np.ndarray
            A levelset function defining the original subdomain.

        fix : np.ndarray
            A field with nonpositive values identifying a set of points
            included in the result.

        Returns
        -------
        np.ndarray
            A field with nonpositive values identifying the points located in
            maximal connected subset.
        """

        if fillField is None:
            fillField = phi

        sol = np.abs(fillField +1e-5)
        treated = np.zeros((self.support.GetNumberOfNodes(),),dtype=bool)
        nextpoint = np.zeros((self.support.GetNumberOfNodes(),),dtype=MuscatIndex)

        nextpointcpt = 0
        indexs = np.nonzero(fix<= 0)[0]

        if len(indexs) == 0:# pragma: no cover
            print("Warning:  non negative value of phi")
            return fillField

        for index in indexs:
            if phi[index] <= 0. :
                nextpoint[nextpointcpt] = index
                treated[index] = True
                nextpointcpt += 1

        cpt = 0
        while cpt < nextpointcpt:

            workingIndex = nextpoint[cpt]
            indexes = self.dualGraph[workingIndex,0:self.NumberOfNeighbours[workingIndex]]
            for index in indexes:
                if not treated[index] :
                    treated[index] = True
                    if phi[index] <= 0.:
                        nextpoint[nextpointcpt] = index
                        nextpointcpt +=1

            cpt += 1
        sol[treated] = fillField[treated]

        return sol

def GetConnexPart_OLD(support,phi, fix, fillField=None):
    from Muscat.MeshTools.ConstantRectilinearMeshTools import GetMultiIndex, GetMonoIndex
    if fillField is None:
        fillField = phi

    sol = np.abs(fillField +1e-5)
    treated = np.zeros((support.GetNumberOfNodes(),))
    nextpoint = np.zeros((support.GetNumberOfNodes(),),dtype=MuscatIndex)

    nextpointcpt = 0
    indexs = np.array(range(support.GetNumberOfNodes()))[fix<= 0]


    if len(indexs) == 0:
        print("Warning:  now negative value of phi")
        return fillField

    for index in indexs:
      if phi[index] <= 0. :
        nextpoint[nextpointcpt] = index
        treated[index] = 1
        sol[index] = fillField[index]
        nextpointcpt += 1


    cpt = 0
    while cpt < nextpointcpt:

        workingIndex = nextpoint[cpt]
        indexes = GetMultiIndex([workingIndex], support.props.get("dimensions"))[0]
        min_indexes = np.maximum(indexes-1,0)
        max_indexes = np.minimum(indexes+1,support.props.get("dimensions")-1)

        ijk = np.zeros(3, dtype=int)
        for i in range(min_indexes[0], max_indexes[0]+1):
           ijk[0] = i
           for j in range(min_indexes[1], max_indexes[1]+1):
              ijk[1] = j
              if support.GetElementsDimensionality() == 3:
                for k in range(min_indexes[2], max_indexes[2]+1):
                    ijk[2] = k
                    index = GetMonoIndex([ijk],support.props.get("dimensions") )[0]

                    if treated[index] == 0:
                        sol[index] = fillField[index]
                        treated[index] = 1
                        if phi[index] <= 0.:

                            nextpoint[nextpointcpt] = index
                            nextpointcpt +=1
              else:
                 index = GetMonoIndex([ijk], support.props["dimensions"])[0]

                 if treated[index] == 0:
                    sol[index] = fillField[index]
                    treated[index] = 1
                    if phi[index] <= 0.:

                        nextpoint[nextpointcpt] = index
                        nextpointcpt +=1


        cpt += 1

    return sol

from Muscat.MeshTools.MeshCreationTools import CreateCube

def FunctionInit(dim,i):
    if (dim ==2):
        return (lambda xy: np.cos(np.pi+np.pi*2*(i/5.)*np.sqrt((xy[:,0])**2+(xy[:,1]*1.1)**2 ) ))
    else:
        return (lambda xyz: np.cos(np.pi+np.pi*2*(i/5.)*np.sqrt((xyz[:,0])**2+(xyz[:,1]*1.1)**2 + (xyz[:,2]*1.2)**2) ))


def CheckIntegrity_2D():
    import OpenPisco.Structured.Levelset2D as Levelset2D
    from Muscat.MeshTools.ConstantRectilinearMeshTools import CreateConstantRectilinearMesh

    nx = 10
    ny = 11
    support = CreateConstantRectilinearMesh(dimensions=[nx,ny],spacing=[1./(nx-1), 1./(ny-1)],origin=[0,0])

    ls = Levelset2D.LevelSet2D(support=support)
    fix = np.zeros((support.GetNumberOfNodes(),))+1
    fix[int(ny/3)]= 0.
    fix[int(ny*3/4)]= 0.

    return RunAndCheck(support,ls,fix)

def CheckIntegrity_3D():
    import OpenPisco.Structured.Levelset3D as Levelset3D
    from Muscat.MeshTools.ConstantRectilinearMeshTools import CreateConstantRectilinearMesh

    nx = 10
    ny = 11
    nz = 12
    support = CreateConstantRectilinearMesh(dimensions=[nx, ny, nz],spacing=[1./(nx-1), 1./(ny-1), 1./(nz-1)],origin=[0, 0, 0])

    ls = Levelset3D.LevelSet3D(support=support)
    fix = np.zeros((support.GetNumberOfNodes(),))+1
    fix[int(nz/3)]= 0.
    fix[int(nz*3/4)]= 0.

    return RunAndCheck(support,ls,fix)

def CheckIntegrity_Unstructured_3D():
    from OpenPisco.Unstructured.Levelset import LevelSet
    from Muscat.MeshTools.MeshCreationTools import CreateCube

    mesh = CreateCube(dimensions=[10,11,12], spacing=[1./10,1./11,1./12], origin=[0.,0.,0],ofTetras=True)

    ls = LevelSet(support=mesh)

    fix = np.zeros((mesh.GetNumberOfNodes(),))+1
    fix[4]= 0.
    fix[9]= 0.
    return RunAndCheck(mesh,ls,fix)

def RunAndCheck(support,ls,fix):
    from Muscat.IO.XdmfWriter import XdmfWriter
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory

    CP = ConnexPart(support)

    writer = XdmfWriter(TemporaryDirectory.GetTempPath()+'ExtractConnexPart_'+str(support.GetElementsDimensionality())+'D.xmf')
    writer.SetTemporal()
    writer.SetBinary()
    writer.SetXmlSizeLimit(0)
    writer.Open()
    ## the old function works only for Rectilinear meshs
    if support.props.get("IsConstantRectilinear",False):
        for i in range(1,10):
          ls.Initialize(FunctionInit(support.GetElementsDimensionality(),i) )
          connexPart = GetConnexPart_OLD(support,ls.phi, fix )
          writer.Write(support,
                    PointFields     = [ls.phi,connexPart, fix ],
                    PointFieldsNames= ['phi', 'connexPart','fix'],
                    Time = 0
                    )
    for i in range(1,10):
      ls.Initialize(FunctionInit(support.GetElementsDimensionality(),i) )
      connexPart2 = CP.Apply(ls.phi, fix)
      writer.Write(support,
                PointFields     = [ls.phi,connexPart2, fix ],
                PointFieldsNames= ['phi', 'connexPart','fix'],
                Time = 0
                )

    writer.Close()

    if support.props.get("IsConstantRectilinear",False):
        diff = max(abs(connexPart-connexPart2))
        assert diff ==0, "There should be no difference"

    return 'OK'

def CheckIntegrity_NodeToElementContainer():
    import time
    mesh = CreateCube(dimensions=[20,20,20],spacing=[1./4, 1./4, 1./4],origin=[0, 0, 0],ofTetras=True)

    def SlowGetNodeToElementContainerConnectivity(elements,output):
        for i in range(elements.GetNumberOfElements()):
            coon = elements.connectivity[i,:]
            for j in range(len(coon)):
                myIndex = coon[j]
                c = np.union1d(output[myIndex,:],[i])
                output[myIndex,0:(len(c)-1)] = c[1:]

    surfaceElements = mesh.GetElementsOfType(ED.Triangle_3)
    bodyElements = mesh.GetElementsOfType(ED.Tetrahedron_4)
    for elemType in [surfaceElements,bodyElements]:
        start=time.time()
        nodetoelementconn=GetNodeToElementContainerConnectivity(mesh=mesh,elements=elemType,maxNumConnections=50)
        print("Optimized way: ",time.time()-start)

        output = np.zeros((mesh.GetNumberOfNodes(), 50), dtype=int) - 1
        start=time.time()
        SlowGetNodeToElementContainerConnectivity(elements=elemType,output=output)
        print("Slow way: ",time.time()-start)

        np.testing.assert_almost_equal(output,nodetoelementconn)
    return 'OK'

def CheckIntegrity():
    totest = [
        CheckIntegrity_2D,
        CheckIntegrity_3D,
        CheckIntegrity_Unstructured_3D,
        CheckIntegrity_NodeToElementContainer
              ]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity())
