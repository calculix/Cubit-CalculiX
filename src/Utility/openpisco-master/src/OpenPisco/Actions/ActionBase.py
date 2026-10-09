# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

"""Base class object from which all the actions of OpenPisco are inherited
"""
import abc
from typing import Dict

from OpenPisco.CLApp.OpenPiscoCL import MainApp

class ActionBase(metaclass=abc.ABCMeta):
    """ Base class for all the actions"""
    needToCancel = False

    @classmethod
    def SetNeedToInterrupt(cls):
        cls.needToCancel = True

    @classmethod
    def NeedToInterrupt(cls,keepState:bool = False)->bool:
        if cls.needToCancel:
            if not keepState:
                cls.needToCancel = False
            return True
        return False

    def __init__(self):
       super(ActionBase, self).__init__()

    @abc.abstractmethod
    def Apply(self, app:MainApp, ops: Dict):
        """Operation fulfilled by the action, 
        to be redefined by derived class and should return OpenPisco.RETURN_SUCCESS

        Parameters
        ----------
        app : MainApp
            OpenPisco application
        ops : Dict
            Parameters required by the action
        """
        pass

def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
