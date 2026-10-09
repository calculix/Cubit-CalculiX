# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np
import math
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.Helpers.Logger import Info
import Muscat.Helpers.ParserHelper as PH

import OpenPisco.TopoZones as TZ
from OpenPisco.PhysicalSolvers.AsterSolverBase import AsterSolverBase,BoundaryConditionDefinitionError
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.PhysicalSolvers.AsterAnalysisSolverFactory import RegisterAsterClass
import OpenPisco.ExternalTools.Aster.AsterCommonWriter as AsterCommonWriter
import OpenPisco.ExternalTools.Aster.AsterHarmonicWriter as AsterHarmonicWriter

def CreateAsterHarmonicForcedPhysicalProblem(ops):
     res = AsterHarmonicForced()
     del(ops['type'])
     for tag,child in ops["children"]:
        if tag.lower()=="analysisparams":
            analysisparams={
                      "freqExcitMin":PH.ReadFloat(child["freqExcitMin"]),
                      "freqExcitMax":PH.ReadFloat(child["freqExcitMax"]),
                      "freqExcitStep":PH.ReadFloat(child["freqExcitStep"]),
                      "Numbermaxfreq" :PH.ReadFloat(child["Numbermaxfreq"])
                  }
            res.harmonicParams=analysisparams

        elif tag.lower() == "material":
           if "eTag" in child:
              z = child["eTag"]
           else:
              z = "AllZones"
           material = {}
           material["young"] = PH.ReadFloat(child["young"])
           material["poisson"] = PH.ReadFloat(child["poisson"])
           material["density"] = PH.ReadFloat(child["rho"])
           material["amor_alpha"] = PH.ReadFloat(child["Amor_alpha"])
           material["amor_beta"] = PH.ReadFloat(child["Amor_beta"])
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
                raise BoundaryConditionDefinitionError("set of dofs (dofs) and a value (value)")

        elif tag.lower() == "pressure":
            res.neumann["1"] = []
            res.problems.append("1")
            assert "eTag" in child,"Need a eTag or nTag for this boundary condition "
            tagname = child["eTag"]
            conditionValues=dict()
            assert "value" in child,"Need a scalar (value) for this boundary condition "
            conditionValues["val"] = PH.ReadFloat(child["value"])
            assert "type" in child,"Need a type (value) for this boundary condition "
            conditionValues["type"] = PH.ReadString(child["type"])
            res.neumann["1"].append([tagname,conditionValues])
        else:
            raise ValueError("This type of object is not supported " + (tag))

     del ops["children"]

     PH.ReadProperties(ops,ops.keys(),res)
     return res


class AsterHarmonicForced(AsterSolverBase):
    def __init__(self):
        super(AsterHarmonicForced,self).__init__()
        #Only conformal allowed for this analysis for now
        assert self.conform, "Ersatz material approach in dynamic not handled (yet)"
        self.criteriaParams = None
        self.followRadiaTag=False
        self.fixedZone=None
        self.name = 'AsterHarmonic'
        nodalFields=[FN.ERP_sensitivity,FN.dominant_SolutionMode,FN.ERP_density,FN.grad_Ure,FN.grad_Uim,FN.AllModeDEPL]
        for fieldName in nodalFields:
            self.auxiliaryFieldGeneration[fieldName][FN.Nodes] = False
        scalars=[FN.ModalCumulMassFractiondX,FN.ModalCumulMassFractiondY,FN.ModalCumulMassFractiondZ,FN.ModalLastEnergyRatio]
        self.auxiliaryScalarGeneration={scalar:False for scalar in scalars}
        self.auxiliaryScalarsbyOrderGeneration = {scalarsByOrder:False for scalarsByOrder in [FN.ERP,FN.eigen_Frequencies,FN.GeneDisp]}

    @property
    def harmonicParams(self):
        return self._harmonicParams

    @harmonicParams.setter
    def harmonicParams(self, value):
        modalBasisParameters=["AllFreq","Numbermaxfreq","autoModalBasisRangeFactor","modalBasisFreqMin","modalBasisFreqMax"]
        import itertools
        modalParamComboForbidden=[list(ele) for ele in list(itertools.combinations(modalBasisParameters,2))]
        modalParamComboForbidden.remove(["modalBasisFreqMin","modalBasisFreqMax"])
        freqIntervalParameters=["freqExcitMin","freqExcitMax","freqExcitStep"]
        expectedParameters=modalBasisParameters+freqIntervalParameters

        #Pseudo Invariants to check consistency of parameters
        if any(Parameter not in expectedParameters for Parameter in value.keys()):
            Info("The following parameters are not available for harmonic analysis ",value.keys()-set(expectedParameters))
        if not(any(Parameter in value.keys() for Parameter in modalBasisParameters) or all(Parameter in value.keys() for Parameter in ["modalBasisFreqMin","modalBasisFreqMax"])):
            raise ValueError("The modal basis is not properly defined. Either AllFreq, Numbermaxfreq, or autoModalBasisRangeFactor or (modalBasisFreqMin,modalBasisFreqMax) are expected")
        for singleModalCombo in modalParamComboForbidden:
            if all(Parameter in value.keys() for Parameter in singleModalCombo):
                raise ValueError("There is an incompatibility issue regarding the construction of the modal basis: combo "+str(singleModalCombo)+" not allowed")
        if any(Parameter not in value.keys() for Parameter in freqIntervalParameters):
            raise ValueError("The excitation is not properly defined. All following parameters are mandatory: freqExcitMin, freqExcitMax,freqExcitStep")
        self._harmonicParams=value

    def GetAvailableIndices(self):
        return int(self.GetNumberOfSolutions()/2)

    def GetNodalSolution(self,i,onCleanMesh=False):
        solIndex=math.floor(i/2)
        #Since the solution is complex, we return the real and complex part of each solution
        if (i%2) == 0:
            solution=self.GetField("DEPL_R","nodes",solIndex)
        else:
            solution=self.GetField("DEPL_I","nodes",solIndex)
        if onCleanMesh:
           self.u = solution
        else:
           self.u = self.ExtendingNodalFieldToOriginalSupport(solution)
        return self.u

    def WriteParametersInput(self,writeFile):
        self.solverInterface.SetExtraFiles(True)
        assert len(self.problems) <= 1,"GeneralAster: Sorry multi loads problem not implemented yet "
        AsterHarmonicWriter.WriteMaterialParametersInput(writeFile,self.materials)
        self.WriteBoundaryConditions(writeFile)
        assert not self.bodyforce and not self.forcenodale, "Bodyforce and nodalforces are not handled yet for this analysis"
        self.WriteAnalysisParametersInput(writeFile)
        if self.criteriaParams is not None:
            self.WriteCriteriaParametersInput(writeFile)

    def GetNumberOfSolutions(self):
        deltaFreqExcit,freqExcitStep=self.harmonicParams["freqExcitMax"]-self.harmonicParams["freqExcitMin"],self.harmonicParams["freqExcitStep"]
        if bool(deltaFreqExcit%freqExcitStep):
            numberOfSolutions= math.ceil(deltaFreqExcit/freqExcitStep) + 1
        else:
            numberOfSolutions= math.floor(deltaFreqExcit/freqExcitStep) + 1
        return 2*numberOfSolutions

    def UpdateRadiatingSurfaceTag(self,mesh):
        """
        .. py:classmethod:: UpdateRadiatingSurfaceTag(self, mesh)

        Update the surface tag corresponding to the radiating surface to follow it

        :param mesh mesh : current mesh
        :return: mesh mesh : new mesh with updated tag
        """
        ff = ElementFilter(zone = self.fixedZone,zones=[])
        ff.SetDimensionality(2)
        nonOptimElem={}
        for name,_,ids in ff(mesh):
            nonOptimElem[name]=ids

        tris = mesh.GetElementsOfType(ED.Triangle_3)
        allTria=range(tris.GetNumberOfElements())
        nonOptimTriaIds=nonOptimElem['tri3']
        newRadiaTagIds=list(set(allTria)-set(nonOptimTriaIds))

        if self.criteriaParams["radia_Surface"] in tris.tags.keys():
            mesh.DeleteElemTags([self.criteriaParams["radia_Surface"]])
        tris.tags.CreateTag(self.criteriaParams["radia_Surface"]).SetIds(newRadiaTagIds)
        return mesh

    def WriteCriteriaParametersInput(self,writeFile):
        writeFile.write("harmonicCritParams =")
        writeFile.write(str(self.criteriaParams)+"\n")

    def WriteAnalysisParametersInput(self,writeFile):
        AsterHarmonicWriter.WriteModalBasisAnalysisParametersInput(writeFile,self.harmonicParams)

    def WriteBoundaryConditions(self,writeFile):
        dimensionality = self.originalSupport.GetElementsDimensionality()
        AsterCommonWriter.WriteDirichletParametersInput(writeFile,self.dirichlet,dimensionality,method="BY_MULTIPLIER")
        AsterHarmonicWriter.WritePressureParametersInput(writeFile,self.neumann,self.problems)

    def RebuildGeneDisp(self):
        nbfreq=int(self.GetNumberOfSolutions()/2)
        harmonicParams=self.harmonicParams
        nbModes=harmonicParams["Numbermaxfreq"]
        excitFreqs=[harmonicParams["freqExcitMin"]+delta*harmonicParams["freqExcitStep"] for delta in range(nbfreq)]
        geneDisp=self.GetScalarsbyOrders(FN.GeneDisp)
        geneDispReal,geneDispImag=geneDisp[:len(geneDisp)//2],geneDisp[len(geneDisp)//2:]
        geneDispCByFreq={}
        for numOrder,freq in enumerate(excitFreqs):
            geneDispR=geneDispReal[numOrder*nbModes:(numOrder+1)*nbModes]
            geneDispI=geneDispImag[numOrder*nbModes:(numOrder+1)*nbModes]
            geneDispCByFreq[str(freq)]=[complex(disp_r,disp_i) for disp_r,disp_i in zip(geneDispR,geneDispI)]
        return geneDispCByFreq

    def SolveByLevelSet(self, levelset):
        if self.followRadiaTag:
            levelset.support=self.UpdateRadiatingSurfaceTag(levelset.support)
        return super(AsterHarmonicForced,self).SolveByLevelSet(levelset)
RegisterAsterClass("harmonic_forced_elastic", AsterHarmonicForced,CreateAsterHarmonicForcedPhysicalProblem)

from OpenPisco.ExternalTools.Aster.AsterInterface import SkipAsterTest
from OpenPisco.Unstructured.Levelset import LevelSet

def CheckIntegrity(GUI=False):
    skipTestPhase,message = SkipAsterTest()
    if skipTestPhase:
        return message

    analysisParams={"freqExcitMin":515.0,"freqExcitMax":525.0,"freqExcitStep":10,"modalBasisFreqMin":0,"modalBasisFreqMax":2.1e5}
    criteriaParams={"density_flu":1.189e-12,"veloc_flu":3.43e5,"radia_fact":1.0,"radia_Surface":'ETag2'}
    PPM = AsterHarmonicForced()
    PPM.harmonicParams=analysisParams
    PPM.conform=True
    PPM.SetAuxiliaryFieldGeneration(FN.ERP_density,on=FN.Nodes)
    PPM.SetAuxiliaryFieldGeneration(FN.ERP_sensitivity,on=FN.Nodes)
    PPM.SetAuxiliaryScalarGeneration(FN.ModalCumulMassFractiondX)
    PPM.SetAuxiliaryScalarGeneration(FN.ModalCumulMassFractiondY)
    PPM.SetAuxiliaryScalarGeneration(FN.ModalCumulMassFractiondZ)
    PPM.SetAuxiliaryScalarGeneration(FN.ModalLastEnergyRatio)
    PPM.SetAuxiliaryScalarsbyOrderGeneration(FN.ERP,listSize=PPM.GetNumberOfSolutions()/2)
    PPM.criteriaParams=criteriaParams
    material1={"young":210000.0,"poisson":0.3,"density":7.85e-09,"amor_alpha":1.0,"amor_beta":0.0}
    PPM.materials = [['AllZones',material1],]
    PPM.problems = ['idx1']
    PPM.neumann= {"idx1": [['ETag1',{"type":"uniform","val":1e+04}]]}
    PPM.dirichlet= [['ETag3',(0.,0.,0.)]]
    from OpenPisco.TestData import  GetTestDataPath as GetTestDataPath
    dir1 = GetTestDataPath()
    FileName = dir1+"PhysicalSolversTestsBase/AsterBase/simple_beam.mesh"
    from Muscat.IO import MeshReader
    reader = MeshReader.MeshReader()
    reader.SetFileName(FileName)
    mesh = reader.Read()

    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.ExterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet( support=mesh)
    ls.conform = True
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    print("Run harmonic analysis...")
    PPM.SolveByLevelSet(ls)
    print("Value of ERP at each frequency : ",PPM.GetAuxiliaryScalarsbyOrders(FN.ERP))
    print("Modal Basis: Mass contribution along dx : ",PPM.GetAuxiliaryScalar(FN.ModalCumulMassFractiondX))
    print("Modal Basis: Mass contribution along dy : ",PPM.GetAuxiliaryScalar(FN.ModalCumulMassFractiondY))
    print("Modal Basis: Mass contribution along dz : ",PPM.GetAuxiliaryScalar(FN.ModalCumulMassFractiondZ))
    print("Modal Basis: Energy ratio : ",PPM.GetAuxiliaryScalar(FN.ModalLastEnergyRatio))
    return "ok"

if __name__ == '__main__':
    import time
    stime = time.time()
    print(CheckIntegrity())
    print("Total Time " + str(time.time()-stime))
