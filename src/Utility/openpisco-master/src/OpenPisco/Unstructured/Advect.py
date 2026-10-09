# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import numpy as np

from Muscat.Helpers.Logger import Debug
from Muscat.Helpers.IO.Which import Which
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter

from OpenPisco.ExternalTools.AppExecutableTools  import AppExecutableBase, CommandLineBuilder
from OpenPisco.BaseElements import elementsTypeSupportedForDimension
import OpenPisco.TopoZones as TZ

advectExec = "advect"

class Advect(AppExecutableBase):
    transportcpt = 0

    def __init__(self):
        super(Advect, self).__init__(advectExec)
        self.SetBinaryFlag(True)
        self.support = None
        self.base_filename = \
                "_".join(("meshfileLS", str(Advect.transportcpt), ""))
        Advect.transportcpt += 1

        self.keepGeneratedFiles = False
        self.disableCFLTruncation= True

    def DeleteGeneratedFiles(self):
        filesToDelete = []
        filesToDelete.extend(  self.base_filename+ext for ext in  ['.mesh', '.chi.mesh', '.sol','.chi.sol',".chi.new.sol"])
        self.DeleteFiles(filesToDelete=filesToDelete)

    def SetMesh(self, mesh):
        self.SetSupport(mesh)

    def SetPhi(self, phi):
        self.WriteSupport(phi)

    def SetVelocityScalar(self, scalarVelocity):
        self.WriteVelocity(scalarVelocity)

    def SetSupport(self, support):
        self.support = type(support)()
        self.support.nodes = support.nodes
        self.support.originalIDNodes = support.originalIDNodes
        dimensionality = support.GetElementsDimensionality()
        elementType=elementsTypeSupportedForDimension[dimensionality]
        self.support.elements.AddContainer(support.GetElementsOfType(elementType))

    def WriteSupport(self, field):
        self.DeleteGeneratedFiles()
        self.support.nodeFields["phi"] = field
        self.writer.Open( \
                self._filePath(".".join((self.base_filename, "mesh"))))
        self.writer.Write(self.support)
        self.writer.Close()

        self.writer.SetFileName( \
                self._filePath(".".join((self.base_filename, "chi.mesh"))))
        self.writer.OpenSolutionFile(self.support)
        self.writer.WriteSolutionsFields(self.support, [field])
        self.writer.Close()

    def WriteVelocity(self, velocity,computeGradPhiWith = "gradphi" ):
        self.writer.SetFileName( \
                self._filePath(".".join((self.base_filename, "mesh"))))
        self.writer.OpenSolutionFile(self.support)
        dimensionality = self.support.GetElementsDimensionality()

        if computeGradPhiWith == None :
            errorMessage = "Velocity argument does not have the correct shape"
            assert len(velocity.shape) == 2 and velocity.shape[1] == dimensionality,errorMessage
            numpyData = velocity

        elif computeGradPhiWith == "vtk" :
            Debug("Computing gradients using vtk")
            import vtk
            from Muscat.Bridges.vtkBridge import MeshToVtk
            vtkmesh = MeshToVtk(self.support)
            # In this case we have to calculate direction using the gradient of phi
            f = vtk.vtkGradientFilter()
            f.SetInputData(vtkmesh)

            f.SetInputArrayToProcess(0, 0, 0, 0, "phi")
            f.Update()
            grad_array = f.GetOutput().GetPointData().GetArray("Gradients")
            from vtk.util import numpy_support
            numpyData = numpy_support.vtk_to_numpy(grad_array)
            norm = np.linalg.norm(numpyData, axis=1)
            norm.shape = (len(velocity), 1)
            norm[norm<1e-20] = 1
            numpyData /= norm

            tmp = velocity.view()
            tmp.shape = (len(tmp), 1)

            numpyData *= tmp

        elif computeGradPhiWith.lower() == "gradphi" :
            from OpenPisco.Unstructured.LevelsetTools import ComputeGradientOnBodyElements
            data = self.support.nodeFields["phi"]
            numpyData = ComputeGradientOnBodyElements(self.support,data)
            numpyData *= velocity[:,np.newaxis]

        elif computeGradPhiWith == "proj" :
            Debug("Computing gradients using Muscat")
            import Muscat.FE.SymWeakForm as wf
            symPhi = wf.GetField("phi",1,sdim=dimensionality)
            symGradPhi = wf.GetField("GradPhi",dimensionality,sdim=dimensionality)
            symGradPhiT = wf.GetTestField("GradPhi",dimensionality,sdim=dimensionality)

            symEner = wf.Gradient(symPhi).T*symGradPhiT + symGradPhi.T*symGradPhiT

            from Muscat.FE.Integration import IntegrateGeneral
            from Muscat.FE.Spaces.FESpaces import LagrangeSpaceP1

            from Muscat.FE.DofNumbering import ComputeDofNumbering
            numbering = ComputeDofNumbering(self.support,LagrangeSpaceP1,fromConnectivity=True)

            from Muscat.FE.Fields.FEField import FEField
            field = FEField(name="phi",
                            mesh=self.support,
                            space=LagrangeSpaceP1,
                            numbering = numbering,
                            data = self.support.nodeFields["phi"]
                            )

            elementType=elementsTypeSupportedForDimension[dimensionality]
            nbBodyElems = self.support.GetElementsOfType(elementType).GetNumberOfElements()
            self.support.GetElementsOfType(elementType).GetTag(TZ.Bulk).SetIds(np.arange(nbBodyElems) )

            self.support.nodes = np.ascontiguousarray(self.support.nodes)

            ff = ElementFilter(tag=TZ.Bulk)

            dofsAllowed=["GradPhi_0","GradPhi_1","GradPhi_2"]
            dofs=[dofsAllowed[axeId] for axeId in range(dimensionality)]
            unknownFields = [ FEField(name=name,
                                    mesh=self.support,
                                    space=LagrangeSpaceP1,
                                    numbering = numbering
                                    ) for name in dofs  ]

            m,f = IntegrateGeneral(mesh=self.support,
                                    wform=symEner,
                                    constants={},
                                    fields=[field],
                                    unknownFields=unknownFields,
                                    integrationRuleName="NodalEvalGeo",
                                    onlyEvaluation=False,
                                    elementFilter=ff)

            f /= m.diagonal()
            f.shape = (dimensionality,len(velocity))
            numpyData = f.T

            norm = np.linalg.norm(numpyData, axis=1)
            norm.shape = (len(velocity), 1)
            numpyData /= norm

            tmp = velocity.view()
            tmp.shape = (len(tmp), 1)

            numpyData *= tmp

        Debug("Max velocity " + str(np.max(numpyData)) )
        self.writer.WriteSolutionsFields(self.support, PointFields= [numpyData])
        self.writer.Close()
        return numpyData

    def PerformTransport(self, stepsize):
        cmdBuilder = CommandLineBuilder(self.GetAppName())
        cmdBuilder.optionAdd("v", prefix="+")
        if self.disableCFLTruncation:
           cmdBuilder.optionAdd("nocfl")
        cmdBuilder.optionAdd("dt", arg=str(stepsize))
        cmdBuilder.argumentAdd(".".join((self.base_filename, "meshb")))
        cmdBuilder.optionAdd("c", arg=".".join((self.base_filename, "chi.solb")))
        cmdBuilder.optionAdd("s", arg=".".join((self.base_filename, "solb")))
        cmdBuilder.optionAdd("o", arg=".".join((self.base_filename, "chi.new.solb")))
        cmd = cmdBuilder.result()

        self._executeCommand(cmd)

    def GetTransportedField(self):
        self.reader.dim = self.support.GetElementsDimensionality()
        data = self.reader.ReadExtraFields( \
                self._filePath(".".join((self.base_filename, "chi.new.sol"))))

        if not self.keepGeneratedFiles:
            self.DeleteGeneratedFiles()
        return data["SolAtVertices0"][:, 0]

    def GetNewPhi(self):
        return self.GetTransportedField()
    
def CheckIntegrity(GUI=False):
    if not Which(advectExec):
        return "Not Ok, " + str(advectExec) + " not found!!"
    return "ok"

if __name__ == '__main__': # pragma: no cover
    print(CheckIntegrity(True))
