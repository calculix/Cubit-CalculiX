# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

"""Factory for handling actions"""
from __future__ import annotations

from typing import Optional, Dict
from collections.abc import Callable
from typing import TypeVar, Type

T = TypeVar('T')

from Muscat.Helpers.Factory import Factory

from OpenPisco.Actions.ActionBase import ActionBase

def RegisterClass(name:str, classtype:Type[T], constructor:Optional[Callable]=None, withError:bool = True):
    """
    Allow to save a mapping between an action name and the associated operation.

    Examples: 
    
    class MySimpleOperationAction(ActionBase):
        def __init__(self):
            super(MySimpleOperationAction,self).__init__()

        def Apply(self, app, ops):
            #Do something here
            return RETURN_SUCCESS

    RegisterClass(name="MySimpleOperation", classtype=MySimpleOperationAction)

    class MyComplexOperationAction(ActionBase):
        def __init__(self,initOps):
            super(MyComplexOperationAction,self).__init__()
            self.initOps = initOps

        def Apply(self, app, ops):
            #Do something here
            return RETURN_SUCCESS

    def CreateMyComplexAction(initOps)
        action = MyComplexAction(initOps)
        return action

    RegisterClass(name="MyComplexOperation", classtype=MyComplexOperationAction,constructor=CreateMyComplexAction)


    Parameters
    ----------
    name : str
        Action name
    classtype : Type[T]
        Operation associated to the action name
    constructor : Optional[Callable], optional
        Constructor associated to the operation, by default None
    withError : bool, optional
        if false then the user can override an existent class/constructor , by default True
    """
    return ActionFactory.RegisterClass(name,classtype, constructor=constructor, withError = withError )


def Create(name:str,ops:Optional[Dict]=None)->ActionBase:
    """Create a instance of a class associated to the action name. The class must have been registered first.

    Parameters
    ----------
    name : str
        Action name to instantiate
    ops : Optional[Dict], optional
        Parameters required by the action, by default None

    Returns
    -------
    ActionBase
        Action instance
    """
    return ActionFactory.Create(name,ops)

class ActionFactory(Factory):
    _Catalog = {}
    _SetCatalog = set()

    def __init__(self):
        super(ActionFactory,self).__init__()

def InitAllActions():
    """In order to initialize the action factory, all the modules calling the 'RegisterClass' function must be imported here"""
    import OpenPisco.Actions.GeneralActions
    import OpenPisco.Actions.UnstructuredActions
    import OpenPisco.Actions.FieldsActions
    import OpenPisco.Actions.OptimAlgoActions
    import OpenPisco.Actions.OptimCriteriaActions
    import OpenPisco.Actions.MeshActions
    import OpenPisco.Actions.AdditiveManufacturingActions
    import OpenPisco.Actions.PhysicalSolversActions


def CheckIntegrity():
    InitAllActions()
    return "ok"

if __name__ == '__main__':
        print(CheckIntegrity())
