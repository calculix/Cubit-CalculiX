# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

from Muscat.Types import MuscatFloat
from Muscat.Helpers.Logger import Debug
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.Fields.FieldTools import FillFEField, VectorToFEFieldsData, IntegrationPointWrapper
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceGeo, ConstantSpaceGlobal, LagrangeSpaceP0
from Muscat.FE.Spaces.IPSpaces import GenerateSpaceForIntegrationPointInterpolation
from Muscat.FE.IntegrationRules import GetIntegrationRuleByName
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.FE.Integration import IntegrateGeneral
from Muscat.FE.SymWeakForm import GetTestField, Inner
from Muscat.FE.Fields.IPField import IPField
from Muscat.LinAlg.LinearSolver import GetAvailableSolvers

import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.FieldsNames import GetFieldNumberOfComponents
from OpenPisco import RETURN_SUCCESS
from OpenPisco.PhysicalSolvers.SolverBase import SolverBase

class UnstructuredFEASolverBase(SolverBase):
    def __init__(self):
        super(UnstructuredFEASolverBase,self).__init__()

    def Reset(self):
        self.__numberingCache__ = {}

    def SetLinearSolver(self, linearSolverName):
        availableSolvers = GetAvailableSolvers()
        assert linearSolverName in availableSolvers,"Unknown linear solver name: '{}' ".format(linearSolverName)
        self.internalSolveur.solver.SetAlgo(linearSolverName)

    def GetSupport(self):
        return self.originalSupport

    def GetAllFieldsOnCleanMesh(self):
        fields = []
        fields.extend(self.internalSolveur.unknownFields)
        fields.extend(self.internalSolveur.fields.values())
        return fields

    def GetNumberingOnOriginalMesh(self,space,elementFilter=None,fromConnectivity=False):
        key = (id(self.cleanMesh),id(space),elementFilter,fromConnectivity)
        if key not in self.__numberingCache__:
            self.__numberingCache__[key] = ComputeDofNumbering(self.cleanMesh,space,elementFilter = elementFilter)
        return self.__numberingCacheOriginalMesh__[key]

    def GetAuxiliaryScalar(self,name):
        return self.postFields.get(name,None)

    def Solve(self):
        Debug("In solve")
        if len(self.propsFields):
            numberingP0 = ComputeDofNumbering(mesh=self.cleanMesh, space= LagrangeSpaceP0)
            Debug("Numbering done")
            for fieldName, fieldDefinition in self.propsFields.items():
                res = FEField(name=fieldName, mesh=self.cleanMesh,space=LagrangeSpaceP0,numbering=numberingP0 )
                res.Allocate()
                FillFEField(res,fieldDefinition)
                self.internalSolveur.fields[fieldName] = res
            Debug("FillFEField done")
        Debug(self.internalSolveur.totalNumberOfDof)
        Debug(self.internalSolveur.unknownFields)
        k,f = self.internalSolveur.GetLinearProblem()
        if self.extraRHSVector:
            f+=self.GetExtraRHSTermFromextraRHSVector()

        tangentMatrixNonZeroTermNumber=k.count_nonzero()
        Debug("Number of non zero terms within tangent matrix:"+str(tangentMatrixNonZeroTermNumber))
        assert tangentMatrixNonZeroTermNumber!=0,"Tangent matrix is empty: not allowed"
        self.internalSolveur.ComputeConstraintsEquations()
        Debug(self.internalSolveur.solver.constraints.numberOfEquations)

        self.internalSolveur.Solve(k,f)
        self.internalSolveur.PushSolutionVectorToUnknownFields()

    def Resolve(self,linearWeakFormulations):
        _,f = self.internalSolveur.GetLinearProblem(computeK=False, lform=linearWeakFormulations)
        self.internalSolveur.Resolve(f)
        self.internalSolveur.PushSolutionVectorToUnknownFields()

    def PostSolver(self):
        fields = self.GetAllFieldsOnCleanMesh()
        nodalSpaceNumbering = [None,None,None]
        scalarSpaceNumbering = [None,None,None]
        ipSpaceNumbering = [None,None,None]

        def GetNodalSpaceNumbering():
            if nodalSpaceNumbering[0] is None:
                nodalSpaceNumbering[0] = LagrangeSpaceGeo
                nodalSpaceNumbering[1] = ComputeDofNumbering(self.cleanMesh,LagrangeSpaceGeo,fromConnectivity =True)
                nodalSpaceNumbering[2] = ElementFilter(eTag=self.tagForPostProcess)
            return nodalSpaceNumbering

        def GetScalarSpaceNumbering():
            if scalarSpaceNumbering[0] is None:
                postProcessFilter = ElementFilter(eTag=self.tagForPostProcess)
                scalarSpaceNumbering[0] = ConstantSpaceGlobal
                scalarSpaceNumbering[1] = ComputeDofNumbering(self.cleanMesh,ConstantSpaceGlobal,elementFilter = postProcessFilter )
                scalarSpaceNumbering[2] = postProcessFilter
            return scalarSpaceNumbering

        def GetIPSpaceNumbering():
            if ipSpaceNumbering[0] is None:
                postProcessFilter = ElementFilter(eTag=self.tagForPostProcess)
                ipSpaceNumbering[0] = GenerateSpaceForIntegrationPointInterpolation(GetIntegrationRuleByName(integrationRule))
                ipSpaceNumbering[1] = ComputeDofNumbering(self.cleanMesh,ipSpaceNumbering[0])
                ipSpaceNumbering[2] = postProcessFilter
            return ipSpaceNumbering

        self.postFields = {}
        constants=self.internalSolveur.constants

        for phys in self.internalSolveur.physics:
            postData = phys.PostTreatmentFormulations()

            keysToGenerateByPostDataKeys = {}
            for key in postData.keys():
                keysToGenerateByPostDataKeys[key] = key
            keysToGenerateByPostDataKeys[FN.squared_von_mises] = FN.von_mises

            for name,wform in postData.items():
                fieldname =keysToGenerateByPostDataKeys[name]

                nbComponents = GetFieldNumberOfComponents(name,phys.spaceDimension)

                if fieldname in self.auxiliaryFieldGeneration.keys() and self.auxiliaryFieldGeneration[fieldname][FN.IPField]:
                    integrationRule = phys.integrationRule

                    ipSpace,ipNumbering,ipPostProcessFilter = GetIPSpaceNumbering()

                    if nbComponents == 1:
                        unknownFields = [ FEField(name,mesh=self.cleanMesh,space=ipSpace,numbering=ipNumbering)]
                    else:
                        unknownFields = [ FEField(name+"_"+str(i),mesh=self.cleanMesh,space=ipSpace,numbering=ipNumbering) for i in range(nbComponents)]

                    _,f = IntegrateGeneral(mesh=self.cleanMesh,
                                           wform=wform,
                                           constants=constants,
                                           fields=fields,
                                           unknownFields= unknownFields,
                                           integrationRuleName=integrationRule,
                                           onlyEvaluation=True,
                                           elementFilter=ipPostProcessFilter
                                 )

                    if name == FN.squared_von_mises and fieldname == keysToGenerateByPostDataKeys[name]:
                        f = np.sqrt(f)

                    sol = IPField(name=fieldname,mesh=self.cleanMesh,ruleName=integrationRule)
                    sol.Allocate()

                    VectorToFEFieldsData(f,unknownFields)

                    globalOffset = 0

                    for elname, eldata in sol.mesh.elements.items():
                        nbIntegrationPoints = sol.GetRuleFor(elname).GetNumberOfPoints()
                        nbElements = eldata.GetNumberOfElements()

                        if nbElements ==0:
                            continue

                        if nbComponents == 1:
                            dataOnComponent = unknownFields[0].data[globalOffset:globalOffset+ nbElements*nbIntegrationPoints]
                            datafield = dataOnComponent
                        else:
                            datafield = np.zeros((nbElements, nbIntegrationPoints, nbComponents), dtype=MuscatFloat)
                            for i in range(nbComponents):
                                dataOnComponent = unknownFields[i].data[globalOffset:globalOffset+ nbElements*nbIntegrationPoints]
                                datafield[...,i] = dataOnComponent.reshape(nbElements,nbIntegrationPoints)

                        sol.data[elname]= datafield
                        globalOffset += nbElements

                    self.postFields[fieldname] = sol

                if fieldname in self.auxiliaryFieldGeneration.keys() and self.auxiliaryFieldGeneration[fieldname][FN.Nodes]:
                    nSpace,nNumbering,nPostProcessFilter = GetNodalSpaceNumbering()
                    unknownField = FEField()
                    unknownField.numbering = nNumbering
                    unknownField.name = name
                    unknownField.data = None
                    unknownField.mesh = self.cleanMesh
                    unknownField.space = nSpace

                    _,f = IntegrateGeneral(mesh=self.cleanMesh,
                                           wform=wform,
                                           constants=constants,
                                           fields=fields,
                                           unknownFields= [unknownField],
                                           integrationRuleName="NodalEvaluationIsoParam",
                                           onlyEvaluation=True,
                                           elementFilter=nPostProcessFilter
                                 )

                    wformMass = GetTestField("mass",1, sdim=self.cleanMesh.GetPointsDimensionality())

                    unknownFieldMass = FEField()
                    unknownFieldMass.numbering = nNumbering
                    unknownFieldMass.name = "mass"
                    unknownFieldMass.data = None
                    unknownFieldMass.mesh = self.cleanMesh
                    unknownFieldMass.space = nSpace

                    _,mass = IntegrateGeneral(mesh=self.cleanMesh,
                                           wform=wformMass,
                                           constants=constants,
                                           fields=fields,
                                           unknownFields= [unknownFieldMass],
                                           integrationRuleName="NodalEvaluationIsoParam",
                                           onlyEvaluation=True,
                                           elementFilter=nPostProcessFilter
                                 )
                    mass[mass==0] = 1.

                    f /= mass

                    if name == FN.squared_von_mises and fieldname == keysToGenerateByPostDataKeys[name]:
                        f = np.sqrt(f)
                        unknownField.name = fieldname

                    unknownField.data = f

                    self.postFields[fieldname] = unknownField

                if "int_"+ name in self.auxiliaryScalarGeneration.keys()  and self.auxiliaryScalarGeneration["int_"+ name]:
                    gSpace,gNumbering,gPostProcessFilter = GetScalarSpaceNumbering()

                    unknownField = FEField()
                    unknownField.numbering = gNumbering
                    unknownField.name = name
                    unknownField.data = None
                    unknownField.mesh = self.cleanMesh
                    unknownField.space = gSpace

                    _,f = IntegrateGeneral(mesh = self.cleanMesh,
                                    wform = wform,
                                    constants=constants,
                                    fields=fields,
                                    unknownFields= [unknownField],
                                    elementFilter=gPostProcessFilter)

                    self.postFields["int_" + name] = f[0]

    def DelayedInitialize(self):
        if self.delayedInitialization is not None:
            for delayedInitializeName,delayedInitializeData in self.delayedInitialization.items():
                if delayedInitializeName=="HeterogeneousDirichlet":
                    mesh=self.cleanMesh
                    fieldByName=delayedInitializeData["field"]
                    condition=delayedInitializeData["condition"]
                    tagNamesInMesh= [[singleTag.name for singleTag in elem.tags] for elem in mesh.elements.values()]
                    tagNamesInMesh = [item for sublist in tagNamesInMesh for item in sublist]
                    for zone in condition.on:
                        assert zone in tagNamesInMesh, "Can not enforce heterogeneous dirichlet condition: elementary tag %s does not exist" %zone
                    condition.FixUsingNodeFields(mesh, fieldByName)
                    self.internalSolveur.solver.constraints.AddConstraint(condition)
                else:
                    raise NotImplementedError("Not handled yet")

    def SolveByLevelSet(self,levelSet): # pragma: no cover
        self.PreSolver(levelSet)
        self.DelayedInitialize()
        self.Solve()
        self.PostSolver()
        return RETURN_SUCCESS

    def GetAuxiliaryField(self, name, on, onCleanMesh=False):
        if name in self.primalUnknownNames:
            index = self.primalUnknownNames.index(name)
            field = self.internalSolveur.unknownFields[index]
        elif name in self.internalSolveur.fields:
            field = self.internalSolveur.fields[name]
        elif name in self.postFields:
            field  = self.postFields.get(name,None)
        else:
            raise ValueError("Auxiliary field provided is neither a primal unknown, an internal fielf or a post field")

        self.transportFieldOP.oldMesh = self.originalSupport
        if on == FN.FEField:
            if isinstance(field,FEField):
                Debug("Getting " + name)
                if onCleanMesh:
                    return field
                else:
                    return self.transportFieldOP.TransportFieldToOldMesh(field,fillValue=0.0)
            else:
                raise NotImplementedError("Can only transport FEField field back to old mesh (?)")
        Debug("----")
        if on == FN.Nodes:
            if isinstance(field,FEField):
                # we compute the solution at points first so the value at the points is not polluted by the element of the outside
                if onCleanMesh:

                    return field.GetPointRepresentation()
                else:
                    res = np.zeros(self.originalSupport.GetNumberOfNodes(),dtype=MuscatFloat)
                    dd = field.GetPointRepresentation()
                    Debug(len(dd))
                    Debug(len(self.cleanMesh.originalIDNodes))
                    Debug(len(res))
                    res[self.cleanMeshToOriginalNodal] = field.GetPointRepresentation()
                    return res
            elif isinstance(field,IPField):
                if onCleanMesh:
                    return field.GetPointRepresentation()
                else:
                    return self.transportFieldOP.TransportFieldToOldMesh(field,fillValue=0.0).GetPointRepresentation()
            elif type(field).__module__ == np.__name__:
                if onCleanMesh:
                    expectedNumNodes = self.cleanMesh.GetNumberOfNodes()
                else:
                    expectedNumNodes = self.originalSupport.GetNumberOfNodes()
                assert field.size == expectedNumNodes," numpy array size ({}) incompatible with the mesh size({})".format(field.size,expectedNumNodes)
                return field
            else:
                raise TypeError("I have a {} field but you asked a {}".format(type(field),on))

        if on ==FN.IPField:
            if isinstance(field,FEField):
                if onCleanMesh:
                    res = field
                else:
                    res = self.transportFieldOP.TransportFieldToOldMesh(field)
                return IntegrationPointWrapper(res,GetIntegrationRuleByName('LagrangeIsoParamQuadrature')).GetIpField()
            elif isinstance(field,IPField):
                if onCleanMesh:
                    return field
                else:
                    return self.transportFieldOP.TransportFieldToOldMesh(field)
            else:
                raise TypeError("I have a {} field but you asked a {}".format(type(field),on))

        assert on == FN.Nodes,"Can return only on Nodes"

        data  = self.postFields.get(name,None)
        if data is None:
            return None

        dataOnMeshNumbering = self.internalSolveur.PushVectorToMesh(True, data,name,[self.nNumbering], justReturn=True)
        if onCleanMesh:
           return dataOnMeshNumbering
        else:
            dataOnOriginalMesh = np.zeros((self.originalSupport.GetNumberOfNodes(),1),dtype=MuscatFloat)
            dataOnOriginalMesh[self.cleanMesh.originalIDNodes,:] = dataOnMeshNumbering
            return dataOnOriginalMesh

    def GetNodalSolution(self,onCleanMesh=False):
        values = np.vstack(list(self.GetAuxiliaryField(self.primalUnknownNames[i],FN.Nodes,onCleanMesh=onCleanMesh) for i in range(len(self.internalSolveur.numberings)))).T
        return values

    def SetNodalSolution(self,res):
        n = res.shape[1]
        for i in range(n):
            invec = res[self.cleanMeshToOriginalNodal,i]
            f = self.internalSolveur.unknownFields[i]
            f.data[f.numbering.doftopointRight] = invec[f.numbering.doftopointLeft]
        self.internalSolveur.PushUnknownFieldsToSolutionVector()

    def AddField(self,field):
        self.transportFieldOP.newMesh=self.cleanMesh
        if field.mesh == self.cleanMesh:
            self.internalSolveur.fields[field.name] =  field
        else:
            res = self.transportFieldOP.TransportFieldToNewMesh(field)
            self.internalSolveur.fields[res.name] =  res

    def PushForceFieldToLinearProblem(self,name,data):
        self.extraRHSVector[name] = data

    def GetExtraRHSTermFromextraRHSVector(self):
        size = 0
        offset = []
        for n in self.internalSolveur.numberings:
            offset.append(size)
            size += n.size
        extraRHSterm = np.zeros(self.internalSolveur.totalNumberOfDof,dtype=MuscatFloat)
        for _,data in self.extraRHSVector.items():
            errorMessage = "UnstructuredFEAGeneric error: the total number of dof is "+str(self.internalSolveur.totalNumberOfDof)+" but RHS has size "+str(len(data))
            assert len(data)==self.internalSolveur.totalNumberOfDof, errorMessage
            for i,inumbering in enumerate(self.internalSolveur.numberings):
                for nodeid in range(self.cleanMesh.GetNumberOfNodes()):
                    dof = inumbering.GetDofOfPoint(nodeid)+offset[i]
                    extraRHSterm[dof] += data[nodeid+offset[i]]
        return extraRHSterm

def CheckIntegrity():
    UnstructuredFEASolverBase()
    return 'ok'

if __name__ == '__main__':
    print(CheckIntegrity())# pragma: no cover
