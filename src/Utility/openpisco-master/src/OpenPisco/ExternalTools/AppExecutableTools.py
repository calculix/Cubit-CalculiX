# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import os
import subprocess
import shutil
from typing import Iterable

from Muscat.Helpers.Logger import Info, ForcePrint
from Muscat.IO import MeshReader, MeshWriter
from Muscat.Helpers.IO.Which import Which
from Muscat.Helpers.IO.TemporaryDirectory import TemporaryDirectory

class AppExecutableBase():
    """
    Base class for a file-exchange-based interface to external tools.

    Parameters
    ----------
    app_name : str
        Human-readable name of the tool.
    """
    def __init__(self, app_name):
        super(AppExecutableBase, self).__init__()
        self.workingDirectory = None
        self.writer = MeshWriter.MeshWriter()
        self.reader = MeshReader.MeshReader()
        self.app_name = Which(app_name)
        if self.app_name is None:
            raise RuntimeError(f'{app_name} not found')
        self.filename = ""

    def SetFilename(self,filename):
        self.filename = filename

    def GetAppName(self):
        return self.app_name

    def GetWorkingDirectory(self):
        """
        Get the absolute path to the directory where temporary files will be
        located.

        If the location is not yet set, a temporary directory is created
        beforehand.
        """
        if self.workingDirectory is None:
            self.SetWorkingDirectory(None)
        return self.workingDirectory

    def SetWorkingDirectory(self, wd):
        if wd is None:
            self.workingDirectory = TemporaryDirectory.GetTempPath()
            Info(" ".join( \
                    ("All temporary files are located in", \
                    self.workingDirectory)))
        else:
            self.workingDirectory = wd

    def GetBinaryFlag(self):
        return self.writer.isBinary()

    def SetBinaryFlag(self, value):
        """
        Parameters
        ----------
        value : bool
            It `True`, binary files are used. Otherwise, ASCII files are used.
        """
        self.writer.SetBinary(value)

    def CopyFile(self,sourceFilePath,targetFilePath):
        sourceFilePath = self._filePath(sourceFilePath, binary=False)
        targetFilePath = self._filePath(targetFilePath, binary=False)
        shutil.copyfile(sourceFilePath, targetFilePath)

    def DeleteDirectories(self,directoriesToDelete:Iterable):
        for directoryToDelete in directoriesToDelete:
            try:
                directoryToDelete = self._filePath(directoryToDelete, binary=False)
                if os.path.isdir(directoryToDelete):
                    shutil.rmtree(directoryToDelete)
            except OSError as e:
                print("Error: %s - %s." % (e.filename, e.strerror))

    def DeleteFiles(self,filesToDelete:Iterable):
        for fileToDelete in filesToDelete:
            try:
                filetodelete = self._filePath(fileToDelete, binary=False)
                if os.path.isfile(filetodelete):
                    os.remove(filetodelete)
            except Exception as e:
                ForcePrint("Error deleting file " + fileToDelete+"( "+e+" )")

    def RenameFiles(self, filesToRename: Iterable, suffix="_old"):
        for fileToRename in filesToRename:
            Info("Rename " + fileToRename)
            try:
                file_path = self._filePath(fileToRename, binary=False)
                if os.path.isfile(file_path):
                    new_file_path = file_path + suffix
                    os.rename(file_path, new_file_path)
            except Exception as e:
                ForcePrint("Error renaming file " + fileToRename + " (" + str(e) + " )")
                
    def _executeCommand(self, cmd):
        out = self._runCommand(cmd)
        self._checkCommand(out)

    def _runCommand(self,cmd):
        Info("Launching " + self.GetAppName())
        Info("Working Directory : " + self.GetWorkingDirectory())
        Info(cmd)

        try :
            out = self._executeSubprocess(cmd)
        except Exception as inst:
            ForcePrint("Error in the " + self.GetAppName() + " process: ")
            Info(inst.output)
            raise inst
    
        Info("Launching " + self.GetAppName() +" done")
        return out

    def _checkCommand(self,out):
        if out.find("problem") != -1:
            ForcePrint("Error in the " + self.GetAppName() + " process: ")
            ForcePrint(out)
        else:
            Info(out)
    

    def _executeSubprocess(self, cmd):
        return subprocess.run(cmd, cwd=self.GetWorkingDirectory(), shell=False, check=True, capture_output=True).stdout.decode("utf-8",errors="ignore")

    def _filePath(self, base_filename, binary=None):
        separator = "/"
        directory = self.GetWorkingDirectory().rstrip(separator)
        filename = self._fileName(base_filename, binary)
        return separator.join((directory, filename)) if directory else filename

    def _fileName(self, base_filename, binary=None):
        if binary is None:
            binary = self.GetBinaryFlag()
        return base_filename + "b" if binary else base_filename


class CommandLineBuilder(object):
    """
    Simple helper class to build command lines.

    Parameters
    ----------
    executable_name : str
        The name of the executable file, prepended to the command line.
    """
    def __init__(self, executable_name):
        self._components = [executable_name]

    def optionAdd(self, keyword, prefix="-", arg=None):
        """
        Append an option to the command line

        Parameters
        ----------
        keyword : str
            Textual keyword identifying the option.
        prefix : str, optional
            The symbol(s) intoducing the option (default is a single hyphen).
        arg : str, optional
            Argument appended to the option.
        """
        optionComponent = "".join((prefix, keyword))
        if arg is None:
            self._components.append(optionComponent)
        else:
            self._components.extend((optionComponent, arg))

    def argumentAdd(self, arg):
        """
        Append an argument to the command line

        Parameters
        ----------
        arg : str
        """
        self._components.append(arg)

    def result(self):
        """
        Returns
        -------
        str
            The command line
        """
        return self._components

def CheckIntegrity(GUI=False):
    return "OK"

if __name__ == '__main__':
    print(CheckIntegrity(True))# pragma: no cover

