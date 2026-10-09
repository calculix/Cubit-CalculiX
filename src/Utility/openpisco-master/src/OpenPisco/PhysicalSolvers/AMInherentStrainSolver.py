# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import time
import numpy as np
from sympy import Array, tensorproduct, tensorcontraction

import Muscat.Helpers.ParserHelper as PH
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshTools.MeshFieldOperations import GetFieldTransferOp
from Muscat.IO.IOFactory import CreateWriter,InitAllWriters

from Muscat.Types import MuscatFloat, ArrayLike
from Muscat.FE.SymPhysics import MechPhysics
from Muscat.FE.KR.KRBlock import KRBlockVector
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceP0
from Muscat.FE.FETools import PrepareFEComputation
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.Fields.FieldTools import VectorToFEFieldsData,ElemFieldsToFEField,Maximum
from Muscat.FE.SymWeakForm import ToVoigtEpsilon,Strain, Inner, Diag,FromVoigtSigma, GetScalarField, GetTestField, GetField, ToVoigtSigma
from Muscat.FE.Integration import IntegrateGeneral
import Muscat.FE.DofNumbering as DN
from Muscat.MeshContainers.Filters.FilterTools import ReadElementFilter
from Muscat.FE.Fields.FieldTools  import Maximum
from Muscat.Helpers.Logger import Info


from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import  RegisterClass
from OpenPisco.PhysicalSolvers.UnstructuredFEAGenericSolver import UnstructuredFEAMecaProblem
from OpenPisco.PhysicalSolvers.AMProcessBase import AMProcessBase
import OpenPisco.TopoZones as TZ
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.Unstructured.MmgMesher import MmgAvailable
from OpenPisco import RETURN_SUCCESS

InitAllWriters()

def CreateStructuredAMProcess(ops):
    """
    Parameters
    ----------
    ops : dict
    Parameters to setup a 'APProcess'.

    Notes
    -----
    'ops' can be populated via a string of the form:

        <AMProcess id="1"  type="IStrains"  IStrains="-0.01 -0.01 -0.01" useSupport="False" filename="testAMProcess" >
        for the moment only constant properties are available
        0.*<Material NotImplemented(ef="*everyElement|EFExpression") young="1" poisson="0.3" density="1" />
        by default we block U0,  0 1 2
        0.*<Block on="U0 Y0 Z0 U1 Y1 Z1"  block="0 1 2" />
        </AMProcess>

        by default we block U0,  0 1 2
        0.*<Block on="U0 Y0 Z0 U1 Y1 Z1"  block="0 1 2" />
        </AMProcess>

    """
    res = AMInherentStrainSolver()
    del ops["type"]

    dim = PH.ReadInt(ops.get('dimensionality',3))
    assert dim != 2,"(Inherent Strain Solver) Unable to treat this dimensionality"
    if 'dimensionality' in ops:
        del(ops['dimensionality'])

    if "useSupport" in ops:
        res.SetUseSupport(ops["useSupport"])
    else:
        res.useSupport = False

    res.nx = PH.ReadInt(ops.get('nx',10))
    res.ny = PH.ReadInt(ops.get('ny',10))
    res.nz = PH.ReadInt(ops.get('nz',10))

    bc = []
    for tag, child in ops["children"]:
        if tag == "Material":
            elementFilter = ReadElementFilter(child.get("ef",""))
            for prop in ["young","poisson","density", "referenceTemperature", "thermalCapacity", "thermalDiffusion", "thermalExpansion", "thermalConvection" ]:
                if prop in child:
                    val = PH.ReadFloat(child[prop])
                    if prop in res.propsFields:
                        res.propsFields[prop].append( (elementFilter,val) )
                    else:
                        res.propsFields[prop] =  [ (elementFilter,val) ]
            continue

        if tag == "Block":
            on = PH.ReadStrings(child["on"])
            block = PH.ReadInts(child["block"])
            bc.append((on,block))

    if len(bc) != 0:
        res.SetBoundaryConditions(bc)

    del ops["children"]
    PH.ReadProperties(ops,ops.keys(), res, typeConversion=True)
    return res

class MecaInherentStrain(MechPhysics):

    def __init__(self):
        super(MecaInherentStrain,self).__init__(3)
        self.UnFrozen()
        self.inherentStrainsCoefficients = np.array([-0.001,-0.001, -0.001])

    def GetStressVoigt(self, utGlobal, HookeLocalOperator=None, deltaTemperature=None, XLi=None, factor=None):
        """Function to compute the stress using the Inherent Strain, the XLi is the field where the inherent Strain is Active"""
        if deltaTemperature is None:
            deltaTemperature = GetScalarField("deltaTemperature")

        if XLi is None:
            XLi = GetScalarField("XLi")

        thermalExpansionOp = Diag([self.coeffs["thermalExpansionCoefficient"]]*3)
        inherentStrainsMatrix = Diag([float(self.coeffs["inherentStrainsCoefficients_"+str(x)]) for x in range(3) ])

        if HookeLocalOperator is None:
            HookeLocalOperator = self.GetHookeOperator(self.coeffs["young"], self.coeffs["poisson"], factor)
        return  HookeLocalOperator @  ToVoigtEpsilon(Strain(utGlobal,self.spaceDimension) + inherentStrainsMatrix*XLi + thermalExpansionOp*deltaTemperature )


    def GetVonMisesData(self, stress:ArrayLike):
        """Computes the VonMises for stress tensor S

        Parameters
        ----------
        s : ArrayLike
            a 3x3 symmetric tensor

        Returns
        -------
        scalar
            the von Mises for the stress tensor
        """
        return  (1/np.sqrt(2))*np.sqrt((stress[0,0]-stress[1,1])**2 +(stress[1,1]-stress[2,2])**2 +(stress[2,2]-stress[0,0])**2 + 6*(stress[0,1]**2 +stress[0,2]**2 +stress[1,2]**2 ))


    def VonMises(self,stress,bulkAlpha):
        """Return a von Mises FE field

        Parameters
        ----------
        stress : FE field

        Returns
        -------
        FE field
            the von Mises for the stress tensor
        """
        vonmises = FEField(name = "VonMises",mesh=stress[0].mesh,space=stress[0].space,numbering=stress[0].numbering)
        vonmises.Allocate()
        stressElem = [s.GetCellRepresentation()*bulkAlpha for s in stress ]
        vonmises.data = self.GetVonMisesData(FromVoigtSigma(np.array(stressElem)))

        return vonmises

    def DeviatoricStress(self,stress,bulkAlpha):
        """Return a deviatoric stress FE field

        Parameters
        ----------
        stress : FE field

        Returns
        -------
        FE field
            the deviatoric stress tensor
        """
        deviatoricstress  = [ FEField(name = "DeviatoricStress"+str(i),mesh=stress[0].mesh,space=stress[0].space,numbering=stress[0].numbering) for i in range(6)]
        stressElem = [s.GetCellRepresentation()*bulkAlpha for s in stress ]
        trace = (1./3)*(stressElem[0]+stressElem[1]+stressElem[2])
        for i in range(3):
            deviatoricstress[i].Allocate()
            deviatoricstress[i].data =  stressElem[i] - trace
        for i in range(3,6):
            deviatoricstress[i].Allocate()
            deviatoricstress[i].data =  stressElem[i]

        return deviatoricstress

    def Stress(self, u, XLi, factor=None):
        stressVoigt = self.GetStressVoigt(u, XLi=XLi, factor=factor)
        return FromVoigtSigma(stressVoigt.T)

    def Strain(self, u):
        return Strain(u,self.spaceDimension)

class AMInherentStrainSolver(AMProcessBase):
    """Class to simulate a AM process with the inherent strain method

    Parameters
    ----------
    inherentStrainsCoefficients : 3 np.ndarray
        _description_

    """
    def __init__(self):
        super().__init__()
        self.inherentStrainsCoefficients = np.array([-0.001,-0.001,-0.001], dtype=MuscatFloat)
        self.propsFields = {}
        self.elemFields = {}
        self.addPreconstraintFormulation = False

    def GetMaterialPropertyCoefficient(self,name):
        return self.propsFields[name][0][1]

    def SetIStrains(self,vals):
        self.inherentStrainsCoefficients = PH.ReadFloats(vals)

    def SetUseSupport(self,vals):
        self.useSupport = PH.ReadBool(vals)


    def Solve(self, _levelSet):

        self.PreStart()
        self.initialSupport = _levelSet.support
        boundingMin, boundingMax =_levelSet.support.ComputeBoundingBox()
        numberOfStepZ = self.nz 
        deltaZ = (boundingMax[2]-boundingMin[2])/(numberOfStepZ-1)
        computationLevelSet = self.GetComputationalLevelset(_levelSet, useSupport=self.useSupport)
        self.computationLevelSet = computationLevelSet
        computationMesh = computationLevelSet.support

        # the z for the first layer
        zBulk = boundingMin[2] + deltaZ

        # Definition for the Filter to define the bulk and the top layer
        bulkElementFilter = ElementFilter(dimensionality=3)
        bulkElementFilter.SetZonesTreatment("CENTER")

        # Initialization of the FE Analysis class
        solver = UnstructuredFEAMecaProblem()
        solver.ComputeDomainToTreatLs(_levelSet)

        # physics definition
        mecaInherentPhysics = MecaInherentStrain()
        young = self.propsFields["young"][0][1]
        poisson = self.propsFields["poisson"][0][1]
        mecaInherentPhysics.coeffs["young"] = young
        solver.internalSolveur.constants["young"] = young
        mecaInherentPhysics.coeffs["poisson"] = poisson
        solver.internalSolveur.constants["poisson"] = poisson
        if "thermalExpansion" not in self.propsFields.keys():
            mecaInherentPhysics.coeffs["thermalExpansionCoefficient"]=0

        numberingP0 = ComputeDofNumbering(computationMesh, LagrangeSpaceP0)

        for i in range(3):
            mecaInherentPhysics.coeffs["inherentStrainsCoefficients_"+str(i)] = self.inherentStrainsCoefficients[i]

        # this is not very clean we ne to put some order the places to store material data

        mecaInherentPhysics.SetSpaceToLagrange(isoParam=True)

        # Add weak formulation to be treated
        inside3DElementFilter = ElementFilter(eTag=TZ.Inside3D,dimensionality=3)
        mecaInherentPhysics.AddBFormulation(inside3DElementFilter, mecaInherentPhysics.GetBulkFormulation(young=young,poisson=poisson) )

        if self.addPreconstraintFormulation:
           e = Strain(mecaInherentPhysics.primalTest,3)
           preSigma = GetField("PreSigma",6,sdim=3)
           wf = tensorcontraction(tensorproduct(FromVoigtSigma(preSigma),e),(0,3),(1,2))

           mecaInherentPhysics.AddLFormulation(inside3DElementFilter,wf)
           preSigmafield = [ ElemFieldsToFEField(computationMesh, {"PreSigma_"+str(i):self.elemFields["PreSigma_"+str(i)]}) for i in range(6)]
           for field in preSigmafield:
               solver.internalSolveur.fields.update(field)

        solver.internalSolveur.physics.append(mecaInherentPhysics)

        Info(" apply of boundary conditions")
        # Apply boundary conditions
        self.dirichletBCs = []
        for on,block in self.boundaryConditions:
            dirichlet = KRBlockVector()
            dirichlet.AddArg("u").To(offset=[0,0,0])
            for name in on:
                dirichlet.On(name)
                solver.tagsToKeep.append(name)
            for dir in block:
                getattr(dirichlet, f"BlockDirection{dir}")(dir)
            dirichlet.constraintDirections= "Global"
            self.dirichletBCs += [dirichlet]
            solver.internalSolveur.solver.constraints.AddConstraint(dirichlet)
        Info(" apply of boundary conditions Done")

        # we work on conform meshes,
        solver.conform = computationLevelSet.conform

        originalPhi = np.copy(computationLevelSet.phi)

        # accumulated variables
        accumulatedDisplacement = np.zeros((computationMesh.GetNumberOfNodes(),3))
        stressNames = ["xx", "yy", "zz", "yz", "xz", "xy"]
        accumulatedStress = [0.,0.,0.,0.,0.,0.]
        individualDisplacements = []

        # Tolerance for the layed detection
        self.layerDetectionTol =  deltaZ/100.

        if  self.useProjectionInitialization:
            transOP =  self.GetProjectionInitialization(computationMesh, deltaZ)
        else:
            transOP = None


        # we keep track of the old solution so we can transported by a delta_z
        oldSol = np.zeros((computationMesh.GetNumberOfNodes(),3))

        # mask over the node to define the extractor
        usedNodes = np.zeros(computationMesh.GetNumberOfNodes())

        zAtIP = self.GetZPosAtIntegrationPoints(computationMesh)

        Info("----------------------- Preprocessing Done ------------------------" )
        zBulk = 0.

        negPhi = computationLevelSet.phi <= 0

        self.mecaInherentStrain = mecaInherentPhysics
        self.numberOfStepZ = numberOfStepZ

        self.solutionByLayer = []
        self.XliByLayer = []
        self.stressByLayer = []
        self.phiByLayer = []
        self.bulkAlphaByLayer = []
        self.strainByLayer = []
        self.vonMisesByLayer=[]
        self.deviatoricstressByLayer = []
        accumulatedVonMises = 0.

        _, boundingMax = computationMesh.ComputeBoundingBox()

        for zCpt in range( numberOfStepZ):
            initTime = time.time()

            #% clean cache of numberings
            DN.__cache__ = {}

            # z defining the current z
            zBulk = boundingMin[2] + deltaZ*(zCpt+1)
            Info(f"Computing for total height of {zBulk:.2f}/{boundingMax[2]:.2f} {zCpt}/{numberOfStepZ}" )

            #% Cut the levelSet by the zBulk
            computationLevelSet.phi[:] = np.maximum(originalPhi, computationMesh.nodes[:,2]-(zBulk+self.layerDetectionTol) )
            self.phiByLayer += [computationLevelSet.phi[:]]

            #Update the bulkElementFilter with the correct zone
            bulk = lambda xyz,zBulkVal = zBulk: xyz[:,2]-(zBulkVal+self.layerDetectionTol)
            bulkElementFilter.zones = [bulk]

            #% reset mask
            usedNodes.fill(0)

            #% create the Inside3D  tag to integrate the weak formulation
            for selection in bulkElementFilter(computationMesh):
                negPhiIds = np.nonzero(np.sum(negPhi[selection.elements.connectivity],axis=1) > 0)[0]
                inside3dIds = np.intersect1d(negPhiIds,selection.indices,assume_unique=True)
                selection.elements.tags.CreateTag(TZ.Inside3D,False).SetIds(inside3dIds)
                usedNodes[selection.elements.connectivity[inside3dIds,:].ravel()] = True

            #Info("PreSolver")
            solver.PreSolver(computationLevelSet)
            #Info("PreSolver Done")

            #% creation of the XLi characteristic function this
            XLi = (Maximum(-(zAtIP-(zBulk-deltaZ) ),zAtIP-zBulk)<0)*1.
            XLi.name = "XLi"
            self.XliByLayer += [XLi]
            solver.AddField(XLi)

            Info("Solve")

            #% transport of old solution
            if self.useProjectionInitialization:
                oldSol = transOP.dot(oldSol)
                # initialization of of the solver with an approximation of the solution
                solver.SetNodalSolution(oldSol)

            #% Solve
            solver.Solve()
            Info("Solve Done")

            Info("Accumulate the solution only on used nodes")
            #% Accumulate the solution only on used nodes
            oldSol  = solver.GetNodalSolution()
            self.solutionByLayer += [oldSol.copy()]

            accumulatedDisplacement += oldSol* usedNodes[:,None]

            Info("Store the current solution")
            #% Store the current solution
            individualDisplacements.append(oldSol.copy())

            XliField = solver.GetAuxiliaryField("XLi", on=FN.IPField)
            Info("Store the current solution 2")
            uFields = [ solver.GetAuxiliaryField("u_"+str(i),on =FN.FEField)  for i in range(3) ]
            Info("Store the current solution 3")
            stress = self.ComputeStress(numberingP0,mecaInherentPhysics,uFields,XliField,on=inside3DElementFilter,computationMesh=computationMesh)
            strain = self.ComputeStrain(numberingP0,mecaInherentPhysics, uFields, XliField, on=inside3DElementFilter,computationMesh=computationMesh)

            self.stressByLayer += [stress]
            self.strainByLayer += [strain]

            Info("stress on cells")
            #% stress on cells
            bulkAlpha = (zAtIP.GetCellRepresentation() < zBulk)*1.0
            self.bulkAlphaByLayer += [bulkAlpha]
            vonMises = mecaInherentPhysics.VonMises(stress,bulkAlpha)
            deviatoricstress = mecaInherentPhysics.DeviatoricStress(stress,bulkAlpha)
            self.deviatoricstressByLayer += [deviatoricstress]
            self.vonMisesByLayer += [vonMises]
            stress = [s.GetCellRepresentation()*bulkAlpha for s in stress ]
            accumulatedStress = [s+a for s,a in zip(stress,accumulatedStress)]
            vmises = mecaInherentPhysics.GetVonMisesData(FromVoigtSigma(np.array(stress)))

            Info("levelSet on the top")
            #% levelSet on the top
            #top = topFunc(computationLevelSet.support.nodes)
            accumulatedVonMises += vmises


            if self._debugWriter_ is not None:
                computationLevelSet.support.nodeFields["ADisplacement"] = accumulatedDisplacement.copy()
                computationLevelSet.support.nodeFields["U"] = oldSol
                computationLevelSet.support.nodeFields["XLi"] = XLi.GetPointRepresentation()
                computationLevelSet.support.elemFields = {"Stress"+str(a):f for a,f in zip(stressNames,accumulatedStress)}
                computationLevelSet.support.elemFields.update({"VonMises":vmises})
                computationLevelSet.support.elemFields.update({"AVonMises":accumulatedVonMises})

                computationLevelSet.support.elemFields["XLi"] = XLi.GetCellRepresentation()
                self.WriteDebug(computationLevelSet)

            Info(str(time.time()-initTime) +  "--------------------------------------------------------------------------------------")

        computationLevelSet.support.nodeFields["ADisplacement"] = accumulatedDisplacement
        computationLevelSet.support.nodeFields["U"] = oldSol


        space, numberings, _, _ = PrepareFEComputation(computationLevelSet.support,numberOfComponents=1)
        tempfield = FEField("",mesh=computationLevelSet.support,space=space,numbering=numberings[0])
        op, _ , _ = GetFieldTransferOp(tempfield,_levelSet.support.nodes,method="Interp/Clamp" )
        self.nodalsolution = op.dot(accumulatedDisplacement)

        self.solveCpt += 1
        self.PostSolve()

    def ComputeStress(self, numberingP0, mecaInherentPhysics, uFields, XliField, on, computationMesh):
        u = mecaInherentPhysics.primalUnknown
        stressT = GetTestField("SS",6,sdim=3)
        stressVoigt = mecaInherentPhysics.GetStressVoigt(u)
        wf = Inner(stressVoigt ,stressT)
        numP0 = numberingP0
        fields = [XliField]
        fields.extend(uFields)

        unknownFields = [ FEField("SS_"+str(i),mesh=computationMesh,space=LagrangeSpaceP0,numbering=numP0 ) for i in range(6)]
        _,f = IntegrateGeneral(mesh=computationMesh,
                               wform=wf,
                               constants={},
                               fields=fields,
                               unknownFields= unknownFields,
                               integrationRule=XliField.rule,
                               onlyEvaluation=True,
                               elementFilter=on
                               )

        VectorToFEFieldsData(f,unknownFields)
        return unknownFields

    def ComputeStrain(self, numberingP0, mecaInherentPhysics, uFields, XliField, on, computationMesh):
        u = mecaInherentPhysics.primalUnknown
        strainT = GetTestField("EE",6,sdim=3)

        wf = ToVoigtEpsilon(mecaInherentPhysics.Strain(u)).T @ strainT

        numP0 = numberingP0
        fields=[]
        fields.extend(uFields)

        unknownFields = [ FEField("EE_"+str(i),mesh=computationMesh,space=LagrangeSpaceP0,numbering=numP0 ) for i in range(6)]
        _,f = IntegrateGeneral(mesh=computationMesh,
                               wform=wf,
                               constants={},
                               fields=fields,
                               unknownFields= unknownFields,
                               integrationRule=XliField.rule,
                               onlyEvaluation=True,
                               elementFilter=on
                               )

        VectorToFEFieldsData(f,unknownFields)
        return unknownFields

    def SolveByLevelSet(self, levelSet):
        #self.PreSolver(levelSet)
        self.Solve(levelSet)
        #self.PostSolver()
        return RETURN_SUCCESS

    def GetNodalSolution(self, i = 0):
        return self.nodalsolution

    def GetNodalSolutionLayer(self, layer=0):
        return self.solutionByLayer[layer]

RegisterClass("AMProcess", None,CreateStructuredAMProcess)

from Muscat.MeshTools.MeshCreationTools import CreateCube
from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometryCylinder,ImplicitGeometryAxisAlignBox
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
from Muscat.Helpers.IO.Which import Which
from OpenPisco.CLApp.XmlToDic import XmlToDic
from OpenPisco.Unstructured.Levelset import LevelSet
import  OpenPisco.Unstructured.MmgMesher as MmgMesher

def CheckIntegrityStructured(GUI=False):
    dimensions = np.array([3,3,6],dtype=int)
    mesh = CreateCube(dimensions=dimensions,origin=[0,0,0],spacing=[5., 5., 10.]/(dimensions-1),ofTetras=False)

    ls = LevelSet(support=mesh)
    ls.conform = True
    AAB = ImplicitGeometryAxisAlignBox(origin=[1,1,-1], size=[4,4,4])
    ls.Initialize(AAB)
    class Almanac():
        def __init__(self):
            self.grids = {}
            self.levelSets = {}
            self.outputs = {}
            self.physicalProblems = {}

    objects = Almanac()
    objects.grids[1] = mesh
    objects.levelSets[1] = ls

    tempPath = TemporaryDirectory.GetTempPath()
    filename = tempPath + "CheckIntegrity_AMInherentStrainSolver.xdmf"
    writer = CreateWriter(filename )
    writer.SetFileName(filename )
    writer.SetTemporal()
    writer.SetBinary(False)
    writer.SetHdf5(True)
    writer.automaticOpen = True
    objects.outputs[1] = writer
    writer.Write(mesh,PointFields= [ls.phi],PointFieldsNames=["phi"])
    writer.Close()

    stringIStrains = f"""
        <AMProcess id="1" type="IStrains" IStrains="-0.001 -0.001 -0.001" debugFilename="{tempPath}IStrainsAMProcessStructured"  >
         <Material  young="110000000000" poisson="0.26" density="1" />
         <Block  on="Z0" block="0 1 2"  />
         <Block  on="X0" block="0"  />
         <Block  on="Y0" block="1"  />
        </AMProcess>
        """
    inputReader = XmlToDic()
    _, data = inputReader.ReadFromString(stringIStrains)
    data.pop("id",None)
    inputReader.ReplaceObjectFromAlmanac(data,objects)
    obj = CreateStructuredAMProcess(data)
    obj.SolveByLevelSet(ls)
    return "ok"


def CheckIntegrityUnstructured(GUI=False):
    if not MmgAvailable:
        return "Not Ok, mmg3d_O3 not found!!"

    tempPath = TemporaryDirectory.GetTempPath()
    string = f"""
        <AMProcess id="1" type="IStrains" IStrains="-0.001 -0.001 -0.001" debugFilename="{tempPath}IStrainsAMProcessUnstructured"  >
         <Material  young="110000000000" poisson="0.26" density="1" />
         <Block  on="Z0" block="0 1 2"  />
         <Block  on="X0" block="0"  />
         <Block  on="Y0" block="1"  />
        </AMProcess>
        """
    inputReader = XmlToDic()
    _, data = inputReader.ReadFromString(string)
    data.pop("id",None)
    mesh = CreateCube(dimensions=[3,3,6],origin=[0,0,0],spacing=[1./2, 1./2, 5./6], ofTetras=True)

    remeshInfo={"hausd":0.3, "hmin":0.3, "hmax":0.7, "nr":False}


    unstructMesh=MmgMesher.MmgMesherActionRemesh(mesh,remeshInfo)

    ls = LevelSet(support=unstructMesh)

    Phi0 = ImplicitGeometryCylinder(center1=[0.5,0.5,-0.01], center2=[0.5,0.5,5.1],radius=0.25)
    ls.Initialize(Phi0)

    class Almanac():
        def __init__(self):
            self.grids = {}
            self.levelSets = {}
            self.outputs = {}
            self.physicalProblems = {}

    objects = Almanac()
    objects.grids[1] = unstructMesh
    objects.levelSets[1] = ls
    filename = tempPath+"AMProcessUnstructured.xdmf"
    writer = CreateWriter(filename)
    writer.SetFileName(filename)
    writer.SetTemporal()
    writer.SetBinary()
    writer.automaticOpen = True
    objects.outputs[1] = writer
    inputReader.ReplaceObjectFromAlmanac(data,objects)
    obj = CreateStructuredAMProcess(data)
    obj.useLevelsetMesh = False
    obj.SolveByLevelSet(ls)
    accDisplacement = obj.GetNodalSolution()
    writer.Write(unstructMesh,PointFields= [ls.phi,accDisplacement],PointFieldsNames=["phi","displacement"])
    writer.Close()
    return "ok"

def CheckIntegrity(GUI=False):
    totest = [CheckIntegrityUnstructured,
    CheckIntegrityUnstructured]

    for test in totest:
        res =  test()
        if  res.lower() != "ok" :
            return res
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
