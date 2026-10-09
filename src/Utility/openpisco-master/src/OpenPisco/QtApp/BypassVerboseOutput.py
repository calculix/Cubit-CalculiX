# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import re

from Muscat.Helpers.Logger import GetDiffTime

import OpenPisco.QtApp.QtImplementation as QT

COLOR_REGEX_FILTER = re.compile(r'\x1b\[(?P<arg_1>\d+)(;(?P<arg_2>\d+)(;(?P<arg_3>\d+))?)?m')

class ToConsole(QT.QtCore.QObject):
    """
    Splits the output stream into 2 streams, one stream with all lines with a leading
    arrow and the other stream with the other lines.

    Only lines without leading arrows ("->") are printed out on the Qt object.
    """

    signalWrite = QT.Signal(str)

    def __init__(self, qtTextEdit):
        super().__init__()

        if qtTextEdit == None:
            return
        from  Muscat.Helpers.PrintBypass import PrintBypass as PB
        self.pb = PB()
        self.pb.ToRedirect(self)
        self.qtTextEdit = qtTextEdit
        self.signalWrite.connect(self.update_text)
        self.color = True
        self.logFileMain = None
        self.logFileSecond = None
        self.cpt = 0

    def close(self):
        pass

    def flush(self):
        pass

    def detach(self):
        self.pb.Restore()
        if self.logFileMain is not None:
            self.logFileMain.close()
        if self.logFileSecond is not None:
            self.logFileSecond.close()

    def OpenOutputDuplicationFiles(self, path):
        import os
        self.logFileMain = open(path+os.sep+"OpenPisco.Main.txt","w")
        self.logFileSecond = open(path+os.sep+"OpenPisco.Second.txt","w")

    def write(self, data):
        if data.find("-> ") != -1:
            self.pb.stdout_.write(data)
            if self.logFileSecond is not None:
                self.logFileSecond.write(COLOR_REGEX_FILTER.sub('', data) )
                self.logFileSecond.flush()
            return

        #self.emit(QtCore.SIGNAL("write"), data)
        self.signalWrite.emit( data)
        #self.signalWrite.emit( str(self.cpt) +")"  +data)
        if self.logFileMain is not None :
            if len(data) > 1:
                self.logFileMain.write(("[%f] " % GetDiffTime()))
                self.cpt += 1
                #if self.cpt == 100:
                #    raise

            self.logFileMain.write(COLOR_REGEX_FILTER.sub('', data))
            self.logFileMain.flush()

    def update_text(self, value1):
        if len(value1) ==0 :
            return
        if self.qtTextEdit is None:
            return
        self.qtTextEdit.moveCursor(QT.QtGui.QTextCursor.End)
        self.qtTextEdit.ensureCursorVisible()

        #self.qtTextEdit.insertPlainText(str(value1))
        from OpenPisco.QtApp.AnsiToHtmlColors import ansi_to_html as AtH
        import html
        self.qtTextEdit.insertHtml('<pre>'+AtH(html.escape( value1  ) )+"</pre>")

    def Clean(self):
        if self.qtTextEdit is None:
            return
        self.qtTextEdit.setText("")


def CheckIntegrity():
    from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory

    import sys
    QT.GetQApplication(sys.argv)
    view = QT.QMainWindow()
    textEdit = QT.QTextEdit(view)
    tc = ToConsole(textEdit)
    tc.OpenOutputDuplicationFiles(TemporaryDirectory.GetTempPath())
    print("coucou I")
    print("-> coucou II")
    tc.Clean()
    tc.detach()

    print("coucou III")
    print("-> coucou IV")
    return 'ok'

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
