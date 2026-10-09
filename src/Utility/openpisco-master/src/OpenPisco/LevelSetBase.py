# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import abc

import numpy as np

from Muscat.Types import MuscatFloat

class LevelSetBase(metaclass=abc.ABCMeta):
    def __init__(self, other=None, support=None):
        self.support = None
        self.originalSupport = None
        self.phi  = None
        self.savePhis = []
        self.extraNodesFields = {}
        self.conform = False
        self.filterLevelSetField = False

        if other is None:
            if support is not None:
                self.SetSupport(support)
            self.conform = False
        else:
            self.SetSupport(other.support)
            self.conform = other.conform

    def StoreState(self):
        self.savePhis.append(np.copy(self.phi))

    def DiscardState(self):
        if len(self.savePhis) == 0:
            raise ValueError(" You must call StoreState before") 
        self.phi = self.savePhis.pop()

    def AcceptChanges(self,newphi=None):
        if len(self.savePhis) == 0:
            raise ValueError(" You must call StoreState before")
        self.savePhis.pop()
        if newphi is not None:
            self.phi[:] = newphi


    def SetSupport(self,support):
        self.support = support
        self.originalSupport = None
        self.phi = np.empty((self.support.GetNumberOfNodes(),))
        np.copyto(self.phi, -1.0)
        self.extraNodesFields = {}

    def IsNodal(self):
        return True

    def ResetState(self):
        self.savePhis = []

    @abc.abstractmethod
    def InterfaceIntegral(self, field):
        """
        The value of the surface integral applied to the trace of a
        scalar-valued field defined on the whole design domain.
        """
        pass

    def Initialize(self, Function):
        xyz = self.support.GetPosOfNodes()
        self.phi = np.copy(Function(xyz))

    def InitializePointByPoint(self, Function):
        if self.phi is None or len(self.phi) != self.support.GetNumberOfNodes():
            self.phi = np.empty(self.support.GetNumberOfNodes(), dtype=MuscatFloat)

        for i in range(self.support.GetNumberOfNodes()):
           xyz = self.support.nodes[i,:]
           self.phi[i] = Function(xyz)

    @abc.abstractmethod
    def Regularize(self, field, lengthscaleParameter=None, extra=None, on=[]):
        pass

    @abc.abstractmethod
    def TransportAndReinitialize(self, velocity, velocity_normalization=1.0):
        pass

    @abc.abstractmethod
    def Reinitialize(self, length=None):
        """
        Reinitialize the levelset function as a signed distance.

        The levelset function guaranteed to be a signed distance function at
        least at a distance of `length` from the interface.

        Parameters
        ----------
        length: real, optional
            sufficient length of redistanciation, expressed as an absolute
            distance from the interface
        """
        pass

    def __str__(self):
        return "size of self.phi : " + str(self.phi.size)

    def GetNumberOfDesignVariables(self):
        return self.support.GetNumberOfNodes()

    def TakeValuesFrom(self,other):
        self.phi = np.copy(other.phi)
        self.extraNodesFields = {}
        for name in other.extraNodesFields:
            self.extraNodesFields[name]  = np.copy(other.extraNodesFields[name])
        self.support = other.support

def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity()) # pragma: no cover
