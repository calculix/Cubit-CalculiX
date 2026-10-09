# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.Logger import Info, Debug, ResetStartTime
from Muscat.IO.IOFactory import CreateWriter,InitAllWriters
from Muscat.Types import MuscatFloat, MuscatIndex
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshTools.MeshModificationTools import ComputeSkin
from Muscat.MeshTools.MeshFieldOperations import GetFieldTransferOp
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.FE.UnstructuredFeaSym import UnstructuredFeaSym
from Muscat.FE.SymPhysics import ThermalPhysics
from Muscat.FE.KR.KRBlock import KRBlockVector
from Muscat.FE.Fields.FieldTools import GetPointRepresentation, Minimum, Maximum
from Muscat.FE.Fields.FieldTools  import NodeFieldToFEField, GetTransferOpToIPField
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceGeo
from Muscat.FE.SymWeakForm import ToVoigtEpsilon,Strain, Inner, Diag, FromVoigtSigma, GetScalarField
import Muscat.FE.DofNumbering as DN

from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import  RegisterClass
from OpenPisco.PhysicalSolvers.AMProcessBase import AMProcessBase, ReadZoneMaterialData
from OpenPisco.PhysicalSolvers.UnstructuredFEAGenericSolver import CreateUnstructuredFEAGenericProblem, MecaThermalStrain
import OpenPisco.TopoZones as TZ

InitAllWriters()

def TagInMeshForDimension(mesh,dim):
    from Muscat.MeshContainers.ElementsDescription import dimensionality
    tagsInDim = [elemContainer.tags.storage for elemContainer in mesh.elements.storage if dimensionality[elemContainer.elementType]==dim]
    tagsInDim = [tag.name for tag in tagsInDim[0]]
    return tagsInDim

def CreateAMThermalDilatationSolver(ops):
    res = AMThermalDilatationSolver()
    del ops["type"]

    dim = PH.ReadInt(ops.get('dimensionality',3))
    assert dim == 3,"(Thermal dilatation Solver) Can only treat dimensionality 3"
    if 'dimensionality' in ops:
        del(ops['dimensionality'])

    bc = []
    for tag, child in ops["children"]:
        if tag == "Powder":
            res.powder = ReadZoneMaterialData(child)
        elif tag == "Solid":
            res.solid = ReadZoneMaterialData(child)
        elif tag == "Block":
            on = PH.ReadStrings(child["on"])
            block = PH.ReadInts(child["block"])
            bc.append((on,block))

    if len(bc) != 0:
        res.SetBoundaryConditions(bc)

    del ops["children"]
    PH.ReadProperties(ops,ops.keys(), res, typeConversion=True)

    return res
RegisterClass("AMThermalDilatationSolver", None,CreateAMThermalDilatationSolver)

class AMThermalDilatationSolver(AMProcessBase):
    def __init__(self) -> None:
        super().__init__()
        self.materials = []

        self.boundaryConditions = [(['Z0'],[0,1,2])] # default
        self.Tini = 0.
        self.HeatTime = 0.
        self.CoolTime = 0.
        self.deltaTime = 0.
        self.nodeFields = {}
        self.otherLevelSetMeshofTetra = False
        self.theta = 0.75
        self.nx = 30
        self.ny = 30
        self.saveStepFreq = 1

    def SolveByLevelSet(self, _levelSet):
        self.PreStart()

        origin, boundingMax = _levelSet.support.ComputeBoundingBox()
        numberOfStepZ = self.nz
        deltaZ = (boundingMax[2]-origin[2])/(numberOfStepZ-1)

        Info(f"numberOfStepZ: {numberOfStepZ}")
        computationLevelSet = self.GetComputationalLevelset(_levelSet)
        computationMesh = computationLevelSet.support
        computationMesh.nodesTags.DeleteTags(computationMesh.nodesTags.keys())
        computationMesh.ComputeBoundingBox()
        computationMesh.DeleteElemTags(["X0","X1","Y0","Y1","Z1","Skin"])
        self.computationLevelSet = computationLevelSet

        zBulk = origin[2] + deltaZ

        # Definition for the Filter to define the solid only missing the mask for the top layer
        solidElementFilter = ElementFilter(dimensionality=3, eTag="Solid")
        solidElementFilter.SetZonesTreatment("CENTER")

        # Definition for the Filter to define the powder only missing the mask for the top layer
        bulkElementFilter = ElementFilter(dimensionality=3, eTag="Powder")
        bulkElementFilter.SetZonesTreatment("CENTER")

        inside3DElementFilter = ElementFilter(eTag=TZ.Inside3D, dimensionality=3)

        internalSolverThermal = UnstructuredFeaSym()
        thermalPhysics = ThermalPhysics()
        internalSolverThermal.physics.append(thermalPhysics)

        thermalPhysics.SetSpaceToLagrange(isoParam=True)
        # thermal weak formulation to be treated
        thermalMassMatrix =  (inside3DElementFilter,  thermalPhysics.GetMassOperator(rho="rho",cp="cp") )
        thermalRigidityMatrix =  (inside3DElementFilter,  thermalPhysics.GetBulkFormulation(alpha="thermalDiffusion") )
        volumeFlux = (inside3DElementFilter,thermalPhysics.GetNormalFlux("QXLi") )
        #WARNING for the moment I didnt have the time to verified the validity of this
        # I use a volume model for the surface, (I know that I have to create 2D element for doing the correct integration)
        # but
        surfaceFlux = (inside3DElementFilter ,thermalPhysics.GetRobinFlux(beta="BetaXli", Tinf=self.Tini ) )

        internalSolverMeca = UnstructuredFeaSym()
        mecaThermalStrain = MecaThermalStrain()
        mecaThermalStrain.SetSpaceToLagrange(isoParam=True)
        #the Basic Tools cant treat field in the denominator, so poisson value must be constant for each integral
        mecaThermalStrain.coeffs["poisson"]=0.25
        mecaThermalStrain.AddBFormulation(inside3DElementFilter, mecaThermalStrain.GetBulkFormulation(young="young",poisson=0.25) )
        internalSolverMeca.physics.append(mecaThermalStrain)
        Info(" apply of boundary conditions for the meca")

        # Apply boundary conditions
        for on,block in self.boundaryConditions:
            assert on[0] in TagInMeshForDimension(mesh=computationLevelSet.support,dim=2),"Surfacic tag "+str(on[0])+" for dirichlet condition does not exist in the mesh"

            dirichlet = KRBlockVector()
            dirichlet.AddArg("u").To(offset=[0,0,0])
            for name in on:
                dirichlet.On(name)
            for dir in block:
                getattr(dirichlet, f"BlockDirection{dir}")(dir)
            dirichlet.constraintDirections= "Global"
            internalSolverMeca.solver.constraints.AddConstraint(dirichlet)
        Info(" apply of boundary conditions for the meca Done")

        # accumulated variables
        accumulatedDisplacement = np.zeros((computationMesh.GetNumberOfNodes(),3))

        # Tolerance for the layed detection
        numberingPGeo = DN.ComputeDofNumbering(computationMesh, LagrangeSpaceGeo,fromConnectivity=True)

        # mask over the node to define the extractor
        usedNodes = np.zeros(computationMesh.GetNumberOfNodes())

        feFields = NodeFieldToFEField(computationLevelSet.support, {"phi":computationLevelSet.phi,"coordZ":computationLevelSet.support.nodes[:,2]})
        AtIpOp = GetTransferOpToIPField(inputField=feFields["coordZ"])

        zAtIP = AtIpOp.dot(feFields["coordZ"])
        phiAtIP = AtIpOp.dot(feFields["phi"])

        def GenerateBiMaterialField(propname,fieldname):
            lf = (phiAtIP>0)*self.powder[propname] + (phiAtIP<=0)*self.solid[propname]
            lf.name = fieldname
            return lf

        Q = GenerateBiMaterialField("IncomePower","Q")
        rho = GenerateBiMaterialField("density","rho")
        cp = GenerateBiMaterialField("thermalCapacity","cp")
        thermalDiffusion = GenerateBiMaterialField("thermalDiffusion","thermalDiffusion")
        young = GenerateBiMaterialField("young","young")
        thermalExpansion = GenerateBiMaterialField("thermalExpansion","thermalExpansion")
        referenceTemperature = GenerateBiMaterialField("referenceTemperature","referenceTemperature")
        thermalConvection = GenerateBiMaterialField("thermalConvection","thermalConvection")

        TrefSolide = self.solid["referenceTemperature"]

        Debug("----------------------- Preprocessing Done ------------------------" )
        zBulk = 0.

        internalSolverMeca.Reset()
        internalSolverMeca.solver.SetAlgo("Direct")
        internalSolverMeca.fields["young"] = young

        internalSolverThermal.Reset()
        internalSolverThermal.fields["rho"] = rho
        internalSolverThermal.fields["cp"] = cp
        internalSolverThermal.fields["thermalDiffusion"] = thermalDiffusion
        Tglobal = np.full(computationMesh.GetNumberOfNodes(),self.Tini, dtype=MuscatFloat)

        allSubdomainDisplacements=[]
        for zCpt in range( numberOfStepZ):
            ResetStartTime()

            #% clean cache of numberings
            DN.__cache__ = {}

            # z defining the current z
            zBulk = origin[2] + deltaZ*(zCpt+1)
            Info(f"computing layer number {zCpt} of {numberOfStepZ}, at z: {zBulk:.5f}")

            #% Cut the levelSet by the zBulk
            #% reset mask
            usedNodes.fill(0)

            #% create the Inside3D  tag to integrate the weak formulation
            for selection in ElementFilter(dimensionality=3, zone=lambda xyz: xyz[:,2]-zBulk, zonesTreatment="CENTER")(mesh=computationMesh):
                selection.elements.tags.CreateTag(TZ.Inside3D,False).SetIds(selection.indices)
                usedNodes[selection.elements.connectivity[selection.indices,:].ravel()] = True

            self.numbering = None

            internalSolverThermal.SetMesh(computationLevelSet.support)
            internalSolverThermal.ComputeDofNumbering([TZ.Inside3D])

            internalSolverMeca.SetMesh(computationLevelSet.support)
            internalSolverMeca.ComputeDofNumbering([TZ.Inside3D])

            #% creation of the XLi characteristic function this
            XLi = (Maximum(-(zAtIP-(zBulk-deltaZ) ),zAtIP-zBulk)<0)*1.
            XLi.name = "XLi"
            internalSolverThermal.fields["XLi"] = XLi
            QXLi = Q*XLi/deltaZ
            QXLi.name = "QXLi"
            internalSolverThermal.fields["QXLi"] = QXLi

            thermalConvectionXLi = thermalConvection*XLi/deltaZ
            thermalConvectionXLi.name = "BetaXli"
            internalSolverThermal.fields["BetaXli"] = thermalConvectionXLi
            internalSolverMeca.fields["XLi"] = XLi

            thermalExpansionXLi = thermalExpansion*XLi
            thermalExpansionXLi.name = "thermalExpansion"
            internalSolverMeca.fields["thermalExpansion"] = thermalExpansionXLi

            internalSolverThermal.unknownFields[0].SetDataFromPointRepresentation(Tglobal)

            T = internalSolverThermal.unknownFields[0].data
            time = 0

            M,_ = internalSolverThermal.GetLinearProblem(bform= [thermalMassMatrix])
            K,_ = internalSolverThermal.GetLinearProblem(bform= [thermalRigidityMatrix])
            _,f1 = internalSolverThermal.GetLinearProblem(lform= [volumeFlux ])
            M2,f2 = internalSolverThermal.GetLinearProblem(bform= [surfaceFlux ])
            M = M + M2
            f = f1 + f2
            rhs = f - K.dot(T)
            internalSolverThermal.Solve(M, rhs)
            Tp = internalSolverThermal.sol
            theta = self.theta

            basedofs = np.array([],dtype=MuscatIndex)
            for selection in ElementFilter(dimensionality=2,eTag="Z0")(computationMesh):
                basedofs= np.union1d(np.unique(internalSolverThermal.numberings[0][selection.elementType][selection.indices,:].ravel()),basedofs)

            if self._debugWriter_ is not None and zCpt%self.saveStepFreq == 0:
                computationLevelSet.support.nodeFields["accumulatedDisplacement"] = accumulatedDisplacement.copy()
                computationLevelSet.support.nodeFields["Temperature"] = Tglobal
                computationLevelSet.support.nodeFields["u"] = GetPointRepresentation(internalSolverMeca.unknownFields)
                self.WriteDebug(computationLevelSet,Time=self.solveCpt*10)
            dt = self.deltaTime

            Info("Computing for time: ")
            timeStepCpt = 0
            while time <  (self.HeatTime+ self.CoolTime)  :
                time += dt

                Info(f"{time}, ")
                TemperaturePredictor = T + (1-theta)*dt*Tp

                internalSolverThermal.Solve((M + theta*dt*K) ,f - K.dot(TemperaturePredictor) )
                Tp = internalSolverThermal.sol
                T =  TemperaturePredictor + theta*dt*Tp
                T[basedofs] = self.Tini
                T[T>TrefSolide] = TrefSolide

                if time >= self.HeatTime and dt == self.deltaTime:
                    f = f2
                    dt = self.deltaTime*10
                    internalSolverThermal.Solve((M + theta*dt*K) ,f - K.dot(TemperaturePredictor) )
                    Tp = internalSolverThermal.sol

                internalSolverThermal.sol = T
                internalSolverThermal.PushSolutionVectorToUnknownFields()
                TatPointsData = internalSolverThermal.unknownFields[0].GetPointRepresentation()
                TatPoints = FEField(name="TatPoints", mesh=computationMesh, space=LagrangeSpaceGeo, numbering=numberingPGeo,data=TatPointsData)
                TatIP = AtIpOp.dot(TatPoints)
                timeStepCpt +=1

                Tglobal = internalSolverThermal.unknownFields[0].GetPointRepresentation(fillValue= self.Tini)
                assert np.max(np.abs(Tglobal))<1e10,"Thermal solver has failed at time step "+str(timeStepCpt)

            deltaTemperature = Minimum(TatIP, referenceTemperature) - referenceTemperature
            deltaTemperature.name = "deltaTemperature"
            internalSolverMeca.fields["deltaTemperature"] = deltaTemperature
            Kmeca,fmeca = internalSolverMeca.GetLinearProblem()
            internalSolverMeca.ComputeConstraintsEquations()
            internalSolverMeca.Solve(Kmeca,fmeca)

            internalSolverMeca.PushSolutionVectorToUnknownFields()
            u = GetPointRepresentation(internalSolverMeca.unknownFields)
            assert np.max(np.abs(u))<1e10,"Mechanical solver has failed"
            allSubdomainDisplacements.append(u)
            accumulatedDisplacement += u
            if self._debugWriter_ is not None and zCpt%self.saveStepFreq == 0:
                computationLevelSet.support.nodeFields["Temperature"] = TatPointsData
                computationLevelSet.support.nodeFields["u"] = u
                computationLevelSet.support.nodeFields["accumulatedDisplacement"] = accumulatedDisplacement.copy()
                self.WriteDebug(computationLevelSet,Time=time+self.solveCpt*10)

            self.solveCpt += 1

        self.allSubdomainDisplacements=np.array(allSubdomainDisplacements)
        Info("Done")
        self.PostSolve()
        self.accumulatedDisplacement = accumulatedDisplacement
        self.mecaThermalStrain = mecaThermalStrain
        #transfer computationLevelSet -> _levelSet.support

    def GetPointRepresentation(self):
        from Muscat.FE.Fields.FieldTools import GetPointRepresentation
        return GetPointRepresentation(self.internalSolver.unknownFields)


def ComputeNodalFieldProjection(nodalField,originalSupport,targetSupport):
    from Muscat.FE.FETools import PrepareFEComputation
    space, numberings, _, _ = PrepareFEComputation(originalSupport,numberOfComponents=3)
    from Muscat.FE.Fields.FEField import FEField
    tempfield = FEField("",mesh=originalSupport,space=space,numbering=numberings[0])
    op, _, _ = GetFieldTransferOp(tempfield,targetSupport.nodes,method="Interp/Clamp" )
    projectedNodalField = op.dot(nodalField)
    return projectedNodalField

def ExtractThermoMecaInformations(thermoMecaProblem):
    aggregatedDispTh=ComputeThermalAggregatedDisplacement(thermoMecaProblem)
    mecaProperties={"young":thermoMecaProblem.solid["young"],
                    "poisson":thermoMecaProblem.solid["poisson"]
                    }
    computationLevelSetTh=thermoMecaProblem.computationLevelSet
    return aggregatedDispTh,mecaProperties,computationLevelSetTh

def ComputeThermalAggregatedDisplacement(thermoMecaProblem):
    thermoMecaSubdomainDisps=thermoMecaProblem.allSubdomainDisplacements
    thermoMecaSubdomainDispsNorm=np.linalg.norm(thermoMecaSubdomainDisps,ord=2,axis=2)
    maxDispsNormIndex=np.argmax(thermoMecaSubdomainDispsNorm,axis=0)
    nbNodes=thermoMecaSubdomainDisps.shape[1]
    aggregatedDispTh=thermoMecaSubdomainDisps[maxDispsNormIndex,np.arange(nbNodes),:]
    return aggregatedDispTh

def ComputeReactionDisplacement(levelSet,mecaProperties,aggregatedDisp,boundEtag):
    mecaProblemData={'type': 'static_elastic', 'p': '1',
                    'children': [('Material', mecaProperties)]}
    phyProblem = CreateUnstructuredFEAGenericProblem(mecaProblemData)

    from Muscat.FE.KR.KRFromDistribution import KRFromDistribution
    dirichlet = KRFromDistribution()
    dirichlet.AddArg("u").On(boundEtag)
    phyProblem.tagsToKeep.append(boundEtag)
    phyProblem.delayedInitialization = {"HeterogeneousDirichlet":{"field":{"u":aggregatedDisp}, "condition":dirichlet}}
    phyProblem.SolveByLevelSet(levelSet=levelSet)
    reacDisp=phyProblem.GetNodalSolution()

    return phyProblem,reacDisp

def ComputeReactionForces(levelSet,problem):

    k,f = problem.internalSolveur.GetLinearProblem()

    fr2_in_internal_numbering = k.dot(problem.internalSolveur.sol)-f

    fr2_in_FEFiels_on_cleanMesh = []
    for i in range(3):
        fr2_in_FEFiels_on_cleanMesh.append(problem.internalSolveur.unknownFields[i].copy())
        fr2_in_FEFiels_on_cleanMesh[-1].name = "f_"+str(i)
    from Muscat.FE.Fields.FieldTools import VectorToFEFieldsData
    VectorToFEFieldsData(fr2_in_internal_numbering, fr2_in_FEFiels_on_cleanMesh)

    # get f in the mesh
    mesh = levelSet.support
    from Muscat.Types import MuscatFloat
    fr2_in_mesh = np.zeros((mesh.GetNumberOfNodes(),3),dtype=MuscatFloat)
    for i in range(3):
        fr2_in_mesh[fr2_in_FEFiels_on_cleanMesh[i].mesh.originalIDNodes,i] = fr2_in_FEFiels_on_cleanMesh[i].GetPointRepresentation()

    return fr2_in_mesh

def CheckIntegrity(GUI=False):
    from OpenPisco.CLApp.XmlToDic import XmlToDic
    from Muscat.MeshTools.MeshCreationTools import CreateCube

    from OpenPisco.Unstructured.Levelset import LevelSet

    n = np.array([26,26,51],dtype=int)
    ofTetras=True
    mesh = CreateCube(dimensions =n ,origin=[0,0,0],spacing=[0.05, 0.05, 0.10]/(n-1), ofTetras=ofTetras )
    assert TagInMeshForDimension(mesh=mesh,dim=3)==["3D"]

    ComputeSkin(mesh=mesh,inPlace=True)
    tetra = mesh.GetElementsOfType(ED.Tetrahedron_4)
    tetra.tags.CreateTag(TZ.Inside3D,False).SetIds(np.arange(tetra.GetNumberOfElements()))
    ls = LevelSet(support=mesh)
    ls.conform = True

    from Muscat.ImplicitGeometry.ImplicitGeometryObjects import ImplicitGeometryAxisAlignBox
    from Muscat.ImplicitGeometry.ImplicitGeometryOperators import ImplicitGeometryUnion

    AABI = ImplicitGeometryAxisAlignBox(origin=[0.005,0.005, 0.02], size=[0.04,0.04,0.01])
    AABII = ImplicitGeometryAxisAlignBox(origin=[0.02,0.02,-0.0001], size=[0.01,0.01,0.03])
    AAB = ImplicitGeometryUnion([AABI,AABII])
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
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
    tempPath = TemporaryDirectory.GetTempPath()

    filename = tempPath + "CheckIntegrity_AMThermalDilatationSolver.xdmf"
    writer = CreateWriter(filename )
    writer.SetFileName(filename )
    writer.SetTemporal()
    writer.SetBinary()
    writer.automaticOpen = True
    objects.outputs[1] = writer
    writer.Write(mesh,PointFields= [ls.phi],PointFieldsNames=["phi"])
    writer.Close()

    xmlData  = f"""
    <AMProcess id="1"  type="Thermal" debugFilename="{tempPath}ThermalAMProcess" Tini="24" HeatTime="0.1" CoolTime="5" deltaTime="0.02" theta="1.0" otherLevelSetMeshofTetra="True">


        <Powder
            young="1.6"
            poisson="0.25"
            thermalCapacity="700"
            thermalDiffusion="0.25"
            thermalExpansion="0.0000001"
            thermalConvection="50"
            density="2100"
            referenceTemperature="290-273"
            IncomePower="80000"/>

        <Solid
            young="110"
            poisson="0.25"
            thermalCapacity="610"
            thermalDiffusion="15"
            thermalExpansion="0.000009"
            thermalConvection="50"
            density="4300"
            referenceTemperature="1923-273"
            IncomePower="80000000"
            />
        </AMProcess> """

    inputReader = XmlToDic()
    _, data = inputReader.ReadFromString(xmlData)
    data.pop("id",None)
    inputReader.ReplaceObjectFromAlmanac(data,objects)
    obj = CreateAMThermalDilatationSolver(data)
    obj.SolveByLevelSet(ls)
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity(True))
