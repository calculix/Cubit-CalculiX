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

from Muscat.Helpers import ParserHelper as PH


def CheckIntegrity(GUI=False):
    if not GUI:
        return "OK"
    raise NotImplementedError("for the moment we dont know how to test a QT app")

window = None

def LaunchPiscoGUI_Workload():
    from .SimpleView import SimpleView
    import OpenPisco.QtApp.QtImplementation as QT
    import OpenPisco.CLApp.OpenPiscoCL as MainApp
    from OpenPisco.QtApp.res import ResourcePath

    from Muscat.Helpers.IO.PathController import PathController

    logdir = os.path.expanduser("~")+ os.sep +".openpisco" + os.sep
    if not os.path.isdir(logdir):
        os.mkdir(logdir)

    PathController.SetCurrentDirectory(os.getcwd())

    from Muscat.Helpers.Logger import SetUseDifferentialTime
    SetUseDifferentialTime(True)

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('-l','--loadlastfile',help="Load the last file opened",action="store_true")
    parser.add_argument('-r','--run',help="Run loaded file",action="store_true")
    parser.add_argument('-v','--verbose',help="Force Verbosity", dest='verbose', action='store_true')
    parser.add_argument('-s','--single_window',help="Force app to open only one windows", dest='single_window', action='store_true')
    parser.add_argument('-f','--full_screen',help="Force full screen", dest='full_screen', action='store_true')
    parser.add_argument('-b','--bashmode',help="Do not launch the GUI, run only in command line (not compatible with other options) ", dest='bashmode',action='store_true')
    parser.add_argument("File", help="File to load",nargs="?")
    args = parser.parse_args()

    if args.bashmode:
        MainApp.RunSimulation(args.File)
        sys.exit()

    # change locale to read correctly stl files
    os.environ["LANG"] = "en_GB"
    #os.environ["LANG"] = "fr_FR"

    app = QT.GetQApplication(sys.argv)
    #[u'Windows', u'Motif', u'CDE', u'Plastique', u'GTK+', u'Cleanlooks']

    #print(app.style())
    #app.setStyle(u"Windows")
    #app.setStyle(u"CDE")
    #app.setStyle(u"Motif")
    #app.setStyle(u"Plastique")
    #app.setStyle(u"GTK+")
    #app.setStyle(u"Cleanlooks")

    app.setOrganizationName("The OpenPisco authors")
    app.setOrganizationDomain("")
    app.setApplicationName("OpenPisco")
    app.setWindowIcon(QT.QtGui.QIcon(ResourcePath()+'K0205_favicon_OpenPISCO_mesh.svg'))

    global window
    window = SimpleView(with_demo = True)
    window.single_window = args.single_window
    window.inYarss = PH.ReadBool(os.environ.get("INYARSS","false") )

    window.setWindowIcon(QT.QIcon(ResourcePath()+'K0205_favicon_OpenPISCO_mesh.svg'))
    window.show()

    def receive_signal(signum, stack):
        if signum == 10:
            window.setFullScreen.emit()

    if args.full_screen:
        window.showFullScreen()
        signal.signal(10,receive_signal)
    else:
        window.showNormal()


    filename = args.File

    if args.loadlastfile :
        settings = QT.QSettings()
        recentFilesList = settings.value('recentFileList')
        if  recentFilesList  is None :
            print ("recent files list empty")
            exit(1)
        files = recentFilesList#.toPyObject()
        if len(files) :
            filename = str(files[0])

    if filename is not None:
        from Muscat.Helpers.IO.PathController import PathController
        fname = PathController.GetFullFilenameCurrentDirectory(filename)

        if args.run :
            window.OpenFile(fname,callback=window.RunLoadedFile)
        else:
            window.OpenFile(fname)

    res = app.exec()
    window.vtk.vtkWidget.GetRenderWindow().Finalize()
    window.vtk.iren.TerminateApp()
    window.thread.exiting = True
    del window.vtk
    del window
    del app
    # this kills the gui but
    # continue the last action
    sys.exit()
    #
    #This kills the app but not really a nice solution
    # the GUI freeses during shutdonw.
    # os._exit(1)
    #

def LaunchPiscoGUI():
    """Muscat uses
    """
    from threading import Thread
    QtThread = Thread(target=LaunchPiscoGUI_Workload)
    QtThread.start()
    from time import sleep
    import sys
    while(window is None):
        print("Waiting for the main window to open")
        sleep(1)
        if not QtThread.is_alive():
            print("Fail opening the main windows exiting")
            sys.exit(1)
    window.thread.WaitForActions()

    sys.exit()



if __name__ == "__main__":
    LaunchPiscoGUI()

