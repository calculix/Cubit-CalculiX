# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from collections.abc import Iterable

import numpy as np

import Muscat.Helpers.ParserHelper as PH

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.MuscatExtentions.FoamReader import FieldNotFoundInLastStepDirectoryError
from OpenPisco import RETURN_SUCCESS

from OpenPisco.MuscatExtentions.FoamReader import FieldNotFoundError

def CreateTopoCriteriaForce(ops):
    res=TopoCriteriaForce()
    res.SetName(PH.ReadString(ops["name"]))
    return res

def CreateTopoCriteriaDrag(ops):
    res=TopoCriteriaForce()
    res.SetName("drag")
    res.direction = PH.ReadFloats(ops.get("direction",np.array([1.0,0.0,0.0])))
    return res

def CreateTopoCriteriaLift(ops):
    res=TopoCriteriaForce()
    res.SetName("lift")
    res.direction = PH.ReadFloats(ops.get("direction",np.array([0.0,1.0,0.0])))
    return res

def CreateTopoCriteriaPtLosses(ops):
    res=TopoCriteriaPtLosses()
    res.SetName("PtLosses")
    return res

# A force criterion in OpenFOAM is described by the following equation
# J(\Omega)  = \frac{int_{S_w} \rho (-τ_{ij} n_j + p n_i) r_i dS}{0.5 \rho A U_{\infty}}
# where 
#       - \tau_{ij} are the components of the stress tensor
#       - \rho is the constant density
#       - p is the pressure divided by \rho
#       - n the unit normal vector. 
#       - Vector r defines the direction in which the force vector should be projected (e.g. parallel to the farfield velocity to minimize
# drag). 
#       - S_w is the surface on which the force is defined
#       - A is the frontal area
#       - U_{\infty} the farfield velocity magnitude
# 
# In what follows, repeated indices imply summation. In addition, SW are the
# wall patches on which force is defined, A is the frontal area and U_{\infty} the farfield velocity magnitude

class TopoCriteriaForce(PhysicalCriteriaBase):
    def __init__(self,other=None):
        super(TopoCriteriaForce, self).__init__(other)
        self.__name__ = "force"

        self.update = True
        self.parameterNames=["weight","type","patches","direction","Aref","rhoInf","UInf"]

        if other is None:
            self.SetName(self.GetName())
            self.adjointProblemName = None
            self.weight = 1
            self.type = "force"
            self.patches = []
            self.direction = np.array([1.0,0.0,0.0])
            self.Aref = 1.0 
            self.rhoInf = 1.292 # dry air at 0C and 101.325kPa
            self.UInf = 1.0
        else:
            self.f_val = other.f_val
            self.fSensitivity_val = np.copy(other.fSensitivity_val)
            self.SetName(other.GetName())
            self.adjointProblemName = other.adjointProblemName
            self.weight = other.weight
            self.type = other.type
            self.patches = other.patches
            self.direction = other.direction
            self.Aref = other.Aref
            self.rhoInf = other.rhoInf
            self.UInf = other.UInf

    def SetAuxiliaryQuantities(self, levelSet):
        self.problem.AddCriterion(self)
        self.problem.SetAuxiliaryFieldGeneration(self.GetName()+"_sensitivity",on=FN.Nodes)

    def GetNumberOfSolutions(self):
        return 4

    def GetCriteriaSolution(self,i,onCleanMesh=False):
        solutionName = self.GetCriteriaSolutionName(i)
        if solutionName in [ "U","p"]:
            solutionAtNodes = self.problem.GetNodalSolution(onCleanMesh=onCleanMesh)[solutionName]
        else:
            try:
                solutionAtCell = self.problem.RetrieveExtraSolutionField(solutionName)
                solutionAtNodes = self.problem.TransportFieldToNode(fieldCellValue=solutionAtCell,onCleanMesh=onCleanMesh)
            except FieldNotFoundInLastStepDirectoryError:
                solutionName=solutionName.replace(self.problem.problemNamePrefix+"_"+self.GetName(),"")
                solutionAtCell = self.problem.RetrieveExtraSolutionField(solutionName)
                solutionAtNodes = self.problem.TransportFieldToNode(fieldCellValue=solutionAtCell,onCleanMesh=onCleanMesh)

        return solutionAtNodes

    def GetCriteriaSolutionName(self,i):
        if len(self.problem.optimCriteria) > 1:
            solutionName=[
                        "U",
                        "p",
                        "pa"+self.problem.problemNamePrefix+"_"+self.GetName(),
                        "Ua"+self.problem.problemNamePrefix+"_"+self.GetName(),
                        ]
        else:
            solutionName=[
                        "U",
                        "p",
                        "pa",
                        "Ua",
                        ]
        return solutionName[i]

    def UpdateValues(self,levelSet):
        self.f_val = self.problem.GetAuxiliaryScalar(self.GetName())

        fSensitivity_val= self.problem.GetAuxiliaryField(self.GetName()+"_sensitivity",
                                                         on="Nodes",
                                                         onCleanMesh=False)
        self.fSensitivity_val = fSensitivity_val.flatten()
        return RETURN_SUCCESS

    def ToSolverFormat(self):
        indentStr = "\t\t"
        solverParams = {paramName:getattr(self, paramName) for paramName in self.parameterNames}
        solverParams["type"] = "force"
        solverParams["direction"] = "("+" ".join(str(singleValue) for singleValue in solverParams["direction"])+")"
        solverParams["patches"] =  "("+" ".join(solverParams["patches"])+")"
        return indentStr+self.GetName()+"\n"+indentStr+"  {\n"+" ;\n ".join(f"\t\t\t{k} {v}" for k, v in solverParams.items())+";\n"+indentStr+"}"

    def __str__(self):
        out=""
        solverParams = {paramName:getattr(self, paramName) for paramName in self.parameterNames}
        out+="Criteria name: "+self.GetName()+"\n"
        for solverParamName,solverParamValue in solverParams.items():
            out+=solverParamName+" "+str(solverParamValue)+"\n"
        return out

class TopoCriteriaPtLosses(PhysicalCriteriaBase):
    def __init__(self,other=None):
        super(TopoCriteriaPtLosses, self).__init__(other)
        self.__name__ = "PtLosses"

        self.update = True
        self.parameterNames=["weight","type","patches"]

        if other is None:
            self.SetName(self.GetName())
            self.adjointProblemName = None
            self.weight = 1
            self.type = "PtLosses"
            self.patches = []
        else:
            self.f_val = other.f_val
            self.fSensitivity_val = np.copy(other.fSensitivity_val)
            self.SetName(other.GetName())
            self.adjointProblemName = other.adjointProblemName
            self.weight = other.weight
            self.type = other.type
            self.patches = other.patches
            
    def SetAuxiliaryQuantities(self,levelSet):
        self.problem.AddCriterion(self)
        self.problem.SetAuxiliaryFieldGeneration(self.GetName()+"_sensitivity",on=FN.Nodes)

    def GetNumberOfSolutions(self):
        return 4

    def GetCriteriaSolution(self,i,onCleanMesh=False):
        solutionName = self.GetCriteriaSolutionName(i)
        if solutionName in [ "U","p"]:
            solutionAtNodes = self.problem.GetNodalSolution(onCleanMesh=onCleanMesh)[solutionName]
        else:
            try:
                solutionAtCell = self.problem.RetrieveExtraSolutionField(solutionName)
                solutionAtNodes = self.problem.TransportFieldToNode(fieldCellValue=solutionAtCell,onCleanMesh=onCleanMesh)
            except FieldNotFoundError as e:
                raise Exception("Adjoint field file found but does not contain proper field value (adjoint solver may have failed)") from e

        return solutionAtNodes

    def GetCriteriaSolutionName(self,i):
        #naming of adjoint fields in OpenFOAM depends on number of criteria ?
        #problem : criteria seems to be added at every iteration ! number of criteria increases
        #temporary fix: does not handle multi adjoint
        solutionName=[
                    "U",
                    "p",
                    "pa"+self.problem.problemNamePrefix+"_"+self.GetName(),
                    "Ua"+self.problem.problemNamePrefix+"_"+self.GetName(),
                    #"pa",
                    #"Ua",
                    ]
        return solutionName[i]

    def UpdateValues(self,levelSet):
        self.f_val = self.problem.GetAuxiliaryScalar(self.GetName())

        fSensitivity_val= self.problem.GetAuxiliaryField(self.GetName()+"_sensitivity",
                                                         on="Nodes",
                                                         onCleanMesh=False)
        self.fSensitivity_val = fSensitivity_val.flatten()
        return RETURN_SUCCESS

    def ToSolverFormat(self):
        indentStr = "\t\t"
        solverParams = {paramName:getattr(self, paramName) for paramName in self.parameterNames}
        #solverParams["type"] = "force"
        if solverParams["type"] == "force":
            solverParams["direction"] = "("+" ".join(str(singleValue) for singleValue in solverParams["direction"])+")"
        solverParams["patches"] =  "("+" ".join(solverParams["patches"])+")"
        return indentStr+self.GetName()+"\n"+indentStr+"  {\n"+" ;\n ".join(f"\t\t\t{k} {v}" for k, v in solverParams.items())+";\n"+indentStr+"}"

    def __str__(self):
        out=""
        solverParams = {paramName:getattr(self, paramName) for paramName in self.parameterNames}
        out+="Criteria name: "+self.GetName()+"\n"
        for solverParamName,solverParamValue in solverParams.items():
            out+=solverParamName+" "+str(solverParamValue)+"\n"
        return out

    def __eq__(self,other):
        allParamsToCheck = ["adjointProblemName"]+self.parameterNames
        for paramName in allParamsToCheck:
            refValue = getattr(self, paramName)
            otherValue = getattr(other, paramName)
            if isinstance(refValue, Iterable):
                if len(refValue)!=len(otherValue) or any(refVal!=otherVal for refVal,otherVal in zip(refValue,otherValue)):
                    return False
            else:
                if refValue!=otherValue:
                    return False
        return True

    def __hash__(self):
        return hash(str(self))

RegisterCriteriaClass("force", TopoCriteriaForce,CreateTopoCriteriaForce)
RegisterCriteriaClass("drag", TopoCriteriaForce,CreateTopoCriteriaDrag)
RegisterCriteriaClass("lift", TopoCriteriaForce,CreateTopoCriteriaLift)
RegisterCriteriaClass("PtLosses", TopoCriteriaPtLosses,CreateTopoCriteriaPtLosses)

if __name__ == '__main__':
    crit1 = CreateTopoCriteriaDrag(ops=dict())
    crit2 = CreateTopoCriteriaDrag(ops=dict())
    assert crit1==crit2, "Criteria should be seen as identical"
    assert hash(crit1)==hash(crit2), "Hash should be identical"
    critSet = set([crit1,crit2])
    assert len(critSet)==1, "Instances of same criteria should be considered identical"
