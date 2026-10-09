# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from functools import partial

import numpy as np
import vtk
from vtk.util import numpy_support

from Muscat.Helpers.Logger import Debug

import OpenPisco.QtApp.QtImplementation as QT
import OpenPisco.TopoZones as TZ

class StopoVTKRenderZone(QT.QObject):
    update3D = QT.Signal()

    def __init__(self, parent=None):
        super().__init__(parent)  # Modern cooperative initialization
        self._initialized = False


    def setup3DView(self,MainWindow):

        self.ren = vtk.vtkRenderer()
        self.ren.GradientBackgroundOn()
        self.ren.SetBackground(0.3176,0.3412,0.4314)
        self.ren.SetBackground2(0,0,0.1647)

        self.vtkWidget = QT.QVTKRenderWindowInteractor(MainWindow.ui.tab3D)
        self.vtkWidget.setSizePolicy(QT.QSizePolicy.Expanding, QT.QSizePolicy.Expanding)
        MainWindow.ui.gridlayoutTag3D.addWidget(self.vtkWidget, 0, 0, 1, 1)

        self.vtkWidget.GetRenderWindow().AddRenderer(self.ren)
        self.iren = self.vtkWidget.GetRenderWindow().GetInteractor()

        axesActor = vtk.vtkAxesActor()
        self.axes = vtk.vtkOrientationMarkerWidget()
        self.axes.SetOrientationMarker(axesActor)
        self.axes.SetInteractor(self.iren)

        self.axes.EnabledOn() # <== application freeze-crash
        self.axes.InteractiveOn()

        # add the custom style
        from OpenPisco.QtApp.VtkQtCode import MouseInteractorHighLightActor
        self.style = MouseInteractorHighLightActor(self.iren, MainWindow )
        self.style.SetDefaultRenderer(self.ren)

        #self.iren.SetInteractorStyle(vtk.vtkInteractorStyleTrackballCamera())
        self.iren.SetInteractorStyle(self.style)
        # Create a mapper
        self.phi_mapper = vtk.vtkPolyDataMapper()
        self.phi_mapper.ScalarVisibilityOn()
        self.phi_mapper.SetScalarModeToUsePointData()
         # Create an actor
        actor = vtk.vtkActor()
        actor.SetMapper(self.phi_mapper)
        self.ren.AddActor(actor)
        self.mesh_actor = actor

        self.plotZones = False
        # Create a mapper for the zones
        self.zone_mapper = vtk.vtkPolyDataMapper()

        actor = vtk.vtkActor()
        actor.SetMapper(self.zone_mapper)
        actor.GetProperty().SetOpacity(0.5)
        self.ren.AddActor(actor)
        self.zone_actor = actor

        # create the scalar_bar
        self.scalar_bar = vtk.vtkScalarBarActor()
        self.scalar_bar.SetOrientationToHorizontal()
        #self.scalar_bar.SetLookupTable(lut)
        #self.scalar_bar.SetHeight(0.005)
        self.scalar_bar.SetWidth(0.90)
        self.scalar_bar.SetPosition(0.04, 0.02)
        #self.scalar_bar.MaximumWidthInPixels = 10
        self.scalar_bar.SetMaximumWidthInPixels(100)

        # create the scalar_bar_widget
        self.scalar_bar_widget = vtk.vtkScalarBarWidget()
        self.scalar_bar_widget.SetInteractor(self.iren)
        self.scalar_bar_widget.SetScalarBarActor(self.scalar_bar)

        self.firtTimeRender = True

        self.update3D.connect(partial(self.PlotLevelset_slot,MainWindow,MainWindow.WhatToPlotFunction))

    def Reset(self):
        self.phi_mapper.SetInputConnection(None)
        self.zone_mapper.SetInputConnection(None)

    def SetViewType(self,char):
        if char.lower() == "w":
            self.mesh_actor.GetProperty().SetRepresentationToWireframe()
            self.zone_actor.GetProperty().SetRepresentationToWireframe()
        elif char.lower() == "s":
            self.mesh_actor.GetProperty().SetRepresentationToSurface()
            self.zone_actor.GetProperty().SetRepresentationToSurface()
        else:
            raise ValueError("iew type not supported")

    def Initialize(self):
        if self._initialized:
            return
        self.iren.Initialize()
        self.vtkWidget.show()
        self._initialized = True

    def GetPlotedSurfaceData(self):
        icon = self.phi_mapper.GetInputConnection(0,0)
        if icon is None:
            return None

        cutTriangles= vtk.vtkTriangleFilter()
        cutTriangles.SetInputConnection(icon)
        cutTriangles.Update()
        skin = cutTriangles.GetOutput(0)

        from Muscat.Bridges.vtkBridge import VtkToMesh
        return VtkToMesh(skin)

    def PlotLevelset_slot(self,MainWindow,WhatToPlotFunction):

        _,ls = MainWindow.GetActiveLevelset()
        if ls is None:
            if self.plotZones:
                self.PlotZones(MainWindow)
                if self.firtTimeRender:
                    self.firtTimeRender = False
                    self.ren.ResetCamera()
                    self.vtkWidget.GetRenderWindow().Render()
            return

        res = WhatToPlotFunction()
        if len(res) == 0:
            return

        whatsidetoplot = res["sideToPlot"]
        levelsetOrMesh = res["cutOrExtractInsideElements"]
        sval = res["cutByValue"]
        cutByName = res["cutByName"]
        cutBy = res["cutByField"]
        fieldForColormap = res["colorByField"]
        colorByName = res["colorByName"]

        ## Create the color map#################################################
        #print(fieldForColormap)
        mmin = np.amin(fieldForColormap)
        mmax = np.amax(fieldForColormap)

        cmin = np.amin(cutBy)
        cmax = np.amax(cutBy)

        import math

        if ls.IsNodal():
            self.phi_mapper.SetScalarModeToUsePointData()
            if sval > 50:
                vmax = math.atan(cmax)
                ls_or_density = math.tan( vmax*(sval-50.)/50)
            elif sval < 50:
                vmin = -math.atan(cmin)
                ls_or_density = math.tan( vmin*(sval-50.)/50)
            else:
                ls_or_density = 0.
        else:
            self.phi_mapper.SetScalarModeToUseCellData()
            if sval > 50:
                vmax = math.atan(0.5)
                ls_or_density = math.tan( vmax*(sval-50.)/50)+0.5
            elif sval < 50:
                vmin = -math.atan(-0.5)
                ls_or_density = math.tan( vmin*(sval-50.)/50)+0.5
            else:
                ls_or_density = 0.5

        Debug(" cutBy :'" + cutByName +"' at :"+str(ls_or_density)+"  colormapBy : '" +str(colorByName) + "' ("+str(mmin)+","+str(mmax)+") ")
        MainWindow.statusBar().showMessage( " cutBy :'" + cutByName +"' at :"+str(ls_or_density)+"  colormapBy : '" +str(colorByName) + "' ("+str(mmin)+","+str(mmax)+") ")

        if colorByName[0:3].lower() == "phi" :
            if mmin >= mmax:
                mmin = -1
                mmax = 1

            ctfun = vtk.vtkColorTransferFunction()
            ctfun.AddRGBPoint(         mmin, 0.0, 1.0, 1.0)

            # in the case of mmin is not in the negative side
            if mmin < ls_or_density:
                  ctfun.AddRGBPoint( 1e-02  *mmin, 0.0, 0.0, 1.0)
                  ctfun.AddRGBPoint( 0.9e-02*mmin, 0.0, 1.0, 0.0)

            # in the case of mmax is not in the positive side
            if mmax > ls_or_density:
                ctfun.AddRGBPoint(0.9e-02 *mmax, 0.0, 1.0, 0.0)
                ctfun.AddRGBPoint(1e-02   *mmax, 1.0, 0.0, 0.0)

            ctfun.AddRGBPoint(         mmax, 1.0, 1.0, 0.0)
        else:
            ctfun = vtk.vtkColorTransferFunction()
            #colors
            c1rgb = [ 0.231, 0.298, 0.752]
            c2rgb = [ 0.713, 0.807, 0.980]
            c3rgb = [ 0.865, 0.865, 0.865]
            c4rgb = [ 0.964, 0.745, 0.643]
            c5rgb = [ 1, 0, 0]

            if mmin < 0 and mmax > 0 :
                if mmin < -1e-20:
                    ctfun.AddRGBPoint(mmin,     c1rgb[0],c1rgb[1],   c1rgb[2])
                    ctfun.AddRGBPoint(mmin/100, c2rgb[0],c2rgb[1],   c2rgb[2])
                else:
                    ctfun.AddRGBPoint(-1e-20, c1rgb[0],c1rgb[1],   c1rgb[2])
                    ctfun.AddRGBPoint(-1e-22, c2rgb[0],c2rgb[1],   c2rgb[2])

                ctfun.AddRGBPoint(0,         c3rgb[0],c3rgb[1],   c3rgb[2])

                if mmax > 1e-20:
                    ctfun.AddRGBPoint(mmax/100, c4rgb[0],c4rgb[1],   c4rgb[2])
                    ctfun.AddRGBPoint(mmax,         0.70588200000000001,         0.0156863,         0.14902000000000001)
                else:
                    ctfun.AddRGBPoint(1e-22, c4rgb[0],c4rgb[1],   c4rgb[2])
                    ctfun.AddRGBPoint(1e-20,         0.70588200000000001,         0.0156863,         0.14902000000000001)
            else:
                if mmax < 0 :
                    ctfun.AddRGBPoint(mmin,c1rgb[0],c1rgb[1],   c1rgb[2])
                    ctfun.AddRGBPoint(mmax,c2rgb[0],c2rgb[1],   c2rgb[2])
                else:
                    ctfun.AddRGBPoint(mmin,c4rgb[0],c4rgb[1],   c4rgb[2])
                    ctfun.AddRGBPoint(mmax,c5rgb[0],c5rgb[1],   c5rgb[2])

        self.phi_mapper.SetInterpolateScalarsBeforeMapping(True)
        self.phi_mapper.SetScalarRange(mmin, mmax)
        self.phi_mapper.SetUseLookupTableScalarRange(True)
        self.phi_mapper.SetLookupTable(ctfun)

        self.scalar_bar.SetLookupTable(ctfun)
        self.scalar_bar.SetTitle(colorByName)
        self.scalar_bar_widget.On()

        self.phi_mapper.SelectColorArray('CColor')

        if ls.IsNodal():
            if ls.support.props.get("IsConstantRectilinear",False):
                self.PlotStructuredMeshNodal(whatsidetoplot,ls,cutBy,fieldForColormap,ls_or_density)
            else:
                self.PlotMeshNodal(whatsidetoplot,ls,cutBy,fieldForColormap,ls_or_density,levelsetOrMesh )
        else:
            # for a density field
            if ls.support.props.get("IsConstantRectilinear",False):
                self.PlotStructuredMeshElement(whatsidetoplot,ls,cutBy,fieldForColormap,ls_or_density)
            else:
                self.PlotMeshElement(whatsidetoplot,ls,cutBy,fieldForColormap,ls_or_density,levelsetOrMesh)

        if self.plotZones:
            self.PlotZones(MainWindow)
        else:
            self.zone_mapper.SetInputConnection(None)

        if self.firtTimeRender:
            self.firtTimeRender = False
            self.ren.ResetCamera()

        ##self.ren.SetUseHiddenLineRemoval()
        self.vtkWidget.GetRenderWindow().Render()

    def PlotZones(self,MainWindow):
        if MainWindow.MainApp is not None:
            appendFilter = vtk.vtkAppendPolyData()
            plot = False
            for key,delayedZone in MainWindow.MainApp.zones.items():
                delayedZone.Init()
                zone = delayedZone.internalImplicitGeometry
                import Muscat.ImplicitGeometry.ImplicitGeometryObjects as IGObs
                tz = type(zone)
                if tz == IGObs.ImplicitGeometrySphere:
                    source = vtk.vtkSphereSource()
                    source.SetCenter(zone.center)
                    source.SetRadius(zone.radius)
                    appendFilter.AddInputConnection(source.GetOutputPort()) # gris
                    plot = True
                elif tz == IGObs.ImplicitGeometryStl:
                    appendFilter.AddInputData(zone.surface) # gris
                    plot = True
                elif tz == IGObs.ImplicitGeometryPlane:
                    source = vtk.vtkPlaneSource()
                    #source.SetOrigin(zone.point)
                    #source.SetNormal(zone.normal)
                    #source.Push(zone.offset)
                    source.Update()
                    normalizedX = [0 for _ in range(3)]
                    normalizedY = [0 for _ in range(3)]
                    normalizedZ = zone.normal

                    import random
                    math = vtk.vtkMath()

                    # The Z axis is an arbitrary vector cross X
                    arbitrary = [0 for _ in range(3)]
                    if normalizedZ[0] != 0 :
                        arbitrary[0] = random.uniform(-10,10)
                        arbitrary[1] = random.uniform(-10,10)
                        arbitrary[2] = 1e-10
                    else :
                        arbitrary[0] = 1e-10
                        arbitrary[1] = random.uniform(-10,10)
                        arbitrary[2] = random.uniform(-10,10)
                    math.Cross(normalizedZ, arbitrary, normalizedX)
                    math.Normalize(normalizedX)

                    # The Y axis is Z cross X
                    math.Cross(normalizedZ, normalizedX, normalizedY)
                    matrix = vtk.vtkMatrix4x4()

                    # Create the direction cosine matrix
                    matrix.Identity()
                    for i in range(3):
                      matrix.SetElement(i, 0, normalizedX[i])
                      matrix.SetElement(i, 1, normalizedY[i])
                      matrix.SetElement(i, 2, normalizedZ[i])

                    # Apply the transforms
                    transform = vtk.vtkTransform()

                    transform.Translate(zone.point+ zone.normal*(zone.offset))
                    transform.Concatenate(matrix)
                    #transform.Scale(length, length, length)
                    # Transform the polydata
                    transformPD = vtk.vtkTransformPolyDataFilter()
                    transformPD.SetTransform(transform)
                    transformPD.SetInputConnection(source.GetOutputPort())

                    appendFilter.AddInputConnection(transformPD.GetOutputPort()) # gris

                    plot = True
                elif tz == IGObs.ImplicitGeometryAxisAlignBox    :
                    source = vtk.vtkCubeSource()
                    source.SetBounds(zone.origin[0],zone.origin[0]+zone.size[0],
                                    zone.origin[1],zone.origin[1]+zone.size[1],
                                    zone.origin[2],zone.origin[2]+zone.size[2],
                                    )
                    appendFilter.AddInputConnection(source.GetOutputPort()) # gris
                    plot = True
                elif tz == IGObs.ImplicitGeometryCylinder:

                    cylinder = vtk.vtkCylinderSource()
                    cylinder.SetResolution(20)
                    h = np.linalg.norm(zone.center2-zone.center1)
                    cylinder.SetHeight(h )
                    cylinder.SetRadius(zone.radius)
                    # The X axis is a vector from start to end
                    normalizedX = [0 for _ in range(3)]
                    normalizedY = [0 for _ in range(3)]
                    normalizedZ = [0 for _ in range(3)]
                    endPoint = [x for x in zone.center2]
                    startPoint = [x for x in zone.center1]
                    #startPoint[2] += h/2.
                    #endPoint[2] += h/2.
                    math = vtk.vtkMath()
                    math.Subtract(endPoint,startPoint, normalizedX)
                    #length = math.Norm(normalizedX)
                    math.Normalize(normalizedX)
                    #print(normalizedX)
                    import random

                    # The Z axis is an arbitrary vector cross X
                    arbitrary = [0 for _ in range(3)]
                    arbitrary[0] = random.uniform(-10,10)
                    arbitrary[1] = random.uniform(-10,10)
                    arbitrary[2] = random.uniform(-10,10)
                    math.Cross(normalizedX, arbitrary, normalizedZ)
                    math.Normalize(normalizedZ)

                    # The Y axis is Z cross X
                    math.Cross(normalizedZ, normalizedX, normalizedY)
                    matrix = vtk.vtkMatrix4x4()

                    # Create the direction cosine matrix
                    matrix.Identity()
                    for i in range(3):
                      matrix.SetElement(i, 0, normalizedZ[i])
                      matrix.SetElement(i, 1, normalizedX[i])
                      matrix.SetElement(i, 2, normalizedY[i])

                    # Apply the transforms
                    transform = vtk.vtkTransform()
                    transform.Translate(np.array(startPoint)+np.array(normalizedX)*h/2)
                    transform.Concatenate(matrix)
                    #transform.Scale(length, length, length)
                    # Transform the polydata
                    transformPD = vtk.vtkTransformPolyDataFilter()
                    transformPD.SetTransform(transform)
                    transformPD.SetInputConnection(cylinder.GetOutputPort())

                    appendFilter.AddInputConnection(transformPD.GetOutputPort()) # gris
                    plot = True
            if plot :
                self.zone_mapper.SetInputConnection(appendFilter.GetOutputPort())
            else:
                self.zone_mapper.SetInputConnection(None)

    def PlotStructuredMeshNodal(self,whatsidetoplot,ls,cutBy,fieldForColormap,ls_or_density):
        image = vtk.vtkImageData()

        support3D = (ls.support.GetElementsDimensionality() == 3)

        if support3D:
            image.SetDimensions(ls.support.props.get("dimensions"))
            image.SetOrigin(ls.support.props.get("origin"))
            image.SetSpacing(ls.support.props.get("spacing"))
        else:
            d = ls.support.props.get("dimensions")
            image.SetDimensions(d[0],d[1],1)
            o = ls.support.props.get("origin")
            image.SetOrigin(o[0],o[1],0.)
            s = ls.support.props.get("spacing")
            image.SetSpacing(s[0],s[1],0)

        fieldForColormapView = fieldForColormap.view()
        fieldForColormapView.shape = ls.support.props.get("dimensions")

        if support3D:
            VTK_data_color = numpy_support.numpy_to_vtk(num_array=np.swapaxes(fieldForColormapView,0,2).ravel(), deep=True, array_type=vtk.VTK_FLOAT)
        else:
            VTK_data_color = numpy_support.numpy_to_vtk(num_array=np.swapaxes(fieldForColormapView,0,1).ravel(), deep=True, array_type=vtk.VTK_FLOAT)

        VTK_data_color.SetName('CColor')
        #image.GetPointData().SetScalars(VTK_data_color)
        image.GetPointData().AddArray(VTK_data_color)

        phi = cutBy.view()
        phi.shape = ls.support.props.get("dimensions")
        if support3D:
            VTK_data = numpy_support.numpy_to_vtk(num_array=np.swapaxes(phi,0,2).ravel(), deep=True, array_type=vtk.VTK_FLOAT)
        else:
            VTK_data = numpy_support.numpy_to_vtk(num_array=np.swapaxes(phi,0,1).ravel(), deep=True, array_type=vtk.VTK_FLOAT)
        VTK_data.SetName('phi')
        image.GetPointData().AddArray(VTK_data)
        image.GetPointData().SetActiveScalars("phi")

        if whatsidetoplot != 10:
            iso = vtk.vtkContourFilter()
            iso.SetInputData(image)

            iso.ComputeGradientsOff()
            iso.ComputeNormalsOn()
            iso.SetNumberOfContours(1)
            iso.SetValue(0,ls_or_density)

            iso.SetInputArrayToProcess(0,0,0,0,'phi')
            #iso.SetInputArrayToProcess(3,0,0,0,'Color')		# colors
            iso.SetComputeScalars(True)
            #iso.SetGenerateTriangles(False)
            iso.SetComputeNormals(False)

        if whatsidetoplot != 0 :

            extractSurface1 = vtk.vtkDataSetSurfaceFilter()
            extractSurface1.SetInputData(image)

            extractSurface1.SetInputArrayToProcess(0,0,0,0,'phi')
            #extractSurface1.SetInputArrayToProcess(3,0,0,0,"Color")		# colors

            if whatsidetoplot == -1 or  whatsidetoplot == 1 :
                clip = vtk.vtkClipPolyData()
                clip.SetInputConnection(extractSurface1.GetOutputPort())
                clip.SetInputArrayToProcess(0,0,0,0,'phi')
                clip.SetInputArrayToProcess(3,0,0,0,"CColor")		# colors
                if whatsidetoplot == -1  :
                    clip.InsideOutOn()
                else:
                    clip.InsideOutOff()

                clip.SetValue(ls_or_density)
                #clip.GenerateClippedOutputOn()
                #clip.GenerateClipScalarsOn()

                appendFilter = vtk.vtkAppendPolyData()

                appendFilter.AddInputConnection(iso.GetOutputPort()) # gris
                appendFilter.AddInputConnection(clip.GetOutputPort()) #ok
                #self.phi_mapper.SetInputConnection(appendFilter.GetOutputPort())
                if support3D:
                    out = appendFilter
                else:
                    out = clip

            else:
                #self.phi_mapper.SetInputConnection(extractSurface1.GetOutputPort())
                out = extractSurface1
        else :
            #self.phi_mapper.SetInputConnection(iso.GetOutputPort())
            out = iso

        out.Update()
        out2 = vtk.vtkAssignAttribute()
        out2.SetInputConnection(out.GetOutputPort())
        out2.Assign("CColor", "SCALARS", "POINT_DATA")
        self.phi_mapper.SetInputConnection(out2.GetOutputPort())

    def PlotMeshNodal(self,whatsidetoplot,ls,cutBy,fieldForColormap,ls_or_density,levelsetOrMesh):

        from Muscat.Bridges.vtkBridge  import MeshToVtk
        from Muscat.MeshTools.MeshInspectionTools  import ExtractElementsByTags

        a = ExtractElementsByTags(ls.support,["Ridges"])
        a.nodeFields["phi"] = ls.phi[a.originalIDNodes]
        a.nodeFields["CColor"] = fieldForColormap[a.originalIDNodes]
        ridges_old = MeshToVtk(a)
        passArrays = vtk.vtkPassArrays()
        passArrays.SetInputData(ridges_old)
        passArrays.RemoveArraysOn()
        passArrays.AddPointDataArray("originalIds")
        passArrays.AddCellDataArray("originalIds")
        passArrays.Update()
        ridges = passArrays.GetOutput()

        if levelsetOrMesh:
            ls.support.nodeFields["phi"] = cutBy

            ls.support.nodeFields["CColor"] = fieldForColormap

            vtkmesh_old = MeshToVtk(ls.support)
            passArrays = vtk.vtkPassArrays()
            passArrays.SetInputData(vtkmesh_old)
            passArrays.RemoveArraysOn()
            passArrays.AddPointDataArray("originalIds")
            passArrays.AddCellDataArray("originalIds")
            passArrays.Update()
            vtkmesh = passArrays.GetOutput()

            if whatsidetoplot != 10:
                iso = vtk.vtkContourFilter()

                iso.SetInputData(vtkmesh)

                iso.ComputeGradientsOff()
                iso.ComputeNormalsOff()
                iso.SetNumberOfContours(1)
                iso.SetValue(0,ls_or_density)
                iso.SetInputArrayToProcess(0,0,0,0,'phi')
                iso.Update()

            if whatsidetoplot != 0:
                extractSurface1 = vtk.vtkDataSetSurfaceFilter()

                extractSurface1.SetInputData(vtkmesh)

                extractSurface1.SetInputArrayToProcess(0,0,0,0,'phi')

                extractSurface1.Update()

                if whatsidetoplot == -1 or whatsidetoplot == 1:
                    clip = vtk.vtkClipPolyData()
                    #clip.SetInputConnection(extractSurface1.GetOutputPort())
                    ES1 = extractSurface1.GetOutput()
                    ES1.GetPointData().SetActiveScalars("phi")
                    clip.SetInputData(ES1)

                    clip.SetInputArrayToProcess(0,0,0,0,'phi')
                    clip.SetInputArrayToProcess(3,0,0,0,"CColor")		# colors

                    if whatsidetoplot == -1 :
                        clip.InsideOutOn()
                    else:
                        clip.InsideOutOff()

                    #clip.GenerateClipScalarsOff()
                    #clip.GenerateClippedOutputOff()
                    clip.SetValue(ls_or_density)

                    #clip.GenerateClippedOutputOn()

                    appendFilter = vtk.vtkAppendPolyData()

                    clip.Update()
                    appendFilter.AddInputConnection(iso.GetOutputPort())
                    appendFilter.AddInputConnection(clip.GetOutputPort())
                    appendFilter.Update()
                    res = appendFilter.GetOutput()
                else:
                    res = extractSurface1.GetOutput()
            else:
                iso.Update()
                res = iso.GetOutput()

        else:
            if whatsidetoplot == -1:
                if ls.support.GetNamesOfElementTags().count(TZ.Inside3D):
                    toplot = ExtractElementsByTags(ls.support,[TZ.Inside3D],allNodes=True )
                    toplot.nodeFields["phi"] = ls.phi[toplot.originalIDNodes]
                    toplot.nodeFields["CColor"] = fieldForColormap[toplot.originalIDNodes]
                else:
                    toplot  = ls.support
                    toplot.nodeFields["phi"] = ls.phi
                    toplot.nodeFields["CColor"] = fieldForColormap

            elif whatsidetoplot == 0:
                if ls.support.GetNamesOfElementTags().count(TZ.InterSurf):
                    toplot = ExtractElementsByTags(ls.support,[TZ.InterSurf],allNodes=True )
                    toplot.nodeFields["phi"] = ls.phi[toplot.originalIDNodes]
                    toplot.nodeFields["CColor"] = fieldForColormap[toplot.originalIDNodes]
                else:
                    toplot  = ls.support
                    toplot.nodeFields["phi"] = ls.phi
                    toplot.nodeFields["CColor"] = fieldForColormap
            elif whatsidetoplot == 1:
                if ls.support.GetNamesOfElementTags().count(TZ.Outside3D):
                    toplot = ExtractElementsByTags(ls.support,[TZ.Outside3D],allNodes=True )
                    toplot.nodeFields["phi"] = ls.phi[toplot.originalIDNodes]
                    toplot.nodeFields["CColor"] = fieldForColormap[toplot.originalIDNodes]
                else:
                    toplot  = ls.support
                    toplot.nodeFields["phi"] = ls.phi
                    toplot.nodeFields["CColor"] = fieldForColormap
            elif whatsidetoplot == 10:
                 #all
                toplot  = ls.support
                toplot.nodeFields["phi"] = ls.phi
                toplot.nodeFields["CColor"] = fieldForColormap
            res = MeshToVtk(toplot)

        appendFilter = vtk.vtkAppendFilter()

        appendFilter.AddInputData(res)
        appendFilter.AddInputData(ridges)
        #appendFilter.Update()
        #res = appendFilter.GetOutput()

        surface = vtk.vtkDataSetSurfaceFilter()
        #surface.SetInputData(res)
        surface.AddInputConnection(appendFilter.GetOutputPort())
        #self.phi_mapper.SetInputConnection(surface.GetOutputPort())

        #out.Update()
        out2 = vtk.vtkAssignAttribute()
        out2.SetInputConnection(surface.GetOutputPort())
        out2.Assign("CColor", "SCALARS", "POINT_DATA")
        self.phi_mapper.SetInputConnection(out2.GetOutputPort())

    def PlotStructuredMeshElement(self,whatsidetoplot,ls,cutBy,fieldForColormap,ls_or_density):
        image = vtk.vtkImageData()
        image.SetDimensions(ls.support.props.get("dimensions"))
        image.SetOrigin(ls.support.GetOrigin())
        image.SetSpacing(ls.support.props.get("spacing"))

        fieldForColormapView = fieldForColormap.view()
        fieldForColormapView.shape = ls.support.props.get("dimensions")-1
        VTK_data_color = numpy_support.numpy_to_vtk(num_array=np.swapaxes(fieldForColormapView,0,2).ravel(), deep=True, array_type=vtk.VTK_FLOAT)
        VTK_data_color.SetName('CColor')
        #image.GetPointData().SetScalars(VTK_data_color)
        image.GetCellData().AddArray(VTK_data_color)

        phi = cutBy.view()
        phi.shape = ls.support.props.get("dimensions")-1
        VTK_data = numpy_support.numpy_to_vtk(num_array=np.swapaxes(phi,0,2).ravel(), deep=True, array_type=vtk.VTK_FLOAT)
        VTK_data.SetName('phi')
        image.GetCellData().AddArray(VTK_data)
        image.GetCellData().SetActiveScalars("phi")

        if whatsidetoplot != 10:
            if whatsidetoplot != 0:
                threshold = vtk.vtkThreshold()
                threshold.SetInputData(image)

                extractSurface1 = vtk.vtkDataSetSurfaceFilter()

                threshold.ThresholdByUpper(1.0)
                threshold.ThresholdByLower(0)

                if whatsidetoplot == 1  :
                    threshold.ThresholdByLower(ls_or_density)
                elif whatsidetoplot == -1  :
                    threshold.ThresholdByUpper(ls_or_density)

                extractSurface1.SetInputConnection(threshold.GetOutputPort())

                extractSurface1.SetInputArrayToProcess(0,0,0,1,'phi')
                out = extractSurface1
            else:
                cellToPoint = vtk.vtkCellDataToPointData()
                cellToPoint.SetInputData(image)

                cellToPoint.PassCellDataOff()
                cellToPoint.Update()
                iso = vtk.vtkContourFilter()
                iso.SetInputConnection(cellToPoint.GetOutputPort())

                iso.ComputeGradientsOff()
                iso.ComputeNormalsOn()
                iso.SetNumberOfContours(1)
                iso.SetValue(0,ls_or_density)

                iso.SetInputArrayToProcess(0,0,0,0,'phi')
                iso.Update()
                #iso.SetInputArrayToProcess(3,0,0,0,'Color')		# colors
                #iso.SetComputeScalars(True)
                #iso.SetGenerateTriangles(False)
                #iso.SetComputeNormals(False)

                extractSurface1 = vtk.vtkDataSetSurfaceFilter()
                extractSurface1.SetInputData(cellToPoint.GetOutput())

                extractSurface1.SetInputArrayToProcess(0,0,0,0,'phi')


                clip = vtk.vtkClipPolyData()
                clip.SetInputConnection(extractSurface1.GetOutputPort())
                clip.SetInputArrayToProcess(0,0,0,0,'phi')
                clip.SetInputArrayToProcess(3,0,0,0,"CColor")		# colors

                #clip.InsideOutOn()
                #clip.InsideOutOff()

                clip.SetValue(ls_or_density)
                clip.GenerateClippedOutputOn()

                appendFilter = vtk.vtkAppendPolyData()

                appendFilter.AddInputConnection(iso.GetOutputPort())
                appendFilter.AddInputConnection(clip.GetOutputPort())


                out = appendFilter
                self.phi_mapper.SetScalarModeToUsePointData()

        else:
            extractSurface1 = vtk.vtkDataSetSurfaceFilter()

            extractSurface1.SetInputData(image)

            extractSurface1.SetInputArrayToProcess(0,0,0,1,'phi')
            out = extractSurface1

        out.Update()
        #print(out.GetOutput())
        out2 = vtk.vtkAssignAttribute()
        out2.SetInputConnection(out.GetOutputPort())
        out2.Assign("CColor", "SCALARS", "POINT_DATA")
        self.phi_mapper.SetInputConnection(out2.GetOutputPort())


    def PlotMeshElement(self,whatsidetoplot,ls,cutBy,fieldForColormap,ls_or_density,levelsetOrMesh):
        from Muscat.Bridges.vtkBridge  import MeshToVtk

        from Muscat.MeshTools.MeshInspectionTools import ExtractElementByDimensionalityNoCopy
        libmesh =ExtractElementByDimensionalityNoCopy(ls.support,dimensionalityFilter=3)
        #print(libmesh)
        vtkmesh = MeshToVtk(libmesh)

        #print(vtkmesh)
        wireMesh = ExtractElementByDimensionalityNoCopy(ls.support,dimensionalityFilter=-3)
        wireMesh.nodeFields = {}
        wireMesh.elemFields = {}

        vtkmeshII = MeshToVtk(wireMesh)
        extractSurfaceII = vtk.vtkDataSetSurfaceFilter()

        extractSurfaceII.SetInputData(vtkmeshII)


        fieldForColormapView = fieldForColormap.view()
        VTK_data_color = numpy_support.numpy_to_vtk(num_array=fieldForColormapView.ravel(), deep=True, array_type=vtk.VTK_FLOAT)
        VTK_data_color.SetName('CColor')
        vtkmesh.GetCellData().AddArray(VTK_data_color)

        phi = cutBy.view()
        VTK_data = numpy_support.numpy_to_vtk(num_array=phi.ravel(), deep=True, array_type=vtk.VTK_FLOAT)
        VTK_data.SetName('phi')
        vtkmesh.GetCellData().AddArray(VTK_data)
        vtkmesh.GetCellData().SetActiveScalars("phi")

        if whatsidetoplot != 10:
            if whatsidetoplot != 0:
                threshold = vtk.vtkThreshold()
                threshold.SetInputData(vtkmesh)

                extractSurface1 = vtk.vtkDataSetSurfaceFilter()

                threshold.ThresholdByUpper(1.0)
                threshold.ThresholdByLower(0)

                if whatsidetoplot == 1  :
                    threshold.ThresholdByLower(ls_or_density)
                elif whatsidetoplot == -1  :
                    threshold.ThresholdByUpper(ls_or_density)

                extractSurface1.SetInputConnection(threshold.GetOutputPort())

                extractSurface1.SetInputArrayToProcess(0,0,0,1,'phi')
                out = extractSurface1
            else:
                cellToPoint = vtk.vtkCellDataToPointData()
                cellToPoint.SetInputData(vtkmesh)


                cellToPoint.PassCellDataOff()
                cellToPoint.Update()
                iso = vtk.vtkContourFilter()
                iso.SetInputConnection(cellToPoint.GetOutputPort())

                iso.ComputeGradientsOff()
                iso.ComputeNormalsOn()
                iso.SetNumberOfContours(1)
                iso.SetValue(0,ls_or_density)

                iso.SetInputArrayToProcess(0,0,0,0,'phi')
                iso.Update()
                #iso.SetInputArrayToProcess(3,0,0,0,'Color')		# colors
                #iso.SetComputeScalars(True)
                #iso.SetGenerateTriangles(False)
                #iso.SetComputeNormals(False)

                extractSurface1 = vtk.vtkDataSetSurfaceFilter()
                extractSurface1.SetInputData(cellToPoint.GetOutput())

                extractSurface1.SetInputArrayToProcess(0,0,0,0,'phi')


                clip = vtk.vtkClipPolyData()
                clip.SetInputConnection(extractSurface1.GetOutputPort())
                clip.SetInputArrayToProcess(0,0,0,0,'phi')
                clip.SetInputArrayToProcess(3,0,0,0,"CColor")		# colors

                #clip.InsideOutOn()
                #clip.InsideOutOff()

                clip.SetValue(ls_or_density)
                clip.GenerateClippedOutputOn()

                appendFilter = vtk.vtkAppendPolyData()

                appendFilter.AddInputConnection(iso.GetOutputPort())
                appendFilter.AddInputConnection(clip.GetOutputPort())


                out = appendFilter
                self.phi_mapper.SetScalarModeToUsePointData()


        else:
            extractSurface1 = vtk.vtkDataSetSurfaceFilter()

            extractSurface1.SetInputData(vtkmesh)

            extractSurface1.SetInputArrayToProcess(0,0,0,1,'phi')
            out = extractSurface1

        out.Update()
        #print(out.GetOutput())
        appendFilterDim_1 = vtk.vtkAppendPolyData()


        appendFilterDim_1.AddInputData(out.GetOutput())
        appendFilterDim_1.AddInputData(extractSurfaceII.GetOutput())

        out2 = vtk.vtkAssignAttribute()
        #out2.SetInputConnection(appendFilterDim_1.GetOutputPort())
        out2.SetInputConnection(out.GetOutputPort())

        out2.Assign("CColor", "SCALARS", "POINT_DATA")
        self.phi_mapper.SetInputConnection(out2.GetOutputPort())


def CheckIntegrity():
    return "OK"

if __name__ == '__main__':
        print(CheckIntegrity())
