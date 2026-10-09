# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import functools
import numpy as np

import OpenPisco.QtApp.QtImplementation as QT
from OpenPisco.QtApp.QtOptionsEditor import Editor,OptionFactory

from Muscat.ImplicitGeometry.ImplicitGeometryBase import ImplicitGeometryBase
ListOfTypesToDevelop = (ImplicitGeometryBase)

class QtObjectEditor(Editor):

    def __init__(self, obj,idd,cleanName,logfile=None,father=None):

        self.obj = obj
        self.idd = idd

        nn = str(type(obj)).split("'")[1]

        if idd == -1:
            description = nn + " (internal) "
        else:
            description = nn + " Id : " + str(idd)


        sortedatrs =  sorted(obj.__dict__, key=lambda x:x.lower())
        listOfOptions = []

        for o in sortedatrs:
            value = obj.__dict__[o]
            if callable(value): continue

            if len(o) > 1 and  o[0] == "_" : continue
            readOnly = False
            if o[-1] == "_" :
                readOnly = True

            t = type(value)
            def pullValueFunction(obj,name):
                return obj.__dict__[name]

            def pushValueFunction(obj,name,val):
                if type(obj.__dict__[name]) == dict:
                    obj.__dict__[name].update(val)
                else:
                    obj.__dict__[name] = val

            if value is None:
                continue

            if isinstance(value,ListOfTypesToDevelop):
                value = [value]
                constructor = OptionFactory.GetClass(list)
                listOfOptions.append(constructor(name=o,default=value,readOnly=readOnly) )
            elif t in OptionFactory.keys():
                constructor = OptionFactory.GetClass(t)
                listOfOptions.append(constructor(name=o,default=value,readOnly=readOnly,
                                        pullValueFunction=functools.partial(pullValueFunction,obj,o),
                                        pushValueFunction=functools.partial(pushValueFunction,obj) ) )
            else:
                constructor = OptionFactory.GetClass(str)
                listOfOptions.append(constructor(name=o,default="not editable",readOnly=True))

        super(QtObjectEditor,self).__init__(listOfOptions,description=description)


        if idd == -1:
            self.setWindowTitle("Editing "+cleanName + " (internal) " + nn )
        else:
            self.setWindowTitle("Editing "+cleanName + " Id : " + str(idd) +" type " + nn )

        self.resize(103, 153)

def CheckIntegrity(GUI=False,res = None):
    import sys
    QT.GetQApplication(sys.argv)
    QT.QMainWindow()
    
    class TestClassII():
        def __init__(self):
            super(TestClassII,self).__init__()
            self.float = 0.1

        def __str__(self):
            return "<instance of TestClassII> "

    class TestClass():
        def __init__(self):
            super(TestClass,self).__init__()
            self.boolVar = True
            self.intVar = 10
            self.floatVar = 3.14159
            self.stringVar = "hola"
            self.numpyvec = np.array([1,2,3])
            self.numpyvect = np.array([[1],[2],[3]]).T
            self._internalcounter = 0
            self._hideData = 1
            self.readOnlyData_ = 2
            self.stringVarReadOnly_ = "ReadOnly"
            self.stringVarReadOnlyLong_ = "ReadOnly plus de 50 chars ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
            self.boolVarReadOnly_ = True
            self.numpyArrayReadOnly_ = np.array([[11,12,13],[21,22,23],[31,32,33]])
            self.dictionary = {"toto":1.,"tata":3.5 }
            self.dictionaryReadOnly_ = {"toto":1.,"tata":3.5 }
            self.emptydic = {}
            self.listOfClasses = [TestClassII(),TestClassII(),TestClassII()]

        def __str__(self):
            self._internalcounter +=1
            res = "this is the str representation of this class \n"
            res += "Internal conter :GenerateQLineEdit " + str(self._internalcounter ) + "\n"
            return res


    myinstance = TestClass()
    w = QtObjectEditor(myinstance,"0", "myinstance")
    w.show()
    myinstance.floatVar *= 2
    myinstance.stringVar = "chao"
    myinstance.intVar = 666
    myinstance.boolVar = False
    myinstance.numpyvec = np.array([10,20,30])
    myinstance.emptydic["tutu"] = 25.

    return 'ok'

if __name__ == "__main__":
    res = []
    CheckIntegrity(GUI=True,res=res )
    res[0].exec_()

