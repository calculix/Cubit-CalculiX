# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from collections import defaultdict
import copy
import numpy as np

from Muscat.Types import MuscatFloat
import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.Logger import Error, Info
from Muscat.FE.SymPhysics import MechPhysics

from Muscat.FE.KR.KRBlock import KRBlockVector
from Muscat.FE.KR.KRFromDistribution import KRFromDistribution
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter, NodeFilter

from OpenPisco.PhysicalSolvers.UnstructuredFEAThermalSolver import CreateUnstructuredFEAThermalProblem
from OpenPisco.PhysicalSolvers.UnstructuredFEAMecaSolver import UnstructuredFEAMecaProblem, UnstructuredFEAMecaThermalStrainProblem, MecaThermalStrain
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import  RegisterClass
import OpenPisco.TopoZones as TZ
import OpenPisco.PhysicalSolvers.FieldsNames as FN


def CreateUnstructuredFEAGenericProblem(ops):
    """
    Constructor for a UnstructuredFEAGenericProblem

    Parameters
    ----------
    ops
        A dictionary-like object


    Returns
    -------
    UnstructuredFEAGenericProblem
        A populated `UnstructuredFEAGenericProblem` instance

    Notes
    -----
    The `ops` can be generated from a string like this::

        a = <XXXXXXXX id="X" type="static_elastic"  p="*1|2"  >

    not available#   <UseTemplateFile name="" />

    0.*<Material    eTag="*everyelement|eTag" young="1" poisson="0.3" density="1" />
    0.*<Dirichlet   eTag="eTag" dofs="[012]" value="[float]" />
    0.*<Dirichlet   eTag="eTag" aVoir[nTag=""] u1="0" />
    0.*<Acceleration     eTag="*everyelement|eTag" g="3*float" />
    0.*<Centrifugal eTag="*everyelement|eTag" point="3*float" axis="3*float" angularSpeed="omega" />
    0.*<Force [eTag="ET9"|nTag=""] value="3*float" />
    0.*<Force  eTag="eTag_Name_3" phi="4.5" theta="5.6" mag="2" >
    not available#   <TotalForce  eTag="eTag_Name_3" phi="4.5" theta="5.6" mag="2" >
    0.*<Pressure  eTag="2D_eTag"  value="Toto" />

    0.*<LoadCase id="1" >
        0.*<Gravity>
        0.*<Centrifugal>
        0.*<Force>
        0.*<Pressure>
    </LoadCase>

</XXXXXXXX>

    """
    dictProperties = copy.deepcopy(ops)
    spaceDim = PH.ReadInt(ops.get('dimensionality',3))

    if 'dimensionality' in ops:
        del(ops['dimensionality'])

    tagsToKeep = []
    def readEtag(data):
        if "eTag" in data:
            tagName = PH.ReadString( data["eTag"])
            ef = ElementFilter(eTag = tagName )
            tagsToKeep.append(tagName)
        else:
            ef = ElementFilter()

        if "dim" in child:
            ef.dimensionality = PH.ReadInt(data["dim"])

        return ef

    def readNtag(data):
        if "nTag" in data:
            tagName = PH.ReadString( data["nTag"])
            nf = NodeFilter(nTag =tagName)
        else:
            nf = NodeFilter()

        return nf

    def ReadLoading(res, LoadingName,tag,child,mecaPhysics):
        if tag == "Acceleration":
            vals = PH.ReadFloats(child["g"] )
            ef = readEtag(child)
            mecaPhysics.AddLFormulation( ef, mecaPhysics.GetAccelerationFormulation(vals,density="density"))

        elif tag == "Centrifugal":
            ef = readEtag(child)
            point = PH.ReadFloats(child["point"])
            axis = PH.ReadVectorXYZ(child["axis"],True)
            angularSpeed = PH.ReadFloat(child["angularSpeed"])

            def CentrifugalForce(x):
                proj = np.dot( (x-point), axis)
                centrifugalDirection = (x-point)-axis*proj
                return centrifugalDirection*angularSpeed**2

            def CFX(x):
                CentrifugalForce(x)[0]

            def CFY(x):
                CentrifugalForce(x)[1]

            def CFZ(x):
                CentrifugalForce(x)[2]


            direc = [CFX,CFY,CFZ ][0:spaceDim]
            directionNames = [LoadingName+x for x in ["CFX","CFY","CFZ"][0:spaceDim] ]

            for name,func in zip(directionNames,direc):
                res.propsFields[name].append( (ef,func) )

            wf = mecaPhysics.GetAccelerationFormulation(directionNames,density="density")

            mecaPhysics.AddLFormulation( ef, wf )

        elif tag == "Force":
            if "phi" in child and "theta" in child:
                fx,fy,fz = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"],child.get("mag",1.)],False )
            elif "dir" in child :
                if spaceDim == 2:
                    fx,fy = PH.ReadVectorXY(child["dir"],True)
                    fz = 0
                else:
                    fx,fy,fz = PH.ReadVectorXYZ(child["dir"],True)
            else:
                message = "I need a direction (dir) or Euler angles (phi, theta)"
                Error(message)
                raise ValueError(message)

            value = child.get("value","1.")
            try:
                val = PH.ReadFloat(value )
            except TypeError:
                val = PH.ReadString(value )
            if "eTag" in child:
                ef = readEtag(child)
                mecaPhysics.AddLFormulation( ef, mecaPhysics.GetForceFormulation([fx,fy,fz][0:spaceDim],val)  )
            elif "nTag" in child:
                nf = readNtag(child)
                mecaPhysics.AddToRHSTerm(nf,[fx*val,fy*val,fz*val][0:spaceDim] )
            else:
                raise ValueError("Need a eTag or a nTag")

        elif tag == "Pressure":
            val = PH.ReadFloat(child["value"] )
            ef = readEtag(child)
            mecaPhysics.AddLFormulation( ef, mecaPhysics.GetPressureFormulation(val))

        else:
            message = "This kind of boundary is not supported : " + str(tag)
            Error(message)
            raise TypeError(message)


    assert ops["type"] in ["static_elastic","static_thermoelastic","thermal"],"Only static_elastic, static_thermoelastic, thermal types are allowed: '{}' ".format(ops["type"])

    P = PH.ReadInt(ops.get("p",1))
    ops.pop("p",None)

    linearSolverName = PH.ReadString(ops.get("linearSolverName","EigenLU"))
    ops.pop("linearSolverName",None)

    if ops["type"] == "static_elastic":
        res = UnstructuredFEAMecaProblem(dim=spaceDim)
        mecaPhysics = MechPhysics(dim=spaceDim)
        res.internalSolveur.physics.append(mecaPhysics)
        res.SetLinearSolver(linearSolverName)
        mecaPhysics.SetSpaceToLagrange(P=P)
        loadingCases = {}
        dirichletNodeFilters = []
        neumannNodeFilters = []

        for tag,child in ops["children"]:
            if tag == "Temperature":
                if child["type"]=="radial":
                    fileName = child["useFile"]
                    temperature = np.loadtxt(fileName,converters=lambda s: s.replace(b',', b'.'))
                    radius = np.arange(PH.ReadInt(child["xstart"]),PH.ReadInt(child["xend"]))
                    res.inputTemperature = np.vstack((radius,temperature))
                elif child["type"]=="xyz_coordinates":
                    fileName = child["useFile"]
                    data  = np.loadtxt(fileName, skiprows=1)
                    res.inputTemperature = data
                else:
                    raise ValueError("temperature type should not be "+str(child["type"]))
                continue

            if tag == "Material":
                ef = readEtag(child)
                for prop in ["young","poisson","density"]:
                    if prop in child:
                        val = PH.ReadFloat(child[prop])
                        res.propsFields[prop].append( (ef,val))
                        if prop == "poisson":
                            mecaPhysics.SetPoisson(PH.ReadFloat(child[prop]))
                            Info(f"For the moment poisson must be a constant value, using the last defined value {mecaPhysics.coeffs['poisson']}")
                            res.propsFields["poisson"] = [(ElementFilter(),mecaPhysics.coeffs["poisson"]) ]
                        elif prop=="young":
                            mecaPhysics.SetYoung(PH.ReadFloat(child[prop]))
                            res.internalSolveur.constants["young"] = val
                    if "children" in child:
                        for tag2,child2 in child["children"]:
                            if tag2=="YoungField" and child2["type"]=="tabular":
                                fileName = child2["useFile"]
                                data  = np.loadtxt(fileName, skiprows=1)
                                res.tabularYoungModulus = data
                                res.fYoung = res.youngModulusFunctionImplementation()
                                res.fTemperature = res.inputTemperatureFunctionImplementation()
                                res.propsFields["young"] =[(ef, lambda x: res.youngModulusFunction(x))]

                continue

            if tag == "Dirichlet":
                eTag = PH.ReadString(child["eTag"])
                dirichletNodeFilters.append(NodeFilter(eTag=eTag))
                ef = readEtag(child)
                vals = [0]*3
                dofs = []
                if "dofs" in child:
                    dofs = PH.ReadInts(child["dofs"])
                    val =  PH.ReadFloat(child.get("value") )
                    for i in dofs:
                        vals[i] = val
                else:
                    for i in range(3):
                        name = "u"+str(i)
                        if name in child:
                            val = child.get(name)
                            dofs.append(i)
                            vals[i] = val

                # so we have a numbering for the elements in the dirichlet
                mecaPhysics.AddLFormulation( eTag, None  )
                dirichlet = KRBlockVector().AddArg("u").On(eTag).To(offset=[vals[0],vals[1],vals[2]])
                for i in dofs:
                    if i not in (0, 1, 2):
                        raise ValueError(
                            f"Index for dirichlet dof should be in [0, 1, 2], not {i}"
                        )

                    getattr(dirichlet, f"BlockDirection{i}")()
                dirichlet.constraintDirections= "Global"
                res.internalSolveur.solver.constraints.AddConstraint(dirichlet)
                continue

            elif tag == "HeterogeneousDirichlet":
                dirichlet = KRFromDistribution()
                eTag = PH.ReadString(child["eTag"])
                dirichletNodeFilters.append(NodeFilter(eTag=eTag))
                dirichlet.AddArg("u").On(eTag)
                ef = readEtag(child)

                fieldFileToLoad = PH.ReadString(child["fieldDatafile"])
                imposedU = np.loadtxt(fieldFileToLoad)
                res.delayedInitialization = {tag:{"field":{"u":imposedU}, "condition":dirichlet}}
                continue

            elif tag == "LoadingNodeField":
                if "name" in child:
                    res.extraRHSVector = {child["name"]:0.}
                if "eTag" in child:
                    neumannNodeFilter = NodeFilter(eTag=PH.ReadString(child["eTag"]))
                    neumannNodeFilters.append(neumannNodeFilter)
                if "nTag" in child:
                    neumannNodeFilter = NodeFilter(nTag=PH.ReadString(child["nTag"]))
                    neumannNodeFilters.append(neumannNodeFilter)
                continue

            if tag == "LoadCase":
                loadCaseName =  PH.ReadString(child["id"])
                loadingCases[loadCaseName] = MechPhysics()
                for tagtag,childchild in child["children"]:
                    try:
                        if "eTag" in childchild:
                            neumannNodeFilter = NodeFilter(eTag=PH.ReadString(childchild["eTag"]))
                        else:
                            neumannNodeFilter = NodeFilter(nTag=PH.ReadString( childchild["nTag"]))
                        neumannNodeFilters.append(neumannNodeFilter)
                    except KeyError:
                        raise KeyError("Need a eTag or a nTag")
                    try:
                        ReadLoading(res,loadCaseName,tagtag,childchild,loadingCases[loadCaseName])
                    except TypeError:
                        raise TypeError("Error reading tag {} in loading case {}".format(tagtag,loadCaseName))
                continue

            try:
                try:
                    if "eTag" in child:
                        neumannNodeFilter = NodeFilter(eTag=PH.ReadString(child["eTag"]))
                    else:
                        neumannNodeFilter = NodeFilter(nTag=PH.ReadString( child["nTag"]))
                    neumannNodeFilters.append(neumannNodeFilter)
                except KeyError:
                    pass
                ReadLoading(res,"baseCase",tag,child,mecaPhysics)
            except TypeError as e:
                raise Exception("Can not treat key : {}".format(tag)) from e

        ops.pop("type")
        res.tagsToKeep.extend(tagsToKeep)
        res.tagsToKeep.extend(list(PH.ReadStrings(ops.get("tagsToKeep",""))))
        res.dirichletNodeFilters = dirichletNodeFilters
        res.neumannNodeFilters = neumannNodeFilters
        ops.pop("tagsToKeep",None)
        del ops["children"]
        res.dictProperties = dictProperties
        PH.ReadProperties(ops,ops.keys(), res, typeConversion=True)
        return res

    if ops["type"] == "static_thermoelastic" :
        res = UnstructuredFEAMecaThermalStrainProblem(dim=spaceDim)
        mecaPhysics = MecaThermalStrain(dim=spaceDim)
        res.internalSolveur.physics.append(mecaPhysics)
        res.SetLinearSolver(linearSolverName)
        mecaPhysics.SetSpaceToLagrange(P=P)
        loadingCases = {}

        for tag,child in ops["children"]:
            if tag == "ThermalSolver":
                if "problem" in child:
                    res.thermalSolver = child["problem"]
                else:
                    res.thermalSolver=CreateUnstructuredFEAThermalProblem(child)
                continue
            elif tag == "Temperature":
                if child["type"]=="radial":
                    fileName = child["useFile"]
                    temperature = np.loadtxt(fileName,converters=lambda s: s.replace(b',', b'.'))
                    radius = np.arange(PH.ReadInt(child["xstart"]),PH.ReadInt(child["xend"]))
                    res.inputTemperature = np.vstack((radius,temperature))
                elif child["type"]=="xyz_coordinates":
                    fileName = child["useFile"]
                    data  = np.loadtxt(fileName, skiprows=1)
                    res.inputTemperature = data
                else:
                    raise ValueError("temperature type should not be "+str(child["type"]))
                res.solveThermalProblem = False
                continue
            if tag == "Material":
                ef = readEtag(child)
                for prop in ["young","poisson","density","thermalExpansion","referenceTemperature"]:
                    if prop in child:
                        val = PH.ReadFloat(child[prop])
                        res.propsFields[prop].append((ef,val))
                        if prop == "poisson":
                            mecaPhysics.SetPoisson(PH.ReadFloat(child[prop]))
                            Info(f"For the moment poisson must be a constant value, using the last defined value {mecaPhysics.coeffs['poisson']}")
                            res.propsFields["poisson"] = [(ElementFilter(),mecaPhysics.coeffs["poisson"])]
                        elif prop=="young":
                            mecaPhysics.SetYoung(PH.ReadFloat(child[prop]))
                            res.internalSolveur.constants["young"] = val
                if "children" in child:
                    for tag2,child2 in child["children"]:
                        if tag2=="YoungField" and child2["type"]=="tabular":
                            fileName = child2["useFile"]
                            data  = np.loadtxt(fileName, skiprows=1)
                            res.tabularYoungModulus = data
                            res.fYoung = res.youngModulusFunctionImplementation()
                            res.fTemperature = res.inputTemperatureFunctionImplementation()
                            res.propsFields["young"] = [ (ef, lambda x: res.youngModulusFunction(x))]
                        if tag2=="ThermalExpansionField" and child2["type"]=="tabular":
                            fileName = child2["useFile"]
                            data  = np.loadtxt(fileName, skiprows=1)
                            res.tabularThermalExpansion = data
                            res.fThermalExpansion= res.thermalExpansionFunctionImplementation()
                            res.fTemperature = res.inputTemperatureFunctionImplementation()
                            res.propsFields["thermalExpansion"] =[ (ef, lambda x: res.thermalExpansionFunction(x))]
                continue

            if tag == "Dirichlet":
                eTag = PH.ReadString(child["eTag"])
                ef = readEtag(child)
                vals = [0]*3
                dofs = []
                if "dofs" in child:
                    dofs = PH.ReadInts(child["dofs"])
                    val =  PH.ReadFloat(child.get("value") )
                    for i in dofs:
                        vals[i] = val
                else:
                    for i in range(3):
                        name = "u"+str(i)
                        if name in child:
                            val = child.get(name)
                            dofs.append(i)
                            vals[i] = val
                # so we have a numbering for the elements in the dirichlet
                mecaPhysics.AddLFormulation(eTag, None)
                dirichlet = KRBlockVector().AddArg("u").On(eTag).To(offset=[vals[0],vals[1],vals[2]])
                for i in dofs:
                    if i not in (0, 1, 2):
                        raise ValueError(
                            f"Index for dirichlet dof should be in [0, 1, 2], not {i}"
                        )

                    getattr(dirichlet, f"BlockDirection{i}")()
                dirichlet.constraintDirections= "Global"
                res.internalSolveur.solver.constraints.AddConstraint(dirichlet)
                continue

            elif tag == "HeterogeneousDirichlet" :
                dirichlet = KRFromDistribution()
                eTag = PH.ReadString(child["eTag"])
                dirichlet.AddArg("u").On(eTag)
                ef = readEtag(child)
                fieldFileToLoad = PH.ReadString(child["fieldDatafile"])
                imposedU = np.loadtxt(fieldFileToLoad)
                res.delayedInitialization = {tag:{"field":{"u":imposedU}, "condition":dirichlet}}
                continue

            elif tag == "LoadingNodeField" :
                if  "name" in child:
                    res.extraRHSVector = {child["name"]:0.}
                continue

            if tag == "LoadCase":
                loadCaseName =  PH.ReadString(child["id"])
                loadingCases[loadCaseName] = MecaPhysics()
                for tagtag,childchild in child["children"]:
                    try:
                        ReadLoading(res,loadCaseName,tagtag,childchild,loadingCases[loadCaseName])
                    except TypeError:
                        raise TypeError("Error reading tag {} in loading case {}".format(tagtag,loadCaseName))
                continue

            try:
                ReadLoading(res,"baseCase",tag,child,mecaPhysics)
            except TypeError as e:
                raise Exception("Can not treat key : {}".format(tag)) from e

    if ops["type"] == "thermal":
        return CreateUnstructuredFEAThermalProblem(ops)

    ops.pop("type")
    res.tagsToKeep.extend(tagsToKeep)
    res.tagsToKeep.extend(list(PH.ReadStrings(ops.get("tagsToKeep",""))))
    ops.pop("tagsToKeep",None)
    del ops["children"]
    res.dictProperties = dictProperties
    PH.ReadProperties(ops,ops.keys(), res, typeConversion=True)
    return res

RegisterClass("Unstructured", None,CreateUnstructuredFEAGenericProblem)

import xml.etree.ElementTree as ET

from Muscat.MeshTools.MeshCreationTools import CreateCube, CreateSquare
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
from Muscat.FE.Fields.FieldTools import VectorToFEFieldsData
from OpenPisco.Unstructured.Levelset import LevelSet
from OpenPisco.CLApp.XmlToDic import XmlToDic


def CheckIntegrity_ReadXml(GUI=False):
    string2 = """<InternalSolver id="1" type="static_elastic"  p="1"  >

    <Material    eTag="3D" young="1" poisson="0.3" density="1" />
    <Dirichlet   eTag="eTag" dofs="0 1 2" value="0" />
    <Dirichlet   eTag="eTag" u1="0" />
    <Acceleration   g="0 0 -9" />
    <Centrifugal eTag="*everyelement|eTag" point="0 1 2" axis="3 4 5" angularSpeed="6" />
    <Force eTag="ET9" dir="3 4 5" value="1" />
    <Force  eTag="eTag_Name_3" phi="4.5" theta="5.6" mag="2" />
    <!--<TotalForce  eTag="eTag_Name_3" phi="4.5" theta="5.6" mag="2" />-->
    <Pressure  eTag="2D_eTag"  value="0.0" />

    <LoadCase id="1" >
        <!-- <Gravity />-->
        <Centrifugal eTag="*everyelement|eTag" point="0 1 2" axis="3 4 5" angularSpeed="6"/>
        <Force eTag="ET9" dir="3 4 5" value="1"/>
        <Pressure eTag="2D_eTag"  value="0.0" />
    </LoadCase >

    </InternalSolver>"""

    root2 = ET.fromstring(string2)
    obj = XmlToDic()
    _,data2 = obj.XmlToDic(root2)
    data2.pop("id",None)
    CreateUnstructuredFEAGenericProblem(data2)
    return "ok"


def CheckIntegrity_UnstructuredLinearElasticity(GUI=False):
    mesh = CreateCube(dimensions=[5,5,5], spacing=[1./4, 1./4, 1./4], origin=[0, 0, 0],ofTetras=True)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))

    for conformity in [True,False]:
        ls = LevelSet( support=mesh)
        ls.phi = np.empty(ls.support.GetNumberOfNodes())
        ls.phi.fill(-1)
        ls.conform = conformity
        if not conformity:
            ls.phi[tetra.connectivity[0:1,0:3]] *= -1

        xmlInputString = """<InternalSolver id="1" type="static_elastic"  p="1"  >
        <Material    young="1" poisson="0.3"/>
        <Dirichlet   eTag="X0" dofs="0 1 2" value="0" />
        <Force eTag="X1" dir="1 0 0" value="0.002" />
        </InternalSolver>"""

        root = ET.fromstring(xmlInputString)
        obj = XmlToDic()
        _,data = obj.XmlToDic(root)
        data.pop("id",None)
        phyProblem = CreateUnstructuredFEAGenericProblem(data)
        phyProblem.SolveByLevelSet(levelSet=ls)
        solution = phyProblem.GetNodalSolution()
        _ = phyProblem.GetNodalSolution(onCleanMesh=True)
        assert np.max(solution)<1e10
    return "ok"


def CheckIntegrity_LinearElasticity2D(GUI=False):
    mesh = CreateSquare(dimensions=[30,30], spacing=[1./4, 1./4], origin=[0, 0], ofTriangles=True )
    tria = mesh.GetElementsOfType(ED.Triangle_3)
    tria.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tria.GetNumberOfElements()) )


    ls = LevelSet( support=mesh)
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    ls.conform = True

    xmlInputString = """<InternalSolver id="1" type="static_elastic"  p="1"  >
    <Material    young="1" poisson="0.3"/>
    <Dirichlet   eTag="X0" dofs="0 1" value="0" />
    <Force nTag="x1y1" dir="0 1" value="0.002" />
    </InternalSolver>"""

    root = ET.fromstring(xmlInputString)
    obj = XmlToDic()
    _,data = obj.XmlToDic(root)
    data.pop("id",None)
    data["dimensionality"] = 2
    phyProblem = CreateUnstructuredFEAGenericProblem(data)
    phyProblem.SetAuxiliaryFieldGeneration(FN.strain,on=FN.IPField)
    phyProblem.SetAuxiliaryFieldGeneration(FN.von_mises,on=FN.IPField)
    phyProblem.SetAuxiliaryFieldGeneration(FN.potential_energy,on=FN.Nodes)
    phyProblem.SetAuxiliaryFieldGeneration(FN.tr_strain_,on=FN.Nodes)
    phyProblem.SolveByLevelSet(levelSet=ls)
    return "ok"

def CheckIntegrity_EnforceHeterogeneousDirichlet(GUI=False):
    tempPath = TemporaryDirectory.GetTempPath()
    mesh = CreateCube(dimensions=[3,3,3], spacing=[1./2, 1./2, 1./2], origin=[0, 0, 0],ofTetras=True)
    ls = LevelSet( support=mesh)
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    dummyField = mesh.nodes+1
    np.savetxt(fname=tempPath+"MyTrivialField.data", X=dummyField)

    xmlInputStringNew = f"""<InternalSolver id="1" type="static_elastic"  p="1"  >
    <Material    young="1" poisson="0.3"/>
    <HeterogeneousDirichlet   eTag="Skin" fieldDatafile="{tempPath}MyTrivialField.data"/>
    </InternalSolver>"""

    root = ET.fromstring(xmlInputStringNew)
    obj = XmlToDic()
    _,data = obj.XmlToDic(root)
    data.pop("id",None)
    phyProblem = CreateUnstructuredFEAGenericProblem(data)
    phyProblem.SolveByLevelSet(levelSet=ls)
    disp=phyProblem.GetNodalSolution()

    ids = NodeFilter(eTag=["Skin"]).GetNodesIndices(mesh)
    error = np.abs(disp[ids,:] - dummyField[ids,:])

    if np.linalg.norm(error/dummyField[ids,:]) > 1e-8:
        raise RuntimeError("Error in the CheckIntegrity ")
    return "ok"

def CheckIntegrity_ReactionForce(GUI=False):
    mesh = CreateCube(dimensions=[3,3,3],spacing=[1./2, 1./2, 1./2],origin=[0, 0, 0],ofTetras=True)
    ls = LevelSet( support=mesh)
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)

    dummyField = np.zeros_like(mesh.nodes)
    dummyField[mesh.nodes[:,0]>1-1e-5] = [1,1,1]
    tempPath = TemporaryDirectory.GetTempPath()
    np.savetxt(fname=tempPath+"MyTrivialField.data", X=dummyField)

    xmlInputString = f"""<InternalSolver id="1" type="static_elastic"  p="1"  >
    <Material    young="1" poisson="0.3"/>
    <HeterogeneousDirichlet   eTag="X0" fieldDatafile="{tempPath}MyTrivialField.data"/>
    </InternalSolver>"""

    root = ET.fromstring(xmlInputString)
    obj = XmlToDic()
    _,data = obj.XmlToDic(root)
    data.pop("id",None)
    phyProblem = CreateUnstructuredFEAGenericProblem(data)
    phyProblem.SolveByLevelSet(levelSet=ls)
    phyProblem.GetNodalSolution()

    k,f = phyProblem.internalSolveur.GetLinearProblem()

    fr2_in_internal_numbering = k.dot(phyProblem.internalSolveur.sol)-f

    # from f to internal numbering in the internalSolver format
    fr2_in_FEFiels_on_cleanMesh = []
    for i in range(3):
        fr2_in_FEFiels_on_cleanMesh.append(phyProblem.internalSolveur.unknownFields[i].copy())
        fr2_in_FEFiels_on_cleanMesh[-1].name = "f_"+str(i)

    VectorToFEFieldsData(fr2_in_internal_numbering, fr2_in_FEFiels_on_cleanMesh)

    # get f in the mesh
    fr2_in_mesh = np.zeros((mesh.GetNumberOfNodes(),3),dtype=MuscatFloat)
    for i in range(3):
        fr2_in_mesh[fr2_in_FEFiels_on_cleanMesh[i].mesh.originalIDNodes,i] = fr2_in_FEFiels_on_cleanMesh[i].GetPointRepresentation()

    u0 = phyProblem.internalSolveur.unknownFields[0].GetPointRepresentation()
    u1 = phyProblem.internalSolveur.unknownFields[1].GetPointRepresentation()
    u2 = phyProblem.internalSolveur.unknownFields[2].GetPointRepresentation()

    flattenDisp=np.concatenate([u0,u1,u2 ])
    fr = k.dot(flattenDisp) - f
    nbNodes = phyProblem.cleanMesh.GetNumberOfNodes()
    np.vstack([fr[0:nbNodes],fr[nbNodes:2*nbNodes],fr[2*nbNodes:3*nbNodes]]).T

    return "ok"

def CheckIntegrity_ExtraRHS(GUI=False):
    mesh = CreateCube(dimensions=[3,3,3],spacing=[1./2, 1./2, 1./2],origin=[0, 0, 0],ofTetras=True)
    ls = LevelSet( support=mesh)
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    xmlInputString = """<InternalSolver id="1" type="static_elastic"  p="1"  >
    <Material    young="1" poisson="0.3"/>
    <Dirichlet   eTag="X0" dofs="0 1 2" value="0" />
    </InternalSolver>"""
    root = ET.fromstring(xmlInputString)
    obj = XmlToDic()
    _,data = obj.XmlToDic(root)
    data.pop("id",None)
    phyProblem = CreateUnstructuredFEAGenericProblem(data)

    phyProblem.SolveByLevelSet(levelSet=ls)
    solution = phyProblem.GetNodalSolution()
    assert np.linalg.norm(solution)<1e-10, "Solution should be zero"

    fieldToPush = np.ones(ls.support.GetNumberOfNodes()*3)
    phyProblem.PushForceFieldToLinearProblem("unitaryField",fieldToPush)
    phyProblem.SolveByLevelSet(levelSet=ls)
    solution = phyProblem.GetNodalSolution()
    assert np.linalg.norm(solution)>0, "Solution should not be zero"

    return "ok"


def CheckIntegrity_Thermoelasticity(GUI=False):
    mesh = CreateCube(dimensions=[3,3,3],spacing=[1./2, 1./2, 1./2],origin=[0, 0, 0],ofTetras=True)
    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    xmlInputString = """<InternalSolver id="1" type="static_thermoelastic"  p="1"  >
    <Material    young="1" poisson="0.3" thermalExpansion="0.00003" referenceTemperature="3."/>
    <Dirichlet   eTag="X0" dofs="0 1 2" value="0" />
    <ThermalSolver type="thermal" >
       <Material  lambda="50." />
       <Dirichlet eTag="Skin" value="50.0"/>
        <Source value="1.0"/>
    </ThermalSolver>
    </InternalSolver>"""
    root = ET.fromstring(xmlInputString)
    obj = XmlToDic()
    _,data = obj.XmlToDic(root)
    data.pop("id",None)
    phyProblem = CreateUnstructuredFEAGenericProblem(data)
    phyProblem.SetAuxiliaryFieldGeneration(FN.potential_energy,on=FN.Nodes)
    phyProblem.SetAuxiliaryFieldGeneration(FN.von_mises,on=FN.Nodes)
    phyProblem.SetAuxiliaryFieldGeneration(FN.von_mises,on=FN.IPField)
    phyProblem.SetAuxiliaryFieldGeneration(FN.stress,on=FN.IPField)
    phyProblem.SolveByLevelSet(levelSet=ls)
    solution = phyProblem.GetNodalSolution()
    energy = phyProblem.GetAuxiliaryField(FN.potential_energy,on=FN.Nodes)
    vmises= phyProblem.GetAuxiliaryField(FN.von_mises,on=FN.Nodes)
    _ = phyProblem.thermalSolver.GetNodalSolution()

    from OpenPisco.ExternalTools.Aster.AsterInterface import SkipAsterTest
    skipTestPhase,message = SkipAsterTest()
    if skipTestPhase:
        return message

    from OpenPisco.PhysicalSolvers.AsterThermoElastic import CreateAsterThermoElasticProblem
    teststringField=u"""
    <GeneralAster type="static_thermoelastic" computeThermalSteadyStateSolution="True" >
        <Material  lambda="50." rho="7850.0" cp="1046.0" young="1.0" poisson="0.3" thermalExpansion="0.00003" referenceTemperature="3."/>
        <PrescribedTemperature eTag="Skin" value="50.0"/>
        <Source eTag="Inside3D" value="1.0"/>
        <Dirichlet eTag="X0" dofs="0 1 2" value="0.0"/>
    </GeneralAster>"""

    teststringField = PH.ApplyGlobalDictionary(teststringField)
    reader = XmlToDic()
    _, ops = reader.ReadFromString(teststringField)
    ops["dimensionality"]=3
    phyProblem=CreateAsterThermoElasticProblem(ops)
    phyProblem.SetAuxiliaryFieldGeneration(FN.von_mises,on=FN.Nodes)
    phyProblem.SetAuxiliaryFieldGeneration(FN.potential_energy,on=FN.Nodes)
    phyProblem.SolveByLevelSet(ls)
    displacementAster = phyProblem.GetNodalSolution()
    vmisesAster = phyProblem.GetAuxiliaryField(FN.von_mises,on=FN.Nodes)
    energyAster = phyProblem.GetAuxiliaryField(FN.potential_energy,on=FN.Nodes)
    assert np.max(displacementAster-solution) < 1e-6
    assert np.max(vmises- vmisesAster) < 1e-3
    assert np.max(energy- energyAster) < 1e-3

    return "ok"


def CheckIntegrity(GUI=False):
    totest = [
    CheckIntegrity_LinearElasticity2D,
    CheckIntegrity_Thermoelasticity,
    CheckIntegrity_ReadXml,
    CheckIntegrity_UnstructuredLinearElasticity,
    CheckIntegrity_EnforceHeterogeneousDirichlet,
    CheckIntegrity_ReactionForce,
    CheckIntegrity_ExtraRHS
              ]
    from Muscat.Helpers.CheckTools import RunListOfCheckIntegrities

    return RunListOfCheckIntegrities(totest)

if __name__ == '__main__':
    print(CheckIntegrity(GUI=True))
