# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
""" 
This module provide a factory to register all the optimization algoriths.
"""
from typing import Optional,Type,Dict,Any

from Muscat.Helpers.Factory import Factory

def RegisterClass(name:str, classtype:Type, constructor:Optional[callable]=None, withError:bool = True)->None:
    """Register class

    Parameters
    ----------
    name : str
        key to retrieve the instances of classType
    classtype : Type
        A type of a class
    constructor : Optional[callable], optional
        dedicated constructor, by default None
    withError : bool, optional
        if false then the user can override a existent class/constructor , by default True

    """
    return OptimAlgoFactory.RegisterClass(name,classtype, constructor=constructor, withError = withError )

def Create(name:str,ops:Optional[Dict]=None)-> Any:
    """Create optimization algorithm instance from factory

    Parameters
    ----------
    name : str
        class name registered in factory
    ops : Optional[Dict], optional
        options, by default None

    Returns
    -------
    Any
        optimization algorithm instance
    """
    return OptimAlgoFactory.Create(name,ops)


class OptimAlgoFactory(Factory):
    _Catalog = {}
    _SetCatalog = set()

    def __init__(self):
        super(OptimAlgoFactory,self).__init__()

def InitAllAlgos():
    """Activate registration of all optimization algorithms (thus, RegisterClass should be called in the imported file).
    If a new optimization algorithm is added, an import of the suitable file must be added here
    """
    import OpenPisco.Optim.Algorithms.OptimAlgoUnconstrained
    import OpenPisco.Optim.Algorithms.OptimAlgoNullSpace


def CheckIntegrity():
    return "OK"

if __name__ == '__main__':
        print(CheckIntegrity())
