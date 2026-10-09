# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

from functools import partial

from PySide6 import QtCore, QtGui
from PySide6.QtCore import QThread
from PySide6.QtCore import QRegularExpression
from PySide6.QtCore import Signal
from PySide6.QtCore import Slot
from PySide6.QtCore import QObject
from PySide6.QtCore import QEvent
from PySide6.QtCore import Qt
from PySide6.QtCore import QTimer
from PySide6.QtCore import QSettings
#from PySide6.QtCore import FramelessWindowHint
from PySide6.QtCore import QFileInfo
from PySide6.QtCore import QFile
from PySide6.QtCore import QTextStream
from PySide6.QtCore import QSize

from PySide6.QtGui import QIcon
from PySide6.QtGui import QPixmap
from PySide6.QtGui import QColor
from PySide6.QtGui import QTextCharFormat
from PySide6.QtGui import QFont
from PySide6.QtGui import QSyntaxHighlighter
from PySide6.QtGui import QPalette
from PySide6.QtGui import QAction  # <-- Added here

from PySide6.QtWidgets import QApplication
from PySide6.QtWidgets import QMainWindow
from PySide6.QtWidgets import QTabWidget
from PySide6.QtWidgets import QWidget
from PySide6.QtWidgets import QGridLayout
from PySide6.QtWidgets import QComboBox
from PySide6.QtWidgets import QLabel
from PySide6.QtWidgets import QPushButton
from PySide6.QtWidgets import QSizePolicy
from PySide6.QtWidgets import QSlider
from PySide6.QtWidgets import QCheckBox
from PySide6.QtWidgets import QScrollArea
from PySide6.QtWidgets import QDockWidget
from PySide6.QtWidgets import QTextEdit
from PySide6.QtWidgets import QPlainTextEdit
from PySide6.QtWidgets import QFileDialog
from PySide6.QtWidgets import QDialog
from PySide6.QtWidgets import QInputDialog
from PySide6.QtWidgets import QLineEdit
from PySide6.QtWidgets import QFrame
from PySide6.QtWidgets import QHBoxLayout
from PySide6.QtWidgets import QVBoxLayout
from PySide6.QtWidgets import QMessageBox


# Fix: Ensure backward compatible enum structures for PySide6
stateOn = QIcon.State.On
stateOff = QIcon.State.Off

# Fix: Patch a well-known VTK bug where Qt6 renames 'MidButton' to 'MiddleButton'
if not hasattr(Qt, "MidButton"):
    Qt.MidButton = Qt.MiddleButton

import vtk.qt
vtk.qt.PyQtImpl = "PySide6"
from vtk.qt.QVTKRenderWindowInteractor import QVTKRenderWindowInteractor

def GetQApplication(argv):
    if not QApplication.instance():
        return QApplication(argv)
    else:
        return QApplication.instance()

def CheckIntegrity():
    print(QtCore.__file__)
    return 'ok'

if __name__ == '__main__': # pragma: no cover
    print(CheckIntegrity())
