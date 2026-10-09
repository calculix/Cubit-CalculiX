# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.Logger import Info
import Muscat.MeshContainers.ElementsDescription as ED

import OpenPisco.TopoZones as TZ
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.AsterSolverBase import AsterSolverBase,BoundaryConditionDefinitionError
from OpenPisco.PhysicalSolvers.AsterAnalysisSolverFactory import RegisterAsterClass
import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
import OpenPisco.ExternalTools.Aster.AsterModalWriter as AsterModalWriter

def CreateAsterModalPhysicalProblem(ops):
    res = AsterModal()
    del(ops['type'])

    for tag,child in ops["children"]:
        if tag.lower()=="analysisparams":
            analysisparams={
                      "Numbermaxfreq" :PH.ReadInt(child["Numbermaxfreq"])
                  }
            res.modalParams=analysisparams
        if tag.lower() == "material":
           if "eTag" in child:
              z = child["eTag"]
           else:
              z = "AllZones"
           material = {}
           material["young"] = PH.ReadFloat(child["young"])
           material["poisson"] = PH.ReadFloat(child["poisson"])
           material["density"] = PH.ReadFloat(child["rho"])
           res.materials.append( [z,material])
        elif tag.lower() == "dirichlet":
             if "eTag" in child:
                z = child["eTag"]
             if "dofs" in child and "value" in child:
                dofs = PH.ReadInts(child["dofs"])
                val = PH.ReadFloat(child["value"])
                U=[]
                for i in range(3):
                    if i in dofs:
                       U.append(val)
                    else:
                       U.append(None)
                res.dirichlet.append([z,U])
             else:
                raise BoundaryConditionDefinitionError("a set of dofs (dofs) and a value (value)")
    del ops["children"]
    PH.ReadProperties(ops,ops.keys(),res)
    return res

def AssertAlmostEqualDict(data1,data2):
    assert set(list(data1.keys()))==set(list(data2.keys()))
    for key in data1.keys():
        np.testing.assert_almost_equal(data1[key],data2[key])

class AsterModal(AsterSolverBase):
    def __init__(self):
      super(AsterModal,self).__init__()
      self.name = 'AsterModal'
      assert self.conform,"Ersatz material approach in dynamic not handled (yet!)"
      self.criteriaParams= None
      self.auxiliaryFieldGeneration[FN.EigenFreqSquared_sensitivity][FN.Nodes] = False
      self.auxiliaryScalarsbyOrderGeneration = {FN.EigenFreqSquared:False}

    @property
    def modalParams(self):
        return self._modalParams

    @modalParams.setter
    def modalParams(self, value):
        expectedParameters=["Numbermaxfreq"]
        if any(parameter not in expectedParameters for parameter in value.keys()):
            Info("The following parameters are not available for a modal analysis ",value.keys()-set(expectedParameters))
        elif any(parameter not in value.keys() for parameter in expectedParameters):
            raise Exception("The following parameters were expected for a modal analysis ",set(expectedParameters)-value.keys())
        self._modalParams=value

    def WriteParametersInput(self,writeFile):
        self.solverInterface.SetExtraFiles(True)
        AsterModalWriter.WriteMaterialParametersInput(writeFile,self.materials)
        dimensionality = self.originalSupport.GetElementsDimensionality()
        AsterCommonWriter.WriteDirichletParametersInput(writeFile,self.dirichlet,dimensionality)
        AsterCommonWriter.WriteVariable(writeFile=writeFile,
                                        variableName="NMAX_FREQ",
                                        variableValue="{'NMAX_FREQ' : "+str(self.modalParams['Numbermaxfreq'])+"}")
        if self.criteriaParams is not None:
            self.WriteCriteriaParametersInput(writeFile)

    def GetNodalSolution(self,i):
        solution = self.GetField("DEPL","nodes",i)
        self.u =self.ExtendingNodalFieldToOriginalSupport(solution)
        return self.u

    def GetNumberOfSolutions(self):
        return self.modalParams["Numbermaxfreq"]

    def WriteCriteriaParametersInput(self,writeFile):
        errorMessage = "The criteria can not ask for more eigenvalues than the physical analysis!"
        assert self.criteriaParams['NumModeOptim']<=self.modalParams['Numbermaxfreq'],errorMessage
        #Alternative using conversion in string to ignore detail of criteria (responsability of Aster script)
        writeFile.write("modalcritParams =")
        writeFile.write(str(self.criteriaParams)+"\n")

    def __eq__(self, other):
        AssertAlmostEqualDict(self.modalParams,other.modalParams)
        properties = [self.materials,self.dirichlet]
        otherProperties = [other.materials,other.dirichlet]
        for singleProperty,otherProperty in zip(properties,otherProperties):
            for singleProperty,singleOtherProperty in zip(sorted(singleProperty),sorted(otherProperty)):
                tagOrigin, expressionOrigin = singleProperty
                tagOther, expressionOther = singleOtherProperty
                if tagOrigin != tagOther:
                    return False
                try:
                    if isinstance(expressionOrigin,dict):
                        AssertAlmostEqualDict(expressionOrigin,expressionOther)
                    else:
                        np.testing.assert_almost_equal(expressionOrigin,expressionOther)
                except AssertionError:
                        return False
        return True

RegisterAsterClass("modal_elastic", AsterModal,CreateAsterModalPhysicalProblem)

def CheckIntegrity(GUI=False):
    from OpenPisco.ExternalTools.Aster.AsterInterface import SkipAsterTest
    skipTestPhase,message = SkipAsterTest()
    if skipTestPhase:
        return message

    from Muscat.MeshTools.MeshCreationTools import CreateCube
    from OpenPisco.CLApp.XmlToDic import XmlToDic
    from OpenPisco.Unstructured.Levelset import LevelSet

    mesh = CreateCube(dimensions=[5,5,5],spacing=[1./4, 1./4, 1./4],origin=[0, 0, 0],ofTetras=True)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.ExterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))

    tets = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tets.tags.CreateTag('Tipmass3').AddToTag(0)
    tets.tags.CreateTag('Tipmass2').AddToTag(1)
    tets.tags.CreateTag('Tipmass1',False).SetIds(np.arange(2,tets.GetNumberOfElements()))
    tets.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tets.GetNumberOfElements()))
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.ExterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)

    problemInXml = """<GeneralAster type="modal_elastic" id="1">
            <AnalysisParams Numbermaxfreq="3"/>
            <Material  eTag="Tipmass1" young="210000" poisson="0.3" rho="7.85e-09"/>
            <Material  eTag="Tipmass3" young="230000" poisson="0.3" rho="7.85e-09"/>
            <Material  eTag="Tipmass2" young="220000" poisson="0.3" rho="7.85e-09"/>
            <Dirichlet eTag="X0" dofs="0 1 2" value="0.0"/>
         </GeneralAster>"""

    import xml.etree.ElementTree as ET

    root2 = ET.fromstring(problemInXml)
    obj = XmlToDic()
    _,data2 = obj.XmlToDic(root2)
    data2.pop("id",None)
    PPM = CreateAsterModalPhysicalProblem(data2)

    PPM2 = AsterModal()
    PPM2.modalParams = {"Numbermaxfreq":3}
    material1={"young":210000.0,"poisson":0.3,"density":7.85e-09}
    material2={"young":220000.0,"poisson":0.3,"density":7.85e-09}
    material3={"young":230000.0,"poisson":0.3,"density":7.85e-09}
    PPM2.materials = [['Tipmass1',material1],['Tipmass2',material2],['Tipmass3',material3]]
    PPM2.dirichlet= [['X0',(0.,0.,0.)]]

    assert PPM==PPM2, "Problem described in XML and problem build using python should be the same"

    PPM.conform=True
    PPM.SetAuxiliaryFieldGeneration(FN.EigenFreqSquared_sensitivity,on=FN.Nodes)
    PPM.SetAuxiliaryScalarsbyOrderGeneration(FN.EigenFreqSquared,listSize=PPM.GetNumberOfSolutions())
    criteriaParams={"NumModeOptim":1}
    PPM.criteriaParams=criteriaParams

    print("Run modal analysis...")
    PPM.SolveByLevelSet(ls)
    PPM.GetAuxiliaryField(FN.EigenFreqSquared_sensitivity,on=FN.Nodes)
    print("First "+str(PPM.modalParams["Numbermaxfreq"])+" eigenfrequencies squared in conform: ",PPM.GetAuxiliaryScalarsbyOrders(FN.EigenFreqSquared))
    return "ok"

if __name__ == '__main__':
    import time
    stime = time.time()
    print(CheckIntegrity())
    print("Total Time " + str(time.time()-stime))
