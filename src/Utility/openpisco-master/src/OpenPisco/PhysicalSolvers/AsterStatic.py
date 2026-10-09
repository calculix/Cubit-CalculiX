# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np
import copy

from Muscat.Types import MuscatFloat
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter,NodeFilter
import Muscat.MeshContainers.ElementsDescription as ED
import Muscat.Helpers.ParserHelper as PH

import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
import OpenPisco.ExternalTools.Aster.AsterStaticWriter as AsterStaticWriter
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.AsterSolverBase import AsterSolverBase,BoundaryConditionDefinitionError
import OpenPisco.TopoZones as TZ
from OpenPisco.PhysicalSolvers.AsterAnalysisSolverFactory import RegisterAsterClass
from  OpenPisco.PhysicalSolvers.PhysicalSolversTools import QuadToLinElemNames

def CreateAsterStaticPhysicalProblem(ops):
    res = AsterStatic()
    return PrepareStaticInstance(res=res,ops=ops)

def PrepareStaticInstance(res,ops):
    dictProperties = copy.deepcopy(ops)
    del(ops['type'])
    dim = PH.ReadInt(ops.get('dimensionality',"3"))
    if 'dimensionality' in ops:
        del(ops['dimensionality'])

    if 'maxAllowedRelaxedResidual' in ops:
        maxAllowedRelaxedResidual = PH.ReadFloat(ops['maxAllowedRelaxedResidual'])
        res.maxAllowedRelaxedResidual = maxAllowedRelaxedResidual
        del(ops['maxAllowedRelaxedResidual'])

    if 'hasPreStress' in ops:
        hasPreStress = PH.ReadBool(ops['hasPreStress'])
        res.hasPreStress = hasPreStress
        del(ops['hasPreStress'])

    if 'p' in ops:
        order = PH.ReadInt(ops['p'])
        assert order==1 or order==2,"Invalid Order "+order+" . Possible choices are 1 (linear) and 2 (quadratic)."
        res.space="{}{}".format("LagrangeP",order)
        del(ops['p'])

    for tag,child in ops["children"]:
        if tag.lower() == "usetemplatefile":
            res.templateFileName = PH.ReadString(child['name'])

        elif tag.lower() == "material":
            material={"young":PH.ReadFloat(child["young"]) , "poisson":PH.ReadFloat(child["poisson"])}
            res.materials.append( [child.get("eTag","AllZones"),material])

        elif tag.lower() == "dirichlet":
            if "eTag" in child:
                z = child["eTag"]
            errorMessage = "Need a set of dofs (dofs) and a value (value) for dirichlet boundary condition"
            assert "dofs" in child and "value" in child,errorMessage
            dofs = PH.ReadInts(child["dofs"])
            val = PH.ReadFloat(child["value"])
            U=[]
            for i in range(dim):
                if i in dofs:
                    U.append(val)
                else:
                    U.append(None)
            res.dirichlet.append([z,U])

        elif tag.lower() == "loadcase":
            z = PH.ReadString(child["id"])
            res.neumann[z] = []
            res.forcenodale[z] = []
            res.problems.append(z)

            for tag2,child2 in child["children"]:
                if tag2.lower() =="force":
                    if "eTag" in child2:
                        z2 = child2["eTag"]
                        if "value" in child2 :
                            forceValues = PH.ReadFloats(child2["value"])
                        elif "phi" in child2 and "theta" in child2:
                            if "mag" in child2:
                                forceValues  = PH.ReadVectorPhiThetaMag([child2["phi"],child2["theta"],child2["mag"]])
                            else:
                                forceValues  = PH.ReadVectorPhiThetaMag([child2["phi"],child2["theta"]],True )
                    else:
                        raise BoundaryConditionDefinitionError("a vector (value) or Euler angles (phi and theta)")
                    res.neumann[z].append([z2,forceValues])

                elif "nTag" in child2:
                    z2 = child2["nTag"]
                    if "value" in child2 :
                        forceValues = PH.ReadFloats(child2["value"])
                    elif "phi" in child2 and "theta" in child2:
                        if "mag" in child2:
                            forceValues = PH.ReadVectorPhiThetaMag([child2["phi"],child2["theta"],child2["mag"]])
                        else:
                            forceValues = PH.ReadVectorPhiThetaMag([child2["phi"],child2["theta"]],True )
                    else:
                        raise BoundaryConditionDefinitionError("a vector (value) or Euler angles (phi and theta)")
                    res.forcenodale[z].append([z2,forceValues])
                else:
                    raise BoundaryConditionDefinitionError("eTag or nTag")
        elif tag.lower() == "force":
            res.neumann["1"] = []
            res.forcenodale["1"] = []
            res.problems.append("1")
            if "eTag" in child:
                z = child["eTag"]
                if "value" in child:
                    forceValues = PH.ReadFloats(child["value"])
                elif "phi" in child and "theta" in child:
                    if "mag" in child:
                        forceValues = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"],child["mag"]])
                    else:
                        forceValues = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"]],True )
                else:
                    raise BoundaryConditionDefinitionError("a vector (value) or Euler angles (phi and theta)")
                res.neumann["1"].append([z,forceValues])
            elif "nTag" in child:
                z = child["nTag"]
                if "value" in child:
                    forceValues = PH.ReadFloats(child["value"])
                elif "phi" in child and "theta" in child:
                    if "mag" in child:
                        forceValues = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"],child["mag"]])
                    else:
                        forceValues = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"]],True )
                else:
                    raise BoundaryConditionDefinitionError("a vector (value) or Euler angles (phi and theta)")
                res.forcenodale["1"].append([z,forceValues])
            else:
                raise BoundaryConditionDefinitionError("eTag or nTag")

        elif tag.lower() == "pressure":
            res.neumann["1"] = []
            res.problems.append("1")
            assert "eTag" in child,"Need a eTag or nTag for this boundary condition "
            z = child["eTag"]
            assert "value" in child,"Need a scalar (value )for this boundary condition "
            val = PH.ReadFloat(child["value"])
            res.neumann["1"].append([z,val])
        else:
            raise ValueError("This type of object is not supported " + (tag))

    del ops["children"]

    res.dictProperties = dictProperties
    res.SetSolverInterface()

    PH.ReadProperties(ops,ops.keys(),res)
    return res

def AssertAlmostEqualDict(data1,data2):
    assert set(list(data1.keys()))==set(list(data2.keys()))
    for key in data1.keys():
        np.testing.assert_almost_equal(data1[key],data2[key])

def GetMaterialFields(support,phi,materials,levelset,eVoid):
    youngField = {}
    poissonField = {}
    dimensionality = levelset.support.GetElementsDimensionality()
    for elementName,elems,_ in ElementFilter(dimensionality=dimensionality)(support):
        nbElements = elems.GetNumberOfElements()
        localYoung = np.zeros( (nbElements,1),dtype=MuscatFloat)
        localPoisson = np.zeros( (nbElements,1),dtype=MuscatFloat)

        for tagname,material in materials:
            ids = np.arange(elems.GetNumberOfElements()) if tagname == 'AllZones' else support.GetElementsInTag(tagname)
            young = material["young"]
            poisson = material["poisson"]
            localPoisson[ids] = poisson

            errorMessage = "Please provide a level set function to write material properties in non conformal setting"
            assert levelset.conform or phi is not None,errorMessage
            if levelset.conform:
                localYoung[ids] = young
            elif phi is not None:
                NumberOfNodesInGeoSupport = support.GetElementsOfType(QuadToLinElemNames[elementName]).GetNumberOfNodesPerElement()
                elemconn = elems.connectivity[:,0:NumberOfNodesInGeoSupport]
                chi = np.zeros_like(phi[elemconn[ids,:]])
                chi[phi[elemconn[ids,:]] <=0] = 1.
                fieldatnodes = chi*young + (1-chi)*eVoid*young
                youngFieldMean = np.sum(fieldatnodes, axis=1)/NumberOfNodesInGeoSupport
                youngFieldMean = youngFieldMean.reshape(youngFieldMean.shape[0],1)
                localYoung[ids] = youngFieldMean

        youngField[elementName] = localYoung
        poissonField[elementName] = localPoisson

    materialFieldsNames = ['CHYOUNG','CHNU']
    return materialFieldsNames,[youngField,poissonField]

class AsterStatic(AsterSolverBase):
    def __init__(self):
        super(AsterStatic,self).__init__()
        self.name = 'AsterStatic'
        FieldsICanSupply = [FN.potential_energy,FN.von_mises,FN.stress,FN.strain,FN.zzError]
        for componentName in FN.TensorFieldsComponentsNames:
            FieldsICanSupply.append(FN.stress+"_"+componentName)
            FieldsICanSupply.append(FN.strain+"_"+componentName)

        for fieldname in FieldsICanSupply:
            self.auxiliaryFieldGeneration[fieldname][FN.Centroids] = False
            self.auxiliaryFieldGeneration[fieldname][FN.Nodes] = False
            self.auxiliaryFieldGeneration[fieldname][FN.IPField] = False
        self.auxiliaryFieldGeneration[FN.zzError][FN.Centroids] = False
        self.auxiliaryScalarGeneration = {FN.int_potential_energy:False}
        self.residual = 1e-5
        self.maxAllowedRelaxedResidual = 1e-3
        self.hasPreStress = False
        
    def WriteComputationalSupport(self, levelset):
        self.PrepareComputationalSupport(levelset)
        assert self.CheckComputationalSupportValidity()== True,"Computational support not valid"
        if self.space == "LagrangeP2":
            from  OpenPisco.PhysicalSolvers.PhysicalSolversTools import LinToQuadMesh
            self.cleanMesh = LinToQuadMesh(self.cleanMesh)
            self.cleanMeshToOriginalNodal = self.cleanMesh.originalIDNodes[self.cleanMeshToOriginalNodal]
        materialFieldsNames,materialFields = GetMaterialFields(support=self.cleanMesh,
                                                                                   phi=levelset.phi,
                                                                                   materials=self.materials,
                                                                                   levelset=levelset,
                                                                                   eVoid=self.eVoid)
        self.WriteSupport(self.cleanMesh,cellFieldsNames=materialFieldsNames,cellFields=materialFields)

    def SolveStandard(self):
        solverState = self.SolveWithRelaxedResidual()
        return solverState

    def WriteParametersInput(self,writeFile):
        assert len(self.problems) <= 1,"WriteParametersInput for  multi loads problem not implemented yet."
        dimensionality = self.originalSupport.GetElementsDimensionality()
        AsterStaticWriter.WriteModelisation(writeFile,dimensionality)
        AsterCommonWriter.WriteDirichletParametersInput(writeFile,self.dirichlet,dimensionality)
        AsterCommonWriter.WriteNeumannParametersInput(writeFile,self.neumann,dimensionality,self.problems)
        AsterCommonWriter.WriteNodalForcesParametersInput(writeFile,self.forcenodale,dimensionality,self.problems)
        AsterCommonWriter.WriteBodyForcesParametersInput(writeFile,self.bodyforce,dimensionality,self.problems)
        AsterCommonWriter.WriteVariable(writeFile=writeFile,variableName="resi_rela",variableValue=self.residual)
        AsterCommonWriter.WriteVariable(writeFile=writeFile,variableName="hasPreStress",variableValue=self.hasPreStress)

    def GetNodalSolution(self,onCleanMesh=False,loadcaseName=None):
        if loadcaseName is not None:
            for case in self.problems:
                if loadcaseName==case:
                    name ="DEPL_"+case
        else:
            name="DEPL"
        dimensionality = self.originalSupport.GetElementsDimensionality()
        self.u = self.GetSolution(name,onCleanMesh)[:,0:dimensionality]
        return self.u

    def GetBoundaryConditionsFilters(self):
        dirichletFilters = [NodeFilter(eTag=tag) for tag,_ in self.dirichlet]
        neumannConditions = [x for loadCase in self.neumann.values() for x in loadCase]
        neumannTags = set([tag for tag,_ in neumannConditions])
        nodeforceConditions = [x for loadCase in self.forcenodale.values() for x in loadCase]
        nodeforceTags = set([tag for tag,_ in nodeforceConditions])
        neumannNodeFilters = [NodeFilter(nTag=tag) for tag in  nodeforceTags]+[NodeFilter(eTag=tag) for tag in neumannTags]
        return dirichletFilters,neumannNodeFilters

    def __hash__(self):
        return id(self)

    def __eq__(self, other):
        if self.space!=other.space:
            return False

        #Warning: may not work for actual multi-loads
        neumannProperty = list(self.neumann.values())
        neumannOther = list(other.neumann.values())
        if len(neumannProperty)!=len(neumannOther):
            return False

        properties = [self.materials,self.dirichlet]+neumannProperty
        otherProperties = [other.materials,other.dirichlet]+neumannOther
        for singleProperty,otherProperty in zip(properties,otherProperties):
            for singleProperty,singleOtherProperty in zip(singleProperty,otherProperty):
                tagOrigin, expressionOrigin = singleProperty
                tagOther, expressionOther = singleOtherProperty
                if tagOrigin != tagOther:
                    return False
                try:
                    if isinstance(expressionOrigin,dict):
                        AssertAlmostEqualDict(expressionOrigin,expressionOther)
                    else:
                        np.testing.assert_almost_equal(expressionOrigin,expressionOther)
                except AssertionError:
                        return False
        return True

RegisterAsterClass("static_elastic", AsterStatic,CreateAsterStaticPhysicalProblem)


import typing
from Muscat.MeshTools.MeshCreationTools import CreateCube,CreateSquare
from OpenPisco.Unstructured.Levelset import LevelSet

def CheckIntegrity3D(GUI=False):
    mesh = CreateCube(dimensions=[2,2,2],spacing=[1., 1., 1.],origin=[0, 0, 0],ofTetras=True)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.ExterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    print("Computing static 3D conform ...")
    PPM = AsterStatic()
    PPM.SetAuxiliaryScalarGeneration(FN.int_potential_energy)
    PPM.SetAuxiliaryFieldGeneration(FN.potential_energy,on=FN.IPField)
    PPM.SetAuxiliaryFieldGeneration(FN.stress,on=FN.IPField)
    PPM.SetAuxiliaryFieldGeneration(FN.strain_xy,on=FN.IPField)
    PPM.SetAuxiliaryFieldGeneration(FN.von_mises,on=FN.IPField)
    PPM.SetAuxiliaryFieldGeneration(FN.zzError,on=FN.Centroids)
    PPM.materials = [['AllZones',{"young":1.0,"poisson":0.3}]]
    PPM.dirichlet= [['X0',(0.,0.,0.)]]
    PPM.problems = ['idx1']
    PPM.neumann= {"idx1": [['X1',(0.,0.,-0.1)]]}
    for conform in [True,False]:
        for space in ["LagrangeP1","LagrangeP2"]:
            ls.conform = conform
            PPM.space = space
            PPM.SolveByLevelSet(ls)
            assert isinstance(PPM, typing.Hashable), "AsterStatic Should be hashable!"
            PPM.GetAuxiliaryField(FN.potential_energy,on=FN.IPField)
            PPM.GetNodalSolution()
            PPM.GetAuxiliaryField(FN.stress,on=FN.IPField)
            PPM.GetAuxiliaryField(FN.von_mises,on=FN.IPField)
            PPM.GetAuxiliaryScalar(FN.int_potential_energy)
    return "ok"

def CheckIntegrity2D(GUI=False):
    mesh = CreateSquare(dimensions=[2,3],spacing=[.2,.2],ofTriangles=True)
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    print("Computing static 2D conform ...")
    PPM = AsterStatic()
    PPM.SetAuxiliaryFieldGeneration(FN.zzError,on=FN.Centroids)
    PPM.SetAuxiliaryFieldGeneration(FN.potential_energy,on=FN.Nodes)
    PPM.SetAuxiliaryFieldGeneration(FN.stress,on=FN.IPField)
    PPM.SetAuxiliaryFieldGeneration(FN.stress_xx,on=FN.IPField)
    PPM.SetAuxiliaryFieldGeneration(FN.von_mises,on=FN.Nodes)
    PPM.materials = [['AllZones',{"young":1.0,"poisson":0.3}]]
    PPM.dirichlet= [['X0',(0.,0.)]]
    PPM.problems = ['idx1']
    PPM.neumann= {"idx1": [['X1',(0.,-0.01)]]}
    for space in ["LagrangeP1","LagrangeP2"]:
        PPM.space = space
        PPM.SolveByLevelSet(ls)
        PPM.GetNodalSolution()
        PPM.GetAuxiliaryField(FN.stress,on=FN.IPField)
        PPM.GetAuxiliaryField(FN.stress_xx,on=FN.IPField)
        PPM.GetAuxiliaryField(FN.potential_energy,on=FN.Nodes)
        PPM.GetAuxiliaryField(FN.von_mises,on=FN.Nodes)
    return "ok"

def CheckIntegrity(GUI=False):
    from OpenPisco.ExternalTools.Aster.AsterInterface import SkipAsterTest
    skipTestPhase,message = SkipAsterTest()
    if skipTestPhase:
        return message

    totest = [
        CheckIntegrity3D,
        CheckIntegrity2D,
        ]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
