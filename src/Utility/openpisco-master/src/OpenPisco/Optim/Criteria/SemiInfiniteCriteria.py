# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

# This module provides some basic features to handle the so-called semi-infinite criteria,
# that is to say criteria whose value is not defined globally but localy, i.e. we expect a given condition
# to be fulfilled for each local value living on a specific geometrical support (be it nodes, elements, integration points etc)...
#
# However, we expect the user to provide both the local values and a way to aggregate in a single value these local values (P-norm for instance) in the original criteria.
# If the semi-infinite criteria methods called here are not used, the default behaviour is to use the aggregation method proposed in the original criterion.
#
# Note: all the methods proposed here assume that the criteria considered is to be used as a constraint, they are no feature available to do otherwise.
from __future__ import annotations

from typing import List,Type,Optional

from OpenPisco.Optim.Criteria.Criteria import PhysicalCriteriaBase
from OpenPisco.Optim.Criteria.Regionalization import Regionalization
from OpenPisco.Optim.Criteria.Renormalization import Renormalization

class SemiInfiniteCriteriaBase(PhysicalCriteriaBase):
    """Base class for semi-infinite criteria, it is a specialization of PhysicalCriteriaBase to take into account local values."""
    def __init__(self, other:Optional[SemiInfiniteCriteriaBase]=None):
        super(SemiInfiniteCriteriaBase, self).__init__(other)
        self.localValues=None
        self.method=None
        self.localValueSupport=None
        self.computationalElementFilter=None
        self.dimensionalitySupport=0

    @classmethod
    def CriteriaOperation(cls:Type,*args,**nargs)->Type:
        """The main function of this class, it decorates a criteria class

        Parameters
        ----------
        cls : Type
            a semi-infinite criteria class

        Returns
        -------
        Type
            a semi-infinite criteria instance decorated with the specific method chosen to handle local values
        """
        methodByName = {"Regionalization":Regionalization,"Renormalization":Renormalization }
        return methodByName[args[0]](cls,**nargs)

    def GetDiscreteImplementation(self)->List:
        """Accumulate the constraint(s) to be added in the optimization problem for each method to work

        Returns
        -------
        List
            discrete implementations of constraint specific for each method

        Raises
        ------
        ValueError
            if method provided is neither renormalization not regionalization
        """
        discrteImpl=[]
        if self.method is None:
            discrteImpl.append(self)
        elif self.method=="Regionalization":
            for regionID in range(self.numberOfRegions):
                objCritID=type(self)(self)
                objCritID.SetRegionID(regionID)
                objCritID.SetName("Reg"+str(self.GetName())+str(regionID))
                discrteImpl.append(objCritID)
        elif self.method=="Renormalization":
            discrteImpl.append(self)
        else:
            raise ValueError("Method not recognized")
        return discrteImpl

    def ComputeLocalValues(self):
        """Compute specific local values, to be redefined according to criteria"""
        raise NotImplementedError()

    def ComputeComputationalElementFilter(self):
        """Compute computational support, to be redefined according to criteria"""
        raise NotImplementedError()


def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
