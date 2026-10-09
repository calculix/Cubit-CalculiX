# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import OpenPisco.QtApp.QtImplementation as QT
from Muscat.Helpers.IO.PathController import PathController

from OpenPisco.CLApp.InputReaderBase import InputReaderBase

class PiscoAppReader(InputReaderBase):
    def __init__(self):
        super(PiscoAppReader,self).__init__()
        self.ext = ".piscoApp"

    def ReadFromFile(self,filename,qtApp,mainApp):
        data = open(filename).read()
        self.filenamePath = PathController.GetFullPathCurrentDirectoryUsingFile(filename)
        return self.ReadFromString(data,qtApp,mainApp)


    def ReadFromString(self,data,qtApp,mainApp):
        import OpenPisco.QtApp.QtOptionsEditor as Options
        glob = {"MainWindow":None, "mainApp":mainApp,"Options":Options}

        exec(data,glob)

        from Muscat.Helpers.LocalVariables import AddToGlobalDictionary

        ops = glob.get("AppOptions",[])
        from OpenPisco.QtApp.QtOptionsEditor import Editor
        for op in ops:
            op.pushValueFunction = AddToGlobalDictionary
            op.Push(force=True)

        description = glob.get("AppDescription","")

        from OpenPisco.QtApp.OpenPiscoUIMainWindow import ListOfTypesToDevelop
        self.w = Editor( options = ops, description = description,father=qtApp,ListOfTypesToDevelop=ListOfTypesToDevelop)

        if 'AppXmlData' in glob:
            self.dataType = ".xml"
            from OpenPisco.CLApp.XmlToDic import XmlToDic
            self.inputReader =  XmlToDic()
            inputdata = glob['AppXmlData']

        elif 'AppPiscoData' in glob:
            self.dataType = ".pisco"
            from OpenPisco.CLApp.PiscoToDic import PiscoToDic
            self.inputReader =  PiscoToDic()
            inputdata = glob['AppPiscoData']
        else:
            self.dataType = ""
            return "",self.w

        return inputdata,self.w

def CheckIntegrity():
    data ="""

xmlData = '<data dim="3" debug="True"></data>'

    """
    QT.GetQApplication([])

    obj = PiscoAppReader()
    print(obj.ReadFromString(data, None, None) )

    return "OK"

if __name__ == '__main__':
        print(CheckIntegrity())
