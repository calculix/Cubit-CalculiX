# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import os
import re
import sys
import functools
import signal
import numpy as np
from functools import partial

from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
from Muscat.Helpers.IO.FileTools import WriteTempFile

from Muscat.Helpers import ParserHelper as PH
from Muscat.Helpers import LocalVariables as LV
import Muscat.ImplicitGeometry.ImplicitGeometryObjects as IGObs
from Muscat.Helpers.Logger import Info, Debug
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter
from Muscat.MeshContainers.Filters.FilterOperators import ComplementaryFilter

import OpenPisco.QtApp.QtImplementation as QT
import OpenPisco.CLApp.OpenPiscoCL as MainApp
from OpenPisco.QtApp.res import ResourcePath
from OpenPisco.QtApp.QtWorker import Worker
from OpenPisco.QtApp.OpenPiscoUIMainWindow import UiMainWindow
from OpenPisco.QtApp.OpenPiscoVTKRenderZone import StopoVTKRenderZone

exportSTLPath = "/var/spool/print/"
PISCOAPPEXTENSION = ".piscoApp"

class SimpleView(QT.QMainWindow):
    printInStatusBar = QT.Signal(str)
    setFullScreen = QT.Signal()
    MaxRecentFiles = 10

    def __init__(self, parent = None,with_demo=True):
        super().__init__(parent)  # Modern cooperative initialization
        #self.setWindowFlags(self.windowFlags() | 2048)
        self.with_demo = with_demo

        self.MainApp = MainApp.MainApp()

        self.thread = Worker(self)

        self.ui = UiMainWindow(self)
        self.vtk = StopoVTKRenderZone(self)

        # for recent files
        self.recentFileActs = []
        self.actions = {}
        self.CreateActions()

        self.ui.setupUi(self)
        self.vtk.setup3DView(self)

        self.thread.finished.connect(self.UpdateApp)

        self.printInStatusBar.connect(self.statusBar().showMessage)
        self.setFullScreen.connect(self.SetFullScreen)

        settings = QT.QtCore.QSettings()
        geometry = settings.value("geometry", bytearray())
        if geometry:
            self.restoreGeometry(geometry)#.toPyObject()
        # Do not restore the dock/toolbar state automatically here. Older saved
        # states can preserve native/floating widgets from previous VTK/Qt runs,
        # which shows up as duplicate title bars or detached child windows.
        # Geometry restoration is kept because it does not recreate dock/window
        # hierarchy.
        commandlogfilename = TemporaryDirectory.GetTempPath()+"commands.py"
        self.commandlogfile = open(commandlogfilename,"w")
        self.WriteToCommandLog("import numpy as np")
        self.WriteToCommandLog("import xml.etree.ElementTree as ElementTree")

        self.WriteToCommandLog("import OpenPisco.CLApp.MainApp as MainApp ")
        self.WriteToCommandLog("import Muscat.ImplicitGeometry.ImplicitGeometryObjects as IGObs")
        self.WriteToCommandLog("import Muscat.ImplicitGeometry.ImplicitGeometryOperators as IGOps")
        self.WriteToCommandLog("mainApp = MainApp.MainApp() ")
        print("Sending commands log to "+ commandlogfilename)

        self.colormapBy = 1
        self.single_window = False
        self.time = 0

        self.currentFile = None
        self.erodeDilateScale = 200
        self.interactiveSphereRadius = 40

    def ResetApp(self):
        self.ui.ObjectInspectorEditorScrollArea.setWidget(QT.QWidget())
        self.ResetMainApp()

        self.ui.controlPanelAppScrollArea.setWidget(QT.QWidget())
        self.ui.inputFileEditorTextEdit.setPlainText("")
        self.ResetIsoOrDensity()
        self.vtk.Reset()
        self.ui.tab2D.Reset()

        self.UpdateApp()

    def ResetMainApp(self):
        if self.MainApp is not None:
            self.MainApp.CloseAllOutputs()

        # we keep the same parsertype for the reader
        ext = self.MainApp.inputReader.ext
        self.MainApp =  MainApp.MainApp()
        self.MainApp.SetInternalReaderTo(ext)

        self.ui._pythonConsole.pushVariables({"mainApp":self.MainApp})
        self.WriteToCommandLog("mainApp = MainApp.MainApp() ")

    def SetFullScreen(self,val=True):
        if val == False:
            self.showNormal()
            return

        import time
        ctime = time.time()
        if ctime-self.time > 1:

            self.hide()
            self.showNormal()
            self.hide()

            QT.QTimer.singleShot(500,self.showFullScreen)
            #self.showFullScreen()

            self.time = ctime
        else:
            self.setFullScreen.emit()

    def GetActiveLevelset(self):
        IDD = self.ui.GetActiveLevelset()
        if IDD == -1:
            return (None,None)
        ls = self.MainApp.levelSets[ int(IDD)]
        return (IDD,ls)

    def WhatToPlotFunction(self):

        res = dict()
        res["sideToPlot"] = self.ui.combo_inout.itemData(self.ui.combo_inout.currentIndex())

        _,ls = self.GetActiveLevelset()
        if ls is None:
            return {}

        cutByName = self.ui.GetActiveFieldToPlot()
        if cutByName is None:
            return {}

        if "phi" in cutByName.lower() :
            if "phi" == cutByName.lower() :
                cutBy = ls.phi
            else:
                cutBy = ls.extraNodesFields[cutByName]
        elif cutByName.startswith("field: "):
            cutBy = ls.extraNodesFields[cutByName.split(": ")[1]]
        else:
            activeZone = self.MainApp.zones[int(cutByName)]
            cutBy = activeZone(ls.support)

        res["cutByField"] = cutBy
        res["cutByName"] =  cutByName
        res["cutByValue"] = float(self.ui.ls_slider.value())

        ls.extraNodesFields["phi"] = ls.phi

        colorby = (self.colormapBy)%len(ls.extraNodesFields)
        res["colorByField"] = list(ls.extraNodesFields.values())[colorby]
        res["colorByName"] = list(ls.extraNodesFields.keys())[colorby]

        res["cutOrExtractInsideElements"] = self.ui.viewLevelset

        return res

    def ViewNextField(self):
        self.colormapBy += 1
        self.vtk.update3D.emit()

    def Exit(self):
        settings = QT.QSettings()
        settings.setValue("geometry", self.saveGeometry())
        settings.setValue("windowState", self.saveState())

        if self.MainApp is not None:
            self.MainApp.CloseAllOutputs()

        self.ui.toConsole.detach()

        QT.QApplication.quit()

    def closeEvent(self, event):
        self.Exit()
        event.accept()

    def showEvent(self, event):
        super().showEvent(event)
        # The VTK interactor needs a realized Qt widget/window before native
        # initialization. Initializing during __init__ can make the VTK render
        # window appear as a detached top-level window instead of being embedded
        # in the 3D tab.
        QT.QTimer.singleShot(0, self.vtk.Initialize)

    def About(self):
        box =QT.QMessageBox(self)
        box.setWindowTitle("About OpenPisco")
        logo = QT.QtGui.QPixmap(ResourcePath()+'K0205_Logo_OpenPISCO_RVB.svg')
        box.setIconPixmap (logo.scaledToHeight(150))

        box.setText("""<a href="https://openpisco.irt-systemx.fr/"  >Web Page</a><br>
<a href="https://openpisco.readthedocs.io/" >Documentation</a><br>
<a href="https://gitlab.com/openpisco/openpisco/" >Sources</a><br>
<a href="https://anaconda.org/OpenPisco/"  >Anaconda</a><br>
<a href="https://gitlab.com/openpisco/openpisco/-/blob/master/COPYING"  >License</a><br>
""")
        box.show()

    def ResetIsoOrDensity(self):
        self.ui.ls_slider.setValue(50)

    def LoadInputData(self):
        from functools import partial
        self.setCurrentFile(None)
        string = str(self.ui.inputFileEditorTextEdit.toPlainText())
        self.SaveCopyToTemp(string)
        self.ResetApp()
        self.ui.inputFileEditorTextEdit.setPlainText(string)
        self.thread.SetFunctionToRunAndRun(partial(self.LoadInputFromString_fuction,string))

    def SaveCopyToTemp(self,data,ext=""):
        filename ="PiscoInputFile.back"+ext
        Info("Saving backup of the input data to "+  str(filename) )
        try :
            WriteTempFile(filename,data)
        except:
            Info("Error saving backup of input file")
            Info(data)

    def SaveInputData(self):
        string = str(self.ui.inputFileEditorTextEdit.toPlainText())

        modifiers = QT.QApplication.keyboardModifiers()

        newFile = True

        if modifiers == QT.Qt.ShiftModifier and self.currentFile is not None:
            newFile = False

        ext = self.MainApp.inputReader.ext
        self.SaveCopyToTemp(string,ext)

        if newFile:

            files_types = "OpenPisco Input Files (*"+str(ext)+") (*"+str(ext)+")"
            fname = None
            try:
                dialog = QT.QFileDialog(self)
                fname = dialog.getSaveFileName(None, 'Save file', '', files_types)
            except:
                print("Error saving to file")
                return

            ok = False

            if type(fname) == tuple:
                fname = fname[0]

            if len(fname)  == 0:
                print("Need a filename")
                return

            if len(fname) < len(ext) or fname[-len(ext):] != ext:
                fname += ext

            Info("Saving Input Data to "+  str(fname) )

            with open(fname, 'w') as f:
                f.write(string)
                f.close()
                ok = True

            if ok:
                self.setCurrentFile(fname)
            return

        else:
            fname = self.currentFile


        with open(fname, 'w') as f:
            f.write(string)
            f.close()

    def OpenFileDialog(self):
        import os
        dlg = QT.QFileDialog(self,filter="OpenPisco Input Files (*.xml);;OpenPisco Input Files (*.pisco);;OpenPisco Input Files (*.xml *.pisco);;OpenPisco App Files (*"+PISCOAPPEXTENSION+")")
        dlg.setFileMode(QT.QFileDialog.AnyFile)
#        dlg.setFilter()
        dlg.setDirectory(os.getcwd())
        dlg.setWindowTitle("Open File")
        if dlg.exec_() ==QT.QDialog.Accepted :

            selectedFiles  = dlg.selectedFiles()
            if len (selectedFiles) != 1:
                return
            fname = str(selectedFiles[0])
            #qfd.getOpenFileName( caption= 'Open file')
        else :
            return
        del dlg
        self.OpenFile( fname)

    def OpenFile(self, fname,callback=None):
        self.ResetApp()

        #fname =QT.QFileDialog.getOpenFileName(parent= self, caption= 'Open file', directory=os.getcwd(),filter="*.xml")

        import os
        cleanfilename, file_extension = os.path.splitext(fname)
        LV.AddToGlobalDictionary("INPUTFILE",os.path.basename(cleanfilename))

        from functools import partial

        self.setCurrentFile(fname)

        if file_extension == PISCOAPPEXTENSION:
            from OpenPisco.QtApp.PiscoAppReader import PiscoAppReader
            self.appReader = PiscoAppReader()
            try:
                string, widget = self.appReader.ReadFromFile(fname,self,self.MainApp)
            except Exception as inst:
                print("Error opeing file: "+fname)
                self.setCurrentFile("")
                print(inst)
                raise

            self.ui.controlPanelAppScrollArea.setWidget(widget)
            self.ui.inputFileEditorTextEdit.setPlainText(string)
            self.thread.SetFunctionToRunAndRun(partial(self.LoadInputFromString_fuction,string))

            def reloadData():
                string = str(self.ui.inputFileEditorTextEdit.toPlainText())
                self.SaveCopyToTemp(string)
                self.ResetMainApp()
                self.thread.SetFunctionToRunAndRun(partial(self.LoadInputFromString_fuction,string))

            widget.pushEnded.connect(reloadData )

            self.controlpanelWidgetCached = widget
            return

        self.WriteToCommandLog("mainApp.ReadFile('"+fname+"')\n" )

        if callback is not None:
            self.thread.SetFunctionToRunAndRun(partial(self.OpenFile_function,fname),callback=callback)
        else:
            self.thread.SetFunctionToRunAndRun(partial(self.OpenFile_function,fname))


    def UpdateApp(self):
        self.ui.updateGUI.emit()
        self.vtk.update3D.emit()

    def OpenRecentFile(self):
        action = self.sender()
        if action:
            fname = str(action.data())#.toPyObject()
            self.OpenFile(fname)

    def setCurrentFile(self, fileName):
        if fileName:
            self.setWindowTitle("%s - OpenPisco" % QT.QFileInfo(fileName).fileName() )
            self.currentFile = fileName
        else:
            self.setWindowTitle("OpenPisco")
            self.currentFile = None

        if fileName is None:
            return
        settings = QT.QSettings()
        files = settings.value('recentFileList')#.toPyObject()
        try:
            if files.count(fileName) :
                files.remove(fileName)
            files.insert(0, fileName)
            del files[SimpleView.MaxRecentFiles:]
        except :
            files = [fileName]

        settings.setValue('recentFileList', files)
        settings.sync()
        self.ui.UpdateRecentFileActions()

        _, file_extension = os.path.splitext(fileName)
        print(fileName)
        if file_extension != PISCOAPPEXTENSION and fileName != "":
            with open(fileName,"r",errors='ignore') as content:
                self.ui.inputFileEditorTextEdit.setPlainText(content.read())

    def LoadInputFromString_fuction(self,string):
    # this is a threated function no direct GUI calls!!
        self.printInStatusBar.emit('Loading input from editor')

        from OpenPisco.QtApp.ProxyWriter import ProxyWriter

        self.MainApp.useProxyWriter = partial(ProxyWriter,self)
        self.MainApp.closeOutputFilesIfSIGINT  = False
        if len(str(string).strip()):
            self.MainApp.ReadText(str(string))

        for OA in self.MainApp.optimAlgos:
             self.MainApp.optimAlgos[OA].beforeWritingCallBack =  self.vtk.update3D.emit

        self.ui.updateGUI.emit()

    def ReloadOpenFile(self):
        if self.currentFile is not None:
            self.OpenFile(self.currentFile)

    # this is a threated function no direct GUI calls!!
    def OpenFile_function(self,fname):
        if len(fname) > 0 :
            self.printInStatusBar.emit('Loading File : ' + fname)

            from OpenPisco.QtApp.ProxyWriter import ProxyWriter

            self.MainApp.useProxyWriter = partial(ProxyWriter,self)
            self.MainApp.closeOutputFilesIfSIGINT  = False
            self.MainApp.ReadFile(str(fname))


            for OA in self.MainApp.optimAlgos:
                 self.MainApp.optimAlgos[OA].beforeWritingCallBack =  self.vtk.update3D.emit

    def RunLoadedFile(self):
        self.thread.SetFunctionToRunAndRun (self.RunLoadedFile_function)
        self.WriteToCommandLog("mainApp.MainApp.Start()")

    def RunLoadedFile_function(self):
        if self.MainApp is not None:
            self.MainApp.beforeActionCallBack =  self.vtk.update3D.emit
            for OA in self.MainApp.optimAlgos:
                 self.MainApp.optimAlgos[OA].beforeWritingCallBack = self.vtk.update3D.emit
            self.MainApp.Start()
            #self.MainApp.CloseAllOutputs()
            #self.PlotLevelset()

            #self.MainApp.beforeActionCallBack = None
            #for OA in self.MainApp.optimAlgos:
            #     self.MainApp.optimAlgos[OA].beforeWritingCallBack = None

    def RunAction_Action(self):
        selectedText = str(self.ui.inputFileEditorTextEdit.textCursor().selectedText())

        dlg = QT.QInputDialog(self)
        dlg.setOption(QT.QInputDialog.InputDialogOption.UsePlainTextEditForTextInput)
        dlg.setWindowTitle('Please enter an action (in '+self.MainApp.inputReader.ext+' Format)' )
        dlg.setLabelText('Action (in '+self.MainApp.inputReader.ext+' Format) : ' )
        dlg.setTextValue(selectedText)
        dlg.setOkButtonText("Run Actions")
        dlg.resize(1000,300)
        ok = dlg.exec_()
        text = PH.ApplyGlobalDictionary(str(dlg.textValue()))

        if ok and len(text):
            Debug(text.encode("utf-8"))

            if self.MainApp.inputReader.ext == ".xml":
                text = "<Actions>"+text+"</Actions>"
            try:
                _, dic = self.MainApp.inputReader.ReadFromString(text.encode("utf-8", "ignore").decode() )
            except Exception as inst:
                print("Problem reading input")
                print(inst)
                return

            try:
                self.MainApp.inputReader.ReplaceObjectFromAlmanac(dic,self.MainApp)
            except Exception as inst:
                print("Problem reading input (interpretation)")
                print(inst)
                return

            self.thread.SetFunctionToRunAndRun(partial(self.RunAction_ActionAndPlot, dic) )

            self.WriteToCommandLog("mainApp.RunAction(ElementTree.fromstring('"+str(''.join(text.splitlines()) ).lstrip() +"') )")


    def WriteToCommandLog(self, data):
        if self.commandlogfile is not None:
            self.commandlogfile.write(data)
            self.commandlogfile.write("\n")
            self.commandlogfile.flush()

    def RunAction_ActionAndPlot(self,action):
        for tag, child in action['children']:
            self.MainApp.RunAction(child)

    def runOptim(self,numberofiter):
       IDD = self.ui.GetActiveOptimAlgo()
       if IDD == -1:
           return

       if numberofiter:
           self.MainApp.optimAlgos[IDD].numberOfDesignStepsMax = int(numberofiter)

       action = {'type': 'Repeate', 'times': '1', 'children': [('Action', {'type': 'ResetTopoOp', 'TopoOp': str(IDD)}), ('Action', {'type': 'RunTopoOp', 'TopoOp': str(IDD)})]}
       self.WriteToCommandLog("action = "+ str(action) )

       self.MainApp.inputReader.ReplaceObjectFromAlmanac(action,self.MainApp)
       self.WriteToCommandLog("mainApp.inputReader.ReplaceObjectFromAlmanac(action,self.main)")

       self.thread.SetFunctionToRunAndRun(functools.partial(self.MainApp.RunAction, action) )

       self.WriteToCommandLog("mainApp.RunAction(action)")

    def updatePhys(self):
       IDD = self.ui.GetActiveOptimAlgo()
       if IDD == -1:
           return

       optimalgo = self.MainApp.optimAlgos[IDD]
       optimalgo.UpdateOptimProblem()
       OF = optimalgo.optimProblem

       (newJ, newg, newh)  = optimalgo.GetValuesFromOptimProblem()
       newC = np.concatenate((newg, newh))
       newMerit = optimalgo.ComputeMeritFunction(newJ,newC)

       (DJ, Dg, Dh)        = optimalgo.GetSensitivitiesFromOptimProblem()
       (DJExt,DgExt,DhExt) = optimalgo.GetDirectionsFromGradients(DJ,Dg,Dh)

       OF.GetCurrentPoint().extraNodesFields['JDirection'] = DJExt

       optimalgo.ComputeGain(newMerit)
       optimalgo.PrintCurrentState(OF, newMerit)

       self.WriteToCommandLog("OF = self.MainApp.optimAlgos["+str(IDD)+"].optimProblem ")
       self.WriteToCommandLog("DJExt =  self.MainApp.optimAlgos["+str(IDD)+"].ComputeObjectiveAndGradient()[1] ")
       self.WriteToCommandLog("OF.GetCurrentPoint().extraNodesFields['JDirection'] = DJExt ")

       self.vtk.update3D.emit()

    def CreateActions(self)    :
        # icons from https://material.io/icons/
        openAction = QT.QAction(QT.QIcon(ResourcePath()+'ic_folder_open_black_24px.svg'), '&Open...', self)
        openAction.setShortcut('Ctrl+O')
        openAction.setStatusTip('Open a file from disk...')
        openAction.triggered.connect(self.OpenFileDialog )
        self.actions['open'] = openAction

        # icons from https://material.io/icons/
        reloadAction = QT.QAction(QT.QIcon(ResourcePath()+'ic_folder_reopen_black_24px.svg'),'&Reload From File', self)
        reloadAction.setStatusTip('Reload input file')
        reloadAction.triggered.connect(self.ReloadOpenFile)
        self.actions['reload'] = reloadAction


        # icons from https://material.io/icons/
        loadAction = QT.QAction(QT.QIcon(ResourcePath()+'Reload_From_Editor.svg'),'&Reload From Editor', self)
        loadAction.setStatusTip('Reload input data from editor')
        loadAction.triggered.connect(self.LoadInputData )
        self.actions['load'] = loadAction

        # icons from https://material.io/icons/
        saveAction = QT.QAction(QT.QIcon(ResourcePath()+'baseline-save_alt-24px.svg'),  '&Save Input Data', self)
        saveAction.setStatusTip('Save input data to file... (Shift+Click resave to the current file)')
        saveAction.triggered.connect(self.SaveInputData)
        self.actions['save'] = saveAction

        runAction = QT.QAction(QT.QIcon(ResourcePath()+'ic_play_circle_filled_black_24px.svg'), '&Run', self)
        runAction.setShortcut('Ctrl+R')
        runAction.setStatusTip('Run all actions')
        runAction.triggered.connect(self.RunLoadedFile)
        self.actions['run'] = runAction

        for i in range(SimpleView.MaxRecentFiles):
            self.recentFileActs.append(QT.QAction(self, visible=False,triggered=self.OpenRecentFile))

        updateAction = QT.QAction(QT.QIcon(ResourcePath()+'ic_directions_walk_black_24px.svg'), '&Update Physical Problem', self)
        updateAction.setStatusTip('Update criteria of the the optimization problem')
        updateAction.triggered.connect(self.updatePhys )
        self.actions['update_physical_problems'] = updateAction
        updateAction.setEnabled(False)

        runAction = QT.QAction(QT.QIcon(ResourcePath()+'ic_replay_1_black_24px.svg'), '&Run Optimization', self)
        runAction.setStatusTip('Run again the current optimization problem')
        runAction.triggered.connect(functools.partial(self.runOptim,int(0)) )
        self.actions['run_optim_again'] = runAction
        runAction.setEnabled(False)

        for i in [5,10,30,]:
            runAction = QT.QAction(QT.QIcon(ResourcePath()+'ic_replay_'+str(i)+'_black_24px.svg'), '&Run '+str(i)+' Optimization Iterations', self)
            runAction.setStatusTip('Run '+str(i)+' iterations of current optimization problem')
            runAction.triggered.connect(functools.partial(self.runOptim,int(i)) )
            self.actions['run_optim_'+str(i) ] = runAction
            runAction.setEnabled(False)

        ResetIsoOrDensityAction =QT.QAction(QT.QIcon(ResourcePath()+'ic_vertical_align_center_black_24px.svg'), '&Reset Iso/Density View', self)
        ResetIsoOrDensityAction.setShortcut('Ctrl+Shift+R')
        ResetIsoOrDensityAction.setStatusTip('Reset iso/density view')
        ResetIsoOrDensityAction.triggered.connect(self.ResetIsoOrDensity )
        self.actions['set 0.0 Ls or 0.5 density'] = ResetIsoOrDensityAction

        updateGUIAction =QT.QAction(QT.QIcon(ResourcePath()+'Update_GUI.svg'),'&Update GUI', self)
        #exitAction.setShortcut('')
        updateGUIAction.setStatusTip('Update GUI from internal objects')
        updateGUIAction.triggered.connect(self.ui.updateGUI)
        self.actions['updateGUI'] = updateGUIAction

        exitAction =QT.QAction(QT.QIcon(ResourcePath()+'ic_exit_to_app_black_24px.svg'), '&Exit', self)
        exitAction.setShortcut('Ctrl+Q')
        exitAction.setStatusTip('Exit application')
        exitAction.triggered.connect(self.Exit)
        self.actions['exit'] = exitAction

        runAction =QT.QAction(QT.QIcon(ResourcePath()+'ic_play_arrow_black_24px.svg'), '&Run Action...', self)
        #exitAction.setShortcut('Ctrl+Shift+Q')
        runAction.setStatusTip('Run an action in XML format')
        runAction.triggered.connect(self.RunAction_Action)
        self.actions['run_action'] = runAction

        refreshAction =QT.QAction(QT.QIcon(ResourcePath()+'ic_cached_black_24px.svg'), '&Refresh 3D View', self)
        refreshAction.setStatusTip('Refresh the 3D view')
        refreshAction.triggered.connect(self.vtk.update3D.emit)
        self.actions['refresh_action'] = refreshAction

        icon = QT.QIcon()
        icon.addFile(ResourcePath()+'view_levelset.svg', state =QT.stateOn)
        icon.addFile(ResourcePath()+'view_mesh_inside_outside.svg', state = QT.stateOff)

        viewLsOrMeshAction =QT.QAction(icon, '&Use Levelset Or Mesh To Cut The Domain', self,checkable=True)
        viewLsOrMeshAction.setStatusTip('Use the levelset or the mesh tags to cut the domain')
        viewLsOrMeshAction.triggered.connect(self.SetViewLevelset  )
        self.actions['viewLsOrMesh_action'] = viewLsOrMeshAction

        viewZonesAction =QT.QAction(QT.QIcon(ResourcePath()+'baseline-visibility-24px.svg'), '&Draw Zones', self,checkable=True)
        viewZonesAction.setStatusTip('Draw a graphical representation of the zones')
        viewZonesAction.triggered.connect(self.ShowHideZones  )
        self.actions['viewZones_action'] = viewZonesAction

        viewNextFieldAction =QT.QAction(QT.QIcon(ResourcePath()+'baseline-layers-24px.svg'), '&View Next Field', self)
        viewNextFieldAction.setShortcut('a')
        viewNextFieldAction.setStatusTip('View next field')
        viewNextFieldAction.triggered.connect(self.ViewNextField)
        self.actions['nextfield_action'] = viewNextFieldAction

        updatedistanceAction =QT.QAction(QT.QIcon(ResourcePath()+'ic_autorenew_black_24px.svg'), '&Update Distance', self)
        updatedistanceAction.setStatusTip('Update distance')
        updatedistanceAction.triggered.connect(self.UpdateDistance)
        self.actions['updatedistance_action'] = updatedistanceAction

        remeshAction =QT.QAction('&Remesh ', self)
        remeshAction.setStatusTip('Remesh current levelset')
        remeshAction.triggered.connect(self.Remesh)
        self.actions['remesh_action'] = remeshAction

        debug_clean_log_action =QT.QAction(QT.QIcon(ResourcePath()+'ic_clear_black_24px.svg'), '&Clean Log', self)
        debug_clean_log_action.setStatusTip('Clean log')
        debug_clean_log_action.triggered.connect(self.CleanLogAction)
        self.actions['debug_clean_log_action'] = debug_clean_log_action

        about_action =QT.QAction('&About OpenPisco', self)
        about_action.setStatusTip('About OpenPisco and external links')
        about_action.triggered.connect(self.About)
        self.actions['about_action'] = about_action

        openinparaview_action =QT.QAction(QT.QIcon(ResourcePath()+'To-ParaView.svg')    ,'&Open in ParaView', self)
        openinparaview_action.setStatusTip('Open current fields in ParaView (Shift+Click only re-exports the file)')
        openinparaview_action.triggered.connect(self.OpenInParaView)
        self.actions['openinparaview_action'] = openinparaview_action

        exportSTL_action =QT.QAction(QT.QIcon(ResourcePath()+'To-STL.svg'), '&Export STL', self)
        exportSTL_action.setStatusTip('Save current shape to STL')
        exportSTL_action.triggered.connect(self.ExportStl)
        self.actions['exportSTL_action'] = exportSTL_action


        if self.with_demo :
            from OpenPisco.Demos import GetDemoPath
            from os import listdir
            from os.path import isfile, join, isdir,islink
            self.demofiles = []

            def AddFilesRecursive(path,menu):
                def treatEntries(entries):
                    for entry in entries:
                        fullpath = join( path, entry)
                        if len(entry) > 1 and entry[0:2] == "__":
                            continue
                        if isfile(fullpath) and islink(fullpath) == False and any(entry.endswith(x) for x in [".xml", ".pisco", PISCOAPPEXTENSION] ):
                            CreateFileToMenu(entry,fullpath,menu)
                        if isdir(fullpath) and islink(fullpath) == False :
                            res = []
                            AddFilesRecursive(fullpath,res)
                            if len(res):
                                menu.append((entry,res))

                ff = ["NonConform","Conform","Zset" ]
                entries = sorted(listdir(path))

                fflist = []
                for f in ff:
                    data = list(filter(lambda x,analysisType=f: x.find(analysisType) >= 0, entries))
                    fflist.append(data)
                    entries = list(filter(lambda x,analysisType=f: x.find(analysisType) < 0, entries))

                fflist.append(entries)
                fflist.reverse()
                for f in fflist :
                    treatEntries(f)
                    menu.append(None)

            def CreateFileToMenu(filename,fullpath,menu):
                act = QT.QAction(self, visible=False,triggered=self.OpenRecentFile)
                act.setText(filename)
                act.setData(fullpath)
                act.setVisible(True)
                act.setStatusTip("Open demo file: " + str(fullpath) )
                menu.append(act)

            AddFilesRecursive(GetDemoPath(),self.demofiles)

        printLsStats_action = QT.QAction(QT.QIcon(ResourcePath()+'stats.svg'),'&LS Statistics', self)
        printLsStats_action.setStatusTip('Print levelset statistics')
        printLsStats_action.triggered.connect(self.PrintLsStatsAction)
        self.actions['printLsStats'] = printLsStats_action

        wireframeview_action = QT.QAction(QT.QIcon(ResourcePath()+'baseline-business-wireframe-24px.svg'),'&Wireframe View', self)
        wireframeview_action.setShortcut('w')
        wireframeview_action.setStatusTip('Wireframe view')
        wireframeview_action.triggered.connect(self.SetWireframeViewAction)
        self.actions['wireframeview'] = wireframeview_action

        solidview_action = QT.QAction(QT.QIcon(ResourcePath()+'baseline-business-full-24px.svg'),'&Solid View', self)
        solidview_action.setShortcut('s')
        solidview_action.setStatusTip('Solid view')
        solidview_action.triggered.connect(self.SetSolidViewAction)
        self.actions['solidview'] = solidview_action

        resetcamera3dview_action = QT.QAction(QT.QIcon(ResourcePath()+'zoom_out_map-24px.svg'),'&Reset Camera', self)
        resetcamera3dview_action.setShortcut('r')
        resetcamera3dview_action.setStatusTip('Reset the camera view in the 3D view')
        resetcamera3dview_action.triggered.connect(self.ResetCamera)
        self.actions['resetcamera3dview'] = resetcamera3dview_action

        addEdit_action =QT.QAction(QT.QIcon(ResourcePath()+'baseline-create-plus-24px.svg'),'&Interactive Dilate Levelset', self,checkable=True)
        addEdit_action.setShortcut('+')
        addEdit_action.setStatusTip('Edit: Interactive dilatation levelset using the pointer')
        addEdit_action.triggered.connect(self.AddEditAction)
        self.actions['addedit'] = addEdit_action

        substractEdit_action =QT.QAction(QT.QIcon(ResourcePath()+'baseline-create-minus-24px.svg'),'&Interactive Erode Levelset', self,checkable=True)
        substractEdit_action.setShortcut('-')
        substractEdit_action.setStatusTip('Edit: Interactive erodation levelset using the pointer')
        substractEdit_action.triggered.connect(self.SubstractEditAction)
        self.actions['substractedit'] = substractEdit_action


        moveLSp_action =QT.QAction(QT.QIcon(ResourcePath()+'dilate.svg'),'&Dilate Levelset', self)
        moveLSp_action.setStatusTip('Dilate levelset')
        moveLSp_action.triggered.connect(partial(self.MoveLevelset,"+"))
        self.actions['MoveLS+'] = moveLSp_action

        moveLSm_action =QT.QAction(QT.QIcon(ResourcePath()+'erode.svg'),'&Erode Levelset', self)
        moveLSm_action.setStatusTip('Erode Levelset')
        moveLSm_action.triggered.connect(partial(self.MoveLevelset,"-"))
        self.actions['MoveLS-'] = moveLSm_action

        moveLSd_action =QT.QAction(QT.QIcon(ResourcePath()+'ls_move_direction.svg'),'&Advance Levelset', self)
        moveLSd_action.setStatusTip('Update levelset using the "direction" field ')
        moveLSd_action.triggered.connect(partial(self.MoveLevelset,"d"))
        self.actions['MoveLSd'] = moveLSd_action

    def SetViewLevelset(self,data):
        self.ui.viewLevelset = not data
        self.ui.ls_slider.setEnabled(self.ui.viewLevelset)
        self.ui.updateGUI.emit()
        self.vtk.update3D.emit()

    def MoveLevelset(self,direction):
        from OpenPisco.Actions.FieldsActions import LevelSetOffsetAction

        IDD,ls = self.GetActiveLevelset()
        if IDD == -1 or ls is None:
            self.statusBar().showMessage("No levelset to work on ")
            return

        boundingMin , boundingMax = ls.support.ComputeBoundingBox()
        length = np.linalg.norm(boundingMax -boundingMin)/self.erodeDilateScale

        self.statusBar().showMessage("Operation " + str(direction))
        if direction == "+" or direction == "-":
            if direction == "+":
                length *= -1

            act = LevelSetOffsetAction()
            act.Apply(self.MainApp, {"ls":ls, "val":length})

            self.WriteToCommandLog("from OpenPisco.Actions.FieldsActions import LevelSetOffset")
            self.WriteToCommandLog("act = LevelSetOffsetAction()")
            self.WriteToCommandLog("act.Apply(mainApp, {'ls':" + str(IDD) + ",'val':"+str(length)+"} )" )

        if direction == "d" and "Direction" in ls.extraNodesFields:
            direction = ls.extraNodesFields["Direction"]
            ls.TransportAndReinitialize(direction,length)

        self.vtk.update3D.emit()

    def ViewNextFieldAction(self):
        self.colormapBy = self.colormapBy+1
        self.vtk.update3D.emit()

    def PrintLsStatsAction(self):
        IDD,ls = self.GetActiveLevelset()
        if IDD == -1 or ls is None:
            return
        from OpenPisco.Actions.FieldsActions import ComputeLevelsetStatistics
        ComputeLevelsetStatistics().Apply(self.MainApp, {"ls":ls, "label":IDD})

    def SetWireframeViewAction(self):
        self.vtk.SetViewType("w")
        self.vtk.update3D.emit()

    def SetSolidViewAction(self):
        self.vtk.SetViewType("s")
        self.vtk.update3D.emit()

    def ResetCamera(self):
        self.vtk.ren.ResetCamera()

    def AddEditAction(self,val):
        if val :
            self.vtk.style.mod = "add"
            self.actions['substractedit'].setChecked(False)
        else:
            self.vtk.style.mod = "none"

    def SubstractEditAction(self,val):
        if val:
            self.vtk.style.mod = "minus"
            self.actions['addedit'].setChecked(False)
        else:
            self.vtk.style.mod = "none"

    def OpenInParaView(self):
        from Muscat.Actions.OpenInParaView import OpenInParaView

        IDD,ls = self.GetActiveLevelset()
        if IDD == -1 or ls is None:
            OpenInParaView()
            return
        tempdir = TemporaryDirectory.GetTempPath()
        from Muscat.IO.XdmfWriter import WriteMeshToXdmf
        filename =tempdir+"PiscoSolution"+str(IDD)+".xmf"
        tmpdic = ls.extraNodesFields.copy()
        tmpdic['phi'] = ls.phi.flatten()
        FieldsNames = list(tmpdic.keys())
        Fields =  list(tmpdic.values())

        if ls.IsNodal():

            WriteMeshToXdmf(filename,ls.support,PointFieldsNames=FieldsNames,PointFields=Fields  )
        else:

            from Muscat.IO.XdmfWriter import XdmfWriter

            if ls.support.props.get("IsConstantRectilinear",False) :
                WriteMeshToXdmf(filename,ls.support,CellFieldsNames=FieldsNames,CellFields=Fields  )
            else:
                from Muscat.MeshTools.MeshInspectionTools import ExtractElementsByElementFilter
                writer = XdmfWriter()
                writer.SetBinary(True)
                writer.SetMultidomain()
                writer.Open(filename=filename)
                support = ExtractElementsByElementFilter(ls.support,elementFilter=ElementFilter(dimensionality= ls.support.GetElementsDimensionality() ) )

                writer.Write(support,CellFieldsNames=FieldsNames,CellFields=Fields  )
                support = ExtractElementsByElementFilter(ls.support,elementFilter= ComplementaryFilter(ElementFilter(dimensionality= ls.support.GetElementsDimensionality() ) ) )

                if support.GetNumberOfElements() > 0 :
                    writer.Write(support)

                writer.Close()

        modifiers =QT.QApplication.keyboardModifiers()
        if modifiers == QT.Qt.ShiftModifier:
            print('Shift+Click : only saving data to the same file ')
            return

        OpenInParaView(filename = filename)

    def ExportStl(self):

        data = self.vtk.GetPlotedSurfaceData()
        if data is None:
            return

        from Muscat.IO.StlWriter import WriteMeshToStl
        from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
        fname = "OutputShape.stl"
        fpath = TemporaryDirectory.GetTempPath() +os.sep +  fname
        WriteMeshToStl(fpath,data)
        import shutil

        if self.inYarss:
            shutil.move(fpath,exportSTLPath+fname)
        else:
            print('STL file in ' + fpath)

    def UpdateDistance(self):
        Debug("Update distance" )

        IDD,ls = self.GetActiveLevelset()
        if IDD == -1:
            return
        if self.MainApp is None:
            return
        Debug(IDD)

        self.WriteToCommandLog("from OpenPisco.Actions.FieldsActions import UpdateDistance")
        self.WriteToCommandLog("act = UpdateDistance()")
        self.WriteToCommandLog("act.Apply(mainApp, {'ls':" + str(IDD) + "} )" )
        from OpenPisco.Actions.FieldsActions import UpdateDistance
        act = UpdateDistance()
        act.Apply(self.MainApp, {"ls":ls})

        self.vtk.update3D.emit()

    def Remesh(self):
        Debug("Remesh" )

        IDD,ls = self.GetActiveLevelset()
        if IDD == -1:
            return
        if self.MainApp is None:
            return
        Debug(IDD)

        self.WriteToCommandLog("from OpenPisco.Actions.UnstructuredActions import ActionRemesh")
        self.WriteToCommandLog("act = ActionRemesh()")
        self.WriteToCommandLog("act.Apply(mainApp, {'ls':" + str(IDD) + ",'iso':0.0 } )" )
        from OpenPisco.Actions.UnstructuredActions import ActionRemesh
        act = ActionRemesh()
        act.Apply(self.MainApp, {"ls":ls, 'iso': 0.0})
        self.vtk.update3D.emit()

        f'<Action type="Remesh" ls="{IDD}" iso="0.0" />'


    def ShowHideZones(self):
        if self.actions["viewZones_action"].isChecked():
            self.vtk.plotZones = True
        else:
            self.vtk.plotZones = False

        self.vtk.update3D.emit()

    def CleanLogAction(self):
        self.ui.toConsole.Clean()

    def CleanName(self, dirtyName):
        return re.sub(r"(\w)([A-Z])", r"\1 \2", dirtyName).title()

    def AddSphere(self,pos,mod):
        IDD,ls =  self.GetActiveLevelset()
        if IDD == -1:
            return

        if ls.support.props.get("IsConstantRectilinear",False):
            dxyz = ls.support.props.get("spacing")*ls.support.props.get("dimensions")
        else:
            boundingMin, boundingMax = ls.support.ComputeBoundingBox()
            dxyz = (boundingMax - boundingMin)

        l = np.linalg.norm(dxyz)/self.interactiveSphereRadius
        self.WriteToCommandLog("l = "+str(l))

        self.WriteToCommandLog("ls = mainApp.levelSets[" +str(int(IDD))+"]")
        self.WriteToCommandLog("l = " + str(l))
        self.WriteToCommandLog("pos = " + str(pos))

        if ls.IsNodal():
            isElementfield =  False
        else:
            isElementfield =  True

        if mod == "add":
            sp = (np.exp(-0.5*((IGObs.ImplicitGeometrySphere(radius=l, center=pos).ApplyVector(ls.support,cellCenter=isElementfield) +l)/l)**2)-0.001)*l

            np.clip(sp,0,l,out=sp)
            sp *= (-1)**isElementfield
            ls.phi[:] = ls.phi - sp
            self.WriteToCommandLog("sp = (np.exp(-0.5*((IGObs.ImplicitGeometrySphere(radius=l, center=pos).ApplyVector(ls.support,,cellCenter="+str(isElementfield)+") +l)/l)**2)-0.001)*l")
            self.WriteToCommandLog("np.clip(sp,0,l,out=sp)")
            self.WriteToCommandLog("ls.phi[:] = ls.phi - sp")

        elif mod == "explore" :
            self.statusBar().showMessage('Pos : ' + str(pos))
            return

        elif mod == "minus" :
            sp = (np.exp(-0.5*((IGObs.ImplicitGeometrySphere(radius=l, center=pos).ApplyVector(ls.support,cellCenter=isElementfield) +l)/l)**2)-0.001)*l

            np.clip(sp,0,l,out=sp)
            sp *= (-1)**isElementfield
            ls.phi[:] = ls.phi + sp
            self.WriteToCommandLog("sp = (np.exp(-0.5*((IGObs.ImplicitGeometrySphere(radius=l, center=pos).ApplyVector(ls.support,cellCenter="+str(isElementfield)+") +l)/l)**2)-0.001)*l")
            self.WriteToCommandLog("np.clip(sp,0,l,out=sp)")
            self.WriteToCommandLog("ls.phi[:] = ls.phi + sp")
        elif mod == "smooth":
            sp = (np.exp(-0.5*((IGObs.ImplicitGeometrySphere(radius=l, center=pos).ApplyVector(ls.support,cellCenter=isElementfield) +l)/l)**2)-0.001)*0.1
            np.clip(sp,0,0.1,out=sp)
            ssp = np.sum(sp)
            if ssp != 0 :
                m = np.sum(ls.phi*sp)/ssp
                ls.phi[:] = m*sp+ls.phi*(1-sp)

        if isElementfield:
            np.clip(ls.phi,a_min=0.0,a_max=1.0,out=ls.phi)
            self.WriteToCommandLog("np.clip(ls.phi,a_min=0.0,a_max=1.0,outls.phi)")

        self.vtk.update3D.emit()
