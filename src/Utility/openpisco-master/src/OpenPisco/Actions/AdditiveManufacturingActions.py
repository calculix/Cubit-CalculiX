# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations
from typing import Dict

import numpy as np
import math

import  Muscat.MeshContainers.ElementsDescription as ED
import Muscat.Helpers.ParserHelper as PH
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter as EF
from Muscat.Types import MuscatFloat
from Muscat.Helpers.Logger import Debug
from Muscat.MeshTools.MeshFieldOperations import GetFieldTransferOp
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.FETools import PrepareFEComputation
from Muscat.IO.UniversalReader import ReadMesh
from Muscat.MeshContainers.Filters.FilterObjects import NodeFilter

import OpenPisco.PhysicalSolvers.AMThermalDilatationSolver as AMThermalDilat
from OpenPisco.Actions.ActionFactory import RegisterClass
from OpenPisco.Actions.ActionBase import ActionBase
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL
from OpenPisco.CLApp.OpenPiscoCL import MainApp

class OverHangDetectionAction(ActionBase):
    """
    Detection of overhang surfaces. 
    Modify levelset support inplace to add a "overhangSurface" (default name) element tag containing overhanged surfaces.

    Parameters
    ----------
        ls: levelset object
        angleDetection:"45*|float"
        direction="[0 0 1]*|floats"
        surfaceTags:"everyelement*|strings"
        name:"overhangSurface*|string"
        invertOrientation="False*|bool"

    Example in XML:
    <Action type="OverHangDetection" ls="1" angleDetection="45" direction="0 0 1"/>

    Example in python:
    action = OverHangDetectionAction()
    action.Apply(app=None, ops={"ls":levelSet,"angleDetection":45,"direction":[0 0 1]})
    """
    def __init__(self):
        super(OverHangDetectionAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        support = ls.support

        tagnames =  PH.ReadStrings( ops.get('surfaceTags' ,'everyelement') )

        if len(tagnames)==1 and tagnames[0] == 'everyelement':
            fil = EF(dimensionality=support.GetElementsDimensionality()-1)(support)
        else:
            for tagname in tagnames:
                if tagname not in support.GetNamesOfElementTags():
                    print("surfaceTag provided ("+tagname+") for overhang surfaces does not exist in the mesh")
                    return RETURN_FAIL
            fil = EF(dimensionality=support.GetElementsDimensionality()-1,eTag=tagnames)(support)
        nodes = support.nodes
        if support.GetElementsDimensionality() == 2:
            direction = PH.ReadFloats(ops.get('direction' ,np.array([0.,1.], dtype=MuscatFloat)))
        else:
            direction = PH.ReadFloats(ops.get('direction' ,np.array([0.,0.,1.], dtype=MuscatFloat)))
        direction /= np.linalg.norm(direction)
        angleDetection = PH.ReadFloat(ops.get('angleDetection' ,45.))
        invertOrientation = {True:-1.,False:1.}[PH.ReadBool(ops.get('invertOrientation' ,False))]

        if angleDetection < 0. or angleDetection > 90.:
            print("OverHangDetection angle value should be comprised between 0 and 90.")
            return RETURN_FAIL
        nElements = 0
        cosAngle = np.cos(np.pi*(angleDetection)/180 )
        for name, data, ids in fil:
            overHangIds = []
            nElements+=len(ids)

            if ED.dimensionality[name] == 1:
                p0 = nodes[data.connectivity[ids,0],:]
                p1 = nodes[data.connectivity[ids,1],:]
                delta = p1-p0
                normals = np.hstack((delta[:,[1]],-delta[:,[0]]))
                normals /= invertOrientation*np.linalg.norm(normals,axis=1)[:,None]
                overHangIds = np.nonzero( (- normals.dot(direction[:,None])- cosAngle > 0)[:,0])[0]
            else:
                for i in ids:
                    conn = data.connectivity[i,:]
                    p0 = nodes[conn[0],:]
                    p1 = nodes[conn[1],:]
                    p2 = nodes[conn[2],:]
                    normal = np.cross(p1-p0,p2-p0)
                    normal = invertOrientation*normal/np.linalg.norm(normal)

                    if -np.dot(normal,direction) >= cosAngle:
                        overHangIds.append(i)

            data.GetTag(ops.get('name' ,"overhangSurface")).SetIds(overHangIds)

        if nElements==0:
            print("OverHangDetection : no elements in tag "+str(tagname)+". Unable to detect")
            return RETURN_FAIL

        res  = " *** OverHangDetection *** \n"
        res += " AngleDetection                        : {} \n".format(angleDetection)
        res += " Direction                             : {} \n".format(direction)
        res += " Surface tags                          : {} \n".format(tagnames)
        res += " Number of elements in tags            : {} \n".format(nElements)
        res += " Number of elements in overhang        : {} \n".format(len(overHangIds))

        Debug(res)

        return RETURN_SUCCESS

RegisterClass("OverHangDetection", OverHangDetectionAction)

class BackingSurfaceDetectionAction(ActionBase):
    """
    Detection of backing surfaces:
    Modify levelset support inplace to add a "backingSurface" (default name) element tag containing overhanged surfaces.

    Parameters
    ----------
        ls: levelset object
        valueDetection:" 0.2*|floats"
        direction="[0 0 1]*|floats"
        surfaceTags:everyelement*|strings
        invertOrientation=False*|bool
        name:"backingSurface*|string"

    Example in XML:
    <Action type="BackingDetection" surfaceTag="InsideTube" ls="1" />

    Example in python:
    action = BackingSurfaceDetectionAction()
    action.Apply(app=None, ops={"ls":levelSet,"surfaceTag":"InsideTube"})
    """
    def __init__(self):
        super(BackingSurfaceDetectionAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        support = ls.support

        tagnames = PH.ReadStrings(ops.get('surfaceTags' ,'everyelement'))
        if len(tagnames)==1 and tagnames[0] == 'everyelement':
            fil = EF(dimensionality=support.GetElementsDimensionality()-1)(support)
        else:
            for tagname in tagnames:
                if tagname not in support.GetNamesOfElementTags():
                    print("surfaceTag provided ("+tagname+") for backing surfaces does not exist in the mesh")
                    return RETURN_FAIL
            fil = EF(dimensionality=support.GetElementsDimensionality()-1,eTag=tagnames)(support)
        nodes = support.nodes
        if support.GetElementsDimensionality() == 2:
            direction = PH.ReadFloats(ops.get('direction' ,np.array([0.,1.], dtype=MuscatFloat)))
        else:
            direction = PH.ReadFloats(ops.get('direction' ,np.array([0.,0.,1.], dtype=MuscatFloat)))
        valueDetection = PH.ReadFloat(ops.get('valueDetection' ,0.2))
        invertOrientation = {True:-1.,False:1.}[PH.ReadBool(ops.get('invertOrientation' ,False))]
        nElements = 0
        for name, data, ids in fil:
            Ids = []
            nElements+=len(ids)

            if ED.dimensionality[name] == 1:
                p0 = nodes[data.connectivity[ids,0],:]
                p1 = nodes[data.connectivity[ids,1],:]
                delta = p1-p0
                normals = np.hstack((delta[:,[1]],-delta[:,[0]]))
                normals /= invertOrientation*np.linalg.norm(normals,axis=1)[:,None]
                Ids = np.nonzero( (normals.dot(direction[:,None])- valueDetection > 0)[:,0])[0]
            else:
                for i in ids:
                    conn = data.connectivity[i,:]

                    p0 = nodes[conn[0],:]
                    p1 = nodes[conn[1],:]
                    p2 = nodes[conn[2],:]
                    normal = np.cross(p1-p0,p2-p0)
                    normal = invertOrientation*normal/np.linalg.norm(normal)
                    if np.dot(normal,direction) > valueDetection:
                        Ids.append(i)

            data.GetTag(ops.get('name' ,"backingSurface")).SetIds(Ids)
        
        if nElements==0:
            print("OverHangDetection : no elements in tag "+str(tagname)+". Unable to detect")
            return RETURN_FAIL

        res  = " *** BackingSurfaceDetection *** \n"
        res += " ValueDetection                         : {} \n".format(valueDetection)
        res += " Direction                              : {} \n".format(direction)
        res += " Surface tags                           : {} \n".format(tagnames)
        res += " Number of elements in tags             : {} \n".format(nElements)
        res += " Number of elements in backing surface  : {} \n".format(len(Ids))

        Debug(res)

        return RETURN_SUCCESS

RegisterClass("BackingDetection", BackingSurfaceDetectionAction)

class GenerateEquivalentForceFieldAction(ActionBase):
    """
    Generate thermoelastic equivalent load based on a additive manufacturing process. 
    It runs a thermoelastic computation and compute the reaction forces from the displacement as a postprocessing.

    Parameters
    ----------
        ls: levelset object
        useProblem: problem instance
        forceFieldName:string
        eTag:string

    Example in XML:
    <Action type="GenerateEquivalentForceField" useProblem="1" forceFieldName="MyForceField" eTag="MyZone"/>

    Example in python:
    action = GenerateEquivalentForceFieldAction()
    action.Apply(app=None, ops={"useProblem":myThermoMecaProblem,"forceFieldName":"MyForceField","eTag":"MyZone"})

    """
    def __init__(self):
        super(GenerateEquivalentForceFieldAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        support = ls.support
        AMProcess = ops['problem']
        forceFieldName = PH.ReadString(ops['forceFieldName'])
        eTag = PH.ReadString(ops['eTag'])

        AMProcess.SolveByLevelSet(ls)
        aggregatedDispTh,mecaProperties,computationLevelSetTh=AMThermalDilat.ExtractThermoMecaInformations(thermoMecaProblem=AMProcess)
        computationThermalSupport=computationLevelSetTh.support
        aggregatedDispThProjected=AMThermalDilat.ComputeNodalFieldProjection(nodalField=aggregatedDispTh,originalSupport=computationThermalSupport,targetSupport=support)
        AMProcess.nodeFields["aggregatedDisp"] = aggregatedDispThProjected

        boundEtag=eTag
        phyProblem,_=AMThermalDilat.ComputeReactionDisplacement(levelSet=ls,
                                                        mecaProperties=mecaProperties,
                                                        aggregatedDisp=aggregatedDispThProjected,
                                                        boundEtag=boundEtag)

        equivalentForces=AMThermalDilat.ComputeReactionForces(levelSet=ls,problem=phyProblem)
        AMProcess.nodeFields[forceFieldName] = equivalentForces

        return RETURN_SUCCESS

RegisterClass("GenerateEquivalentForceField", GenerateEquivalentForceFieldAction)

class PushForceFieldToPhysicalProblemAction(ActionBase):
    """
    Add force field from additive manufacturing process to physical problem 

    Parameters
    ----------
        ls: levelset object
        useProblem: problem instance
        forceFieldName:string

    Example in XML:
    <Action type="PushForceFieldToPhysicalProblem" useProblem="1" forceFieldName="MyForceField" eTag="MyZone"/>

    Example in python:
    action = PushForceFieldToPhysicalProblemAction()
    action.Apply(app=None, ops={"useProblem":myThermoMecaProblem,"forceFieldName":"MyForceField","eTag":"MyZone"})

    """
    def __init__(self):
        super(PushForceFieldToPhysicalProblemAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):

        AMProcess = app.physicalProblems[PH.ReadInt( ops['useAMProcess'] )]
        physicalProblem = app.physicalProblems[PH.ReadInt( ops['usePhyProblem'])]
        support = ops['ls'].support 

        forceFieldName = ops['forceFieldName']
        data = AMProcess.nodeFields[forceFieldName]
        
        if support.GetNumberOfNodes() != data.shape[0]:
            data = data[support.originalIDNodes]
        
        flattenedForceField=np.concatenate([data[:,0],data[:,1],data[:,2]])
        physicalProblem.PushForceFieldToLinearProblem(forceFieldName,flattenedForceField)

        return RETURN_SUCCESS

RegisterClass("PushForceFieldToPhysicalProblem", PushForceFieldToPhysicalProblemAction)

class RotateDomainAlongAxisAction(ActionBase):
    """
    Rotate mesh along specified axis 

    Parameters
    ----------
        ls: levelset object
        angle:float
        axis:string ("x","y","z")

    Example in XML:
    <Action type="RotateDomain" ls="1" angle="3.14" axis="x"/>

    Example in python:
    action = RotateDomainAlongAxisAction()
    action.Apply(app=None, ops={"ls":levelSetObject,"angle":3.14,"axis":"x"})

    """
    def __init__(self):
        super(RotateDomainAlongAxisAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        angle = ops['angle']
        axis = ops['axis']
        assert axis.lower in ["x","y","z"],"Axe %s not covered (x,y,z allowed)"
        if axis.lower()=="y":
            rotMat = np.array((
                        [math.cos(angle),0,math.sin(angle)],
                        [0.0,1.0,0.0],
                        [-math.sin(angle),0,math.cos(angle)], ))
        elif axis.lower()=="x":
            rotMat = np.array((
                        [1,0,0],
                        [0.0,math.cos(angle),-math.sin(angle)],
                        [0,math.sin(angle),math.cos(angle)],))
        elif axis.lower()=="z":
            rotMat = np.array((
                        [math.cos(angle),-math.sin(angle),0],
                        [math.sin(angle),math.cos(angle),0],
                        [0,0,1], ))
        mesh = ls.support
        nodes = mesh.nodes.transpose(1,0).copy()
        mesh.nodes = rotMat.dot(nodes).transpose(1,0).copy()
        ls.support = mesh
        return RETURN_SUCCESS

RegisterClass("RotateDomain", RotateDomainAlongAxisAction)

class AddRHSToProblemFromFileAction(ActionBase):
    """
    Push rhs field from file

    Parameters
    ----------
        ls: levelset object
        usePhyProblem:problem object
        filename:string, path to meshFile
        fieldName:string, right hand side nodal field to retrieve
        eTags:strings, filter field on tag

    Example in XML:
    <Action type="AddRHSToProblemFromFile" ls="1" usePhyProblem="1" fieldName="MyField" filename="MyFile" />

    Example in python:
    action = AddRHSToProblemFromFileAction()
    action.Apply(app=None, ops={"ls":levelSetObject,"usePhyProblem":problemInstance,"fieldName":"MyField","filename":"MyFile"})

    """
    def __init__(self):
        super(AddRHSToProblemFromFileAction,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
        originalSupport=ReadMesh(ops['filename'])
        physicalProblem = app.physicalProblems[PH.ReadInt( ops['usePhyProblem'])]
        physicalProblem.ComputeDomainToTreatLs(levelSet=ops['ls'])
        targetSupport = physicalProblem.cleanMesh
        fieldName = ops['fieldName']
        nodalField = originalSupport.nodeFields[fieldName]
        nodalField = np.vstack(np.hsplit(nodalField,originalSupport.GetNumberOfNodes()))
        space, numberings, _, _ = PrepareFEComputation(originalSupport,numberOfComponents=3)
        tempfield = FEField("",mesh=originalSupport,space=space,numbering=numberings[0])
        op, _, _ = GetFieldTransferOp(tempfield,targetSupport.nodes,method="Interp/Clamp" )
        projectedNodalField = op.dot(nodalField)

        inputEtags = ops.get('eTags', None)
        if inputEtags is not None:
            eTags = PH.ReadStrings(ops['eTags'])
            nodesToKeep = NodeFilter(eTag=eTags).GetNodesIndices(targetSupport)
            if not len(nodesToKeep):
                print("Warning AddRHSToProblemFromFileAction: eTags not found in the mesh")
            projNodalFieldFiltered = np.zeros_like(projectedNodalField)
            projNodalFieldFiltered[nodesToKeep] = projectedNodalField[nodesToKeep]
            projectedNodalField = projNodalFieldFiltered

        flattenedForceField=np.concatenate([projectedNodalField[:,0],projectedNodalField[:,1],projectedNodalField[:,2]])
        physicalProblem.PushForceFieldToLinearProblem(fieldName,flattenedForceField)
        return RETURN_SUCCESS

RegisterClass("AddRHSToProblemFromFile", AddRHSToProblemFromFileAction)

def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity())
