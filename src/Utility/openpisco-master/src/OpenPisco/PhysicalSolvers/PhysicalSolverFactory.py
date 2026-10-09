# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

from Muscat.Helpers.Factory import Factory

def RegisterClass(name, classtype, constructor=None, withError = True):
    return PhysicalSolverFactory.RegisterClass(name,classtype, constructor=constructor, withError = withError )

def Create(name,ops=None):
   res = PhysicalSolverFactory.Create(name,ops)
   return res

class PhysicalSolverFactory(Factory):
    _Catalog = {}
    _SetCatalog = set()

    def __init__(self):
        super(PhysicalSolverFactory,self).__init__()

def InitAllPhysicalSolvers():
    import OpenPisco.PhysicalSolvers.GeneralZSetSolverMeca
    import OpenPisco.PhysicalSolvers.StructuredFEAMecaSolver
    import OpenPisco.PhysicalSolvers.UnstructuredFEAGenericSolver
    import OpenPisco.PhysicalSolvers.StaticFreeFemSolverMeca
    import OpenPisco.PhysicalSolvers.GeneralAsterPhysicalSolver
    import OpenPisco.PhysicalSolvers.AMThermalDilatationSolver
    import OpenPisco.PhysicalSolvers.AMInherentStrainSolver
    import OpenPisco.PhysicalSolvers.SolverWithSimulationDomain
    import OpenPisco.PhysicalSolvers.OpenFoamSteadyStateComplete
    import OpenPisco.PhysicalSolvers.OpenFoamSteadyState
    import OpenPisco.PhysicalSolvers.AnsysMechanicalThermal

def CheckIntegrity():
    InitAllPhysicalSolvers()
    return "OK"

if __name__ == '__main__':
        print(CheckIntegrity())
