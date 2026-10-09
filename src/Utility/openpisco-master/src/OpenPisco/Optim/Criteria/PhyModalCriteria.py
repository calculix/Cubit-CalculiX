# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations

from typing import Dict,Optional
import math

import numpy as np

import Muscat.Helpers.ParserHelper as PH

from OpenPisco.Optim.Criteria.CriteriaFactory import RegisterClass as RegisterCriteriaClass
from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
from OpenPisco.Unstructured.Levelset import LevelSet
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS


def CreateTopoCriteriaModal(ops:Dict)->TopoCriteriaModal:
    """Create Modal criterion

    The xml version expected is
    <Objective  type="Modal"  name="Modal" useProblem="1" NUM_MODE_OPTIM="1"/>


    Parameters
    ----------
    ops : Dict
        modal criterion parameter, NUM_MODE_OPTIM (mandatory) refers to the mode arising from the modal analysis to be optimized.
        For instance, NUM_MODE_OPTIM="1" means the criteria take into account the first mode.

    Returns
    -------
    TopoCriteriaModal
        Modal criterion instance
    """
    res=TopoCriteriaModal()
    res.params={"NumModeOptim": PH.ReadInt(ops["NUM_MODE_OPTIM"])}
    del(ops["NUM_MODE_OPTIM"])
    PH.ReadProperties(ops,ops.keys(),res)
    return res


class TopoCriteriaModal(PhysicalCriteriaBase):
    """
    Physical criterion for the modal analysis.
    """
    def __init__(self,other:Optional[TopoCriteriaModal]=None):
        """Constructor of criterion

        Parameters
        ----------
        other : Optional[TopoCriteriaModal], optional
            another instance of this very criteria to copy attributes from, by default None
        """
        super(TopoCriteriaModal, self).__init__(other)

        self.update = True
        self._params=None

        if other is None:
            self.SetName("Modal")
        else:
            self.f_val = other.f_val
            self.fSensitivity_val = np.copy(other.fSensitivity_val)
            self._params = other._params

    @property
    def params(self):
        return self._params

    @params.setter
    def params(self, value:Dict):
        """Criterion parameters setter

        Parameters
        ----------
        value : Dict
            parameters value

        Raises
        ------
        ValueError
            If a parameter provided is not available
        ValueError
            If a mandatory parameter is missing
        """
        expectedCriteriaParameters=["NumModeOptim"]
        #Pseudo Invariants
        if any(Parameter not in expectedCriteriaParameters for Parameter in value.keys()):
            raise ValueError("The following parameters are not available for a modal-related criteria ",value.keys()-set(expectedCriteriaParameters))
        elif any(Parameter not in value.keys() for Parameter in expectedCriteriaParameters):
            raise ValueError("The following parameters were expected for a modal-related criteria but were not provided",set(expectedCriteriaParameters)-value.keys())
        elif value["NumModeOptim"]>self.problem.modalParams["Numbermaxfreq"]:
            raise ValueError("The criteria can not ask for more eigenvalues than the physical analysis!")
        else:
            self._params = value


    def GetNumberOfSolutions(self):
        """Get number of criterion solutions, here the number of modes

        Returns
        -------
        int
            number of solutions
        """
        return self.problem.GetNumberOfSolutions()

    def GetCriteriaSolution(self,i:int)->np.ndarray:
        """Retrieve criterion solution

        Parameters
        ----------
        i : int
            solution id for the physical problem

        Returns
        -------
        np.ndarray
            i-th solution of the physical problem
        """
        return self.problem.GetNodalSolution(i)

    def GetCriteriaSolutionName(self,i):
        """Retrieve criterion solution name

        Parameters
        ----------
        i : int
            i-th solution of the physical problem

        Returns
        -------
        str
            i-th solution name
        """
        return f"DepModal_{i}"

    def GetScalarsOutputs(self)->Dict[str,float]:
        """Retrieve scalar outputs from problem. The quantity retrieved are the eigenfrequencies from the modal analysis.

        Returns
        -------
        Dict[str,float]
            Scalar outputs by scalar outputs respective names
        """
        scalarOutputs={}
        pulsSquared = self.problem.GetAuxiliaryScalarsbyOrders(FN.EigenFreqSquared)
        frequencies=[math.sqrt(-pulsSqu)/(2*math.pi) for pulsSqu in pulsSquared]
        scalarOutputs={"frequency_"+str(indexFrequency+1):frequency for indexFrequency,frequency in enumerate(frequencies)}
        return scalarOutputs

    def SetAuxiliaryQuantities(self,levelSet:LevelSet):
        """Define auxiliary physical quantites to be computed by the physical analysis for the modal criterion.
        In this case, both the criterion (scalar, EigenFreqSquared) and the sensitivity (field,EigenFreqSquared_sensitivity) are to be computed by the physical solver.

        Parameters
        ----------
        levelSet : LevelSet
            levelset object
        """
        self.problem.SetAuxiliaryFieldGeneration(FN.EigenFreqSquared_sensitivity,on=FN.Nodes)
        self.problem.SetAuxiliaryScalarsbyOrderGeneration(FN.EigenFreqSquared,listSize=self.problem.modalParams["Numbermaxfreq"])
        assert self.params is not None,"This criteria can not be computed without its associated parameters."
        self.problem.criteriaParams=self.params

    def UpdateValues(self,point:LevelSet):
        r"""Update value and sensitivity of the criterion for the current levelset

        At each update, a modal problem must be solved. We are looking for the eigenpulsations :math:`\omega_j > 0` and its associated eigenmodes :math:`r_j` such that

        .. math::

            \begin{equation}\label{eq:systemdirectmodal}
            \left\{
            \begin{array}{ccccc}
            -\text{div}(\sigma (r_j))&=&\omega_j^2\rho r_j&\text{in}&\Omega,\\
            r_j&=&0&\text{on}&\Gamma_D,\\
            \sigma (r_j) \cdot n&=&0&\text{on}&\Gamma,\\
            \end{array}
            \right.
            \end{equation}

        where:

        * :math:`\sigma` is the stress tensor satisfying the Hooke's law
        * :math:`\rho` is material the density
        * :math:`j` is the mode id
        * the body is clamped on :math:`\Gamma_D`

        The criteria is computed as follows

        .. math::

            J_k(\Omega)= -\omega_k^2.

        where :math:`\omega_k > 0` is the k-th eigenpulsation.
        For the sake of completeness, we add that this is an auto-adjoint problem.

        Parameters
        ----------
        point : LevelSet
            levelset instance

        Returns
        -------
        int
            paramater indicating if the update was successfull
        """
        self.point = point
        pulsSquared = self.problem.GetAuxiliaryScalarsbyOrders(FN.EigenFreqSquared)
        self.f_val=pulsSquared[self.params["NumModeOptim"]-1]
        self.fSensitivity_val = self.problem.GetAuxiliaryField(FN.EigenFreqSquared_sensitivity,on=FN.Nodes).flatten()
        return RETURN_SUCCESS


RegisterCriteriaClass("Modal", TopoCriteriaModal)


def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
