# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

# This module provides several renormalization approaches for semi-infinite criteria.
# The idea is the following: the p-norm used classically to aggregate the local values for semi infitine criterion may not be an accurate representation of the max,
# in particular it lacks physical meaning. In order to adress such an issue, we use a normalized global measure to better approximate the maximum.
# It also relies on information from the previous optimization iteration to scale, that is to say normalize the global p-norm measure so that
# it better approximate the maximum
#
# Based on article Le, C., Norato, J., Bruns, T. et al. Stress-based topology optimization for continua.
# Struct Multidisc Optim 41, 605–620 (2010). https://doi.org/10.1007/s00158-009-0440-y
#
from __future__ import annotations

from typing import Dict,Type,Optional

import numpy as np

from OpenPisco.Unstructured.Levelset import LevelSet
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS

def Renormalization(criteria:Type,submethod:str,params:Dict)->Type:
    """Decorator of criteria for renormalizing

    Parameters
    ----------
    criteria : Type
        semi-infinite criteria class
    submethod : str
        name of the submethod to be used for defining the scaling
    params : Dict
        parameters specific to the Renormalization method

    Returns
    -------
    NormalizedCriteria
        a semi-infinite criteria instance decorated by the renormalization

    """
    class NormalizedCriteria(criteria):
        def __init__(self, other:Optional[NormalizedCriteria]=None):
            """Constructor of criterion

            Parameters
            ----------
            other : Optional[NormalizedCriteria], optional
                another instance of this very criteria to copy attributes from, by default None
            """
            super(NormalizedCriteria, self).__init__(other)
            self.method="Renormalization"
            self.submethod=submethod
            self.normalizationParameters = params

            if other == None :
                self.normalizationCoeff = 1
                self.lbda = 1
                self.initNormalization=False

            else:
                self.normalizationCoeff = other.normalizationCoeff
                self.submethod = other.submethod
                self.normalizationParameters = other.normalizationParameters
                self.lbda = other.lbda
                self.initNormalization=other.initNormalization

        @property
        def normalizationCoeff(self)->float:
            return self._normalizationCoeff

        @normalizationCoeff.setter
        def normalizationCoeff(self, normalizationCoeff:float):
            """Setter of normalization coefficient

            Parameters
            ----------
            normalizationCoeff : float
                normalization coefficient
            """
            self._normalizationCoeff = normalizationCoeff

        def ComputeRatioNorm(self)->float:
            """Compute criterion ratio to consider for the next criterion update.
            It involves the maximal value of the underlying local value used for semi-infinite criterion (for instance the actual von-mises field rather than a L2 norm)
            and the current aggrgated criteria value, taking into account the normalization coefficient

            Returns
            -------
            float
                criterion ratio

            Raises
            ------
            ValueError
                Criterion value is too close to 0
            """
            assert self.localValueSupport==FN.IPField,"localValueSupport on"+str(self.localValueSupport)+" not handled"
            maxCriteria = np.max( [np.max(self.localValues.data[name]) for name in self.localValues.data.keys() ])

            valueCriteria = self.f_val/self.normalizationCoeff
            if np.isclose(valueCriteria,0.0):
                raise ValueError("valueCriteria is too close to 0.0; it should not happen.")

            ratioNorm = maxCriteria/valueCriteria
            return ratioNorm

        def UpdateNormalizationCoeff(self):
            """Update normalization coefficients"""
            ratioNorm=self.ComputeRatioNorm()
            newlbda=self.ComputeNewLambda(ratioNorm)
            C_I = self.normalizationCoeff
            C_Ip1 = ratioNorm*newlbda + (1-newlbda)*C_I
            self.lbda=newlbda

            if self.submethod=="RegularizeOverIterations" or self.submethod=="RegularizeIfCloseEnough":
                # we protect C_I from increasing too fast
                if C_Ip1 > 2.*C_I :
                    C_Ip1 = 2.*C_I

            self.normalizationCoeff=C_Ip1

        def ComputeNewLambda(self,ratioNorm:float)->float:
            """Compute new lambda coefficient

            Parameters
            ----------
            ratioNorm : float
                ratio norm value

            Returns
            -------
            float
                new lambda coefficient value
            """
            C_I = self.normalizationCoeff
            oldlbda=self.lbda
            assert self.submethod in ["MaximumVariationNormalisation","RegularizeOverIterations","RegularizeIfCloseEnough"],"submethod not available"
            if self.submethod=="MaximumVariationNormalisation":
                percentTol = self.normalizationParameters["percentTol"]
                if not( C_I*(1.-percentTol) < ratioNorm < C_I*(1.+percentTol) ):
                    newlbda = abs(C_I/(ratioNorm-C_I)*percentTol)
                else:
                    newlbda = 1

            elif self.submethod=="RegularizeOverIterations":
                lbdaFactor = self.normalizationParameters["lbdaFactor"]
                assert 0 <= lbdaFactor <= 1,"lbdaFactor must be taken between 0 and 1."
                newlbda = max(oldlbda*lbdaFactor, 0.01)

            elif self.submethod=="RegularizeIfCloseEnough":
                decreaseLbdaFactor = self.normalizationParameters["decreaseLbdaFactor"]
                assert 0. < decreaseLbdaFactor < 1.,"decreaseLbdaFactor must be taken between 0 and 1."
                increaseLbdaFactor = self.normalizationParameters["increaseLbdaFactor"]
                assert increaseLbdaFactor > 1.,"increaseLbdaFactor should be greater than 1."
                closeEnoughRatio = self.normalizationParameters["closeEnoughRatio"]
                isCloseEnough = not( C_I*(1.-closeEnoughRatio) < ratioNorm < C_I*(1.+closeEnoughRatio) )
                if isCloseEnough:
                    newlbda = max(0.01,oldlbda*decreaseLbdaFactor)
                else:
                    newlbda = min(1.,oldlbda*increaseLbdaFactor)
            return newlbda

        def SetNormalizationParameters(self,newnormalizationParameters:Dict):
            """Set normalization parameters

            Parameters
            ----------
            newnormalizationParameters : Dict
                normalization parameters
            """
            self.normalizationParameters=newnormalizationParameters

        def UpdateCriteriaWhenIterationAccepted(self):
            """When an optimization iteration is accepted, the criterion normalization coefficient is updated and its value/sensitivity are also updated accordingly"""
            oldCoeff=self.normalizationCoeff
            self.UpdateNormalizationCoeff()
            newCoeff=self.normalizationCoeff
            self.ReNormalizations(newCoeff/oldCoeff)

        def ReNormalizations(self,normCoeff:float):
            """Renormalization of value/sensitivity

            Parameters
            ----------
            normCoeff : float
                normalization coefficient
            """
            self.f_val=self.f_val*normCoeff
            self.fSensitivity_val=self.fSensitivity_val*normCoeff

        def UpdateValues(self,levelSet:LevelSet):
            """Update value and sensitivity of the criterion for the current levelset

            At each update, the criterion is updated (calling original "undecorated" criterion update) and then a renormalization procedure is applied.
            Except when this method is called for the first time, the normalization coefficient is corrected based on the ratio norm.

            Parameters
            ----------
            levelSet : LevelSet
                levelset object

            Returns
            -------
            int
                paramater indicating if the update was successfull
            """
            super(NormalizedCriteria, self).UpdateValues(levelSet)
            self.ReNormalizations(self.normalizationCoeff)
            if not self.initNormalization:
                self.normalizationCoeff=self.normalizationCoeff*self.ComputeRatioNorm()
                self.ReNormalizations(self.normalizationCoeff)
                self.initNormalization=True

            return RETURN_SUCCESS

    return NormalizedCriteria()

def CheckIntegrity():
    return "ok"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
