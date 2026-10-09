# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
"""Factory for handling problems"""
from __future__ import annotations

from typing import TypeVar, Type, Optional, Callable, Dict
T = TypeVar('T')

from Muscat.Helpers.Factory import Factory
import Muscat.Helpers.ParserHelper as PH

from OpenPisco.Optim.Problems.OptimProblemBase import OptimProblemBase


def RegisterClass(name:str, classtype:Type[T], constructor:Optional[Callable]=None, withError:bool=True):
    """
    Allow to save a mapping between a problem name and the associated operation.

    Parameters
    ----------
    name : str
        Problem name
    classtype : Type[T]
        Operation associated to the problem name
    constructor : Optional[Callable], optional
        Constructor associated to the operation, by default None
    withError : bool, optional
        if false then the user can override an existent class/constructor , by default True
    """
    return ProblemFactory.RegisterClass(name, classtype, constructor=constructor, withError=withError)

def Create(name:str, ops:Optional[Dict]=None)->OptimProblemBase:
    """Create a instance of a class associated to the problem name. The class must have been registered first.

    Parameters
    ----------
    name : str
        problem name to instantiate
    ops : Optional[Dict], optional
        Parameters required by the problem, by default None

    Returns
    -------
    OptimProblemBase
        problem instance
    """
    res = ProblemFactory.Create(name,ops, propertiesAssign = False)

    ## Just assign props
    props = ["OnZone", "OffZone"]
    PH.ReadProperties(ops, props, res, typeConversion=False)

    ## delete the already asigned info
    for key in props:
       del ops[key]

    if "ls" in ops:
       res.point = ops["ls"]
       del ops["ls"]

    PH.ReadProperties(ops, ops, res, typeConversion=True)

    return res

def CreateDerived(name:str, ops:Optional[Dict]=None)->OptimProblemBase:
   ops["internalOptimProblem"] = ops.get("optimProblem", None)
   del ops["optimProblem"]
   res = ProblemFactory.Create(name)
   PH.ReadProperties(ops, ops, res, typeConversion=True)
   return res

class ProblemFactory(Factory):
    _Catalog = {}
    _SetCatalog = set()

    def __init__(self):
        super(ProblemFactory, self).__init__()

def InitAllProblems():
    """In order to initialize the problem factory, all the modules calling the 'RegisterClass' function must be imported here"""
    import OpenPisco.Optim.Problems.OptimProblemTopoGeneric

def CheckIntegrity():
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity())
