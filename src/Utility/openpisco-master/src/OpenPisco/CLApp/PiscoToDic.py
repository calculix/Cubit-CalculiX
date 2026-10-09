# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
"""PISCO DSL implementation"""
from typing import Dict
from OpenPisco.CLApp.InputReaderBase import InputReaderBase

class PiscoToDic(InputReaderBase):
    def __init__(self):
        super(PiscoToDic,self).__init__()
        self.ext = ".pisco"

    def ReadFromString(self,data:str):
        from Muscat.IO.ReaderBase import ReaderBase

        internalReader = ReaderBase()
        internalReader.commentChar = "#"
        internalReader.SetStringToRead(data)
        internalReader.StartReading()
        res = {}

        def lineToDic(line:str):
          try:
            fsindex = line.find(' ')
            if line[0] != "*"  :
                key = None
                nstars = None
                rest = line.split(",")
            else:
                if fsindex == -1:
                    key = line.strip()
                    nstars = key.count("*")
                    key = key.strip("*")
                    rest = []
                else:
                    key = line[0:fsindex].strip()
                    nstars = key.count("*")
                    key = key.strip("*")
                    rest = line[fsindex+1:].split(",")

            res = {}

            for item in rest:
                if len(item) == 0 :continue
                try:
                    k,d = item.split("=")
                    res[k.strip()]=d.strip()
                except:
                    print("Error : string  '" +str(item) +"' not understood (line "+str(internalReader.lineCounter)+") ")
                    raise

            while True:
                line = internalReader.ReadCleanLine()
                if line is None:
                    break
                if line[0] == "*":
                    break
                (_,_,resI,line) = lineToDic(line)
                res.update(resI)
                if line is None:
                    break
                if line[0] == "*":
                    break

            return (nstars,key,res,line)
          except Exception as e:
              print("Error : string  '" +str(line) +"' not understood (line "+str(internalReader.lineCounter)+") ")
              raise e


        currentRes = [res]
        currentstar = [0]
        a = internalReader.ReadCleanLine()
        while True:
            if a is None:
                break
            nstars,key,r,a = lineToDic(a)

            if key is None:
                currentRes[-1].update(r)
            else:
                if nstars > currentstar[-1]:
                    l = currentRes[-1].get("children",[])
                    currentRes[-1]["children"] = l
                    l.append((key,r))
                    currentRes.append(r)
                    currentstar.append(nstars)
                elif nstars == currentstar[-1]:
                    l = currentRes[-2].get("children",[])
                    currentRes[-2]["children"] = l
                    l.append((key,r))
                else:
                    while(nstars <= currentstar[-1]):
                        currentRes.pop()
                        currentstar.pop()
                    l = currentRes[-1].get("children",[])
                    currentRes[-1]["children"] = l
                    l.append((key,r))
                    currentRes.append(r)
                    currentstar.append(nstars)

        internalReader.EndReading()
        return ("data",res)

def ConvertToPisco(dic:Dict,stars:int=1,indent:int=0):
    res = ""
    lres = ""
    cpt = len(dic) - int("children" in dic)
    for item,data in dic.items():
        if item != "children" :
            lres += str(item ) + "=" + str(data)
            cpt -= 1
        else:
            continue


        if len(lres)+stars+indent > 80:
            if cpt :
                lres += ","
            res += lres + "\n"
            if cpt :
                lres = " "*(stars+indent)
        else:
            if cpt :
                lres += ", "

    res += lres
    res += '\n'

    for item,data in dic.items():
        if item == "children" :
            for subname,subdata in data:
                res += "*"*stars + subname
                if len(subdata)-int("children" in dic) > 0 :
                    res += " "
                res += ConvertToPisco(subdata,stars+1,indent=len(subname))
        if stars == 2 :
            res += "\n"

    return res

def CheckIntegrity(GUI=False):

    from  OpenPisco.TestData import GetTestDataPath
    import os
    filename = GetTestDataPath() + os.sep + "TestInput.pisco"
    reader = PiscoToDic()

    _, data = reader.ReadFromFile(filename)
    string = ConvertToPisco(data)
    _, data2 = reader.ReadFromString(string)

    if str(data) != str(data2):
        print(data)
        print(data2)
        print("error in the conversion")
        return "not ok "

    string= """**Action type=InitLevelset, ls=1, zone=3
**Action type=RunTopoOp, TopoOp=1"""
    print(reader.ReadFromString(string))

    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity(GUI=True))
