# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
import sys,os

from OpenPisco.CLApp.XmlToDic import XmlToDic,ConvertToXml
from OpenPisco.CLApp.PiscoToDic import PiscoToDic,ConvertToPisco

if __name__ == '__main__':
    inputfilename = sys.argv[1]
    file = open(inputfilename)
    filename, file_extension = os.path.splitext(inputfilename)
    if file_extension == ".xml":
        inputReader =  XmlToDic()
        outputfunction = ConvertToPisco
        outputfilename = filename + ".pisco"

    elif file_extension == ".pisco":
        inputReader =  PiscoToDic()
        outputfunction = ConvertToXml
        outputfilename = filename + ".xml"

    tag, data = inputReader.ReadFromString(file.read())
    print(data)
    print("writing data to : "+outputfilename)
    file2 = open(outputfilename,"w")
    file2.write(outputfunction(data))
