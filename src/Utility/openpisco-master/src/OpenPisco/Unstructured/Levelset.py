# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np
import scipy as sp

import Muscat.Helpers.ParserHelper as PH
from Muscat.Types import MuscatFloat
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.FE.KR.KRBlock import KRBlockScalar
from Muscat.MeshTools.MeshInspectionTools import GetVolumePerElement,ComputeMeshMinMaxLengthScale
from Muscat.FE.SymPhysics import BasicPhysics
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.UnstructuredFeaSym import UnstructuredFeaSym
from Muscat.FE.Spaces.FESpaces import LagrangeSpaceGeo
from Muscat.FE.Integration import IntegrateGeneral
from Muscat.Helpers.Logger import Info, Debug, Error
from Muscat.Helpers.IO.Which import Which

from OpenPisco.LevelSetBase import LevelSetBase
import OpenPisco.TopoZones as TZ
from OpenPisco.ConceptionFieldFactory import RegisterClass
from OpenPisco.Unstructured.Advect import Advect
from OpenPisco.Unstructured.Meshdist import Meshdist
from OpenPisco.BaseElements import elementsTypeSupportedForDimension, complexElementsTypesSupportedForDimension

AdvectionClass = Advect
ReDistancingClass = Meshdist

def ReadMeshInfo(child):
    import Muscat.Helpers.ParserHelper as PH

    opts = {}
    parameters = [("iso","float"),("hmax","float"),("hmin","float"),("ar","float"),
                  ("hausd","float"),("hgrad","float"),("nr","bool"),("optim","bool"),("optimLES","bool"),("keepGeneratedFiles","bool"),
                  ("setPhiZeroOnBoundary","bool"),
                  ("met","bool"),("computeDistanceWith","string"),("hsiz","float"),("rmc","float"),("symbolicMetricExpression","string")]

    for parameterName,parameterType in parameters:
        if parameterName in child:
            if parameterType=="float":
                value = PH.ReadFloat(child[parameterName])
            elif parameterType=="bool":
                value = PH.ReadBool(child[parameterName])
            elif parameterType=="string":
                value = PH.ReadString(child[parameterName])
            else:
                raise TypeError("Type of parameter not covered (handle float/bool/string)")
            opts[parameterName] = value
        else: #warn user that his parameter is not taken into account !!
            print("Parameter",parameterName,"not handled by the mesher")
            
    return opts


def CreateLevelSet(ops):
    """ Unstructured Levelset
        support : Grid (mesh)
        conform : for conform mesh to the iso 0 (remeshing) (False)
        useVtk : use vtk to compute the Gradient of phi (True)
        also a MeshInfo child element in the form of :

        <MesherInfo
        iso="0.0"
        hmin="0.05"
        hmax="0.5"
        hausd="0.1"
        nr="True"
        hgrad="1.1"
        <LocalParams eTag="tagName" hmin="0.05" hmax="0.5" hausd="0.1" dim="3" />
        <LocalParams eTag="2dtagName" hmin="2." hmax="1." hausd="2." dim="2" />
        MesherInfo />

        with the properties for the remesher algorithm

        iso  : iso surface to mesh (0.0)
        hmin : Minimal edge size
        hmax : Maximal edge size
        hsiz : Constant edge size
        rmc  : Remove connected components smaller than a given threshold
        ar   : Value for angle detection
        hausd: Maximal Hausdorff distance for the boundaries approximation
        nr   : No angle detection
        met  : use isotropic metric field 'met' to drive the remeshing

        computeDistanceWith (meshdist/vtk) : update signed distance function using meshdist/vtk
        keepGeneratedFiles                 : keep files generated during remeshing
    """
    assert "support" in ops,"Need a support to init a LevelSet3D object"
    res = LevelSet()
    res.SetSupport(ops["support"])
    res.conform = PH.ReadBool(ops.get("conform",res.conform))
    
    if "filterLevelSetField" in ops:
        res.filterLevelSetField = PH.ReadBool(ops.get("filterLevelSetField"))
        print("filterLevelSetField is set to",res.filterLevelSetField)
        
    if "regularizationLengthscaleParameter" in ops:
        res.regularizationLengthscaleParameter = PH.ReadFloat(ops.get("regularizationLengthscaleParameter"))
        print("regularizationLengthscaleParameter is set to",res.regularizationLengthscaleParameter)
        
    if PH.ReadBool(ops.get("useVtk", False)):
        res.computeGradPhiWith = "vtk"
    elif "gradientMethod" in ops:
        res.computeGradPhiWith = PH.ReadString(ops.get("gradientMethod")).lower()

    if "children" in ops:
        for tag,child in ops["children"]:
            if tag == "MesherInfo":
                opts = ReadMeshInfo(child)

            if "children" in child:
                localParams={}
                for tag2,child2 in child["children"]:
                    if tag2.lower()=="localparams":
                        localParam={}
                        localParam["hmin"] = PH.ReadFloat(child2["hmin"])
                        localParam["hmax"] = PH.ReadFloat(child2["hmax"])
                        localParam["hausd"] = PH.ReadFloat(child2["hausd"])
                        localParam["dim"] = PH.ReadInt(child2["dim"])
                        localParams[PH.ReadString(child2["eTag"])]=localParam
                opts["localParams"] = localParams

        res.MesherInfo = opts
    return res
    

class LevelSet(LevelSetBase):
    def __init__(self, other=None, support=None ):
        super(LevelSet, self).__init__(other=other, support=support)

        if other is not None:
            self.MesherInfo = other.MesherInfo
            self.conform = other.conform
            self.computeGradPhiWith = other.computeGradPhiWith
            self.massMatrix = other.massMatrix
            self.interfaceMassMatrix = other.interfaceMassMatrix
            self.filterLevelSetField = other.filterLevelSetField
            self.regularizationLengthscaleParameter = other.regularizationLengthscaleParameter
            self.__regularize_cache__ = other.__regularize_cache__
            self.__advect_cache__ =  other.__advect_cache__
            self.__redistancing_cache__ = other.__redistancing_cache__
            self.__mass_cache__ = other.__mass_cache__
            self.__lumped_mass_cache__ = other.__lumped_mass_cache__
        else:
            self.MesherInfo = {}
            self.conform = False
            self.computeGradPhiWith = "gradphi"
            self.massMatrix = None
            self.interfaceMassMatrix = None
            self.filterLevelSetField = False
            self.regularizationLengthscaleParameter = 0.0001
            self.__regularize_cache__ = None
            self.__advect_cache__ = None
            self.__redistancing_cache__ = None
            self.__mass_cache__ = None
            self.__lumped_mass_cache__ = None

    def CleanCachePhi(self) -> None:
        """Clean internal cache for phi dependent functions
        """
        self.__regularize_cache__ = None

    def CleanCacheMesh(self) -> None:
        """Clean internal cache for mesh dependent functions """

        self.CleanCachePhi()
        self.__advect_cache__ = None
        self.__redistancing_cache__ = None
        self.__mass_cache__ = None
        self.__lumped_mass_cache__ = None

    def DiscardState(self) -> None:
        super().DiscardState()
        self.CleanCachePhi()

    def SetSupport(self,support):
        super().SetSupport(support)
        self.CleanCacheMesh()

    def Initialize(self, Function):
        super().Initialize(Function)
        self.CleanCachePhi()

    def InitializePointByPoint(self, Function):
        super().InitializePointByPoint(Function)
        self.CleanCachePhi()

    def TakePhiAndMesh(self,phi,mesh):
        #crash if self.phi is not defined !
        #lenOldPhi = len(self.phi)
        if phi is not self.phi:
            self.phi = phi[:]
        if mesh is not self.support:
            self.support = mesh

        if len(self.phi) != self.support.GetNumberOfNodes() :
              self.phi = np.empty(self.support.GetNumberOfNodes(),dtype=MuscatFloat)

        #if lenOldPhi != self.support.GetNumberOfNodes() :
              #self.extraNodesFields = {}

        self.CleanCacheMesh()

    #
    def GetElementsVolumicFractions(self,phi=None ):
        """ get the negative fraction of the volume (only for tets)

        Parameters
        ----------
        phi : ndarray, optional
            a phi field, if None self.phi is used, by default None

        Returns
        -------
        ndarray
            a ndarray with the volume fraction for the tetras
        """
        if phi is None:
            phi = self.phi

        dimensionality = self.support.GetElementsDimensionality()
        elementType=elementsTypeSupportedForDimension[dimensionality]
        if not (elementType in self.support.elements and self.support.elements[elementType].GetNumberOfElements() > 0):
            elementType = complexElementsTypesSupportedForDimension[dimensionality]
        elements = self.support.GetElementsOfType(elementType)
        conn = elements.connectivity
        phis = phi.flatten()[conn]
        nbNodesInBodyElement = elements.GetNumberOfNodesPerElement()
        if np.any(np.sum(phis==0,axis=1)==nbNodesInBodyElement) :
            Info("Warning Element with all zeros phis (new warning will be suppressed")
        assert elements.GetNumberOfElements()!=0,"There is no "+str(elementType)+" body element in the mesh, can not compute volumic fraction"

        if dimensionality==2:
            if elementType == ED.Triangle_3:
                import OpenPisco.Structured.Geometry2D as Geometry2D
                return Geometry2D.triangle_volumic_fractions(phis)
            elif elementType == ED.Quadrangle_4:
                import OpenPisco.Structured.Geometry2D as Geometry2D
                return Geometry2D.quadrangle_volumic_fractions(phis)
            else:
                raise RuntimeError(f"Element type ({elementType}) not supported.")
        elif dimensionality==3:
            if elementType == ED.Tetrahedron_4 :
                import OpenPisco.Structured.Geometry3D as Geometry3D
                return Geometry3D.tetrahedron_volumic_fractions(phis)
            elif elementType == ED.Hexahedron_8:
                import OpenPisco.Structured.Geometry3D as Geometry3D
                return Geometry3D.cuboid_volumic_fractions(phis)
            else:
                raise RuntimeError(f"Element type ({elementType}) not supported.")
        else:
            raise ValueError("Dimension not supported.")

    def GetVolumeOfNegativePartbyElement(self, phi=None):
        dimensionality = self.support.GetElementsDimensionality()
        measureOfBodyElement = GetVolumePerElement(self.support, ElementFilter(dimensionality=dimensionality) )
        return measureOfBodyElement*self.GetElementsVolumicFractions(phi)

    def InterfaceIntegral(self, field):
        op = interface_lumped_mass_matrix(self.support)
        v = op * field
        return np.sum(v)

    def ApplyVelocityRestriction(self,raw_v,moldingDirection):
        """
        Parameters
        ----------
        raw_v : np.ndarray
            The (scalar) advection field

        Returns
        -------
        np.ndarray
            The (vector) advection field restricted according to the molding condition

        Notes
        -----
            The molding constraint is enforced as in the paper A level set based method for the optimization of cast parts
        """
        if np.all(moldingDirection)==0:
           print(" Attention: the molding direction is (0,0,0), will give zero velocity ")
        raw_v_molding = np.zeros( ( self.support.GetNumberOfNodes(),3), dtype = MuscatFloat)
        from OpenPisco.Unstructured.LevelsetTools import ComputeGradientOnTetrahedrons
        gradphi = ComputeGradientOnTetrahedrons(self.support,self.phi)
        for p in range(self.point.support.GetNumberOfNodes()):
              raw_v_molding[p,:] = np.dot(gradphi[p,:],moldingDirection)*raw_v[p]*moldingDirection
        return raw_v_molding

    def Regularize(self, field_, lengthscaleParameter = 1.0, on = []):
        if self.__regularize_cache__ is None:
            spaceDimension=self.support.GetElementsDimensionality()
            phys = BasicPhysics(spaceDimension=spaceDimension)
            phys.SetSpaceToLagrange(P=1)
            problem = UnstructuredFeaSym()

            if self.conform:
                interZone=TZ.InterZoneByDim[spaceDimension]
                phys.AddBFormulation( TZ.Outside3D ,phys.GetBulkLaplacian(lengthscaleParameter)  )
                phys.AddBFormulation( TZ.Inside3D ,phys.GetBulkLaplacian(lengthscaleParameter)  )
                phys.AddBFormulation( interZone, phys.GetBulkMassFormulation()  )
                phys.AddLFormulation( interZone, phys.GetFlux("grad")  )
            else:
                phys.AddBFormulation( TZ.Bulk ,phys.GetBulkLaplacian(lengthscaleParameter)  )
                phys.AddBFormulation( TZ.Bulk, phys.GetBulkMassFormulation("dirac")  )
                phys.AddLFormulation( TZ.Bulk, phys.GetFlux("grad")  )
                dirac = FEField()
                dirac.numbering = ComputeDofNumbering(self.support,phys.spaces[0],fromConnectivity = True)
                dirac.name = "dirac"
                dirac.mesh = self.support
                dirac.space = phys.spaces[0]
                dirac.data =  self.InterfaceRestriction(np.ones(len (field_) ))
                problem.fields["dirac"] = dirac

            problem.physics.append(phys)
            problem.SetMesh(self.support)

            if on:
                block = KRBlockScalar()
                block.AddArg("u").On(on)
                block.SetValue(0.)
                problem.solver.constraints.AddConstraint(block)

            problem.ComputeDofNumbering(fromConnectivity=True)

            grad = FEField()
            grad.numbering = problem.numberings[0]
            grad.name = "grad"
            grad.mesh = self.support
            grad.space = phys.spaces[0]
            problem.fields["grad"] = grad

            problem.fields["grad"].data =  self.InterfaceRestriction(field_.ravel())
            k,f = problem.GetLinearProblem(computeK=True,computeF=True)
            problem.Solve(k,f)
            self.__regularize_cache__ = problem

        else:
            problem = self.__regularize_cache__

            problem.fields["grad"].data =  self.InterfaceRestriction(field_.ravel())
            _,f = problem.GetLinearProblem(computeK=False,computeF=True)

            problem.Resolve(f)

        problem.PushSolutionVectorToUnknownFields()

        return problem.unknownFields[0].GetPointRepresentation()

    def Reinitialize(self, length=None):

        if self.__redistancing_cache__ is None:
            self.__redistancing_cache__ = ReDistancingClass()
            self.__redistancing_cache__.SetMesh(self.support)

        self.__redistancing_cache__.SetPhi(self.phi)
        self.__redistancing_cache__.GenerateDist()
        self.phi[:] = self.__redistancing_cache__.GetNewPhi()

        self.CleanCachePhi()

    def Transport(self, velocity, velocity_normalization=1.0, disableCFLTruncation= True):

        if self.__advect_cache__ is None :
            self.__advect_cache__ = AdvectionClass()
            self.__advect_cache__.SetMesh(self.support)

        self.__advect_cache__.SetPhi(self.phi[:])
        self.__advect_cache__.SetVelocityScalar(velocity*velocity_normalization)
        self.__advect_cache__.disableCFLTruncation = disableCFLTruncation
        self.__advect_cache__.PerformTransport(1.0)
        self.phi[:] = self.__advect_cache__.GetNewPhi()

        self.CleanCachePhi()

    def TransportAndReinitialize(self, velocity, velocity_normalization=1.0, disableCFLTruncation = True):
        self.Transport(velocity, velocity_normalization, disableCFLTruncation = disableCFLTruncation)
        self.Reinitialize()

    def RegularizePhi(self):
        assert self.conform,"Regularization filter not coded in non conformal setting "

        lengthscaleParameter = self.regularizationLengthscaleParameter
        phys = BasicPhysics(spaceDimension=self.support.GetElementsDimensionality())
        phys.SetSpaceToLagrange(P=1)
        problem = UnstructuredFeaSym()

        phys.AddBFormulation( TZ.Outside3D ,phys.GetBulkLaplacian(lengthscaleParameter**2)  )
        phys.AddBFormulation( TZ.Inside3D ,phys.GetBulkLaplacian(lengthscaleParameter**2)  )
        phys.AddBFormulation( TZ.Outside3D, phys.GetBulkMassFormulation()  )
        phys.AddBFormulation( TZ.Inside3D, phys.GetBulkMassFormulation()  )
        phys.AddLFormulation( TZ.Inside3D, phys.GetFlux("grad")  )
        phys.AddLFormulation( TZ.Outside3D, phys.GetFlux("grad")  )

        problem.physics.append(phys)
        problem.SetMesh(self.support)

        problem.ComputeDofNumbering()

        grad = FEField()
        grad.numbering = ComputeDofNumbering(self.support,phys.spaces[0],fromConnectivity = True)
        grad.name = "grad"
        grad.mesh = self.support
        grad.space = phys.spaces[0]
        grad.data =  self.phi.ravel()
        problem.fields["grad"] = grad

        k,f = problem.GetLinearProblem()
        problem.Solve(k,f)
        problem.PushSolutionVectorToUnknownFields()
        res = FEField(mesh=self.support,space=None,numbering=phys.numberings[0],data=problem.sol).GetPointRepresentation()

        self.phi = res
        self.CleanCachePhi()

    def AcceptChanges(self,newphi=None):
        super().AcceptChanges(newphi)
        self.CleanCachePhi()

        if self.conform:
            import  OpenPisco.Unstructured.MmgMesher as MmgMesher
            dic = dict(self.MesherInfo)
            originalIso = dic["iso"]
            if "met" in dic.keys() and dic["met"]:
                from OpenPisco.Unstructured.MeshAdaptationTools import ComputeLevelSetMetric
                if "symbolicMetricExpression" in dic.keys():
                    dic["symbolicExpression"] = dic.pop("symbolicMetricExpression")
                    metricfield = ComputeLevelSetMetric(self,dic,"Symbolic")
                    self.support.nodeFields["met"] = metricfield                    
                else:
                    metricfield = ComputeLevelSetMetric(self,dic)
                    self.support.nodeFields["met"] = metricfield
            boundingMin,boundingMax = self.support.ComputeBoundingBox()
            h = np.max(np.abs(boundingMax - boundingMin))

            ok = True
            for i in [0, -1, 1, -2, 2, -3, 3, -4, 4]:
                dic["iso"] -= 0.001*h*i

                try:
                    Debug("Remesh from levelset")
                    MmgMesher.MmgMesherActionLevelset(self,dic)
                    Debug("Remesh from levelset done")
                    ok = True
                    break
                except Exception as e :
                    Debug(e)
                    Error("Problem remeshing with iso " +str(dic["iso"]))
                    ok = False
            dic["iso"] = originalIso
            assert ok, "AcceptChanges Failure"
        self.CleanCacheMesh()

    def GetMassMatrix(self):

        if not self.conform and self.massMatrix is not None:
            return self.massMatrix
            
        TP = BasicPhysics(spaceDimension=self.support.GetElementsDimensionality())

        numbering = ComputeDofNumbering(self.support, space=LagrangeSpaceGeo, fromConnectivity=True)

        field = FEField()
        field.numbering = numbering
        field.name = TP.GetPrimalNames()[0]
        field.mesh = self.support
        field.space = LagrangeSpaceGeo
        field.Allocate()

        ff = ElementFilter(eTag=TZ.Bulk)
        self.massMatrix,_ = IntegrateGeneral(mesh=self.support,constants={},fields=[],wform=TP.GetBulkMassFormulation(), unknownFields= [field],
                                             elementFilter=ff)

        return self.massMatrix

    def GetInterfaceMassMatrix(self):
        if not self.conform and self.interfaceMassMatrix is not None:
            return self.interfaceMassMatrix
        TP = BasicPhysics(spaceDimension=self.support.GetElementsDimensionality())

        numbering = ComputeDofNumbering(self.support, space=LagrangeSpaceGeo, fromConnectivity=True)

        field = FEField()
        field.numbering = numbering
        field.name = TP.GetPrimalNames()[0]
        field.mesh = self.support
        field.space = LagrangeSpaceGeo
        field.Allocate()

        ff = ElementFilter(eTag=TZ.Bulk)

        if self.conform:
            ff.SetTags([TZ.Bulk])
        else:
            ff.SetTags([TZ.InterSurf])

        self.interfaceMassMatrix,_ = IntegrateGeneral(mesh=self.support,constants={},
                                                      fields=[],wform=TP.GetBulkMassFormulation(),
                                                      unknownFields= [field],elementField=ff)

        return self.interfaceMassMatrix

    def GetInterfaceLumpedMassMatrix(self):
        assert self.conform,"Not coded yet (sorry)"
        return interface_lumped_mass_matrix(self.support)

    def InterfaceRestriction(self,field):
        if self.conform:
            from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
            dimensionality = self.support.GetElementsDimensionality()
            skinElements = elementsTypeSupportedForDimension[dimensionality-1]
            skinZone=TZ.InterZoneByDim[dimensionality]

            out = np.zeros(field.size )
            for _,data,ids in ElementFilter(eTag=skinZone)(self.support):
                mask = data.GetNodesIndexFor(ids)
                out[mask] = field.ravel()[mask]

            ids =  self.support.GetElementsOfType(skinElements).tags[skinZone].GetIds()
            mask = self.support.GetElementsOfType(skinElements).GetNodesIndexFor(ids)
            out = np.zeros(field.size )
            out[mask] = field.ravel()[mask]
            return out
        else:
            boundingMin,boundingMax = self.support.ComputeBoundingBox()
            eps = np.linalg.norm(boundingMax - boundingMin)/20
            res = (1+np.cos(self.phi*(np.pi/eps)))/(2*eps)
            res[abs(self.phi)> eps] = 0
            return field * res

    def ComputeInterfaceProjection(self,data):
        res = self.InterfaceRestriction(data)
        if self.__mass_cache__ is None:
            if self.conform:
                M = np.squeeze(np.array(np.sum(self.GetInterfaceLumpedMassMatrix(),axis=1)))
                self.__lumped_mass_cache__ = M
            else:
                M = np.squeeze(np.array(np.sum(self.GetMassMatrix(),axis=1)))
                self.__mass_cache__ = M
        else:
            if self.conform:
                M = self.__lumped_mass_cache__
            else:
                M = self.__mass_cache__
        return res * M

def interface_lumped_mass_matrix(support):
    dimensionality=support.GetElementsDimensionality()
    skin_elems = support.GetElementsOfType(elementsTypeSupportedForDimension[dimensionality-1])
    skin_zone=TZ.InterZoneByDim[dimensionality]
    interface_tag = skin_elems.tags[skin_zone]
    interface_conn = skin_elems.connectivity[interface_tag.GetIds(), :]
    coords = support.GetPosOfNodes()
    return dimensionality_lumped_mass_matrix(dimensionality)(coords, interface_conn)

def dimensionality_lumped_mass_matrix(dimensionality):
    assert dimensionality in [2,3],"Dimension not handled yet"
    if dimensionality==3:
        return triangle_3d_lumped_mass_matrix
    elif dimensionality==2:
        return bar_2d_lumped_mass_matrix

def triangle_3d_lumped_mass_matrix(nodal_coords, elem_conn):
    areas = triangle_3d_areas(nodal_coords[elem_conn])
    weights = np.divide(areas, 3.0, out=areas)
    values = np.repeat(weights, 3)
    indices = np.ravel(elem_conn)
    size = np.prod(nodal_coords.shape[:-1])
    return sp.sparse.coo_matrix((values, (indices, indices)), shape=(size, size)).todia()

def triangle_3d_areas(coords):
    assert coords.shape[-1] == 3 and coords.shape[-2] == 3
    return 0.5 * np.linalg.norm(np.cross(
        coords[..., 1, :] - coords[..., 0, :],
        coords[..., 2, :] - coords[..., 0, :]),
        axis=-1)

def bar_2d_lumped_mass_matrix(nodal_coords, elem_conn):
    areas = bar_2d_lenght(nodal_coords[elem_conn])
    weights = np.divide(areas, 2.0, out=areas)
    values = np.repeat(weights, 2)
    indices = np.ravel(elem_conn)
    size = np.prod(nodal_coords.shape[:-1])
    return sp.sparse.coo_matrix((values, (indices, indices)), shape=(size, size)).todia()

def bar_2d_lenght(coords):
    assert coords.shape[-1] == 2 and coords.shape[-2] == 2
    return np.linalg.norm(coords[..., 1, :] - coords[..., 0, :],axis=-1)

RegisterClass("LevelSet3DU",LevelSet,CreateLevelSet )
RegisterClass("LevelSet2DU",LevelSet,CreateLevelSet )

################## test functions ###########################################
from Muscat.MeshTools.MeshCreationTools import CreateCube
from Muscat.IO.XdmfWriter import XdmfWriter
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
from Muscat.MeshTools.MeshModificationTools import  ComputeFeatures
from OpenPisco.Unstructured.MmgMesher import MmgMesherActionLevelset

def AddRidgesToMesh(UM):
    (edges,_) = ComputeFeatures(UM)
    for name,data in edges.elements.items():
       UM.GetElementsOfType(name).Merge(data)

def CheckIntegrityNonConform(GUI):
    UM = CreateCube(dimensions=[21, 21, 21],spacing=[1., 1, 1],origin=[-10., -10., -10],ofTetras=True)
    ls = LevelSet(support=UM)
    ls.Initialize(lambda XYZs : np.sqrt(XYZs[:, 0]**2 + XYZs[:, 2]**2) - 9)
    writer = XdmfWriter(TemporaryDirectory.GetTempPath() + "UnstructuredLevelSetNonConform.xmf")
    writer.SetTemporal()
    writer.SetBinary()
    writer.SetXmlSizeLimit(0)
    writer.Open()
    def SPEED(support):
        factor = 0.1
        return support.nodes[:, 0]*0+factor
    speed = SPEED(ls.support)
    writer.Write( \
            ls.support, \
            PointFields = [ls.phi,speed], \
            PointFieldsNames = ['phi','speed'], \
            TimeStep = 1 \
            )
    for _ in range(5):
        speed = SPEED(ls.support)
        ls.Transport(speed)
        writer.Write( \
                ls.support, \
                PointFields = [ls.phi,speed], \
                PointFieldsNames = ['phi','speed'], \
                TimeStep = 1 \
                )
        ls.Reinitialize()
        speed = SPEED(ls.support)
        writer.Write( \
                ls.support, \
                PointFields = [ls.phi,speed], \
                PointFieldsNames = ['phi','speed'], \
                TimeStep = 1 \
                )
    writer.Write( \
            ls.support, \
            PointFields = [ls.phi,speed], \
            PointFieldsNames = ['phi','speed'], \
            TimeStep = 1 \
            )
    writer.Close()
    return "ok"

def CheckIntegrityConformMuscatGrad(GUI):
    from OpenPisco.Unstructured.MmgMesher import MmgAvailable
    if not MmgAvailable:
        return "Not Ok, mmg3d_O3 not found!!"
    UM=CreateCube(dimensions=[21, 21, 21],spacing=[1., 1, 1],origin=[-10., -10., -10],ofTetras=True)
    AddRidgesToMesh(UM)
    writer = XdmfWriter(TemporaryDirectory.GetTempPath() + "UnstructuredLevelSet_MuscatGrad.xmf")
    writer.SetTemporal()
    writer.SetBinary()
    writer.SetXmlSizeLimit(0)
    writer.Open()
    def SPEED(support):
        return (1 + 1/(0.5 + 0.1*support.nodes[:, 2]**2 +  support.nodes[:, 1]**2))/10
    ls = LevelSet(support=UM)
    ls.computeGradPhiWith = "proj"
    ls.MesherInfo = {"iso":0, "nr":True, "hmax":2, "hmin":0.5, "hausd":0.1}
    ls.Initialize(lambda XYZs : XYZs[:, 0])
    ls.TakePhiAndMesh(ls.phi, UM)
    MmgMesherActionLevelset(ls ,ls.MesherInfo)
    for _ in range(5):
        speed = SPEED(ls.support)
        ls.Transport(speed)
        writer.Write( \
                ls.support, \
                PointFields = [ls.phi,speed], \
                PointFieldsNames = ['phi','speed'], \
                TimeStep = 1 \
                )
        MmgMesherActionLevelset(ls,ls.MesherInfo)
        ls.Reinitialize()
    writer.Close()
    return 'OK'

def CheckIntegrityConform(GUI):
    from OpenPisco.Unstructured.MmgMesher import MmgAvailable
    if not MmgAvailable:
        return "Not Ok, mmg3d_O3 not found!!"
    UM = CreateCube(dimensions=[21, 21, 21],spacing=[1., 1, 1],origin=[-10., -10., -10],ofTetras=True)
    writer = XdmfWriter(TemporaryDirectory.GetTempPath() + "UnstructuredLevelSet.xmf")
    writer.SetTemporal()
    writer.SetBinary()
    writer.SetXmlSizeLimit(0)
    writer.Open()
    def SPEED(support):
        return (1 + 1/(0.5 + 0.1*support.nodes[:, 2]**2 +  support.nodes[:, 1]**2))/10
    ls = LevelSet(support=UM)
    ls.MesherInfo = {"iso":0, "nr":True, "hmax":2, "hmin":0.5, "hausd":0.1}
    ls.Initialize(lambda XYZs : XYZs[:, 0])
    ls.TakePhiAndMesh(ls.phi, UM)
    MmgMesherActionLevelset(ls ,ls.MesherInfo)
    for _ in range(5):
        speed = SPEED(ls.support)
        ls.Transport(speed)
        writer.Write( \
                ls.support, \
                PointFields = [ls.phi,speed], \
                PointFieldsNames = ['phi','speed'], \
                TimeStep = 1 \
                )
        MmgMesherActionLevelset(ls,ls.MesherInfo )
        ls.Reinitialize()
    writer.Close()
    ls.conform = True
    ls.RegularizePhi()

    return 'OK'

def CheckIntegrityNonConformExtension(GUI):
    UM = CreateCube(dimensions=[21, 21, 21],spacing=[1., 1., 1.],origin=[-10., -10., -10],ofTetras=True)
    tets = UM.GetElementsOfType(ED.Tetrahedron_4)
    tets.tags.CreateTag(TZ.Bulk).SetIds(np.arange(tets.GetNumberOfElements()))
    ls = LevelSet(support=UM)
    ls.conform = False
    ls.Initialize(lambda XYZs : np.sqrt(XYZs[:, 0]**2 + XYZs[:, 2]**2) - 9)
    writer = XdmfWriter(TemporaryDirectory.GetTempPath() + "UnstructuredLevelSetExtensionNonConform.xmf")
    writer.SetTemporal()
    writer.SetBinary()
    writer.SetXmlSizeLimit(0)
    writer.Open()
    def SPEED(support):
        return np.cos(support.nodes[:, 0]/1.5)
    speed = SPEED(ls.support)
    writer.Write( \
            ls.support, \
            PointFields = [ls.phi,speed], \
            PointFieldsNames = ['phi','speed'], \
            TimeStep = 1 \
            )
    speed = SPEED(ls.support)
    speed = ls.Regularize(speed)
    writer.Write( \
            ls.support, \
            PointFields = [ls.phi,speed], \
            PointFieldsNames = ['phi','speed'], \
            TimeStep = 1 \
            )
    writer.Write( \
            ls.support, \
            PointFields = [ls.phi,speed], \
            PointFieldsNames = ['phi','speed'], \
            TimeStep = 1 \
            )

    writer.Close()
    return "ok"

def CheckIntegrityInterfaceIntegral(GUI):
    mesh = CreateCube(dimensions=[5,5,5],spacing=[1./4, 1./4, 1./4],origin=[0, 0, 0],ofTetras=True)
    tris = mesh.GetElementsOfType(ED.Triangle_3)
    tris.tags.CreateTag(TZ.InterSurf).SetIds(np.arange(tris.GetNumberOfElements()))
    ls = LevelSet(support=mesh)
    ls.conform = True
    field = np.ones(ls.support.GetNumberOfNodes(),dtype = MuscatFloat) #np.arange(ls.support.GetNumberOfNodes(),dtype = MuscatFloat)
    field_new = ls.ComputeInterfaceProjection(field)
    print(field_new)
    integral = ls.InterfaceIntegral(field)
    expected = np.sum(ls.ComputeInterfaceProjection(field))
    if np.abs(integral - expected) > 1e-10:
       return "not OK"
    return "ok"

def CheckIntegrity(GUI=False):
    from OpenPisco.Unstructured.Meshdist import mshdistExec
    if not Which(mshdistExec):
        return "Not Ok, " + str(mshdistExec) + " not found!!"
    from OpenPisco.Unstructured.Advect import advectExec
    if not Which(advectExec):
        return "Not Ok, " + str(advectExec) + " not found!!"

    if CheckIntegrityNonConformExtension(GUI).lower() != "ok" :
        return "not OK : on CheckIntegrityNonConformExtension "

    if CheckIntegrityConform(GUI).lower() != "ok" :
        return "not OK : on CheckIntegrityConform "

    if CheckIntegrityNonConform(GUI).lower() != "ok" :
        return "not OK : on CheckIntegrityNonConform "

    if CheckIntegrityConformMuscatGrad(GUI).lower() != "ok" :
        return "not OK : on CheckIntegrityConformMuscatGrad "

    if CheckIntegrityInterfaceIntegral(GUI).lower() != "ok" :
        return "not OK : on CheckIntegrityInterfaceIntegral "

    return "ok"

if __name__ == '__main__': # pragma: no cover
    print(CheckIntegrity(True))
