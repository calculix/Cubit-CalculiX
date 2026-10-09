# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

class ProxyWriter():
    def __init__(self,app,writer):
        super(ProxyWriter,self).__init__()
        self.__app = app
        self.writer = writer
    def Write(self,*args,**kargs):
        if (self.__app is not None) and "GridFieldsNames" in kargs and "GridFields" in kargs:
            names = kargs["GridFieldsNames"]
            data = kargs["GridFields"]
            self.__app.ui.Plot2D(names,data)
        self.writer.Write(*args,**kargs)

    def Close(self):
        self.writer.Close()

    def Open(self):
        self.writer.Open()
