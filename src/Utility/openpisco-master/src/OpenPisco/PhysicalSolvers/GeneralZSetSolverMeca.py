# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import subprocess
import numpy as np

from Muscat.Types import MuscatFloat
from Muscat.IO.CodeInterface import Interface as Interface
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.Helpers.IO.Which import Which
import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.Logger import Info, Debug, Error, ForcePrint
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory as TestTempDir
from Muscat.Helpers.IO.PathController import PathController
from Muscat.IO.GeofWriter import GeofName
import Muscat.IO.PostReader as PostReader
from Muscat.IO.ZsetTools import nbIntegrationsPoints
from Muscat.IO.GeofWriter import WriteMeshToGeof
from Muscat.IO.UtReader import UtReader

from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import RegisterClass
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL_EXTERNAL_TOOL

elasticProblemStringBase="""
****calcul
 ***mesh
  **file {meshfile}

 ***linear_solver mumps

 ***resolution newton
  **sequence  1
   *time      1.
   *increment 1
   *iteration  10
   *ratio  automatic 1.e3
   *algorithm p1p2p3


{BC}

 ***table
  **name tabunit
   *time  0.    1.0
   *value 0.    1.0

 ***material
   *this_file 1

 ***output
  **value_at_integration
#  **output_first
  **save_parameter

{MATERIAL}
****return
"""

conformMaterialString="""

***behavior gen_evp
  **elasticity isotropic
     young {YOUNG}
     poisson {POISSON}
  **coefficient
     masvol 1.
***return

"""

densityMaterialString="""
 ***parameter
  **file idx
   *ip
    0. file ./{materialFileName} 0
    1. file ./{materialFileName} 0
****return

***behavior gen_evp
  **elasticity isotropic
     young = idx
     poisson {POISSON}
  **coefficient
     masvol 1.
"""


strain_energy_String_local = """
***local_post_processing
  **file node
  **process function
    *output ener
    *expression 0.5*(sig11*eto11+ sig12*eto12 + sig31*eto31 + sig12*eto12+ sig22*eto22 + sig23*eto23 + sig31*eto31+ sig23*eto23 + sig33*eto33);

***local_post_processing
 **file integ
 **elset ALL_ELEMENT
 **process function
    *output enerinteg
    *expression 0.5*(sig11*eto11+ sig12*eto12 + sig31*eto31 + sig12*eto12+ sig22*eto22 + sig23*eto23 + sig31*eto31+ sig23*eto23 + sig33*eto33);

"""
strain_energy_String_global = """
***global_post_processing
 **file integ
 **elset ALL_ELEMENT
 **process volume_integrate
  *list_var enerinteg
"""

tr_strain_String_local = """
***local_post_processing
  **file node
  **process function
    *output treto
    *expression eto11+ eto22+ eto33;
"""

tr_stress_String_local = """
***local_post_processing
  **file node
  **process function
    *output trsig
    *expression sig11+ sig22+ sig33;

"""

volume_String_global = """

***global_post_processing
 **file integ
 **elset ALL_ELEMENT
 **process volume

"""
postProcessString = "****post_processing \n"
postProcessString += tr_strain_String_local
postProcessString += tr_stress_String_local
postProcessString += strain_energy_String_local
postProcessString += volume_String_global
postProcessString += strain_energy_String_global
postProcessString += """
****return
"""

def CreateGeneralZSetPhysicalProblemMecaNewAPI(ops):
    """
    Constructor for a GeneralZSetPhysicalProblemMeca

    Parameters
    ----------
    ops
        A dictionary-like object


    Returns
    -------
    GeneralZSetPhysicalProblemMeca
        A populated `GeneralZSetPhysicalProblemMeca` instance

    Notes
    -----
    The `ops` can be generated from a string like this::

        a = <XXXXXXXX id="X" type="static_elastic"  not available(p="*1|2")  >

              <UseTemplateFile name="" />

-    0.*<Material    eTag="*everyelement|eTag" young="1" poisson="0.3" density="1" />
     0.*<Dirichlet   eTag="eTag" dofs="[012]" value="[float]" />
     0.*<Dirichlet   eTag="eTag" aVoir[nTag=""] u1="0" />
     0.*<Acceleration     eTag="*everyelement|eTag" g="3*float" />
     0.*<Centrifugal eTag="eTag" point="3*float" axis="3*float" angularSpeed="float" />
     0.*<Force [eTag="ET9"|nTag=""] value="3*float" />
     0.*<Force [eTag="ET9"|nTag=""] phi="4.5" theta="5.6" mag="2" >
-    not available#   <TotalForce  eTag="eTag_Name_3" phi="4.5" theta="5.6" mag="2" >
	 0.*<Pressure  eTag="2D_eTag"  value="0.0" />

-    0.*<LoadCase id="1" >
-		0.*<Gravity>
-		0.*<Centrifugal>
-		0.*<Force>
-		0.*<Pressure>
-	</LoadCase>


</XXXXXXXX>"""


    if 'dimensionality' in ops:
        del(ops['dimensionality'])

    res = GeneralZSetPhysicalProblemMeca()
    tempdir = TestTempDir.GetTempPath()

    res.zsetInterface.SetWorkingDirectory(tempdir )
    res.zsetInterface.SetProcessDirectory(tempdir )

    if 'exe' in ops:
        res.zsetInterface.SetCodeCommand(ops['exe'])
        del ops['exe']

    assert ops["type"] == "static_elastic","for the moment only static_elastic are allowed: '{}' ".format(ops["type"])

    if 'UseTemplateFile' in ops:
        res.ReadBaseInp(ops['UseTemplateFile'])
        del ops['UseTemplateFile']
    else:

        res.zsetInterface.tpl = elasticProblemStringBase
        res.zsetInterface.tpl += postProcessString
        res.PostProcessNumberToRun = 1

        res.eTagsToBeConvertedToNTagsBeforeOuput = set()
        res.ntagsPrefix = "N_"
        for tag,child in ops["children"]:
            if tag == "Dirichlet":

                eTag = PH.ReadString(child["eTag"])
                vals = [0]*3
                dofs = []
                if "dofs" in child:
                    dofs = PH.ReadInts(child["dofs"])
                    val =  PH.ReadFloat(child.get("value") )
                    for i in dofs:
                        vals[i] = val
                else:
                    for i in range(3):
                        name = "u"+str(i)
                        if name in child:
                            val = PH.ReadFloat(child.get(name))
                            dofs.append(i)
                            vals[i] = val

                bcstring += "**impose_nodal_dof \n"
                for i in dofs:
                    res.eTagsToBeConvertedToNTagsBeforeOuput.add(eTag)
                    bcstring += "   {}{} U{} {} \n".format(res.ntagsPrefix,eTag,i+1,vals[i])

                continue

            if tag == "Force":

                if "phi" in child and "theta" in child:
                    f = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"],child.get("mag",1.)],False )
                elif "dir" in child :
                    f = PH.ReadVectorXYZ(child["dir"],True)
                else:
                    message = "I need a direction (dir) or Euler angles (phi, theta)"
                    Error(message)
                    raise ValueError(message)

                val = PH.ReadFloat(child.get("value") )

                if "eTag" in child:
                    eTag = PH.ReadString(child["eTag"])
                    bcstring += "**impose_nodal_dof_density \n"
                    for i in range(3):
                        bcstring += "   {} U{} {} \n".format(eTag,i+1,f[i]*val)
                else:
                    nTag = PH.ReadString(child["nTag"])
                    for i in range(3):
                        bcstring += "   {} U{} {} \n".format(nTag,i+1,f[i]*val)

                continue

            if tag == "Centrifugal":
                eTag = PH.ReadString(child["eTag"])
                point = PH.ReadFloats(child["point"])
                axis = PH.ReadVectorXYZ(child["axis"],True)
                axisdir = None
                assert sum(axis != 0) <= 1,"the axis must be aligned with the coordinate system"
                for i in range(3):
                    if axis[i]!=0:
                        axisdir = i+1
                        break

                angularSpeed = PH.ReadFloat(child["angularSpeed"])

                bcstring += "**centrifugal  \n"
                bcstring += "   {} ({} {} {}) d{} {} tab1\n".format(eTag,point[0],point[1],point[2],axisdir,  angularSpeed)
                continue

            if tag == "Acceleration":
                vals = PH.ReadFloats(child["g"] )
                norm = np.linalg.norm(vals)
                eTag = PH.ReadString(child["eTag"])

                bcstring += "**gravity {} {} {} tab1\n".format(eTag, vals/norm,norm)
                continue

            if tag == "Pressure":
                val = PH.ReadFloat(child["value"] )
                eTag = PH.ReadString(child["eTag"])
                bcstring += "**pressure\n"
                bcstring += "   {} {} tab1\n".format(eTag, val)
                continue

            raise NotImplementedError("dont know how to treat tag: {}".format(tag))

        res.zsetInterface.parameters['BC'] = bcstring

    res.tagsToKeep = PH.ReadStrings(ops['tagsToKeep'])
    del ops['tagsToKeep']

    PH.ReadProperties(ops,ops.keys(),res)
    return res

def CreateGeneralZSetPhysicalProblemMeca(ops):
     #<GeneralZset id="1"
     #             TagsToKeep="PhyTag1 PhyTag2 PhyTag3 PhyTag4 PhyTag5"
     #             inputfile="basic.inp"
     #             exe="/path/to/Zrun"
     #             inputDir="/path/to/dir/"
     #             processDir="/path/to/dir/"
     #             >
     #</GeneralZset>
     res = GeneralZSetPhysicalProblemMeca()
     res.zsetInterface.SetWorkingDirectory(PathController.GetFullPathnameOnWorkingDirectory(ops['inputDir']) )
     del ops['inputDir']

     res.zsetInterface.SetProcessDirectory(PathController.GetFullPathnameOnWorkingDirectory(ops['processDir']) )
     del ops['processDir']
     res.zsetInterface.SetCodeCommand(ops['exe'])
     del ops['exe']

     if 'inputfile' in ops:
         res.ReadBaseInp(ops['inputfile'])
         del ops['inputfile']
     else:

        res.zsetInterface.tpl = elasticProblemStringBase
        res.zsetInterface.tpl += postProcessString
        res.PostProcessNumberToRun = 1

        res.zsetInterface.parameters['BC'] = ops["CDATA"]
        del ops['CDATA']

     res.tagsToKeep = PH.ReadStrings(ops['tagsToKeep'])
     del ops['tagsToKeep']

     PH.ReadProperties(ops,ops.keys(),res)
     return res

class GeneralZSetPhysicalProblemMeca(SolverBase):
    def __init__(self):
        super(GeneralZSetPhysicalProblemMeca,self).__init__()
        self.zsetInterface = Interface()
        self.TM =  None
        self.globalCompliance = 0.0

        self.zsetInterface.parameters['meshfile'] = "meshfile.geof"
        self.zsetInterface.SetCodeCommand("Zrun")
        self.zsetInterface.parameters['BC'] = ""

        self.PostProcessNumberToRun = 1

        self.calculCounter = 0
        self.increaseCounter = False

        self.hasSupport = False
        self.support = None
        self.young = 1
        self.poisson = 0.3

        self.auxiliaryScalarGeneration[FN.int_potential_energy] = False
        self.auxiliaryFieldGeneration[FN.potential_energy][FN.Nodes] = False
        self.auxiliaryFieldGeneration[FN.tr_stress_][FN.Nodes] = False
        self.auxiliaryFieldGeneration[FN.tr_strain_][FN.Nodes] = False


    def SolveByDensities(self, phi,support):
        self.support = support
        self.zsetInterface.parameters['MATERIAL'] = densityMaterialString
        self.zsetInterface.parameters['POISSON'] = self.poisson
        self.zsetInterface.parameters['YOUNG'] = self.young

        l2 = phi.repeat(4).astype(np.float32).byteswap()*self.young
        filename = "phi"+str(self.calculCounter)+".data"
        l2.tofile(self.zsetInterface.processDirectory+"/"+filename, sep="")
        self.zsetInterface.parameters['materialFileName'] = filename
        raise NotImplementedError("Not working yet?")

    def PreSolver(self,levelSet):
        self.numberOfNodesOfTheOriginalSupport = levelSet.support.GetNumberOfNodes()
        self.ComputeDomainToTreatLs(levelSet)

    def SolveByLevelSet(self, levelSet):
        self.zsetInterface.parameters['POISSON'] = self.poisson
        self.zsetInterface.parameters['MATERIAL'] = conformMaterialString
        self.PreSolver(levelSet)

        if not levelSet.conform:
            cpt = 0
            for name,data,ids in ElementFilter(dimensionality=3)(self.cleanMesh):
                nip = nbIntegrationsPoints[GeofName[name]]
                cpt +=data.GetNumberOfElements() * nip

            l2 = np.empty(cpt,dtype=np.float32)

            cpt = 0
            cpt2 = 0
            for name,data,ids in ElementFilter(dimensionality=3)(self.cleanMesh):
                nip = nbIntegrationsPoints[GeofName[name]]
                nel = data.GetNumberOfElements()
                d = self.cleanMeshDensities[np.arange(len(ids))+cpt2].repeat(nip).astype(np.float32)
                l2[cpt:cpt+nel*nip] = d
                cpt += nel*nip
                cpt2 += len(ids)
            l2 *= self.young
            filename = "phi"+str(self.calculCounter)+".data"
            l2.astype(np.float32).byteswap().tofile(self.zsetInterface.processDirectory+"/"+filename, sep="")
            self.zsetInterface.parameters['materialFileName'] = filename
            self.zsetInterface.parameters['MATERIAL'] = densityMaterialString

        return self.WriteMeshAndRun(conform=levelSet.conform)


    def WriteMeshAndRun(self,conform=True):
        self.zsetInterface.parameters['meshfile'] = "meshfile" +str(self.calculCounter)+ ".geof"
        print(self.cleanMesh)
        WriteMeshToGeof(self.zsetInterface.processDirectory + self.zsetInterface.parameters['meshfile'],self.cleanMesh,lowerDimElementsAsElsets=False )
        Info(self.zsetInterface.processDirectory + self.zsetInterface.parameters['meshfile'])
        self.zsetInterface.WriteFile(self.calculCounter)

        Debug("Launching Zset")

        try :
          out = self.zsetInterface.SingleRunComputationAndReturnOutput(self.calculCounter)
        except Exception as inst:
            ForcePrint("Error in Zset process: ")
            ForcePrint(self.zsetInterface.processDirectory)
            ForcePrint(self.zsetInterface.lastCommandExecuted)
            ForcePrint(inst)
            ForcePrint(inst.output)
            return RETURN_FAIL_EXTERNAL_TOOL

        if out.find("ERROR at") != -1:
            Error("Error in Zset")
            ForcePrint(out)
            return RETURN_FAIL_EXTERNAL_TOOL

        Debug(str(self.zsetInterface.lastCommandExecuted))

        cmd = 'cd ' + self.zsetInterface.processDirectory + ' && ' + self.zsetInterface.codeCommand + ' -N ' +str(self.PostProcessNumberToRun) + ' -pp ' + self.zsetInterface.inputFilename + str(self.calculCounter) + self.zsetInterface.inputFileExtension
        Debug("Runing command")
        Debug("Launching Zset pp")

        try:
            out = subprocess.check_output(cmd, cwd=self.zsetInterface.processDirectory, shell=True ).decode("utf-8","ignore")
        except Exception as inst:
            ForcePrint("Error in Zset post process: ")
            ForcePrint(self.zsetInterface.processDirectory)
            ForcePrint(cmd)
            ForcePrint(inst)
            ForcePrint(inst.output)
            return RETURN_FAIL_EXTERNAL_TOOL

        if out.find("ERROR at") != -1:
            Error("Error in Zset")
            ForcePrint(out)
            return RETURN_FAIL_EXTERNAL_TOOL

        self.nodal_elastic_energy = np.zeros(self.originalSupport.GetNumberOfNodes(),dtype=MuscatFloat)

        reader = UtReader()
        reader.SetFileName(self.zsetInterface.processDirectory + "calcul"+str(self.calculCounter)+".utp")
        reader.ReadMetaData()

        data = reader.ReadField("ener")
        self.nodal_elastic_energy[self.cleanMeshToOriginalNodal] = data

        data = reader.ReadField("trsig")
        self.nodal_TrSigma = np.zeros(self.originalSupport.GetNumberOfNodes(),dtype=MuscatFloat)
        self.nodal_TrSigma[self.cleanMeshToOriginalNodal] = data

        data = reader.ReadField("treto")
        self.nodal_TrEpsilon = np.zeros(self.originalSupport.GetNumberOfNodes(),dtype=MuscatFloat)
        self.nodal_TrEpsilon[self.cleanMeshToOriginalNodal] = data

        reader.atIntegrationPoints = True
        ipdata = reader.ReadField( "enerinteg")

        ## work only for trianglular meshes
        element_elastic_energy = np.empty(self.cleanMesh.GetNumberOfElements(dim=3),dtype=MuscatFloat)
        cpt = 0
        cpt2 = 0
        for name,data,ids in ElementFilter(dimensionality=3)(self.cleanMesh):
            nip = nbIntegrationsPoints[GeofName[name]]
            localData = np.sum(ipdata[cpt:cpt+len(ids)*nip].reshape(len(ids),nip),axis=1)
            localData /= nip
            element_elastic_energy[cpt2:cpt2+len(ids)] = localData

            cpt += data.GetNumberOfElements()*nip
            cpt2 += data.GetNumberOfElements()

        self.element_elastic_energy = np.zeros(self.originalSupport.GetNumberOfElements(dim=3), dtype=MuscatFloat)
        print(self.element_elastic_energy.shape)
        print(self.cleanMeshToOriginalElementary.shape)
        print(element_elastic_energy.shape)
        self.element_elastic_energy[self.cleanMeshToOriginalElementary] = element_elastic_energy

        if not conform:
            self.element_elastic_energy /= self.densities
           # we divide by the density to get the correct gradient

        reader = UtReader()
        reader.Reset()
        reader.SetFileName(self.zsetInterface.processDirectory + "calcul"+str(self.calculCounter)+".ut")
        reader.ReadMetaData()
        U1 = reader.ReadField("U1")
        U2 = reader.ReadField("U2")
        U3 = reader.ReadField("U3")
        u = np.vstack((U1,U2,U3)).T

        self.u = np.zeros((self.originalSupport.GetNumberOfNodes(), 3),dtype=MuscatFloat)
        self.u[self.cleanMeshToOriginalNodal,:] = u


        theReader = PostReader.PostReader(fileName=self.zsetInterface.processDirectory+"calcul"+str(self.calculCounter)+".post")
        res = theReader.Read()
        Info("Volume (post) : " + str(res["Volume"][0]) )
        Info("enerinteg (post) : " + str(res["enerinteg"][0]) )
        self.globalCompliance = res["enerinteg"][0]

        self.calculCounter += 1
        Info("Done WriteMeshAndRun")

        return RETURN_SUCCESS

    def ReadBaseInp(self, filename):
        self.zsetInterface.ReadTemplateFile(filename)
        self.zsetInterface.tpl += postProcessString
        self.PostProcessNumberToRun = self.zsetInterface.tpl.count("****post_processing")


    def GetNodalSolution(self):
        return self.u

    def GetAuxiliaryField(self, name,on=FN.Nodes):
        assert on == FN.Nodes,"Can return only on Nodes"

        if name == FN.potential_energy:
            return self.nodal_elastic_energy
        elif name == FN.tr_strain_:
            return self.nodal_TrEpsilon
        elif name == FN.tr_stress_:
            return self.nodal_TrSigma
        else:
            return None

    def GetAuxiliaryScalar(self,name):
        if name == FN.int_potential_energy:
            return self.globalCompliance
        else:
            return None

RegisterClass("GeneralZset", GeneralZSetPhysicalProblemMeca,CreateGeneralZSetPhysicalProblemMeca)
RegisterClass("GeneralZsetNew", GeneralZSetPhysicalProblemMeca,CreateGeneralZSetPhysicalProblemMecaNewAPI)

def CheckIntegrity():
    from Muscat.Helpers.CheckTools import SkipTest
    if SkipTest("ZSET_NO_FAIL"): return "skip"

    if not Which("Zrun"):
        return "skip Ok, Zset not found!!"

    from Muscat.TestData import  GetTestDataPath as MuscatGetTestDataPath
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory as TestTempDir
    from Muscat.IO.GeofReader import ReadGeof
    from Muscat.MeshTools.MeshCreationTools import QuadToLin as QuadToLin

    from OpenPisco.Unstructured.Levelset import LevelSet

    print(TestTempDir.GetTempPath())

    zsetprob = GeneralZSetPhysicalProblemMeca()
    zsetprob.zsetInterface.SetWorkingDirectory(MuscatGetTestDataPath())
    zsetprob.zsetInterface.SetProcessDirectory(TestTempDir.GetTempPath())
    zsetprob.ReadBaseInp('cube.inp')
    res = ReadGeof(fileName=MuscatGetTestDataPath() + "cube.geof")
    res2 = QuadToLin(res,divideQuadElements=True, linearizedMiddlePoints= True)
    zsetprob.tagsToKeep = ["x0","x1","domain000","domain001","domain002","domain003"]
    ls = LevelSet()
    ls.conform = True
    ls.phi = None
    ls.support = res2

    assert zsetprob.SolveByLevelSet(ls) == zsetprob.RETURN_SUCCESS,"Error Running Zset "
    print("Compliance : " + str(zsetprob.GetAuxiliaryField("potential_energy")) )
    print("Global Compliance : " + str(zsetprob.GetAuxiliaryScalar("int_potential_energy") ))

    return "OK"

if __name__ == '__main__':
    import time
    stime = time.time()
    print(CheckIntegrity())
    print("Total Time " + str(time.time()-stime))
