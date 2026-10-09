# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import time
import os


_startTime = time.time()
debug = bool(os.environ.get("PARAVIEW_LOG_PLUGIN_VERBOSITY", False))
def PrintDebug(mes):
    if debug:
        import time
        print(mes,time.time() - _startTime)

from paraview.util.vtkAlgorithm import smproxy, smproperty, smdomain, smhint
from paraview.util.vtkAlgorithm import VTKPythonAlgorithmBase

try:
#----------------------------- The Meshes ------------------------------------
    PrintDebug("Loading Libs")
    from Muscat.Bridges.vtkBridge import GetInputMuscat,  SetOutputMuscat
    from Muscat.Helpers.IO.Which import Which
    import OpenPisco.Unstructured.MmgMesher as MM
    toTest = [MM.mmg3dExec, MM.mmg2dExec, MM.mmgsExec]
    for execname in toTest:
        if not Which(execname):
            print("Error : Executable '"+execname+"' not found !")
    PrintDebug("Loading Libs Done")

    # Element Filter
    @smproxy.filter(name="Remesh")
    @smhint.xml("""<ShowInMenu category="Muscat" />""")
    @smproperty.input(name="Input", port_index=0)
    @smdomain.datatype(dataTypes=["vtkUnstructuredGrid","vtkPolyData"], composite_data_supported=False)
    class MuscatReMeshFilter(VTKPythonAlgorithmBase):
        def __init__(self):
            VTKPythonAlgorithmBase.__init__(self, nInputPorts=1, nOutputPorts=1, outputType="vtkUnstructuredGrid")
            self.__nr = False
            self.__hmin = 1
            self.__hmax = 2
            self.__ar = 20
            self.__hausd = 1.
            self.__hgrad = 1.3
            self.__keepGeneratedFiles = False
            self.__ls = False
            self.__met = False
            self.__optim = False
            self.__rmc = False
            self.__nosurf = False

        def RequestData(self, request, inInfoVec, outInfoVec):
            inMesh = GetInputMuscat(request, inInfoVec, outInfoVec, True)
            from OpenPisco.Unstructured.MmgMesher import MmgMesher

            mymeshInfo={"hausd":self.__hausd,  "hmin":self.__hmin, "hmax":self.__hmax,"optim":self.__optim }
            if self.__nr:
                mymeshInfo["nr"] = True
            else:
                mymeshInfo["ar"] = self.__ar

            if self.__rmc:
                mymeshInfo["rmc"] = True

            if self.__nosurf:
                mymeshInfo["nosurf"] = True

            obj = MmgMesher()
            obj.SetBinaryFlag(True)
            obj.keepGeneratedFiles = self.__keepGeneratedFiles

            if self.__ls and "ls" in inMesh.nodeFields:
                mymeshInfo["iso"] = 0.0
                obj.SetSupport(inMesh)
                obj.SetRemeshWithIso()
            else :
                obj.SetSupport(inMesh,writeTopoZones=True)

            obj.WriteSupport()

            if self.__met  and "met" in inMesh.nodeFields:
                obj.WriteMetricField()
                mymeshInfo["met"] = True

            if self.__ls and "ls" in inMesh.nodeFields:
                obj.WriteLevelSet(inMesh.nodeFields["ls"])
                mymeshInfo["iso"] = 0.0


            if self.__keepGeneratedFiles:
                print('Temporary files in : '+  str(obj.GetWorkingDirectory()))

            obj.WriteInputParametersFile(mymeshInfo)
            obj.GenerateMesh(mymeshInfo)
            outMesh = obj.GetNewMesh()
            SetOutputMuscat(request,inInfoVec,outInfoVec,outMesh, TagsAsFields=True)
            return 1

        @smproperty.xml("""<IntVectorProperty
                             name="No Angle Detection"
                             command="SetNR"
                             number_of_elements="1"
                             default_values="0">
                             <BooleanDomain name="bool"/>
                             <Documentation>No angle detection.. </Documentation>
                             </IntVectorProperty>""")
        def SetNR(self, val):
            val = bool(val)
            if self.__nr != val :
                self.__nr = val
                self.Modified()

        @smproperty.xml("""
        <DoubleVectorProperty command="SetH"
                                default_values="0.1 100.0"
                                label="Element Size Range"
                                name="ElementSizeRange"
                                number_of_elements="2"
                                panel_widget="double_range">
                                <BoundsDomain name="range" mode="scaled_extent" scale_factor="0.05">
                                  <RequiredProperties>
                                    <Property function="Input" name="Input" />
                                  </RequiredProperties>
                                  <Hints>
                                    <NoDefault/>
                                    <MinimumLabel text="Min H"/>
                                    <MaximumLabel text="Max H" />
                                    <HideResetButton/>
                                  </Hints>
                                </BoundsDomain>
                                <Documentation>The -hmin and -hmax options allows to truncate the edge sizes to be (respectively) greater than the hmin parameter and lower than the hmax one</Documentation>
          </DoubleVectorProperty>""")
        def SetH(self, hmin=0.1,hmax=100.0):
            hmax = float(hmax)
            if self.__hmax != hmax :
                self.__hmax = hmax
                self.Modified()
            hmin = float(hmin)
            if self.__hmin != hmin :
                self.__hmin = hmin
                if self.__hmin == 0:
                    self.__hmin = 1e-100
                self.Modified()

        @smproperty.xml("""<DoubleVectorProperty
                             name="Hausdorff"
                             command="SetHausdorff"
                             number_of_elements="1"
                             default_values="0.11">
                                     <BoundsDomain mode="approximate_cell_length" name="bounds" scale_factor="0.05">
              <RequiredProperties>
                <Property function="Input" name="Input" />
              </RequiredProperties>
            </BoundsDomain>
                    <Hints>
              <NoDefault/>
            </Hints>
                                    <Documentation>
            it impose the maximal distance between the piecewise linear representation of the boundary and the reconstructed ideal boundary. Thus, a low Hausdorff parameter leads to refine the high curvature areas.
                                    </Documentation>

                             </DoubleVectorProperty>""")
        def SetHausdorff(self, val):
            val = float(val)
            if self.__hausd != val :
                self.__hausd = val
                self.Modified()


        @smproperty.xml("""<DoubleVectorProperty
                             name="HGradation"
                             command="SetHGradation"
                             label="H Gradation "
                             number_of_elements="1"
                             default_values="1.3">
                             <DoubleRangeDomain name="range" min="0.0" max="5" />
                                                     <Documentation>

                             The hgrad option allows to set the gradation value. It controls the ratio between two adjacent edges
                                                     </Documentation>

                             </DoubleVectorProperty>""")
        def SetHGradation(self, val):
            val = float(val)
            if self.__hgrad != val :
                self.__hgrad = val
                self.Modified()

        @smproperty.xml("""<DoubleVectorProperty
                             name="AngleDetection"
                             command="SetAngleDetection"
                             label="Angle Detection"
                             number_of_elements="1"
                             default_values="90">
                             <DoubleRangeDomain name="range" min="0.0" max="180" />
                             </DoubleVectorProperty>""")
        def SetAngleDetection(self, val):
            val = float(val)
            if self.__ar != val :
                self.__ar = val
                self.Modified()

        @smproperty.xml("""<IntVectorProperty
                             name="KeepGeneratedFiles"
                             command="SetKeepGeneratedFiles"
                             label="Keep Generated Files"
                             number_of_elements="1"
                             default_values="0"
                             panel_visibility="advanced">
                             <BooleanDomain name="bool"/>
                             <Documentation>
                             Do not erase the generated file (for debugging)
                             </Documentation>
                             </IntVectorProperty>""")
        def SetKeepGeneratedFiles(self, val):
            val = bool(val)
            if self.__keepGeneratedFiles != val :
                self.__keepGeneratedFiles = val
                self.Modified()

        @smproperty.xml("""<IntVectorProperty
                             name="NoSurface"
                             command="SetNoSurf"
                             label="No Surface Modification"
                             number_of_elements="1"
                             default_values="0"
                             panel_visibility="advanced">
                             <BooleanDomain name="bool"/>
                             <Documentation>
                             No Surface Modification
                             </Documentation>
                             </IntVectorProperty>""")
        def SetNoSurf(self, val):
            val = bool(val)
            if self.__nosurf != val:
                self.__nosurf = val
                self.Modified()

        @smproperty.xml("""<IntVectorProperty
                             name="RemoveComponents"
                             command="SetRmc"
                             label="Remove Small Components (Level Set mode)"
                             number_of_elements="1"
                             default_values="0"
                             panel_visibility="advanced">
                             <BooleanDomain name="bool"/>
                             <Documentation>
                             enable the removal of components whose volume fraction is less than val (1e-5)
                             </Documentation>
                             </IntVectorProperty>""")
        def SetRmc(self, val):
            val = bool(val)
            if self.__rmc != val:
                self.__rmc = val
                self.Modified()


        @smproperty.xml("""<IntVectorProperty
                             name="LevelSetMode"
                             command="SetLevelSetMode"
                             label="Level Set Mode"
                             number_of_elements="1"
                             default_values="0"
                             panel_visibility="advanced">
                             <BooleanDomain name="bool"/>
                             <Documentation>
                             A level-set field with the name 'ls' must be present. Not compatible with the MetricMode
                             </Documentation>
                             </IntVectorProperty>""")
        def SetLevelSetMode(self, val):
            val = bool(val)
            if self.__ls != val :
                self.__ls = val
                self.Modified()

        @smproperty.xml("""<IntVectorProperty
                             name="MetricMode"
                             command="SetMetricMode"
                             label="Metric Mode"
                             number_of_elements="1"
                             default_values="0"
                             panel_visibility="advanced">
                             <BooleanDomain name="bool"/>
                             <Documentation>
                              A metric field with the name 'met' must be present.
                             </Documentation>
                             </IntVectorProperty>""")
        def SetMetricMode(self, val):
            val = bool(val)
            if self.__met != val :
                self.__met = val
                self.Modified()


        @smproperty.xml("""<IntVectorProperty
                             name="Only Optimization"
                             command="SetOnlyOptimization"
                             label="Only Optimization"
                             number_of_elements="1"
                             default_values="0"
                             panel_visibility="advanced">
                             <BooleanDomain name="bool"/>
                             <Documentation>
                            Mesh improvement without edge size modification.
                             </Documentation>
                             </IntVectorProperty>""")
        def SetOnlyOptimization(self, val):
            val = bool(val)
            if self.__optim != val :
                self.__optim = val
                self.Modified()


        @smproperty.xml("""<IntVectorProperty
                             name="VerboseMuscatOutput"
                             command="VerboseMuscatOutput"
                             label="Verbose Muscat Output"
                             number_of_elements="1"
                             default_values="0"
                             panel_visibility="advanced">
                             <BooleanDomain name="bool"/>
                             <Documentation>
                             </Documentation>
                             </IntVectorProperty>""")
        def VerboseMuscatOutput(self, val):
            val = bool(val)
            from Muscat.Helpers.Logger import VerboseMuscatOutput, DefaultMuscatOutput
            if val:
                VerboseMuscatOutput()
            else:
                DefaultMuscatOutput()

    PrintDebug("OpenPisco ParaView Plugin Loaded")
except:
    print("Error loading OpenPisco ParaView Plugin")
    print("Muscat/OpenPisco in the PYTHONPATH (installed) ??? ")
    if debug:
        raise
