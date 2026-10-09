# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

# -*- coding: utf-8 -*-
""" 
This module is dedicated to the factory for handling criteria
"""
from __future__ import annotations

from typing import Type,Optional,Callable,Any

from Muscat.Helpers.Factory import Factory
import Muscat.Helpers.ParserHelper as PH

from OpenPisco.Optim.Criteria.Criteria import CriteriaBase

def RegisterClass(name:str, classtype:Type, constructor:Optional[Callable]=None, withError:bool=True):
    """
    Allow to save a mapping between a criterion name and the associated criterion.

    Examples: 

    class MySimpleCriterion(CriteriaBase):
     def __init__(self, other=None):
        super(MySimpleCriterion, self).__init__(other)
        self.SetName("MySimpleCriterion")

     def GetNumberOfSolutions(self):
        return 0

     def UpdateValues(self,levelSet):
        # Compute here criterion value
        self.f_val = criterionValue
        
        # Compute here sensitivity value
        self.fSensitivity_val = criterionSensitivityValue
        return RETURN_SUCCESS

    RegisterCriteriaClass("MyInterestingCriterion", MySimpleCriterion)
            

    class MyComplexCriterion(ActionBase):
        def __init__(self):
            super(MyComplexCriterion,self).__init__()
            self.params = None

        def GetNumberOfSolutions(self):
            return 0

        def UpdateValues(self,levelSet):
            # Compute here criterion value that use self.params
            self.f_val = criterionValue
            
            # Compute here sensitivity value that use self.params
            self.fSensitivity_val = criterionSensitivityValue
            return RETURN_SUCCESS

    def CreateMyComplexCriterion(params)
        criterion = MyComplexCriterion()
        criterion.params = params
        return criterion

    RegisterClass(name="MyComplexOperation", classtype=MyComplexCriterion,constructor=CreateMyComplexAction)


    Parameters
    ----------
    name : str
        Criterion name
    classtype : Type[T]
        criterion associated to the criterion name
    constructor : Optional[Callable], optional
        Constructor associated to the criterion, by default None
    withError : bool, optional
        if false then the user can override an existent class/constructor , by default True
    """
    return CriteriaFactory.RegisterClass(name, classtype, constructor=constructor, withError=withError)

def Create(name:str, ops:Optional[Any]=None)->CriteriaBase:
    """Create a instance of a class associated to the criterion name. The class must have been registered first.

    Parameters
    ----------
    name : str
        Criterion name to instantiate
    ops : Optional[Dict], optional
        Parameters required by the criterion, by default None

    Returns
    -------
    CriteriaBase
        Criterion instance
    """
    res = CriteriaFactory.Create(name,ops)

    ## Just assign props without convertion
    props = ["problem"]
    PH.ReadProperties(ops, props, res, typeConversion=False)
    ## delete the already asigned info
    for key in props:
        if key in ops:
          del ops[key]

    PH.ReadProperties(ops, ops, res, typeConversion=True)

    return res

class CriteriaFactory(Factory):
    _Catalog = {}
    _SetCatalog = set()

    def __init__(self):
        super(CriteriaFactory, self).__init__()

def InitAllCriteria():
    """In order to initialize the criteria factory, all the modules calling the 'RegisterClass' function must be imported here"""
    import OpenPisco.Optim.Criteria.ComposedCriteria
    import OpenPisco.Optim.Criteria.GeoCriteria
    import OpenPisco.Optim.Criteria.GeoThicknessCriteria
    import OpenPisco.Optim.Criteria.PhyCriteria
    import OpenPisco.Optim.Criteria.PhyMecaCriteria
    import OpenPisco.Optim.Criteria.PhyThermalCriteria
    import OpenPisco.Optim.Criteria.PhyModalCriteria
    import OpenPisco.Optim.Criteria.PhyHarmonicCriteria
    import OpenPisco.Optim.Criteria.PhyRobustCriteria
    import OpenPisco.Optim.Criteria.PhyBucklingCriteria
    import OpenPisco.Optim.Criteria.AMInherentStrainCriteria
    import OpenPisco.Optim.Criteria.PhyFluidCriteria


def CheckIntegrity():
    return "OK"

if __name__ == '__main__':
        print(CheckIntegrity())
