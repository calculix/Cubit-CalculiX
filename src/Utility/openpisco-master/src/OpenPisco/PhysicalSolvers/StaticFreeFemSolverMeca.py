# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.IO import MeshWriter

from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
import OpenPisco.TopoZones as TZ
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import RegisterClass
from OpenPisco.ExternalTools.FreeFem.FreeFemInterface import FreeFemInterface
from OpenPisco.PhysicalSolvers.PhysicalSolversTools import ExtractInteriorMesh
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS

def CreateStaticFreeFemSolverMeca(ops):
     #TO DO: reintegrated contact bc in this
     import Muscat.Helpers.ParserHelper as PH
     if 'dimensionality' in ops:
         del(ops['dimensionality'])

     assert 'type' in ops,"General Freefem: must provide a physic. Possible choices :  static_elastic "
     physic = PH.ReadString(ops['type'])
     assert physic =="static_elastic","Physic"+str(physic)+" not available in this solver "
     res = StaticFreeFemSolverMeca()
     del(ops['type'])

     for tag,child in ops["children"]:
        if tag.lower() == "material":
           if "eTag" in child:
              z = child["eTag"]
           else:
              z = "AllZones"
           materialProperties={"young":PH.ReadFloat(child["young"]),"poisson":PH.ReadFloat(child["poisson"])}
           res.materials.append( [z,materialProperties ])

        elif tag.lower() == "dirichlet":
             if "eTag" in child:
                z = child["eTag"]
             assert "dofs" in child and "value" in child, "Need a set of dofs (dofs) and a value (value) for dirichlet boundary condition"
             dofs = PH.ReadInts(child["dofs"])
             val = PH.ReadFloat(child["value"])
             U=[]
             for i in range(3):
                 if i in dofs:
                     U.append(val)
                 else:
                     U.append(None)
             res.dirichlet.append([z,U])

        elif tag.lower() == "loadcase":
            z = PH.ReadInt(child["id"])
            res.neumann[z] = []
            res.problems.append(z)

            for tag2,child2 in child["children"]:
                if tag2.lower() =="force":
                   if "eTag" in child2:
                      z2 = child2["eTag"]
                   if "value" in child2 :
                      fx,fy,fz = PH.ReadVectorXYZ(child2["value"])
                      assert "dim" in child,"GeneralFreeFem: Need a dimension for the boundary condition of type Force "
                      dim = PH.ReadInt(child["dim"])
                      assert dim == 2,"GeneralFreeFem: Please provide a surface force "
                   elif "phi" in child2 and "theta" in child2:
                        fx,fy,fz = PH.ReadVectorPhiThetaMag([child2["phi"],child2["theta"]],True )
                   else:
                        raise ValueError("Need a vector (value) or Euler angles (phi and theta) for the boundary condition of type Force ")
                   res.neumann[z].append([z2,(fx,fy,fz)])

        elif tag.lower() == "force":
            res.neumann["1"] = []
            res.problems.append("1")
            if "eTag" in child:
               z = child["eTag"]
            if "value" in child :
               fx,fy,fz = PH.ReadVectorXYZ(child["value"])
            elif "phi" in child and "theta" in child:
                 fx,fy,fz = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"]],True )
            else:
               raise ValueError("GeneralFreeFem: Need a vector (value) or Euler angles (phi and theta) for or the boundary condition of type Force ")
            res.neumann["1"].append([z,(fx,fy,fz)])

        else:
            raise NotImplementedError("GeneralFreeFem: Boundary condition or object not supported " + (tag))

     workingDirectory = res.GetUniqueWorkingDirectory()
     res.workingDirectory = workingDirectory
     res.solverInterface.SetWorkingDirectory(workingDirectory)
     del ops["children"]

     PH.ReadProperties(ops,ops.keys(),res)
     return res

class StaticFreeFemSolverMeca(SolverBase):
    def __init__(self):
        super(StaticFreeFemSolverMeca,self).__init__()
        self.workingDirectory = ""
        self.solverInterface = FreeFemInterface()
        self.materials = [] #[["AllZones",(1,0.3)]]
        self.dirichlet= []#[['Dirichlet1',(0.,0.,0.)], ['Dirichlet2',(1.,0.,0.)]]
        self.neumann= {}#{"idx1": [['Neumann1',(0.,0.,0.)],['Neumann2',(0.,0.,0.)]], "idx2": [['Neumann3',(0.,0.,0.)]] }
        self.problems = [] #["idx1",]
        self.bodyforce = {}
        self.contact =[]# [['Contact1',(0.0)],['Contact2',(0.0)]]

        self.originalSupport = None
        self.cleanMesh = None
        self.writer = MeshWriter.MeshWriter()
        self.conform = False # [False, True]

        FieldsICanSupply = [FN.potential_energy,FN.von_mises,FN.stress]
        for fieldname in FieldsICanSupply:
            self.auxiliaryFieldGeneration[fieldname][FN.Nodes] = False
        self.auxiliaryScalarGeneration = {FN.int_potential_energy:False}

    def GetNodalField(self,name):
        field = self.solverInterface.GetNodalField(name)
        if not self.conform:
           return field
        else:
           fieldextended = np.zeros( (self.originalSupport.GetNumberOfNodes(), ),dtype=float)
           fieldextended[self.cleanMesh.originalIDNodes] = field
           return fieldextended

    def GetAuxiliaryField(self,name,on=FN.Nodes,index=None):
        errorMessage = "Field "+str(name)+" on "+str(on)+" is not in the list of fields to be generated"
        assert self.auxiliaryFieldGeneration[name][on],errorMessage
        return self.GetNodalField(name)

    def GetAuxiliaryScalar(self, name):
        errorMessage = "Field "+str(name)+"is not in the list of scalars to be generated"
        assert self.auxiliaryScalarGeneration[name],errorMessage
        return self.solverInterface.GetScalar(name)

    def GetNodalSolution(self):
        solution = self.solverInterface.GetSolution()
        if not self.conform:
           self.u = solution
        else:
           fieldextended = np.zeros( (self.originalSupport.GetNumberOfNodes(),3 ),dtype=float)
           fieldextended[self.cleanMesh.originalIDNodes] = solution
           self.u = fieldextended
        return self.u

    def GetNumberOfSolutions(self):
        return len(self.problems)

    def GenerateFreeFemScriptAndSetSolverInterface(self):
        from OpenPisco.ExternalTools.FreeFem.FreeFemProblemWriter import FreeFemStaticWriter
        freefemWriter = FreeFemStaticWriter(phyproblem = self)
        freefemWriter.WriteFreefemProblem()
        self.solverInterface.filename = freefemWriter.filename

    def WriteSupport(self, support):
        dirichlet = self.dirichlet
        neumann = self.neumann
        contact = self.contact
        elemRefNumber = np.zeros(support.GetNumberOfElements(), dtype=int)
        nodeRefNumber = np.zeros(support.GetNumberOfNodes(), dtype=int)
        self.cpt= 1
        for indexRef,tagName in enumerate(dirichlet):
           elements = support.GetElementsInTag(tagName[0])
           elemRefNumber[elements] = self.cpt+indexRef
        self.cpt += indexRef+1

        for ProblemId in neumann.keys():
            for indexRef,loadcase in enumerate(neumann[ProblemId]):
                elements = support.GetElementsInTag(loadcase[0])
                elemRefNumber[elements] = self.cpt+indexRef
            self.cpt+=indexRef+1

        for indexRef,contactTagName in enumerate(contact):
            elements = support.GetElementsInTag(contactTagName[0])
            elemRefNumber[elements] = self.cpt+indexRef
        self.cpt+=indexRef+1
        self.writer.Open(TemporaryDirectory.GetTempPath()+"freefeminput.mesh")
        self.writer.Write(support, elemRefNumber = elemRefNumber,nodalRefNumber=nodeRefNumber)
        self.writer.Close()

    def WriteLevelSet(self,phi,support):
        self.writer.SetFileName(TemporaryDirectory.GetTempPath()+"freefeminput.chi.sol")
        self.writer.OpenSolutionFile(support)
        self.writer.WriteSolutionsFields(support, [phi])
        self.writer.Close()

    def SolveByLevelSet(self, levelset):
        self.conform = levelset.conform
        self.SetSupport(levelset.support)
        if self.conform:
            self.cleanMesh = ExtractInteriorMesh(levelset)
        else:
            self.cleanMesh = levelset.support
            self.WriteLevelSet(levelset.phi,levelset.support)
        self.GenerateFreeFemScriptAndSetSolverInterface()
        self.WriteSupport(self.cleanMesh)
        self.solverInterface.ExecuteFreefem()

        return RETURN_SUCCESS

RegisterClass("GeneralFreefem", StaticFreeFemSolverMeca,CreateStaticFreeFemSolverMeca)

from OpenPisco.ExternalTools.FreeFem.FreeFemInterface import FREEFEM_AVAILABLE
from Muscat.Helpers.CheckTools import SkipTest
from OpenPisco.Unstructured.Levelset import LevelSet
from Muscat.MeshTools.MeshCreationTools import CreateCube

def CheckIntegrity_StaticFreeFemSolverMeca(GUI=False,physics=False):
    mesh = CreateCube(dimensions=[5,5,5],spacing=[1./4,1./4,1./4],origin=[0, 0, 0],ofTetras=True)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.ExterSurf,False).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet( support=mesh)
    ls.phi = np.empty(ls.support.GetNumberOfNodes())
    ls.phi.fill(-1)
    print(TemporaryDirectory.GetTempPath())
    PPM = StaticFreeFemSolverMeca()
    PPM.conform = ls.conform
    material1={"young":1.0,"poisson":0.3}
    PPM.materials = [['AllZones',material1],]
    PPM.dirichlet= [['X0',(0.,0.,0.)]]
    PPM.problems = ['idx1']
    PPM.neumann= {"idx1": [['X1',(0.,0.,-0.1)]]}
    if physics=="contact":
        PPM.contact=[['Contact1',(0.),(0.0)]]
    PPM.SetAuxiliaryScalarGeneration(FN.int_potential_energy)
    PPM.SetAuxiliaryFieldGeneration(FN.potential_energy, on=FN.Nodes)
    PPM.SetAuxiliaryFieldGeneration(FN.von_mises, on=FN.Nodes)
    PPM.SetAuxiliaryFieldGeneration(FN.stress, on=FN.Nodes)
    for conform in [True,False]:
        ls.conform = conform
        PPM.SolveByLevelSet(ls)
        PPM.GetNodalSolution()
        PPM.GetAuxiliaryField(FN.potential_energy,on=FN.Nodes)
        PPM.GetAuxiliaryField(FN.von_mises,on=FN.Nodes)
        PPM.GetAuxiliaryField(FN.stress,on=FN.Nodes)
    return "ok"

def CheckIntegrity(GUI=False):
    if SkipTest("FREEFEM_NO_FAIL"): return "skip"
    if not FREEFEM_AVAILABLE:
        return "skip Ok, FreeFem not found!!"

    CheckIntegrity_StaticFreeFemSolverMeca(GUI=GUI)
    return "ok"

if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity())
