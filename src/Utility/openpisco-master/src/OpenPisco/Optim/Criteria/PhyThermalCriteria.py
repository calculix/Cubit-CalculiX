# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations
from typing import Optional

import numpy as np

from Muscat.Types import MuscatFloat
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.Fields.FieldTools import TransferFEFieldToIPField, NodeFieldToFEField
from Muscat.FE.UnstructuredFeaSym import UnstructuredFeaSym
from Muscat.FE.SymPhysics import ThermalPhysics
from Muscat.FE.Integration import IntegrateGeneral
import Muscat.FE.SymWeakForm as wf
from Muscat.FE.KR.KRBlock import KRBlockScalar

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
from OpenPisco.MuscatExtentions.IPFieldIntegration import IPFieldIntegration
from OpenPisco.Unstructured.Levelset import LevelSet
import OpenPisco.TopoZones as TZ
from OpenPisco import RETURN_SUCCESS

class TopoCriteriaTargetTemperature(PhysicalCriteriaBase):
    """Thermal constraint over the temperature"""
    def __init__(self, other:Optional[TopoCriteriaTargetTemperature]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaTargetTemperature], optional
            another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaTargetTemperature, self).__init__(other)
        self.update = True

        if other is None:
            self.SetName("TargetTemperature")
            self.f_val = -1
            self.TSolid = 0.
            self.firstTime = True
            self.adjointSolution = None
        else:
            self.f_val = other.f_val
            self.fSensitivity_val = np.copy(other.fSensitivity_val)
            self.TSolid = other.TSolid
            self.firstTime = other.firstTime
            self.adjointSolution = other.adjointSolution

    def GetNumberOfSolutions(self):
        """Get number of criterion solutions

        Returns
        -------
        int
            number of solutions
        """
        return 2

    def GetCriteriaSolution(self,i:int)->np.ndarray:
        """Retrieve the solution of index i. The index i should be strictly smaller than the total number of solutions (here 2).

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        np.ndarray
            solution value
        """
        solution=[self.problem.GetNodalSolution(),self.adjointSolution]
        return solution[i]

    def GetCriteriaSolutionName(self,i:int)->str:
        """Get criterion solution name

        Parameters
        ----------
        i : int
            solution index

        Returns
        -------
        str
            solution name
        """
        solutionName=["Temperature","Temperature_Adjoint"]
        return solutionName[i]

    def UpdateValues(self,levelSet:LevelSet):
        """Update value and sensitivity of the criterion for the current levelset

        At each update, a thermal problem and its associated adjoint problem is solved

        The criterion is given by

        .. math::
            J(\Omega)  = \int_{\Omega}  max(T-T_s,0.)**2

        T(x) : calculated temperature field
        T_s : temperatureAdjField at solid state

        Ensure that the structure has solidified everywhere

        https://pastel.hal.science/pastel-00937306/file/thesis.pdf
        chapter 5

        Parameters
        ----------
        levelSet : LevelSet
            levelset object

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        from OpenPisco.PhysicalSolvers.AsterThermal import AsterThermal
        errorMessage = "Thermal criterion: need to use Code_Aster to solve the direct physical problem. "
        assert isinstance(self.problem, AsterThermal),errorMessage

        temperature = self.problem.GetNodalSolution(onCleanMesh=True)
        field =  np.maximum(temperature-self.TSolid,0.) **2
        fefield = NodeFieldToFEField(self.problem.cleanMesh,{"integrand":field})["integrand"]
        ipfield = TransferFEFieldToIPField(fefield, ruleName="LagrangeIsoParamQuadrature")
        self.f_val = IPFieldIntegration(ipfield,ElementFilter(dimensionality=self.problem.cleanMesh.GetElementsDimensionality()))

        adjointProblem = UnstructuredFeaSym()
        thermalPhysics = ThermalPhysics()
        thermalPhysics.spaceDimension=self.problem.cleanMesh.GetElementsDimensionality()
        thermalPhysics.SetSpaceToLagrange(P=1)
        material=self.problem.materials[0][1]
        thermalPhysics.AddBFormulation( TZ.Inside3D, thermalPhysics.GetBulkFormulation(alpha=material["lambda"]) )
        thermalPhysics.AddLFormulation(TZ.Inside3D, thermalPhysics.GetNormalFlux("source"))
        thermalPhysics.AddLFormulation("Skin",None)
        dirichlet = KRBlockScalar()
        dirichlet.AddArg("t").On("Skin")
        dirichlet.SetValue(0.0)
        adjointProblem.solver.constraints.AddConstraint(dirichlet)
        adjointProblem.physics.append(thermalPhysics)
        adjointProblem.SetMesh(self.problem.cleanMesh)
        adjointProblem.ComputeDofNumbering([TZ.Inside3D,"Skin"])
        source = FEField(name="source",mesh=self.problem.cleanMesh,space=thermalPhysics.spaces[0],numbering=adjointProblem.numberings[0])
        source.data = -2*np.maximum(temperature-self.TSolid,0.)

        adjointProblem.fields["source"] = source
        adjointProblem.ComputeConstraintsEquations()
        k,f = adjointProblem.GetLinearProblem(computeK=True,computeF=True)
        adjointProblem.Solve(k,f)
        adjointProblem.PushSolutionVectorToUnknownFields()
        temperatureAdjoint = adjointProblem.unknownFields[0].GetPointRepresentation()


        self.adjointSolution = np.zeros(levelSet.support.GetNumberOfNodes(),dtype=MuscatFloat)
        self.adjointSolution[self.problem.cleanMeshToOriginalNodal] = temperatureAdjoint

        spaceDimension = thermalPhysics.spaceDimension
        numbering = adjointProblem.numberings[0]
        space = thermalPhysics.spaces[0]
        symT = wf.GetField("T",1,sdim=levelSet.support.GetPointsDimensionality())
        symTAdj = wf.GetField("TAdjoint",1,sdim=levelSet.support.GetPointsDimensionality())
        nodalEnergyT = wf.GetTestField("sensitivity", 1,sdim=levelSet.support.GetPointsDimensionality())
        lambdafield = wf.GetScalarField(material["lambda"])
        Normal = wf.GetNormal(thermalPhysics.spaceDimension)
        weakform = Normal.T * wf.Gradient(symT,spaceDimension) * (lambdafield) * (wf.Gradient(symTAdj,spaceDimension).T * Normal) * nodalEnergyT

        unknownField = FEField(name="sensitivity",mesh=self.problem.cleanMesh,space=space,numbering=numbering)
        unknownField.data = None
        temperatureField =  FEField(name="T",mesh=self.problem.cleanMesh,space=space,numbering=numbering)
        temperatureField.data =  temperature
        temperatureAdjField =  FEField(name="TAdjoint",mesh=self.problem.cleanMesh,space=space,numbering=numbering)
        temperatureAdjField.data = temperatureAdjoint

        tagByDimension={3:TZ.InterSurf,2:TZ.InterBars}[thermalPhysics.spaceDimension]

        _,f = IntegrateGeneral(mesh=self.problem.cleanMesh,
                                wform=weakform,
                                constants={},
                                fields=[temperatureField,temperatureAdjField],
                                unknownFields= [unknownField],
                                integrationRuleName="NodalEvaluationIsoParam",
                                elementFilter=ElementFilter(eTag=tagByDimension),
                                onlyEvaluation=True
                        )

        wformMass = wf.GetTestField("mass",1)
        unknownFieldMass = FEField(name="mass",mesh=self.problem.cleanMesh,space=space,numbering=numbering)
        unknownFieldMass.data = None

        _,mass = IntegrateGeneral(mesh=self.problem.cleanMesh,
                                wform=wformMass,
                                constants={},
                                fields=[temperatureField,temperatureAdjField],
                                unknownFields= [unknownFieldMass],
                                integrationRuleName="NodalEvaluationIsoParam",
                                onlyEvaluation=True
                        )

        mass[mass==0] = 1.
        f /= mass
        unknownField.data = f

        self.fSensitivity_val = np.zeros(levelSet.support.GetNumberOfNodes(),dtype=MuscatFloat)
        self.fSensitivity_val[self.problem.cleanMeshToOriginalNodal] = -unknownField.GetPointRepresentation()
        self.fSensitivity_val=self.fSensitivity_val.flatten()
        return RETURN_SUCCESS

RegisterCriteriaClass("TargetTemperature", TopoCriteriaTargetTemperature)

def CheckIntegrity(GUI=False):
    return "ok"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
