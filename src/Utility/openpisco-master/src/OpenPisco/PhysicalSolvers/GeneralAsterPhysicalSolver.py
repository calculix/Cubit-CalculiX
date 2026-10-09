# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import Muscat.Helpers.ParserHelper as PH

from OpenPisco.PhysicalSolvers.AsterAnalysisSolverFactory import InitAllPhysicalSolvers,CreateAsterAnalysis
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import RegisterClass


"""
Xml interface
<XXXXXXXX id="X" type="[static_elastic|modal_elastic|harmonic_forced_elastic]"  p="*1|2" hasPreStress="*False|True >

  <UseTemplateFile name="" />

    <Material    eTag="*everyelement|eTag" young="1" poisson="0.3" density="1" />
    <Dirichlet   eTag="eTag" dofs="[012]" value="[float]" />
    <Dirichlet   eTag="eTag" aVoir[nTag=""] u1="0" />
    <Acceleration     eTag="*everyelement|eTag" g="3*float" />
    <Centrifugal eTag="*everyelement|eTag" point="3*float" axis="3*float" angularSpeed="float" />
    <Force [eTag="ET9"|nTag=""] value="3*float" />
    <Force  eTag="eTag_Name_3" phi="4.5" theta="5.6" mag="2" >
    <TotalForce  eTag="eTag_Name_3" phi="4.5" theta="5.6" mag="2" >
  <Pressure  eTag="2D_eTag"  value="0.0" />

    0.*<LoadCase id="1" >
    0.*<Gravity>
    0.*<Centrifugal>
    0.*<Force>
    0.*<Pressure>
  </LoadCase>

</XXXXXXXX>
"""

def CreateAsterPhysicalProblemMeca(ops):
     InitAllPhysicalSolvers()
     physic = PH.ReadString(ops['type'])
     res=CreateAsterAnalysis(name=physic,ops=ops)
     return res

RegisterClass(name="GeneralAster",classtype=None,constructor=CreateAsterPhysicalProblemMeca)


def CheckIntegrity_2DXML(GUI=False):
    from OpenPisco.CLApp.XmlToDic import XmlToDic
    from OpenPisco.PhysicalSolvers.AsterStatic import AsterStatic

    teststringField=u"""
    <GeneralAster type="static_elastic" p="1">
              <Material  young="210000000000" poisson="0.3" />
              <Dirichlet eTag="X0" dofs="0 1" value="0.0"/>
              <Force eTag="X1" value="0. -10000." />
    </GeneralAster>
    """
    teststringField = PH.ApplyGlobalDictionary(teststringField)
    reader = XmlToDic()
    _, ops = reader.ReadFromString(teststringField)
    ops["dimensionality"]=2
    phyProblem=CreateAsterPhysicalProblemMeca(ops)


    materials = [['AllZones',{"young":210000000000,"poisson":0.3}]]
    dirichlet = [['X0',(0.,0.)]]
    problems = ['idx1']
    neumann = {"idx1": [['X1',(0.,-10000)]]}

    phyProblemRef = AsterStatic()
    phyProblemRef.materials = materials
    phyProblemRef.dirichlet= dirichlet
    phyProblemRef.problems = problems
    phyProblemRef.neumann= neumann

    phyProblemRef2 = AsterStatic()
    phyProblemRef2.materials = materials
    phyProblemRef2.dirichlet= [['X0',(0.,1.)]]
    phyProblemRef2.problems = problems
    phyProblemRef2.neumann= neumann

    phyProblemRef3 = AsterStatic()
    phyProblemRef3.materials = materials
    phyProblemRef3.dirichlet= dirichlet
    phyProblemRef3.problems = problems
    phyProblemRef3.neumann= {"idx1": [['X1',(0.,-100000)]]}

    assert phyProblem==phyProblemRef, "Physical problems should be the same!"
    assert phyProblem!=phyProblemRef2, "Physical problems should not be the same!"
    assert phyProblem!=phyProblemRef3, "Physical problems should not be the same!"

    return "ok"

def CheckIntegrity_3DXML(GUI=False):
    from OpenPisco.CLApp.XmlToDic import XmlToDic

    teststringField=u"""
    <GeneralAster type="static_elastic">
              <Material  young="210000000000" poisson="0.3" />
              <Dirichlet eTag="ET1" dofs="0 1 2" value="0.0"/>
              <Dirichlet eTag="ET2" dofs="0" value="0.0"/>
              <Dirichlet eTag="ET7" dofs="2" value="0.0"/>
              <Force eTag="ET3" value="0. -10000. 0." />
    </GeneralAster>
    """
    teststringField = PH.ApplyGlobalDictionary(teststringField)
    reader = XmlToDic()
    _, ops = reader.ReadFromString(teststringField)
    CreateAsterPhysicalProblemMeca(ops)
    return "ok"

def CheckIntegrity(GUI=False):
    totest = [
        CheckIntegrity_3DXML,
        CheckIntegrity_2DXML
              ]
    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
