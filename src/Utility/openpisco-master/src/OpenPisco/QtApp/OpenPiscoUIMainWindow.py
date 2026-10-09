# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import re
import functools
from functools import partial

from Muscat.Helpers.Logger import IsVerboseMuscatOutput, VerboseMuscatOutput, VerboseMuscatOutput, DefaultMuscatOutput
from Muscat.ImplicitGeometry.ImplicitGeometryBase import ImplicitGeometryBase
from Muscat.MeshContainers.Mesh import Mesh
from Muscat.IO.WriterBase import WriterBase

import OpenPisco.CLApp.OpenPiscoCL as OpenPiscoCL
import OpenPisco.QtApp.QtImplementation as QT
from OpenPisco.QtApp.res import ResourcePath
from OpenPisco.LevelSetBase import LevelSetBase
from OpenPisco.PhysicalSolvers.SolverBase import SolverBase
from OpenPisco.Optim.Problems.OptimProblemBase import OptimProblemBase
from OpenPisco.Optim.Algorithms.OptimAlgoBase import OptimAlgoBase

ListOfTypesToDevelop = (Mesh,LevelSetBase, SolverBase, ImplicitGeometryBase, OptimProblemBase, WriterBase, OptimAlgoBase)

class UiMainWindow(QT.QObject):
    updateGUI = QT.Signal()
    update2D = QT.Signal()
    MaxRecentFiles = 5

    def __init__(self, parent=None):
        QT.QObject.__init__(self, parent)
        self.viewLevelset = True
        self.DebugCheckBoxWidget =  None

    def setupUi(self, MainWindow):
        MainWindow.setWindowTitle("OpenPisco")
        MainWindow.resize(603, 553)

        self.centralWidget = QT.QTabWidget(MainWindow)
        self.tab3D = QT.QWidget(self.centralWidget)
        self.centralWidget.addTab(self.tab3D,"3D View" )

        # to activate the 2D view
        from OpenPisco.QtApp.RenderZone2D import RenderZone2D
        self.tab2D = RenderZone2D(self.centralWidget)
        self.centralWidget.addTab(self.tab2D,"2D Graph" )
        self.update2D.connect(self.UpdatePlot2D_slot )

        self.gridlayoutTag3D = QT.QGridLayout(self.tab3D)
        #self.gridlayoutTag3D.setContentsMargins(0, 0, 0, 0)

        MainWindow.setCentralWidget(self.centralWidget)

        self.CreateToolBar(MainWindow)
        self.CreateMenuBar(MainWindow)
        self.CreateDocks(MainWindow)

        self.updateGUI.connect(partial(self.UpdateGUI_slot,MainWindow) )
        self.UpdateGUI_slot(MainWindow)

        self.CancelButton = QT.QPushButton(QT.QIcon(ResourcePath()+'cancel-24px.svg'),'')
        self.CancelButton.setStatusTip('Interrupt Runing Action')

        MainWindow.statusBar().addPermanentWidget(self.CancelButton)
        MainWindow.statusBar().setSizeGripEnabled(False)

        def setNeedToCancel():
            from OpenPisco.Actions.ActionBase import ActionBase
            ActionBase.SetNeedToInterrupt()

        self.CancelButton.clicked.connect(setNeedToCancel)

    def Plot2D(self,names,data):
        if self.tab2D is None:
            return
        self.tab2D.Write({n:d for n,d in zip(names,data)})
        self.update2D.emit()

    def UpdatePlot2D_slot(self):
        if self.tab2D is None:
            return

        self.tab2D.Update()

    def UpdateGUI_slot(self,MainWindow):
        self.UpdateLevelSetComboBox(MainWindow)
        self.UpdateFieldsToPlotComboBox(MainWindow)
        self.UpdateOptimComboBox(MainWindow)
        self.UpdateMenuObjects(MainWindow)
        MainWindow.statusBar().showMessage('Ready')
        self.DebugCheckBoxWidget.setChecked(IsVerboseMuscatOutput())

    def UpdateMenuObjects(self,MainWindow):

        members = [attr for attr in MainWindow.MainApp.__dict__ if not callable(attr) and not attr.startswith("__") and type(MainWindow.MainApp.__dict__[attr]) is dict   ]
        if 'actions' in members:
            members.remove('actions')

        self.objectsMenu.clear()


        for member in members:
            cleanName = re.sub(r"(\w)([A-Z])", r"\1 \2", member).title()

            insidemenu =  self.objectsMenu.addMenu('&'+cleanName)
            for idd,obj in MainWindow.MainApp.__dict__[member].items():
                self.AddEditionMenu(MainWindow.MainApp,insidemenu, obj, cleanName,idd)

        self.AddEditionMenu(MainWindow.MainApp,self.objectsMenu.addMenu("&Self"), MainWindow, "Self",-1)

    def CreateHelpMenu(self,helpMenu,MainWindow):
        a = OpenPiscoCL.MainApp()

        members = [attr for attr in a.__dict__ if not callable(attr) and not attr.startswith("__") and type(a.__dict__[attr]) is dict   ]
        members.append("criteria")
        members.append("readers")

        for member in members:
            cleanName = re.sub(r"(\w)([A-Z])", r"\1 \2", member).title()
            #insidemenu =  helpMenu.addMenu('&'+cleanName)
            self.AddHelpFor(cleanName,helpMenu)

    def AddHelpFor(self,name,menu):
        theAction = QT.QAction("Help for " + name + " ..." , self)
        theAction.setStatusTip("Help for " + name + " ...")
        if name == "Zones":
            from Muscat.ImplicitGeometry.ImplicitGeometryFactory import ImplicitGeometryFactory,InitAllImplicitGeometry
            InitAllImplicitGeometry()
            PrintAvailable = ImplicitGeometryFactory.PrintAvailable
        elif name == "Grids":
            return
        elif name == "Level Sets":
            from OpenPisco.ConceptionFieldFactory import ConceptionFieldFactory,InitAllConceptionField
            InitAllConceptionField()
            PrintAvailable = ConceptionFieldFactory.PrintAvailable
        elif name == "Physical Problems":
            from OpenPisco.PhysicalSolvers.PhysicalSolverFactory import PhysicalSolverFactory,InitAllPhysicalSolvers
            InitAllPhysicalSolvers()
            PrintAvailable = PhysicalSolverFactory.PrintAvailable
        elif name == "Optim Algos":
            from OpenPisco.Optim.Algorithms.OptimAlgoFactory import OptimAlgoFactory, InitAllAlgos
            InitAllAlgos()
            PrintAvailable = OptimAlgoFactory.PrintAvailable
        elif name == "Optim Problems":
            from OpenPisco.Optim.Problems.ProblemFactory import ProblemFactory, InitAllProblems
            InitAllProblems()
            PrintAvailable = ProblemFactory.PrintAvailable
        elif name == "Outputs":
            from Muscat.IO.IOFactory import WriterFactory,InitAllWriters
            InitAllWriters()
            PrintAvailable = WriterFactory.PrintAvailable
        elif name == "Readers":
            from Muscat.IO.IOFactory import ReaderFactory,InitAllReaders
            InitAllReaders()
            PrintAvailable = ReaderFactory.PrintAvailable
        elif name == "Actions":
            from OpenPisco.Actions.ActionFactory import ActionFactory,InitAllActions
            InitAllActions()
            PrintAvailable = ActionFactory.PrintAvailable
        elif name == "Criteria":
            from OpenPisco.Optim.Criteria.CriteriaFactory import CriteriaFactory, InitAllCriteria
            InitAllCriteria()
            PrintAvailable = CriteriaFactory.PrintAvailable
        else:
            return

        def Print():
            print("Documentation for " + name + ":")
            PrintAvailable()

        theAction.triggered.connect(Print )
        menu.addAction(theAction)

    def AddEditionMenu(self,MainApp, menu, obj, cleanName,i):

        hasSubClasses = False

        if type(obj) == list :
            return
        for name,value in obj.__dict__.items() :
            if issubclass(type(value),ListOfTypesToDevelop) and len(name) >=1 and name[0] != '_':
                hasSubClasses = True
                break

        if hasSubClasses :
            menuII = menu.addMenu(cleanName +" "+ (str(i)if i > -1 else "")  + " ...")
            menu.addMenu(menuII)
            menu = menuII
            theAction = QT.QAction( "&Edit", self)
        else:
            theAction = QT.QAction("Edit " + cleanName +" "+ (str(i)if i > -1 else "") + " ...", self)


        theAction.setStatusTip('Modified the ' +cleanName)
        theAction.triggered.connect(functools.partial(self.OpenObjectEditor,MainApp ,obj,cleanName,i) )
        menu.addAction(theAction)

        if hasSubClasses:
            separator = QT.QAction( "", self)
            separator.setSeparator(True)
            menu.addAction(separator)
            for name,value in obj.__dict__.items() :
                if issubclass(type(value),ListOfTypesToDevelop) and len(name) >=1 and name[0] != '_':
                    self.AddEditionMenu(MainApp, menu, value, name,-1)


    def OpenObjectEditor(self,MainApp,obj,cleanName, idd):

        if MainApp is None:
            return

        #to find the id of the object
        if idd == -1 :
            members = [attr for attr in MainApp.__dict__ if not callable(attr) and not attr.startswith("__") and type(MainApp.__dict__[attr]) is dict   ]

            for member in members:
                for iddd,value in MainApp.__dict__[member].items():
                    if value is obj:
                        idd = iddd
                        break
                if idd > -1 :
                    break

        from OpenPisco.QtApp.QtObjectEditor import QtObjectEditor
        self.w = QtObjectEditor(obj,idd, cleanName,father=self.ObjectInspectorEditorScrollArea)
        self.ObjectInspectorEditorScrollArea.setWidget(self.w)

    def UpdateOptimComboBox(self,MainWindow) :
        if MainWindow.MainApp is None:
            return
        self.optim_combo.blockSignals(True)
        self.optim_combo.clear()
        names = []
        for i in MainWindow.MainApp.optimAlgos:
            name = 'Optim : ' + str(i)
            names.append('Optim : ' + str(i) )
            #self.optim_combo.insertItems(1,names,i)
            self.optim_combo.insertItem(i,name,i)

        self.optim_combo.blockSignals(False)

        active = False
        if len(MainWindow.MainApp.optimAlgos):
            active = True

        MainWindow.actions['update_physical_problems'].setEnabled(active)

        MainWindow.actions['run_optim_again'].setEnabled(active)
        for i in [5,10,30]:
            MainWindow.actions['run_optim_'+str(i) ].setEnabled(active)

    def UpdateLevelSetComboBox(self,MainWindow) :
        self.field_combo.blockSignals(True)
        self.field_combo.clear()
        if MainWindow.MainApp is not None:
            for i in MainWindow.MainApp.levelSets:
                name = 'LS : ' + str(i)
                self.field_combo.insertItem(i,name,i)
            self.field_combo.blockSignals(False)

    def AddToFileGUI(self,MainWindow,obj):
        listToAdd = ['open','reload','load','save','openinparaview_action','exportSTL_action']
        for act in listToAdd:
            obj.addAction(MainWindow.actions[act] )


    def AddToRunGUI(self,MainWindow,obj, onToolbar=False):
        listToAddA = ['run','run_action',]

        for act in listToAddA:
            obj.addAction(MainWindow.actions[act] )

        if onToolbar:
            self.optim_combo=QT.QComboBox()
            obj.addWidget(self.optim_combo)
            obj.setObjectName("runtoolbar")

        listToAddB = ['update_physical_problems',
                     'run_optim_again','run_optim_5','run_optim_10',
                     'run_optim_30']

        for act in listToAddB:
            obj.addAction(MainWindow.actions[act] )

    def AddToViewGUI(self,MainWindow,obj):
        listToAdd = [
                     'viewLsOrMesh_action',
                     'nextfield_action',
                     'resetcamera3dview',
                     'viewZones_action',
                     'printLsStats',
                     'wireframeview',
                     'solidview',
                     'debug_clean_log_action',

                     'refresh_action',
                     'updateGUI']

        for act in listToAdd:
            obj.addAction(MainWindow.actions[act] )

    def AddToEditGUI(self,MainWindow,obj):
        listToAdd = ['updatedistance_action',
                     'remesh_action',
                     'MoveLS+',
                     'MoveLS-',
                     'MoveLSd',
                     'addedit',
                     'substractedit',
                     #'endedit'
                     ]

        for act in listToAdd:
            obj.addAction(MainWindow.actions[act] )

    def CreateMenuBar(self,MainWindow):
        #self.statusBar()
        menubar = MainWindow.menuBar()

        fileMenu = menubar.addMenu('&File')
        self.AddToFileGUI(MainWindow,fileMenu)

        self.separatorAct = fileMenu.addSeparator()
        for i in range(MainWindow.MaxRecentFiles):
            fileMenu.addAction(MainWindow.recentFileActs[i])

        fileMenu.addSeparator()
        fileMenu.addAction(MainWindow.actions['exit'])

        self.UpdateRecentFileActions()

        runMenu = menubar.addMenu('&Run')
        self.AddToRunGUI(MainWindow,runMenu)

        viewMenu = menubar.addMenu('&View')
        self.AddToViewGUI(MainWindow,viewMenu)

        editMenu = menubar.addMenu('&Edit')
        self.AddToEditGUI(MainWindow,editMenu)

        self.objectsMenu = menubar.addMenu('&Objects')

        if self.parent().with_demo:
            self.CreateDemoMenubar(MainWindow)

        fileMenu = menubar.addMenu('&Help')
        self.CreateHelpMenu(fileMenu,MainWindow)
        fileMenu.addAction(MainWindow.actions['about_action'] )

    def CreateDemoMenubar(self,MainWindow):

        demoMenu = MainWindow.menuBar().addMenu('&Demo')

        demofiles =self.parent().demofiles

        def fillmenuR(menu,data):
            for i in data:
                if type(i) == tuple:
                    submenu = menu.addMenu(i[0])
                    fillmenuR(submenu,i[1])
                elif i is None:
                    menu.addSeparator()
                else:
                    menu.addAction(i)

        fillmenuR(demoMenu,demofiles)

    def CreateToolBar(self,MainWindow):
        ### file Toolbar
        self.filetoolbar = MainWindow.addToolBar('File')
        self.filetoolbar.setObjectName("filetoolbar")
        self.AddToFileGUI(MainWindow,self.filetoolbar)

        ### run Toolbar
        self.runtoolbar = MainWindow.addToolBar('Run')
        self.AddToRunGUI(MainWindow, self.runtoolbar, True)

        ### run Viewbar
        self.viewtoolbar = MainWindow.addToolBar('View')
        self.viewtoolbar.setObjectName("viewtoolbar")


        self.field_combo = QT.QComboBox()
        self.viewtoolbar.addWidget(self.field_combo)
        def field_combo_on_click(x):
            nonlocal MainWindow
            self.UpdateFieldsToPlotComboBox(MainWindow)
            MainWindow.vtk.update3D.emit()
        self.field_combo.currentIndexChanged.connect(field_combo_on_click)

        self.fieldToPlot_combo = QT.QComboBox()
        self.viewtoolbar.addWidget(self.fieldToPlot_combo)
        self.fieldToPlot_combo.setStyleSheet('''QComboBox { min-width: 75px;}''')
        self.fieldToPlot_combo.currentIndexChanged.connect(lambda x: MainWindow.vtk.update3D.emit())

        self.combo_inout = QT.QComboBox()
        self.combo_inout.insertItem(1,"Inside",-1)
        self.combo_inout.insertItem(2,"Interface",0)
        self.combo_inout.insertItem(3,"Outside",1)
        self.combo_inout.insertItem(0,"All",10)

        self.combo_inout.currentIndexChanged.connect(lambda x : MainWindow.vtk.update3D.emit() )
        self.viewtoolbar.addWidget(self.combo_inout)

        self.ls_slider = QT.QSlider(QT.Qt.Horizontal)
        self.ls_slider.setValue(50)
        self.ls_slider.valueChanged['int'].connect(lambda x : MainWindow.vtk.update3D.emit() )
        self.viewtoolbar.addWidget(self.ls_slider)

        self.viewtoolbar.addAction(MainWindow.actions['set 0.0 Ls or 0.5 density'] )

        self.AddToViewGUI(MainWindow,self.viewtoolbar)

        ## edit Toolbar
        self.edittoolbar = MainWindow.addToolBar('Edit')
        self.edittoolbar.setObjectName("edittoolbar")
        self.AddToEditGUI(MainWindow,self.edittoolbar)

        self.debugtoolbar = MainWindow.addToolBar('Debug')
        self.debugtoolbar.setObjectName("debugtoolbar")

        self.DebugCheckBoxWidget = QT.QCheckBox("Debug Output")

        self.DebugCheckBoxWidget.setChecked(IsVerboseMuscatOutput())
        self.DebugCheckBoxWidget.setTristate (False)
        def VerboseOnOff(val):
            if val :
                VerboseMuscatOutput()
            else:
                DefaultMuscatOutput()

        self.DebugCheckBoxWidget.stateChanged[int].connect(VerboseOnOff )
        self.debugtoolbar.addWidget(self.DebugCheckBoxWidget)

    def UpdateRecentFileActions(self):

        settings = QT.QSettings()
        recentFilesList =settings.value('recentFileList')
        if  recentFilesList  is None :
            return

        if type(recentFilesList) is list:
            files = recentFilesList
        elif type(recentFilesList) is str:
            files = [recentFilesList]
        else:
            files = recentFilesList.toPyObject()

        if files is None :
            return

        MaxRecentFiles = self.parent().MaxRecentFiles
        recentFileActs = self.parent().recentFileActs

        numRecentFiles = min(len(files), MaxRecentFiles)
        import os
        for i in range(numRecentFiles):

            if not os.path.exists(files[i]):
                recentFileActs[i].setVisible(False)
                continue
            text = "&%d %s" % (i + 1, QT.QtCore.QFileInfo(files[i]).fileName() )

            recentFileActs[i].setText(text)
            recentFileActs[i].setData(files[i])
            recentFileActs[i].setVisible(True)
            recentFileActs[i].setStatusTip("Open file: " + str(files[i]))

        for j in range(numRecentFiles, MaxRecentFiles):
            recentFileActs[j].setVisible(False)

        self.separatorAct.setVisible((numRecentFiles > 0))

    def CreateDocks(self,MainWindow):
        self.outputDock = QT.QDockWidget(MainWindow)
        self.outputDock.setWindowTitle("Output")
        self.outputDock.setObjectName("outputDock")
        textEdit = QT.QTextEdit(self.outputDock)
        textEdit.setReadOnly(1)
        self.outputDock.setWidget(textEdit)
        MainWindow.addDockWidget(QT.Qt.DockWidgetArea(8),self.outputDock)


        self.inputFileEditor = QT.QDockWidget(MainWindow)
        self.inputFileEditor.setWindowTitle("Input Data")
        self.inputFileEditorTextEdit = QT.QPlainTextEdit(self.inputFileEditor)
        import OpenPisco.QtApp.XmlHighlighting as XHL
        self.highlight = XHL.PythonHighlighter(self.inputFileEditorTextEdit.document())
        #self.inputFileEditorTextEdit.setReadOnly(1)
        self.inputFileEditor.setWidget(self.inputFileEditorTextEdit)
        self.inputFileEditor.setObjectName("inputFileEditor")
        MainWindow.addDockWidget(QT.Qt.DockWidgetArea(8),self.inputFileEditor)


        self.ObjectInspectorEditor = QT.QDockWidget(MainWindow)
        self.ObjectInspectorEditor.setWindowTitle("Object Editor")
        self.ObjectInspectorEditor.setObjectName("ObjectEditor")
        self.ObjectInspectorEditorScrollArea = QT.QScrollArea(self.ObjectInspectorEditor)
        self.ObjectInspectorEditorScrollArea.setWidgetResizable(True)
        self.ObjectInspectorEditor.setWidget(self.ObjectInspectorEditorScrollArea)
        MainWindow.addDockWidget(QT.Qt.DockWidgetArea(8),self.ObjectInspectorEditor)

        self.controlPanelApp = QT.QDockWidget(MainWindow)
        self.controlPanelApp.setWindowTitle("Main Control Panel")
        self.controlPanelApp.setObjectName("MainControlPanel")
        self.controlPanelAppWidget = QT.QWidget(self.controlPanelApp)
        self.controlPanelAppGridlayout = QT.QGridLayout(self.controlPanelAppWidget)
        self.controlPanelAppGridlayout.setRowStretch(1,1)
        self.controlPanelAppScrollArea = QT.QScrollArea(self.controlPanelApp)
        self.controlPanelAppScrollArea.setWidgetResizable(True)
        self.controlPanelAppGridlayout.addWidget(self.controlPanelAppScrollArea,0,0)
        tb = QT.QPushButton('Run')
        tb.setStatusTip('Run all the actions')
        tb.clicked.connect(MainWindow.RunLoadedFile)
        self.controlPanelAppGridlayout.addWidget(tb,1,0)
        self.controlPanelApp.setWidget(self.controlPanelAppWidget)
        MainWindow.addDockWidget(QT.Qt.DockWidgetArea(8),self.controlPanelApp)

        from OpenPisco.QtApp.EmbededPythonConsole import QIPythonWidget
        self.pythonConsoleDock = QT.QDockWidget(MainWindow)
        self.pythonConsoleDock.setWindowTitle("Python Console")
        self.pythonConsoleDock.setObjectName("PythonConsole")

        self._pythonConsole = QIPythonWidget(customBanner="Welcome to the OpenPisco Python Console\n")
        from Muscat.Actions.OpenInParaView import OpenInParaView
        self._pythonConsole.pushVariables({"OpenInParaView":OpenInParaView})
        self._pythonConsole.pushVariables({"MainWindow":MainWindow})
        self.pythonConsoleDock.setWidget(self._pythonConsole)
        MainWindow.addDockWidget(QT.Qt.DockWidgetArea(8),self.pythonConsoleDock)

        from OpenPisco.QtApp.BypassVerboseOutput import ToConsole
        from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory
        print("Duplicated output on "+ TemporaryDirectory.GetTempPath() )


        #self.toConsole = ToConsole(None)
        self.toConsole = ToConsole(textEdit)
        self.toConsole.OpenOutputDuplicationFiles(TemporaryDirectory.GetTempPath() )

    def GetActiveOptimAlgo(self):
        if self.optim_combo.count() == 0:
            return -1
        return int(self.optim_combo.itemData(self.optim_combo.currentIndex()))

    def GetActiveLevelset(self):
        if self.field_combo.count() == 0:
            return -1
        return int(self.field_combo.itemData(self.field_combo.currentIndex()))

    def GetActiveFieldToPlot(self):
        if self.fieldToPlot_combo.count() == 0:
            return None
        return str(self.fieldToPlot_combo.itemData(self.fieldToPlot_combo.currentIndex()))

    def UpdateFieldsToPlotComboBox(self,MainWindow):

        if MainWindow.MainApp is None:
                return

        self.fieldToPlot_combo.blockSignals(True)
        self.fieldToPlot_combo.clear()

        _,ls = MainWindow.GetActiveLevelset()
        cpt  = 0
        self.fieldToPlot_combo.insertItem(cpt,"phi","phi")
        cpt += 1
        if ls is not None and hasattr(ls, "extraNodesFields"):
            for k in ls.extraNodesFields.keys():
                if "phi" == k.lower():
                    continue
                name = f'field: {k}'
                self.fieldToPlot_combo.insertItem(cpt,name,name)
                cpt += 1

        for i in MainWindow.MainApp.zones:
                name = 'zone : ' + str(i)
                self.fieldToPlot_combo.insertItem(cpt,name,i)
                cpt +=1

        self.fieldToPlot_combo.blockSignals(False)



def CheckIntegrity(GUI = False):
    import sys
    app = QT.GetQApplication(sys.argv)
    from OpenPisco.QtApp.OpenPiscoGUI import SimpleView
    view = SimpleView()
    if GUI :
      view.show()
      sys.exit(app.exec_())

    return "OK"

if __name__ == '__main__':
        print(CheckIntegrity(True))
