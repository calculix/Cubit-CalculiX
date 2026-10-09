# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import numpy as np

import Muscat.Helpers.ParserHelper as PH

# QT imp must be imported before pyqtgraph
import OpenPisco.QtApp.QtImplementation as QT
import pyqtgraph as pg

from OpenPisco.QtApp.res import ResourcePath

pg.setConfigOptions(antialias=True)
class PlotData(object):
    #update2D = QT.Signal()

    def __init__(self,size=0):
        super(PlotData,self).__init__()
        self.values = [np.nan]*size

class RenderZone2D(QT.QWidget):
    def __init__(self, father, **kwargs):
        super(RenderZone2D,self).__init__(father)

        self.firstAxiscolor = "#00ff00"
        self.secondAxiscolor =  "#ff0000"
        self.yDims = 2
        self.onlyOnePlotArea = False

        PH.ReadProperties(kwargs,kwargs,self)


        self.gridlayout = QT.QGridLayout(self)
        self.gridlayout.setContentsMargins(0, 0, 0, 0)

        self.comboBoxs = []
        self.comboNames = []

        self.comboBoxs.append(QT.QComboBox())
        self.comboNames.append("x:")
        for i in range(self.yDims):
            self.comboBoxs.append(QT.QComboBox())
            self.comboNames.append("y"+str(1+i)+":")

        cpt = 0
        for i in range(self.yDims+1):
            label = QT.QLabel(self.comboNames[i])
            label.setAlignment(QT.Qt.AlignRight | QT.Qt.AlignVCenter)
            self.gridlayout.setColumnStretch(cpt, 0)
            self.gridlayout.addWidget(label, 0, cpt, 1, 1); cpt +=1
            self.gridlayout.setColumnStretch(cpt, 1)
            self.gridlayout.addWidget(self.comboBoxs[i], 0, cpt, 1, 1); cpt +=1
            self.comboBoxs[i].currentIndexChanged.connect(lambda _index: self.Update())

        cleanbutton = QT.QPushButton('Clean 2D Data')
        cleanbutton.clicked.connect(self.ClearData)

        self.gridlayout.addWidget(cleanbutton, 0,cpt, 1, 1)
        cpt +=1

        self.pyqtgraphWidget = pg.PlotWidget()

        self.gridlayout.addWidget(self.pyqtgraphWidget, 1, 0, 1, cpt)

        self.data = {}

        self.writeCpt = 0

        self.curves =  []
        self.curves.append(self.pyqtgraphWidget.plot(x=[], y=[],symbolPen='w', pen=pg.mkPen(color=self.firstAxiscolor),fill=pg.mkPen(color=self.firstAxiscolor),connect="finite"))
        self.pyqtgraphWidget.getAxis('left').setPen(pg.mkPen(color=self.firstAxiscolor, width=3))
        self.pyqtgraphWidget.getAxis('bottom').setPen(pg.mkPen(color='#ffffff', width=3))

        if self.yDims == 2:

            if self.onlyOnePlotArea:
                self.p2 = pg.ViewBox()
                self.curves.append(self.pyqtgraphWidget.plot(x=[], y=[],symbolPen='w', pen=pg.mkPen(color=self.secondAxiscolor,fill=pg.mkPen(color=self.secondAxiscolor)),connect="finite"))
                self.pyqtgraphWidget.getAxis('right').setPen(pg.mkPen(color=self.secondAxiscolor, width=3))

                self.pyqtgraphWidget.scene().addItem(self.p2)
                self.pyqtgraphWidget.getAxis('right').linkToView(self.p2)
                self.p2.setXLink(self.pyqtgraphWidget)
                self.p2.addItem(self.curves[-1])
                self.pyqtgraphWidget.getAxis('right').setPen(pg.mkPen(color=self.secondAxiscolor, width=3),fill=pg.mkPen(color=self.secondAxiscolor) )
            else:
                self.pyqtgraphWidget2 = pg.PlotWidget()
                self.gridlayout.addWidget(self.pyqtgraphWidget2, 2, 0, 1, cpt)
                self.curves.append(self.pyqtgraphWidget2.plot(x=[], y=[],symbolPen='w', pen=pg.mkPen(color=self.secondAxiscolor,fill=pg.mkPen(color=self.secondAxiscolor)),connect="finite"))
                self.pyqtgraphWidget2.getAxis('left').setPen(pg.mkPen(color=self.secondAxiscolor, width=3))
                self.pyqtgraphWidget2.getAxis('bottom').setPen(pg.mkPen(color='#ffffff', width=3))
                self.gridlayout.setRowStretch(1, 1)
                self.gridlayout.setRowStretch(2, 1)
                self.pyqtgraphWidget2.sigRangeChanged.connect(self.updateViews)
            self.pyqtgraphWidget.sigRangeChanged.connect(self.updateViews)


            self.updateViews(self.pyqtgraphWidget)
       # self.pyqtgraphWidget.getAxis('left').setPen(pg.mkPen(color=self.firstAxiscolor, width=3))

    def updateViews(self,r):
        w1 =  self.pyqtgraphWidget
        w1.sigRangeChanged.disconnect(self.updateViews)

        if self.yDims == 2:
            if self.onlyOnePlotArea :
                #self.p2.linkedViewChanged(self.pyqtgraphWidget.getViewBox(), self.p2.XAxis)
                self.p2.setGeometry(self.pyqtgraphWidget.getViewBox().sceneBoundingRect())
            else:
                w2 = self.pyqtgraphWidget2
                w2.sigRangeChanged.disconnect(self.updateViews)
                xRange = r.getAxis('bottom').range
                if w1 == r:
                    w2.setRange(xRange=xRange, padding=0)
                elif w2 == r:
                    w1.setRange(xRange=xRange, padding=0)
                w2.sigRangeChanged.connect(self.updateViews)
        w1.sigRangeChanged.connect(self.updateViews)

    def Write(self,datas):
        keys = set(self.data.keys())

        for key,data in datas.items():
            d = self.data.get(key,PlotData(self.writeCpt))
            d.values.append(data)
            self.data[key] = d
            keys.discard(key)

        # we add none to keep all the data with the same size
        for key in keys:
            self.data[key].values.append(np.nan)


        self.writeCpt += 1

        #get active name
        for box in self.comboBoxs:
           if box.count() == 0:
               cname = None
           else:
               cname = box.itemData(box.currentIndex())

           box.blockSignals(True)
           box.clear()

           cpt = 0
           for key in self.data:
               name = str(key)
               box.insertItem(cpt,name,name)
               cpt += 1

           if cname:
                idx = box.findText(cname)
                if idx != -1:
                    box.setCurrentIndex(idx)

           box.blockSignals(False)


        if self.writeCpt == 1:
            # first time we set the x to cpt, and the y1 and y2 to the variables not
            # in the list
            notToPlot = ["run", "step","iter","cpt", "OK", "time"]
            keys = set(self.data.keys())
            toPlot = list(keys.difference(notToPlot))
            toPlot.insert(0,"cpt")
            for box,name in zip(self.comboBoxs,toPlot):
                box.blockSignals(True)
                idx = box.findText(name)
                if idx != -1:
                    box.setCurrentIndex(idx)

                box.blockSignals(False)

    def Reset(self):
        self.ClearData()

    def ClearData(self):
        self.data = {}
        self.writeCpt = 0
        for box in self.comboBoxs:
            box.blockSignals(True)
            box.clear()
            box.blockSignals(False)

        for i in range(self.yDims):
            self.curves[i].setData(x=[0],y=[0])

        self.pyqtgraphWidget.setLabel('bottom', '',  **{'font-size':'20pt'})
        self.pyqtgraphWidget.setLabel('left', '',  **{'font-size':'20pt'})

        if self.yDims == 2:
            if self.onlyOnePlotArea :
                self.pyqtgraphWidget.setLabel('right', '',  **{'font-size':'20pt'})
            else:
                self.pyqtgraphWidget2.setLabel('bottom', '',  **{'font-size':'20pt'})
                self.pyqtgraphWidget2.setLabel('left', '',  **{'font-size':'20pt'})

    def Update(self):
        names = []
        datas = []
        for box in self.comboBoxs:
            name = box.itemData(box.currentIndex())
            names.append(name)
            datas.append(self.data.get(name,None))


        self.pyqtgraphWidget.setLabel('bottom', names[0],  **{'font-size':'20pt'})
        self.pyqtgraphWidget.getAxis('bottom').setPen(pg.mkPen(color='#ffffff', width=3))

        self.pyqtgraphWidget.setLabel('left', names[1],  **{'font-size':'20pt'})
        self.pyqtgraphWidget.getAxis('left').setPen(pg.mkPen(color=self.firstAxiscolor, width=3))

        if self.yDims == 2:
            if self.onlyOnePlotArea:
                self.pyqtgraphWidget.setLabel('right', names[2],  **{'font-size':'20pt'})
                self.pyqtgraphWidget.getAxis('right').setPen(pg.mkPen(color=self.secondAxiscolor, width=3))
            else:
                self.pyqtgraphWidget2.setLabel('left', names[2],  **{'font-size':'20pt'})
                self.pyqtgraphWidget2.getAxis('left').setPen(pg.mkPen(color=self.secondAxiscolor, width=3))
                self.pyqtgraphWidget2.setLabel('bottom', names[0],  **{'font-size':'20pt'})
                self.pyqtgraphWidget2.getAxis('bottom').setPen(pg.mkPen(color='#ffffff', width=3))

        for i in range(self.yDims):
            if datas[i+1] is None : continue

            if datas[0] is None:
                self.curves[i].setData(x=list(range(len(datas[i+1].values))),y=datas[i+1].values)
            else:
                self.curves[i].setData(x=datas[0].values,y=datas[i+1].values)

def CheckIntegrity(GUI = False):

    if GUI == False:
        print("Cant test this class without GUI")
        return 'OK'

    import sys
    class MainWindow(QT.QMainWindow):

        def __init__(self, parent=None):
            QT.QMainWindow.__init__(self, parent)

            self.frame = QT.QFrame()

            self.widgets= []

            self.vl = QT.QHBoxLayout()
            self.widgets.append(RenderZone2D(self,yDims=1))
            self.vl.addWidget(self.widgets[0])

            self.widgets.append(RenderZone2D(self,yDims=2))
            self.vl.addWidget(self.widgets[1])

            self.widgets.append(RenderZone2D(self,yDims=2,onlyOnePlotArea=True))
            self.vl.addWidget(self.widgets[2])

            self.frame.setLayout(self.vl)
            self.setCentralWidget(self.frame)

            self.show()
    app = QT.GetQApplication(sys.argv)

    window = MainWindow()
    for w in window.widgets:
        w.Write({"time":0,"fx":0})
        w.Write({"time":1,"fx":1,"fy":1e-14})
        w.Write({"time":2,"fx":4,"fy":1.414e-14})
        w.Write({"time":3,"fx":8,"fy":1.732e-14})
        w.Write({"time":4,"fx":16})
        w.Update()
    for k,data in window.widgets[0].data.items():
        print(k + " : " + str(data.values))

    sys.exit(app.exec_())
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity(True))
