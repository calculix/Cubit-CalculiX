# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
"""XML DSL implementation"""
import os
from typing import Optional

import xml.etree.ElementTree as ET
import xml.etree.ElementInclude as EI

from OpenPisco.CLApp.InputReaderBase import InputReaderBase
from Muscat.Helpers.IO.PathController import PathController

class XmlToDic(InputReaderBase):
    def __init__(self):
        super().__init__()
        self.ext = ".xml"

    def ReadFromString(self,data:str):
        # parser to include automaticaly the xinclude namespace if not present
        class parser(ET.XMLParser):
            def __init__(self):
                super().__init__()

            def feed(self,data):
                if data.find("xmlns:xi") == -1:
                    data = data.replace('<data ', '<data xmlns:xi="http://www.w3.org/2001/XInclude" ')
                if data.find("&") != -1:
                    data = data.replace('&amp;', '&')
                    data = data.replace('&', '&amp;')
                super(parser,self).feed(data)

        root = ET.fromstring(data, parser=parser())

        # resolution of the xinclude using the correct filename path
        def loader(href:str, parse):
            href = os.path.join(self.filenamePath,href)
            href = PathController.GetFullFilenameCurrentDirectory(href)
            return EI.default_loader(href, parse)

        EI.include(root,loader)
        return self.XmlToDic(root)

    def XmlToDic(self,ops):
      res = {}
      for k,v in ops.attrib.items():
          res[k] = v

      if ops.text is not None and len(ops.text.strip()):
              res["CDATA"] = ops.text

      children = []

      for child in ops:
          children.append(self.XmlToDic(child) )

      if len(children):
          res["children"] =  children
      return (ops.tag, res)

def ConvertFromXmlToPiscoFormat(xmldata):
    root = ET.fromstring(xmldata)

    def toPiscoFormat(data,tag=None,indent=0):
        res = "*"*indent
        if tag is not None:
            res += str(tag) + " "

        for cpt,i in enumerate(data.attrib):
            if cpt :
                res += ", "
            res += i + "=" + str(data.attrib[i])
        res += '\n'

        cpt = None
        for cpt,i in enumerate(data):
            res += toPiscoFormat(i,i.tag,indent+1)

        if cpt is not None and len(res) and res[-2] != "\n":
            res += '\n'
        return res
    res = toPiscoFormat(root)
    return res

def ConvertToXml(dic,name:Optional[str]=None,indent:int=0):
    if name is None:
        res = "<data"
    else:
        res = " "*indent*2+ "<" + str(name)
    cdatapresent =False
    lres = ""
    for item,data in dic.items():
        if item == "CDATA":
            cdatapresent = True
            continue

        if item not in ["children", "CDATA"]:
            lres += " " + str(item ) + '="' + str(data) + '"'
        if len(lres) > 80:
            res += lres + "\n"
            lres = " "*indent*2
    res += lres

    if "children" not in dic and "CDATA" not in dic:
        res += ' />\n'
        return res
    res += '>'
    if not cdatapresent:
        res += '\n'

    for item,data in dic.items():
        if item == "children" :
            for subname,subdata in data:
                res += ConvertToXml(subdata,subname,indent+1)
        elif item == "CDATA" :
            res += "<![CDATA["
            res += data
            res += "]]>"

    if name is not None:
        if cdatapresent:
           res += "</"+name+">\n"
        else:
           res += " "*indent*2+"</"+name+">\n"
    else:
        res += "</data>\n"
    if indent == 1 :
            res += "\n"
    return res

def CheckIntegrity(GUI=False):
    from  OpenPisco.TestData import GetTestDataPath
    import os
    filename = GetTestDataPath() + os.sep + "TestInput.xml"

    data = open(filename).read()
    ConvertFromXmlToPiscoFormat(data)

    reader = XmlToDic()

    _, data = reader.ReadFromFile(filename)
    string = ConvertToXml(data)
    _, data2 = reader.ReadFromString(string)

    if str(data) != str(data2):
        print(data)
        print(data2)
        print("error in the conversion")
        return "not ok "

    str_to_run =  '<Action type="Repeate" times="1"> <Action type="ResetTopoOp" TopoOp="IDD" /><Action type="RunTopoOp" TopoOp="IDD" /></Action >'
    data = reader.ReadFromString(str_to_run )
    print(data)
    print(str(data))
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity(GUI=True))
