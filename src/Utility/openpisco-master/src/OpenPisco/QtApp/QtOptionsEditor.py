# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import functools

import numpy as np

import Muscat.Helpers.ParserHelper as PH
import OpenPisco.QtApp.QtImplementation as QT
from Muscat.Helpers.Factory import Factory

def RegisterClass(name, classtype, constructor=None, withError = True):
    return OptionFactory.RegisterClass(name,classtype, constructor=constructor, withError = withError )

class OptionFactory(Factory):
    _Catalog = {}
    _SetCatalog = set()

    def __init__(self):
        super(OptionFactory,self).__init__()

class OptionBase():
    def __init__(self,name, dtype, default, valueRange=None, slider=False, label=None, pushValueFunction=None, pullValueFunction=None,readOnly=False):
        super(OptionBase,self).__init__()
        self.name = name
        if label is None:
            self.label = name
        else:
            self.label = label
        self.dtype = dtype
        self.slider = slider
        self.valueRange = valueRange
        self.readOnly = readOnly
        self.pushValueFunction = pushValueFunction
        self.pullValueFunction = pullValueFunction
        self.automaticPush = False
        self._modified_ = False
        self.value = None
        if default is None:
            if self.pullValueFunction == None:
                raise ValueError("Cant define initial value, need default or pullValueFunction")
            else:
                self.Pull()
        else:
            if self.dtype in [dict, list]  :
                self.value = default.copy()
            elif self.dtype == np.ndarray:
                self.value = np.copy(default)
            else:
                self.value = default
                self.SetValue(default)

    def ApplyRange(self,val):
        ok = True
        if self.valueRange is not None:
            if val < self.valueRange[0] :
                val = self.valueRange[0]
                ok = False

            if val > self.valueRange[1]:
                val = self.valueRange[1]
                ok = False
        return val,ok

    def SetValue(self,value,key=None):
        try:
            val = PH.ReadScalar(value,self.dtype)
            ok = True
        except:
            ok = False

        if ok:
            val,ok = self.ApplyRange(val)

        if ok:
            if self.value != val:
                self._modified_ = True
            if key is not None:
                self.value[key] = val
            else:
                self.value = val

            if self.automaticPush:
                self.Push()
        return ok

    def Push(self,force=False):
        if self.pushValueFunction is not None and (self._modified_ or force) :
            self.pushValueFunction(self.name,self.value)
        self._modified_  = False

    def Pull(self):
        if self.pullValueFunction is not None:
            self.SetValue(self.pullValueFunction())

    def __str__(self):
        res = str(self.name )+ ": " + str(self.value)
        return res

class OptionBool(OptionBase):
    def __init__(self,name, default, **kwargs):
        super(OptionBool,self).__init__(name, dtype=bool, default=default, **kwargs)

RegisterClass("bool",OptionBool)
RegisterClass(bool,OptionBool)

class OptionInt(OptionBase):
    def __init__(self,name, default, **kwargs):
        super(OptionInt,self).__init__(name, dtype=int, default=default, **kwargs)

RegisterClass("int",OptionInt)
RegisterClass(int,OptionInt)

class OptionFloat(OptionBase):
    def __init__(self,name, default, **kwargs):
        super(OptionFloat,self).__init__(name, dtype=float, default=default, **kwargs)

RegisterClass("float",OptionFloat)
RegisterClass(float,OptionFloat)

class OptionStr(OptionBase):
    def __init__(self,name, default, **kwargs):
        super(OptionStr,self).__init__(name, dtype=str, default=default, **kwargs)

RegisterClass("str",OptionStr)
RegisterClass(str,OptionStr)

class OptionDict(OptionBase):
    def __init__(self,name, default,subtype=None, **kwargs):
        super(OptionDict,self).__init__(name, dtype=dict, default=default, **kwargs)

        if subtype is None :
            if default is None:
                raise ValueError("Need a subtype or a default value") 
            else:
                if len(default):
                    self.subtype = type(default[next(iter(default))])
                else:
                    self.subtype = None
        else:
            self.subtype = subtype

    def Push(self,force=False):
        if self.pushValueFunction is not None and (self._modified_ or force) :
            self.pushValueFunction(self.name,self.value.copy())
        self._modified_  = False

    def SetValue(self,value,key=None):

        if self.dtype == dict:
            if key is None:
                for k,v in value.items():
                    self.SetValue(v,key=k)
                return True
            try:
                if key in self.value:
                    val = PH.ReadScalar(value,type(self.value[key]) )
                elif self.subtype is not None:
                    val = PH.ReadScalar(value,self.subtype )
                else:
                    val = value
            except:
                return False

        val,ok = self.ApplyRange(val)

        if ok:
            if key in self.value:
                if self.value[key] != val:
                    self._modified_ = True
                self.value[key] = val
            else:
                self.value[key] = val
                self._modified_ = True

            if self.automaticPush:
                self.Push()
        return ok

RegisterClass("dict",OptionDict)
RegisterClass(dict,OptionDict)

class OptionNumpyArray(OptionBase):
    def __init__(self,name, default, **kwargs):
        super(OptionNumpyArray,self).__init__(name, dtype=np.ndarray, default=default, **kwargs)

    def Push(self,force=False):
        if self.pushValueFunction is not None and (self._modified_ or force) :
            self.pushValueFunction(self.name,np.copy(self.value))
        self._modified_  = False

    def SetValue(self,value,key=None):
        if key is None:
            val = PH.ReadVector(value,self.value.dtype.type)
        else:
            val = PH.ReadScalar(value,self.value.dtype.type)

        val,ok = self.ApplyRange(val)

        if ok:
            if key is not None:

                if type(key) == int:
                    if self.value[key] != val:
                        self._modified_ = True
                    self.value[key]  = val
                elif len(key) == 1:
                    if self.value[key[0]] != val:
                        self._modified_ = True
                    self.value[key[0]]  = val
                else:
                    if self.value[key[0],key[1]] != val:
                        self._modified_ = True
                    self.value[key[0],key[1]]  = val
            else:
                if not np.all(np.equal(self.value,val)):
                    self._modified_ = True
                self.value = val
            if self.automaticPush:
                self.Push()
        return ok

RegisterClass("np.ndarray,",OptionNumpyArray)
RegisterClass(np.ndarray,OptionNumpyArray)

class OptionList(OptionBase):
    def __init__(self,name, default, **kwargs):
       super(OptionList,self).__init__(name, dtype=list, default=default, **kwargs)

RegisterClass("list",OptionList)
RegisterClass(list,OptionList)

floatFactor = 10000

def updateStyle(ok,modified,sender):
    if ok:
        if ok and modified :
            sender.setStyleSheet("QLineEdit { background: rgb(0,255, 0); selection-background-color: rgb(233, 99, 0); }")
        else:
            sender.setStyleSheet("QLineEdit {  }")
    else:
        sender.setStyleSheet("QLineEdit { background: rgb(255, 0, 0); selection-background-color: rgb(233, 99, 0); }")

def GenerateQLineEdit(value,readOnly):
    w = QT.QLineEdit()
    w.setText(str(value))
    w.setReadOnly(readOnly)
    if readOnly:
        palette = QT.QPalette()
        palette.setColor(QT.QPalette.Text,QT.Qt.gray)
        w.setPalette(palette)
    w.setAlignment(QT.Qt.AlignVCenter)
    return w

def GenerateQTextEdit(value,readOnly):
    w = QT.QTextEdit()
    w.setText(str(value))
    w.setReadOnly(readOnly)
    if readOnly:
        palette = QT.QPalette()
        palette.setColor(QT.QPalette.Text,QT.Qt.gray)
        w.setPalette(palette)
    w.setAlignment(QT.Qt.AlignVCenter)
    return w

def GenerateQCheckBox(value,readOnly):
    w = QT.QCheckBox()
    w.setChecked(bool(value))
    w.setEnabled(not readOnly)
    w.setTristate(False)
    return w

class Editor(QT.QWidget):
    pushEnded = QT.Signal()
    pullEnded = QT.Signal()
    editEnded = QT.Signal()

    def PushValues(self):

        for w,op,_ in self.allWidgets:
            op.Push()
            w.setStyleSheet("QLineEdit {  }")

        self.pushEnded.emit()
        if self.applybutton is not None:
            self.applybutton.setEnabled(False)

    def PullValues(self):
        for w,op,data in self.allWidgets:
            op.Pull()
            w.UpdateValue(w,op,data)
            w.setStyleSheet("QLineEdit {  }")

        self.pullEnded.emit()
        if self.applybutton is not None:
            self.applybutton.setEnabled(False)


    def SetValueScalar(self,sender,op,localset=True):
        @QT.Slot()
        def func():
            try :
                text = str(sender.text())
            except TypeError:
                text = str(sender.toPlainText())
            ok = op.SetValue(text)
            if localset:
                sender.blockSignals(True)
                sender.setText("{}".format(op.value))
                sender.blockSignals(False)
            updateStyle(ok,op._modified_,sender)
            if op._modified_ :
                self.editEnded.emit()
        return func

    def SetValueBool(self,sender,op):
        @QT.Slot()
        def func():
            ok = op.SetValue(sender.isChecked())
            updateStyle(ok,op._modified_,sender)
            if op._modified_ :
                self.editEnded.emit()
        return func

    def SetValueWithKey(self,sender,op,key):
        @QT.Slot()
        def func():
            try :
                text = str(sender.text())
            except TypeError:
                text = str(sender.toPlainText())
            ok = op.SetValue(text,key)

            updateStyle(ok,op._modified_,sender)
            if op._modified_ :
                self.editEnded.emit()
        return func

    def SetValueDict(self,sender,op,key):
        @QT.Slot()
        def func():
            try :
                text = str(sender.text())
            except TypeError:
                text = str(sender.toPlainText())
            if key() is None:
                ok = False
            else:
                ok = op.SetValue(text,key=key())
            updateStyle(ok,op._modified_,sender)
            if op._modified_ :
                self.editEnded.emit()
        return func

    def SetValueSlider(self,sender,w1,op,f=True):
        @QT.Slot()
        def func():
            if f :
                ok = op.SetValue(str(sender.value()/floatFactor))
            else:
                ok = op.SetValue(str(sender.value()))
            updateStyle(ok,op._modified_,w1)
            if op._modified_ :
                self.editEnded.emit()
        return func

    @QT.Slot()
    def _UpdateApplybutton(self):
        if self.applybutton is not None:
            self.applybutton.setEnabled(True)

    def __init__(self, options,description=None,father = None, ListOfTypesToDevelop=()):
        self.enablePull = False
        self.enablePush = False

        self.options = options
        QT.QWidget.__init__(self,father)

        self.allWidgets = []
        self.gridlayout = QT.QGridLayout(self)
        self.gridlayout.setColumnStretch(0,0)
        self.gridlayout.setColumnStretch(1,1)
        self.gridlayout.setColumnStretch(2,1)
        self.gridlayout.setColumnStretch(3,1)
        self.gridlayout.setColumnStretch(3,0)

        cpt = 0

        if description is not None:
            w = QT.QLabel(description)
            w.setWordWrap(True)


            self.gridlayout.addWidget(w, cpt, 0,1,4)
            cpt += 1


        for op in options:
            if op.pullValueFunction is not None:
                self.enablePull = True
            if op.pushValueFunction is not None:
                self.enablePush = True

            if op.label == op.name:
                label = QT.QLabel(op.label+":")
            else:
                label = QT.QLabel(op.label+" ("+op.name+"):")
            self.gridlayout.addWidget(label, cpt, 0)


            if not op.slider:
                if op.dtype in [float,int,str]:
                    if op.dtype == str:
                        if len(op.value) > 50:
                            w = GenerateQTextEdit(op.value,op.readOnly)
                            w.textChanged.connect(self.SetValueScalar(w,op,localset=False) )
                        else:
                            w = GenerateQLineEdit(op.value,op.readOnly)
                            w.editingFinished.connect( self.SetValueScalar(w,op) )
                    else:
                        w = GenerateQLineEdit(op.value,op.readOnly)
                        w.editingFinished.connect( self.SetValueScalar(w,op) )
                    w.UpdateValue = lambda widget,op,data: widget.setText(str(op.value))
                    self.gridlayout.addWidget(w, cpt, 1,1,3,QT.Qt.AlignVCenter)
                elif op.dtype is bool:
                    w = GenerateQCheckBox(op.value,op.readOnly)
                    w.stateChanged.connect(self.SetValueBool(w,op) )
                    w.UpdateValue = lambda widget,ops,data: widget.setChecked(bool(ops.value))
                    self.gridlayout.addWidget(w, cpt, 1,1,3,QT.Qt.AlignVCenter)

                elif  op.dtype is list:
                    if len(op.value) >5:
                        cpt +=1
                        continue
                    if len(op.value) >0 :
                        import inspect
                        if inspect.ismethod(op.value[0]):
                            cpt +=1
                            continue
                    for i in range(len(op.value)):
                        if isinstance(op.value[i],ListOfTypesToDevelop):
                             from OpenPisco.QtApp.QtObjectEditor import QtObjectEditor
                             w = QT.QPushButton()
                             w.setText(str(i)+ ":" + str(type(op.value[i]).__name__) )
                             def func(obj,name1,name2):
                                 if father is None:
                                     w = QtObjectEditor( obj,name1,name2)

                                 else:
                                     w = QtObjectEditor( obj,name1,name2,father=father )
                                     try:
                                         father.setWidget(w)
                                     except:
                                         pass
                                 w.show()
                             w.clicked.connect(functools.partial(func, op.value[i],"0",str(op.name) ) )
                             self.gridlayout.addWidget(w, cpt, 1+i,QT.Qt.AlignVCenter)

                        else:
                             w = GenerateQLineEdit(op.value[i],op.readOnly)
                             w.textChanged.connect(self.SetValueWithKey(w,op,i))

                             self.gridlayout.addWidget(w, cpt, 1+i,QT.Qt.AlignVCenter)
                             w.UpdateValue = lambda widget,opo,i: widget.setText(str(opo.value[i][:]))
                             self.allWidgets.append([w, op, i ] )
                    cpt +=1
                    continue

                elif op.dtype is dict:
                    # verification that all the keys are strings
                    ok = all([type(k) is str for k in op.value.keys()])

                    if not ok:
                        cpt +=1
                        continue

                    w = QT.QComboBox()
                    w2 = GenerateQLineEdit("" ,op.readOnly)

                    def UpdateComboBoxEntries(w,op,widgets):
                         nwidget, _ = widgets
                         nwidget.clear
                         lcpt =0
                         for k in op.value.keys():
                             index = nwidget.findText(k)
                             if index < 0:
                                 nwidget.insertItem(lcpt,k,k)
                             lcpt +=1

                    UpdateComboBoxEntries(w,op,(w,w2))
                    self.gridlayout.addWidget(w, cpt, 1,1,1,QT.Qt.AlignVCenter)

                    def GetCurrectKey(w):
                        return w.itemData(w.currentIndex())

                    w.UpdateValue = UpdateComboBoxEntries
                    self.allWidgets.append((w,op,(w,w2)) )

                    self.gridlayout.addWidget(w2, cpt, 2,1,2,QT.Qt.AlignVCenter)

                    w2.textChanged.connect( self.SetValueDict(w2,op,key=functools.partial(GetCurrectKey,w)) )

                    def updateDict(w, op, widgets,index=None):
                        nwidget, dwitget = widgets
                        data = GetCurrectKey(nwidget)
                        if data is None:
                            return
                        dwitget.blockSignals(True)
                        dwitget.setText(str(op.value[GetCurrectKey(nwidget)]))
                        dwitget.blockSignals(False)

                    w2.UpdateValue = updateDict
                    w2.UpdateValue(w2,op,(w,w2))
                    # if we change the combobox
                    w.currentIndexChanged.connect( functools.partial(w2.UpdateValue,w2,op, (w,w2) ))

                    self.allWidgets.append([w2,op,(w,w2)])
                    cpt +=1
                    continue

                elif  op.dtype == np.ndarray:
                     # expose only 1D or 2D matrices
                    if len(op.value.shape) > 2:
                        cpt +=1
                        continue
                    elif len(op.value.shape) > 1 :
                        # expose only small matrices
                        if op.value.shape[0] > 3:
                            cpt +=1
                            continue
                    else:
                        # expose smal vectors
                        if op.value.size > 12:
                            cpt +=1
                            continue
                     #op.readOnly = True
                    if len(op.value.shape) == 0 :
                         w = GenerateQLineEdit(op.value,op.readOnly)
                         w.textChanged.connect(self.SetValueWithKey(w,op, (0,) ) )
                         self.gridlayout.addWidget(w, cpt, 1,1,3,QT.Qt.AlignVCenter)
                         raise NotImplementedError()
                    elif len(op.value.shape) == 1 :
                        for i in range(op.value.size):
                           w = GenerateQLineEdit(op.value[i],op.readOnly)
                           w.textChanged.connect(self.SetValueWithKey(w,op, (i,) ) )
                           self.gridlayout.addWidget(w, cpt, 1+i,QT.Qt.AlignVCenter)
                           def updateValuei(dwitget,op,data):
                               i = data
                               dwitget.blockSignals(True)
                               dwitget.setText(str(op.value[i]))
                               dwitget.blockSignals(False)
                           w.UpdateValue = updateValuei
                           self.allWidgets.append([ w, op, i ])

                    else:
                         if op.value.shape[0] > 3: continue
                         if op.value.size > 12: continue

                         for i in range(op.value.shape[0]):
                             for j in range(op.value.shape[1]):

                                 def updateValueij(dwitget,op,data):
                                       i,j = data
                                       dwitget.blockSignals(True)
                                       dwitget.setText(str(op.value[i,j]))
                                       dwitget.blockSignals(False)
                                 w = GenerateQLineEdit(op.value[i,j],op.readOnly)
                                 w.textChanged.connect(self.SetValueWithKey(w,op,(i,j)) )
                                 self.gridlayout.addWidget(w, cpt, 1+j,QT.Qt.AlignVCenter)
                                 w.UpdateValue = updateValueij
                                 self.allWidgets.append([w,op, (i,j)])

                             cpt += 1

                    cpt +=1
                    continue
                else:
                    print("type " + str(op.dtype) + " Not treated yet")
                    cpt +=1
                    continue


            else:
                if op.dtype in [float,int]:

                    w = QT.QSlider(QT.Qt.Horizontal)

                    if op.dtype == float:
                        factor = floatFactor
                    else:
                        factor = 1

                    w.setMinimum(op.valueRange[0]*factor)
                    w.setMaximum(op.valueRange[1]*factor)
                    w.setValue(op.value*factor)
                    w1 = QT.QLabel(alignment=QT.Qt.AlignCenter)
                    w1.setNum(op.value)
                    self.gridlayout.addWidget(w, cpt, 1,1,2,QT.Qt.AlignVCenter)

                    def setLabelVal(factor,wid,val):
                        wid.setNum(val/factor)
                    def setSliderVal(factor, wid, op, data):
                        wid.setValue(op.value*factor)

                    w.valueChanged.connect(functools.partial(setLabelVal,factor,w1) )
                    w.valueChanged.connect( self.SetValueSlider(w,w1,op, op.dtype is float))
                    w.UpdateValue = functools.partial(setSliderVal,factor)
                    self.gridlayout.addWidget(w1, cpt, 3,1,1,QT.Qt.AlignVCenter)

                else:
                    raise NotImplementedError("Can only handle float and int for now")


            self.allWidgets.append((w,op,None) )

            cpt +=1
        self.applybutton = None

        if self.enablePush:
            self.applybutton = QT.QPushButton("Apply Changes")
            self.gridlayout.addWidget(self.applybutton, cpt, 0,1,4,QT.Qt.AlignBottom)
            self.gridlayout.setRowStretch(cpt,100)
            cpt +=1
            self.applybutton.clicked.connect(self.PushValues)
            self.applybutton.setEnabled(False)

        if self.enablePull:
            self.pullbutton = QT.QPushButton("Pull current Values")
            self.gridlayout.addWidget(self.pullbutton, cpt, 0,1,4,QT.Qt.AlignBottom)
            #self.gridlayout.setRowStretch(cpt,100)
            cpt +=1
            self.pullbutton.clicked.connect(self.PullValues)
            self.pullbutton.setEnabled(True)
        self.editEnded.connect(self._UpdateApplybutton)

def CheckIntegrity(GUI=False,res = None):
    import sys
    app = QT.GetQApplication(sys.argv)
    QT.QMainWindow()

    class TestClassII():
        def __init__(self):
            super(TestClassII,self).__init__()
            self.float = 0.1

        def __str__(self):
            return "<instance of TestClassII> "

    boolVariable = OptionBool("boolVar",False,label="Bool Variable")
    boolVariableReadOnly = OptionBool("boolVarReadOnly_",True,readOnly=True)

    try:
        error = False
        stringVariable =  OptionStr("desc",None)
        error = True
    except:
        if error:
            raise

    class State():
        def __init__(self):
            self.strData = ""

        def pullValueFunction(self):
            return self.strData

        def pushValueFunction(self,name,val):
            self.strData = val
            print("In push Value Function of " +str(name) +"  "+ str(val))
    strstate = State()

    stringVariable =  OptionStr("desc (automaticPush)",None,pullValueFunction = strstate.pullValueFunction , pushValueFunction=strstate.pushValueFunction)
    stringVariable.automaticPush = True

    dictVariable  = OptionDict("dictionalry (automaticPush)",{"toto":1.,"tata":3.5 })
    dictVariable.automaticPush = True

    dictVariableReadonly = OptionDict("dictionaryReadOnly_",{"toto":1.,"tata":3.5 },readOnly=True)
    emptyDictVariable = OptionDict("emptydic",{},subtype=int)

    floatVar = OptionFloat("floatVar",3.14159)
    intVar   = OptionInt("intVar",10)

    #heterolist = OptionList("heterolist",[1,"3"]) # this does not work
    listOfClasses = OptionBase("listOfClasses",list,[TestClassII(),TestClassII(),TestClassII()])
    numpyArrayReadOnly_=     OptionNumpyArray("numpyArrayReadOnly_",np.array([[11,12,13],[21,22,23],[31,32,33]]))
    numpyvec =       OptionNumpyArray("numpyvec",np.array([1,2,3]))
    numpyvect=       OptionNumpyArray("numpyvect",np.array([[1],[2],[3]]).T)
#            self.readOnlyData_ = 2
#    stringVar       OptionStr("stringVar",str,"hola"),
#           OptionStr("stringVarReadOnly_",str,"ReadOnly",readOnly=True),
#           OptionStr("stringVarReadOnlyLong_",str,"ReadOnly plus de 50 chars ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++",readOnly=True),
#
#
    floatVariableRange = OptionFloat("Float option range",0.1,valueRange=[0,12.5])
    floatVariableRangeSlider  = OptionFloat("Float option with slider",0.1,valueRange=[0,12.5],slider=True)
    intVariableRange       = OptionInt("int option range (30,40) ",30,valueRange=[30,40])
    intVariableRangeSlider = OptionInt("int option with slider",30,valueRange=[30,40],slider=True)
    strVariable = OptionStr("Long text","This is a very long text to edit with more than 50 characters..................")

    ops = [boolVariable,
           boolVariableReadOnly,
           stringVariable,
           dictVariable,
           dictVariableReadonly,
           emptyDictVariable,
           floatVar,
           intVar,
           #heterolist,
           listOfClasses,
           numpyArrayReadOnly_,
           numpyvec,
           numpyvect,

           floatVariableRange,
           floatVariableRangeSlider,
           intVariableRange,
           intVariableRangeSlider,
           strVariable

           ]

    w = Editor(ops,description="Options Editor")
    w.show()
    #ops[0].SetValue(True)
    #ops[1].SetValue(10.1)
    #ops[3].SetValue(6.5)
    #ops[4].SetValue(100)
    #ops[6].SetValue(35)
    #ops[7].SetValue("100")

    stringVariable.SetValue("'data to push'")
    floatVariableRange.SetValue(-1)
    floatVariableRange.SetValue(20)


    dictVariable.SetValue({"tata":3.6})
    dictVariable.SetValue(3.2,key="titi")
    dictVariable.Push()

    numpyvec.automaticPush = True
    numpyvec.SetValue(.2,key=1)
    numpyvec.SetValue(3.25,[1])
    numpyvec.SetValue([5,6,7])
    numpyvec.Push()

    numpyArrayReadOnly_.SetValue(3.25,key=np.array([1,1]))
    intVariableRangeSlider.SetValue(35)

    if res is not  None:
        res.append(app)
        res.append(w)

    w2 = Editor(ops)
    w2.show()

    print([str(x) for x in ops])

    OptionFactory()

    return 'ok'
if __name__ == "__main__":
    res = []
    print(CheckIntegrity(GUI=True,res=res ))
    res[0].exec_()
