# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

# This module contain several regionalization approaches for semi-infinite criteria.
# The idea is the following: instead of computing the aggregated value of the criteria on the whole domain, and assimilate this value as a single constraint value,
# we define N regions (with 1<=N<=number of elements/nodes), we compute the aggregated value of the criteria on each region and assimilate each aggregated value as a single constraint.
# In other words, it creates N constraints for a single criteria.
#
# From now on, we denote by geometrical support the type of support where the local values field lives
#
from __future__ import annotations

from collections import OrderedDict
from typing import Dict,List,Tuple,Type,Optional

import numpy as np

from Muscat.MeshContainers.ElementsDescription import ElementsInfo
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshContainers.Filters.FilterOperators import UnionFilter
from Muscat.MeshContainers.Mesh import Mesh

import OpenPisco.PhysicalSolvers.FieldsNames as FN

def GetGlobalIndicesInRegion(submethod:str,regionID:int,numRegion:int,concatValues:np.ndarray)->List:
    """Get the sorted indices of the geometrical support depending on the values and the region ID.

    Parameters
    ----------
    submethod : str
        name of the submethod to be used for defining the region
    regionID : int
        id number of the region
    numRegion : int
        total number of regions expected
    concatValues : np.ndarray
        value for the geometrical support entities considered

    Returns
    -------
    List
        sorted global numerotation indices of the geometrical support entities part of the region regionID
    """
    assert numRegion <= len(concatValues),"ERROR: The maximum number of regions allowed is: "+str(len(concatValues))
    sorted2initial = np.argsort(concatValues)
    globalIds=[]
    assert submethod in ["Interlacing","NearValues"],"No other submethod available"
    if submethod == "Interlacing":
        for sortedID in range(regionID, len(concatValues),numRegion):
            globalIds.append(sorted2initial[sortedID])
    elif submethod == "NearValues":
        n = len(concatValues)
        r = numRegion
        for sortedID in range(int(regionID*n/r),int((regionID+1)*n/r)):
            globalIds.append(sorted2initial[sortedID])
    return sorted(globalIds)

def GetLocalElemIndicesInRegion(submethod:str,regionId:int,numberOfRegions:np.ndarray,concatDataValues:np.ndarray,numElelmByType:Dict)->Dict:
    """Assign the indices of the elements with respect to their type of elements for a given region

    Parameters
    ----------
    submethod : str
        name of the submethod to be used for defining the region
    regionId : int
        id number of the region
    numberOfRegions : np.ndarray
        total number of regions expected
    concatDataValues : np.ndarray
        value for the geometrical support entities considered
    numElelmByType : Dict
        number of each type of elements

    Returns
    -------
    Dict
        local element ids by element types
    """
    elementsId=np.array(GetGlobalIndicesInRegion(submethod,regionId,numberOfRegions,concatDataValues))
    localElemIdByElemType=dict()
    for name,numElem in numElelmByType.items():
        mask=np.nonzero(elementsId<numElem)
        localElemIdByElemType[name]=elementsId[mask]
        elementsId-=numElem
        compMask=np.nonzero(elementsId>=0)
        elementsId=elementsId[compMask]
    return localElemIdByElemType

def GetElemFilterRegionFromIndices(mesh:Mesh,dimensionality:int,localIdsByElem:Dict)->ElementFilter:
    """Build the tags to be used from the region element indices

    Parameters
    ----------
    mesh : Mesh
        Mesh
    dimensionality : int
        dimension of the computational support
    localIdsByElem : Dict
        local numerotation to the elements of the elements with respect to their type

    Returns
    -------
    ElementFilter
        filter with suitable tags and dimension for a given region
    """
    tagsToAdd=[]
    tagsPrefix="filteredAlter"
    ff=ElementFilter(dimensionality)
    for name,data,ids in ff(mesh):
        data.tags.CreateTag(tagsPrefix+name).SetIds(localIdsByElem[name])
        tagsToAdd.append(tagsPrefix+name)
    newff=ElementFilter(tags=tagsToAdd)
    mesh.DeleteElemTags(tagsToAdd)
    return newff

def GetElemFilterRegion(mesh:Mesh, dimensionality:int,submethod:str,regionId:int,numberOfRegions:int,concatDataValues:np.ndarray)->ElementFilter:
    """Get element filter by region

    Parameters
    ----------
    mesh : Mesh
        _description_
    dimensionality : int
        dimension of the computational support
    submethod : str
        name of the submethod to be used for defining the region
    regionId : int
        id number of the region
    numberOfRegions : int
        total number of regions expected
    concatDataValues : np.ndarray
        elements local value (only one value per element)

    Returns
    -------
    ElementFilter
        element filter by region
    """
    elementsId=np.array(GetGlobalIndicesInRegion(submethod,regionId,numberOfRegions,concatDataValues))
    tagName="filterRegio_"+str(regionId)
    numElemsByType = {elem.elementType.name:(ElementsInfo[elem.elementType].dimension,elem.GetNumberOfElements()) for elem in mesh.elements.values()}
    globalDimensionalityMapper = np.arange(len(concatDataValues))

    elemIdsStart = 0
    for elemDim,elemNum in numElemsByType.values():
        if elemDim==dimensionality:
            elemIdsStart+=elemNum
        else:
            globalDimensionalityMapper[elemIdsStart:]+=elemNum

    if numberOfRegions==1:
        return ElementFilter(dimensionality=dimensionality)
    else:
        mesh.AddElementsToTag(globalDimensionalityMapper[elementsId], tagName)
        return ElementFilter(dimensionality=dimensionality,eTag=[tagName])

def GenerateConcatData(mesh:Mesh,dimensionality:int,dataValues:np.ndarray)->Tuple[np.ndarray,Dict,Dict]:
    """Concatenate values for mesh with mixed elements

    Parameters
    ----------
    mesh : Mesh
        mesh
    dimensionality : int
        dimension of the computational support
    dataValues : np.ndarray
        elements local value (only one value per element)

    Returns
    -------
    Tuple[np.ndarray,Dict,Dict]
        concatenated values, number of each type of element by type of element, local element indices by type of element
    """
    filterDim=ElementFilter(dimensionality)
    idxByName=dict()
    dataValuesByOffset=dict()
    globaloffset = mesh.ComputeGlobalOffset()
    for selection in filterDim(mesh):
        dataValuesByOffset[globaloffset[selection.elementType] ]=(dataValues[selection.elementType],selection.elementType,selection.elements.GetNumberOfElements())
        idxByName[selection.elementType]=list(selection.indices)

    values=np.array([])
    numElelmByType=OrderedDict()
    for globalOffset in sorted(dataValuesByOffset.keys()):
        dataValues,elemName,nbrElem=dataValuesByOffset[globalOffset]
        values=np.concatenate((values,dataValues))
        numElelmByType[elemName]=nbrElem

    return values,numElelmByType,idxByName

def Regionalization(criteria:Type, submethod:str, params:Dict)->Type:
    """Decorator of criteria for regionalization feature

    Parameters
    ----------
    criteria : Type
        semi-infinite criteria class
    submethod : str
        name of the submethod to be used for defining the region
    params : Dict
        parameters specific to the Regionalization method

    Returns
    -------
    RegionalizedCriteria
        semi-infinite criteria instance decorated by the regionalization
    """
    class RegionalizedCriteria(criteria):
        def __init__(self,other:Optional[RegionalizedCriteria]=None):
            """Constructor of criterion

            Parameters
            ----------
            other : Optional[RegionalizedCriteria], optional
                another instance of this very criteria to copy attributes from, by default None
            """
            super(RegionalizedCriteria, self).__init__(other)
            self.method="Regionalization"
            self.submethod=submethod
            self.numberOfRegions=params["numberOfRegions"]
            self.regionID=0

            if other is not None:
                self.numberOfRegions=other.numberOfRegions
                self.regionID=other.regionID

        @property
        def numberOfRegions(self)->int:
            """getter of number of regions 

            Returns
            -------
            int
                number of regions 
            """
            return self._numberOfRegions

        @numberOfRegions.setter
        def numberOfRegions(self, numberOfRegions:int):
            """Setter of number of regions 

            Parameters
            ----------
            numberOfRegions : int
                number of regions 
            """
            assert numberOfRegions>=1,"The number of regions should be greater than 1"
            self._numberOfRegions = numberOfRegions

        def SetRegionID(self,regionID:int):
            """Setter of region ID

            Parameters
            ----------
            regionID : int
                id number of the region
            """
            self.regionID=regionID

        def ComputeComputationalElementFilter(self)->ElementFilter:
            """Compute element filter dedicated to a specific region

            Returns
            -------
            ElementFilter
                computationalElementFilter, filter associated to a specific region
            """
            assert self.localValueSupport==FN.IPField,"localValueSupport "+str(self.localValueSupport)+" not handled"
            computationalElementFilter=self.GetElementFilterByRegion()
            return computationalElementFilter

        def GetElementFilterByRegion(self)->ElementFilter:
            """Retrieve element filter by region

            Returns
            -------
            ElementFilter
                filter associated to a specific region for the elements
            """
            mesh=self.problem.cleanMesh
            ipMaxValuedata={element:np.amax(ipValues,axis=1) for element,ipValues in self.localValues.data.items()}
            concatDataValues,_,_=GenerateConcatData(mesh,self.dimensionalitySupport,ipMaxValuedata)
            filterRegion=GetElemFilterRegion(mesh,self.dimensionalitySupport,self.submethod,self.regionID,self.numberOfRegions,concatDataValues)
            return filterRegion

    return RegionalizedCriteria()

class TestRegionsGeneration():
    def __init__(self):
        #Mesh with mixed 3D element
        from Muscat.IO.MeshReader import ReadMesh as ReadMesh
        from OpenPisco.TestData import GetTestDataPath
        FileName = GetTestDataPath()+"mixedTetraHexa.mesh"
        mesh = ReadMesh(FileName,ReadRefsAsField=True)
        mesh.nodesTags.DeleteTags(mesh.nodesTags.keys())
        mesh.DeleteElemTags(mesh.GetNamesOfElementTags())

        #Check whether the test is bothered with a surfacic element in the mesh
        tris = mesh.GetElementsOfType(ED.Triangle_3)
        tris.AddNewElement([0,5,2],0)
        self.mesh=mesh

        hexValue=np.array([1,9,32,4.8,57,27,900,109])
        tetValue=np.array([0,12,37,3,9,30,10,54,8,90,110,86,6,10,21,30,69,60,64,28,14,9,145,956])
        self.data={ED.Hexahedron_8:hexValue,ED.Tetrahedron_4:tetValue}
        self.numberOfRegionsToTest=range(1,33)
        self.filterdimension=3
        self.submethods=["Interlacing","NearValues"]

    def test_GlobalIndicesAllRegions(self):
        mesh,data,filterdimension=self.mesh,self.data,self.filterdimension
        concatDataValues,_,_=GenerateConcatData(mesh,filterdimension,data)

        for submethod in self.submethods:
            for numberOfRegions in self.numberOfRegionsToTest:
                union_G=[]
                for regionId in range(numberOfRegions):
                    elementsId=GetGlobalIndicesInRegion(submethod,regionId,numberOfRegions,concatDataValues)
                    union_G+=elementsId

                numElems=sum([len(value) for value in data.values()])
                assert sorted(union_G)==list(range(numElems))

    def test_LocalIndicesAllRegions(self):
        mesh,data,filterdimension=self.mesh,self.data,self.filterdimension
        concatDataValues,numElelmByType,idxByName=GenerateConcatData(mesh,filterdimension,data)

        for submethod in self.submethods:
            for numberOfRegions in self.numberOfRegionsToTest:
                union_L={k:[] for k in numElelmByType.keys()}
                for regionId in range(numberOfRegions):
                    localIdsByElem=GetLocalElemIndicesInRegion(submethod,regionId,numberOfRegions,concatDataValues,numElelmByType)
                    union_L={key:np.sort(np.concatenate([d[key] for d in [union_L,localIdsByElem]])) for key in union_L.keys()}
                union_L={key:list(value.astype(int)) for key,value in union_L.items()}
                assert union_L==idxByName

    def test_filtersAllRegion(self):
        mesh, data, filterdimension = self.mesh, self.data, self.filterdimension
        concatDataValues, _, _ = GenerateConcatData(mesh, filterdimension, data)
        for submethod in self.submethods:
            for numberOfRegions in self.numberOfRegionsToTest:
                myUnionFilter = UnionFilter()
                for regionId in range(numberOfRegions):
                    filterRegion=GetElemFilterRegion(mesh, filterdimension, submethod, regionId, numberOfRegions, concatDataValues)
                    myUnionFilter.filters.append(filterRegion)

                for selection in myUnionFilter(mesh):
                    idxRef=list(range(mesh.GetElementsOfType(selection.elementType).GetNumberOfElements()))
                    np.testing.assert_equal(idxRef, selection.indices)



def CheckIntegrity():
    myTest=TestRegionsGeneration()
    myTest.test_GlobalIndicesAllRegions()
    myTest.test_LocalIndicesAllRegions()
    myTest.test_filtersAllRegion()
    return "ok"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
