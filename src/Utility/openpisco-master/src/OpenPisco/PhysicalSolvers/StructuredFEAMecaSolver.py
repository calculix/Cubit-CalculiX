# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from typing import Callable

import numpy as np

import Muscat.Helpers.ParserHelper as PH
import Muscat.FE.ConstantRectilinearFea as CRFEA
from Muscat.FE.SymPhysics import MechPhysics
from Muscat.Helpers.Logger import Debug, Error, Print
from Muscat.MeshContainers.Filters.FilterObjects import NodeFilter

from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import  RegisterClass
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco import RETURN_SUCCESS

def CreateStructuredFEAMecaSolver(ops):
    """
    Parameters
    ----------
    ops : dict
        Parameters to setup a `StructuredFEAPhysicalProblemMeca`.

    Notes
    -----
    `ops` can be populated via a string of the form::

        <StructuredFEA id="1" grid="1" type="static_elastic" eVoid="0.001" narrow="0" internalvariables=... >

          <Material    NOTIMPLEMENTED(eTag="*everyelement|eTag") young="1" poisson="0.3" density="1" />
          <Dirichlet  zone="2"|etag=""|nTag="" dofs="0 1 2" val="0.0"/>

          <Force eTag="ET3" dir="0 0 1" val="-1000"/>
        </StructuredFEA>
    """
    grid = ops.pop("grid")
    spaceDim = ops.pop("dimensionality")
    res = StructuredFEAMecaSolver(spaceDim)
    res.SetSupport(grid)
    res.internalSolver.dofpernode = spaceDim

    mecaPhysics = MechPhysics(spaceDim)

    assert ops.get("p",1) ==1,"Can only treat problems in p=1"

    if "p" in ops:
        del ops["p"]

    mecaPhysics.SetSpaceToLagrange(P=1)
    res.internalSolver.mecaPhysics = mecaPhysics

    assert ops["type"] == "static_elastic","Can treat only static_elastic problems"
    del ops["type"]

    if "narrow" not in ops:
        ops["narrow"] = min(grid.props.get("spacing"))*4
        print("Setting narrow automatically : " + str( ops["narrow"]))

    res.internalSolver.young = PH.ReadFloat(ops.get("young",res.internalSolver.young))
    res.internalSolver.poisson = PH.ReadFloat(ops.get("poisson",res.internalSolver.poisson ))

    for tag,child in ops["children"]:
        if tag == "Material" :
            res.internalSolver.young = PH.ReadFloat(child.get("young",res.internalSolver.young))
            child.pop("young",None)
            res.internalSolver.poisson = PH.ReadFloat(child.get("poisson",res.internalSolver.poisson))
            child.pop("poisson",None)
            res.internalSolver.density = PH.ReadFloat(child.get("density",res.internalSolver.density))
            child.pop("density",None)
            continue

        assert tag != "Acceleration","Acceleration not allowed"
        nodeFilter = NodeFilter()

        if "zone" in child:
            nodeFilter.AddZone(child["zone"])

        if "nTag" in child:
            nodeFilter.AddNTag(child["nTag"])

        if "eTag" in child:
            nodeFilter.AddETag(child["eTag"])

        val = float(child["val"] )

        if tag == "Dirichlet" :
            current_boundary = res.internalSolver.AddDirichletCondition
            if "eTag" in child:
                res.dirichletNodeFilters.append(NodeFilter(eTag=child["eTag"]))
        elif tag == "Neumann":
            current_boundary = res.internalSolver.AddNeumannCondition
            res.neumannNodeFilters.append(nodeFilter)
        elif tag == "NeumannNodal":
            current_boundary = res.internalSolver.AddNeumannNodalCondition
            res.neumannNodeFilters.append(nodeFilter)

        elif tag == "NeumannSpheric":
            current_boundary = res.internalSolver.AddNeumannCondition

            import math
            phi = PH.ReadFloat(child["phi"] )*math.pi/180.
            theta = PH.ReadFloat(child["theta"] )*math.pi/180.

            fx =  val*math.sin(phi)*math.cos(theta)
            fy =  val*math.sin(phi)*math.sin(theta)
            fz =  val*math.cos(phi)

            def GetForceFunction(phi, theta,val) -> Callable:
                def Force(pos) -> list:
                    vals = np.full(pos.shape[0],val)
                    fx =  vals*math.sin(phi)*math.cos(theta)
                    fy =  vals*math.sin(phi)*math.sin(theta)
                    fz =  vals*math.cos(phi)
                    return [fx, fy, fz]
                return Force

            res.neumannNodeFilters.append(nodeFilter)
            current_boundary(nodeFilter, [True, True, True], GetForceFunction(phi,theta,val))
            continue

        elif tag == "NTorsion" :
            axis = PH.ReadFloats(child["axis"])
            axis /= np.linalg.norm(axis)
            current_boundary = res.internalSolver.AddNeumannCondition
            def GetForceFunction(axis, val):
                def Force(pos):
                    loading = np.cross(axis,pos)*val
                    return [loading[:,0], loading[:,1], loading[:,2],]
                return Force

            res.neumannNodeFilters.append(nodeFilter)
            current_boundary(nodeFilter, [True, True, True],GetForceFunction(axis, val))
            continue

        elif tag == "NTorsionZ" :
            current_boundary = res.internalSolver.AddNeumannCondition

            def GetForceFunction(val):
                def Force(pos):
                    return [-pos[1]*val, pos[0]*val, 0]
                return Force

            res.neumannNodeFilters.append(nodeFilter)
            current_boundary(nodeFilter, [True, True, False], GetForceFunction(val))
            continue
        elif tag == "Force":
            if "phi" in child and "theta" in child:
                fx,fy,fz = PH.ReadVectorPhiThetaMag([child["phi"],child["theta"]],True )
            elif "dir" in child :
                if spaceDim == 2 :
                    fz = 0
                    fx,fy = PH.ReadVectorXY(child["dir"],True)
                else:
                    fx,fy,fz = PH.ReadVectorXYZ(child["dir"],True)
            else:
                message = "I need a direction (dir) or Euler angles (phi, theta)"
                Error(message)
                raise ValueError(message)

            try:
                val = PH.ReadFloat(child["val"] )
            except TypeError:
                val = PH.ReadString(child["val"] )

            assert "eTag" in child or "nTag" in child,"missing eTag or nTag"
            if "eTag" in child:
                eTag = child["eTag"]
                res.neumannNodeFilters.append(NodeFilter(eTag=eTag))
                mecaPhysics.AddLFormulation( eTag, mecaPhysics.GetForceFormulation([fx,fy,fz][0:spaceDim],val)  )
            elif "nTag" in child:
                tagName = PH.ReadString( child["nTag"])
                res.neumannNodeFilters.append(NodeFilter(nTag=tagName))
                nf = NodeFilter(nTag =tagName)
                mecaPhysics.AddToRHSTerm(nf,[fx*val,fy*val,fz*val][0:spaceDim] )
            continue
        else:
            message = "This kind of boundary is not supported : " + str(child.tag)
            Error(message)
            raise NotImplementedError(message)

        Debug("boundary")
        dofs = PH.ReadInts(child["dofs"])

        def GetForceFunction(dofs, val):
            def Force(pos):
                res = np.zeros_like(pos)
                res[:,dofs] = val
                return [res[:,[x]] for x in range(spaceDim)]
            return Force

        mask = np.zeros(spaceDim,dtype=bool)
        mask[dofs] = True
        current_boundary(nodeFilter, mask,  GetForceFunction(dofs,val))

    del ops["children"]

    #linear solver options
    lsops = ["linearSolver"]
    PH.ReadProperties(ops, lsops, res.internalSolver)
    for o in lsops:
        if o in ops:
            del ops[o]

    Print("Building Finite Element Problem")
    PH.ReadProperties(ops, ops.keys(), res)


    return res

from Muscat.Helpers.Decorators import froze_it

@froze_it
class StructuredFEAMecaSolver(SolverBase):
    def __init__(self, spaceDim=3):
       super().__init__()

       self.internalSolver = CRFEA.Fea(spaceDim=spaceDim)

       self.auxiliaryScalarGeneration[FN.int_potential_energy] = False
       self.auxiliaryFieldGeneration[FN.potential_energy][FN.Nodes] = False
       self.auxiliaryFieldGeneration[FN.tr_stress_][FN.Nodes] = False
       self.auxiliaryFieldGeneration[FN.tr_strain_][FN.Nodes] = False

       self.phi = None
       self.densities = None

       self.nodal_TrEpsilon = None
       self.nodal_TrSigma = None
       self.dirichletNodeFilters = []
       self.neumannNodeFilters = []

    def GetBoundaryConditionsFilters(self):
        return self.dirichletNodeFilters,self.neumannNodeFilters

    def GetSupport(self):
        return self.internalSolver.mesh

    def SetSupport(self, support):
        super().SetSupport(support)
        self.internalSolver.SetMesh(support)


    def SolveByDensities(self, densities, support):
        self.internalSolver.mesh = support
        self.internalSolver.BuildProblem()

        self.densities = np.copy(densities)
        self.internalSolver.Solve(Eeff=self.densities)

        if self.auxiliaryFieldGeneration[FN.tr_strain_][FN.Nodes] or self.auxiliaryFieldGeneration[FN.tr_stress_][FN.Nodes]:

            Debug("Beginning post-process StructuredFEAPhysicalProblemMeca")
            B,_ = self.myElem.IsoDispB([0,0,0], None)

            element_TrEpsilon = np.empty(self.mesh.props.get("dimensions")-1,dtype=float)
            element_TrSigma = np.empty(self.mesh.GetNumberOfElements(),dtype=float)
            import Muscat.FE.MaterialHelp as MH
            K = MH.HookeIso(1,0.3,dim=self.mesh.GetElementsDimensionality())
            connectivity = self.mesh.GenerateFullConnectivity()

            if self.mesh.GetElementsDimensionality() == 3:

                for i in range(self.mesh.GetNumberOfElements()):
                    res = self.mesh.GetMultiIndexOfElement(i)
                    dofs = np.array([ x*3 + [0,1,2 ]  for x in connectivity[i,:] ]).flatten()
                    epsilon = np.dot(B,self.u[dofs].flatten() )
                    element_TrEpsilon[res[0],res[1],res[2]] = epsilon[0] + epsilon[1] + epsilon[2]
                    sigma = np.dot(K,epsilon)
                    element_TrSigma[i] = sigma[0] + sigma[1] + sigma[2]
            else:
                for i in range(self.mesh.GetNumberOfElements()):
                    res = self.mesh.GetMultiIndexOfElement(i)
                    dofs = np.array([ x*2 + [0,1 ]  for x in connectivity[i,:] ]).flatten()
                    epsilon = np.dot(B,self.u[dofs].flatten() )
                    element_TrEpsilon[res[0],res[1]] = epsilon[0] + epsilon[1]
                    sigma = np.dot(K,epsilon)
                    element_TrSigma[i] = sigma[0] + sigma[1]


            from Muscat.FE.ConstantRectilinearFea import node_averaged_element_field
            self.nodal_TrEpsilon = node_averaged_element_field(element_TrEpsilon,self.mesh)
            self.nodal_TrSigma = node_averaged_element_field(element_TrSigma,self.mesh)
            Debug("End postproces StructuredFEAPhysicalProblemMeca")
        else:
            self.nodal_TrEpsilon = None
            self.nodal_TrSigma = None

        return RETURN_SUCCESS

    def SolveByLevelSet(self, levelSet): # pragma: no cover
        self.internalSolver.mesh = levelSet.support
        self.internalSolver.BuildProblem()

        assert not levelSet.conform,"Can not treat conform fields"
        assert levelSet.support is self.GetSupport(),"Can not change the support in SolveByLevelset"
        self.phi = np.copy(levelSet.phi)

        self.densities = levelSet.GetElementsVolumicFractions()
        self.densities *= (1.-self.eVoid)
        self.densities += self.eVoid

        if self.narrow > 0 :
            from Muscat.FE.ConstantRectilinearFea import element_averaged_node_field as EANF
            phiAtCenter = EANF(self.phi,levelSet.support).ravel()
            self.densities[phiAtCenter >  self.narrow] = 0

        self.SolveByDensities(self.densities,levelSet.support)
        return RETURN_SUCCESS

    def GetNodalSolution(self):
        u = self.internalSolver.u.view()
        dim = self.internalSolver.mesh.GetElementsDimensionality()
        u.shape = (u.size // dim, dim)
        return u

    def GetAuxiliaryField(self, name, on=FN.Nodes):
        if name == FN.potential_energy:
            Eeff = np.copy(self.densities)
            Eeff[self.densities<=self.eVoid] = 0
            return self.internalSolver.nodal_elastic_energy(Eeff=Eeff, OnlyOnInterface=True)/2

        elif name == FN.tr_strain_:
            return self.nodal_TrEpsilon
        elif name == FN.tr_stress_:
            return self.nodal_TrSigma
        else:
            return None

    def GetAuxiliaryScalar(self,name):
        if name == FN.int_potential_energy:
            return self.internalSolver.element_elastic_energy(self.densities).sum()*np.prod(self.internalSolver.mesh.props.get("spacing"))/2
        else:
            return None

    def Write(self):
        self.internalSolver.Write()

RegisterClass("Structured", StructuredFEAMecaSolver,CreateStructuredFEAMecaSolver)


def CheckIntegrity():
    from Muscat.MeshTools.ConstantRectilinearMeshTools import CreateConstantRectilinearMesh
    from Muscat.MeshTools.ConstantRectilinearMeshTools import GetMonoIndex
    import OpenPisco.Structured.Levelset3D as Levelset3D

    nx = 5; ny = 6; nz = 7
    support = CreateConstantRectilinearMesh(dimensions=[nx,ny,nz] , spacing=[1./(nx-1), 1./(ny-1), 1./(nz-1)] , origin=[0,0,0])
    ls = Levelset3D.LevelSet3D(support=support)
    ls.InitializePointByPoint(lambda x: -1 )
    ########################### PhysicalProblemMeca ###########################

    PPM = StructuredFEAMecaSolver()
    PPM.SetSupport(support)

    dtagI = support.nodesTags.CreateTag("Block")
    dtagII = support.nodesTags.CreateTag("Loading")

    for x in range(nx):
        for y in range(ny):
            dtagI.AddToTag(GetMonoIndex([[x,y,0]],[nx,ny,nz])[0] )
            dtagII.AddToTag(GetMonoIndex([[x,y,nz-1]],[nx,ny,nz])[0] )

    PPM.internalSolver.AddDirichletCondition(NodeFilter(nTag="Block"), [True, True, True] , lambda x : [0,0,0])
    PPM.internalSolver.AddNeumannCondition(NodeFilter(nTag="Loading"), [False, False, True] , lambda x : [0,0,1] )
    PPM.internalSolver.BuildProblem(dofpernode=3)
    PPM.GetSupport()
    PPM.SolveByLevelSet(ls)
    PPM.GetAuxiliaryField(FN.potential_energy)
    PPM.GetNodalSolution()
    PPM.Write()
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity())# pragma: no cover
