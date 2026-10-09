# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import OpenPisco.QtApp.QtImplementation as QT

# Import the console machinery from ipython
from qtconsole.rich_jupyter_widget import RichJupyterWidget

from qtconsole.inprocess import QtInProcessKernelManager

class QIPythonWidget(RichJupyterWidget):
    """ Convenience class for a live IPython console widget. We can replace the standard banner using the customBanner argument"""
    def __init__(self,customBanner=None,*args,**kwargs):
        if customBanner is not None: self.banner=customBanner
        super(QIPythonWidget, self).__init__(*args,**kwargs)
        self.kernel_manager =  QtInProcessKernelManager()
        self.kernel_manager.start_kernel()
        self.kernel_client = self._kernel_manager.client()
        self.kernel_client.start_channels()

        def stop():
            self.kernel_client.stop_channels()
            self.kernel_manager.shutdown_kernel()
        self.exit_requested.connect(stop)
        self.cache_size = 0

    def pushVariables(self,variableDict):
        """ Given a dictionary containing name / value pairs, push those variables to the IPython console widget """
        self.kernel_manager.kernel.shell.push(variableDict)
    def clearTerminal(self):
        """ Clears the terminal """
        self._control.clear()
    def printText(self,text):
        """ Prints some plain text to the console """
        self._append_plain_text(text)
    def executeCommand(self,command):
        """ Execute a command in the frame of the console widget """
        self._execute(command,False)

def CheckIntegrity(GUI=False):
    class ExampleWidget(QT.QWidget):
        """ Main GUI Widget including a button and IPython Console widget inside vertical layout """
        def __init__(self, parent=None):
            super(ExampleWidget, self).__init__(parent)
            layout = QT.QVBoxLayout(self)
            self.button = QT.QPushButton('Another widget')
            ipyConsole = QIPythonWidget(customBanner="Welcome to the embedded ipython console\n")
            layout.addWidget(self.button)
            layout.addWidget(ipyConsole)
            # This allows the variable foo and method print_process_id to be accessed from the ipython console
            ipyConsole.pushVariables({"foo":43,"print_process_id":print_process_id})
            ipyConsole.printText("The variable 'foo' and the method 'print_process_id()' are available. Use the 'whos' command for information.")

    def print_process_id():
       import os
       print( 'Process ID is:', os.getpid())


    app  = QT.GetQApplication([])
    widget = ExampleWidget()
    widget.show()
    if GUI:
        app.exec()
    return "ok"

if __name__ == '__main__':
    CheckIntegrity(True)
