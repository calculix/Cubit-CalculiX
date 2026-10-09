# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from Muscat.Helpers.Factory import Factory

def RegisterAsterClass(name, classtype, constructor=None, withError = True):
    return GeneralAsterFactory.RegisterClass(name,classtype, constructor=constructor, withError = withError )

def CreateAsterAnalysis(name,ops=None):
   res = GeneralAsterFactory.Create(name,ops)
   return res

def InitAllPhysicalSolvers():
    import OpenPisco.PhysicalSolvers.AsterModal
    import OpenPisco.PhysicalSolvers.AsterHarmonicForced
    import OpenPisco.PhysicalSolvers.AsterStatic
    import OpenPisco.PhysicalSolvers.AsterBuckling
    import OpenPisco.PhysicalSolvers.AsterThermal

class GeneralAsterFactory(Factory):
    _Catalog = {}
    _SetCatalog = set()

    def __init__(self):
        super(GeneralAsterFactory,self).__init__()

def CheckIntegrity():
    InitAllPhysicalSolvers()
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity())
