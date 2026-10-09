# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#


import OpenPisco.QtApp.QtImplementation as QT
class Worker(QT.QtCore.QObject):
    finished = QT.Signal()

    def __init__(self, parent = None):
        super().__init__(parent=parent)
        self.payload = [False, False, False, False]
        self.exiting = False
        self.finished.connect(self.RunCallback)

    def WaitForActions(self):
        from time import sleep
        while(self.exiting == False):
            if self.payload is not None and self.payload[1]:
                try:
                    self.payload[0]()
                except Exception as e:
                    print(e)
                    import sys
                    (type, value, traceback) = sys.exc_info()
                    print( "********* Error Begin *********" )
                    print( value)
                    print( "********** Error End *  *********" )
                    sys.excepthook(type, value, traceback)

                self.payload[1] = False
                self.finished.emit()
                print("Run DONE")
            sleep(0.05)

    def SetFunctionToRunAndRun(self, func,callback = None):
        if self.payload[1] or self.payload[3]:
            print("Action already running please Wait")
            return
        self.payload = [func, True, callback, callback is not None ]

    def RunCallback(self):
        """ CallBack function is allways run in the main thread
        """
        if self.payload is not None and self.payload[3] :
            if self.payload[2] is not None:
                self.payload[2]()
            self.payload[3] = False

    def wait(self):
        while(self.payload[1] or self.payload[3]):
            pass


def CheckIntegrity():
    # strange !!! coverage unable to track inside the QT library
    def TestFunc(size):# pragma: no cover
        for n in range(size):
            print(n)

    # strange !!! coverage unable to track inside the QT library
    def PrintCoucou():# pragma: no cover
        print("coucou")

    import sys
    from threading import Thread

    obj = Worker()
    QtThread = Thread(target=obj.WaitForActions)
    QtThread.start()

    from functools import partial

    obj.SetFunctionToRunAndRun(partial(TestFunc,5) )
    obj.SetFunctionToRunAndRun(PrintCoucou)

    obj.wait()
    obj.SetFunctionToRunAndRun(PrintCoucou)
    obj.wait()
    obj.exiting = True
    return "ok"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
