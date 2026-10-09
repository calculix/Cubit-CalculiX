# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from abc import ABC, abstractmethod
import os
from typing import Dict,Union

import Muscat.Helpers.ParserHelper as PH
from Muscat.Helpers.IO.PathController import PathController


class InputReaderBase(ABC):
    def __init__(self):
        self.ext = ""

    def ReadFromFile(self, filename:Union[str, os.PathLike])->any:
        """Read input data from a file

        Parameters
        ----------
        filename : Union[str, os.PathLike]
            file path

        Returns
        -------
        any
            parsed input data
        """
        data = open(filename).read()
        self.filenamePath = PathController.GetFullPathCurrentDirectoryUsingFile(filename)
        return self.ReadFromString(data)

    @abstractmethod
    def ReadFromString(self,data:str):
        """Define DSL parsing strategy, to be redefined in the derived class

        Parameters
        ----------
        data : str
            data to parse using appropriate strategy
        """
        pass

    def ReplaceObjectFromAlmanac(self, dic:Dict, almanac:any):
        """Replace specific word by their equivalent in the DSL terminology

        Parameters
        ----------
        dic : Dict
            parsed data
        almanac : any
            structure to operate the replacement on
        """

        keysToReplace = [
            ("useLevelset", "ls"),
            ("levelset", "ls"),
            ("useProblem", "problem"),
            ("useProblems", "problems"),
            ("useOptimProblem", "optimProblem"),
            ("output", "writer"),
            ("z", "Zones"),
            ("CriteriaOffZone", "zone"),
            ("nonOptimSurfZone","zone"),
        ]
        keytomapscalar = [("grid", "grids"), ("support", "grids"), ("zone", "zones"), ("ls", "levelSets"), ("problem", "physicalProblems"), ("optimProblem", "optimProblems"), ("writer", "outputs")]

        keytomapvector = [("useProblems", "physicalProblems"), ("Zones", "zones")]

        for f, t in keysToReplace:
            if f in dic:
                dic[t] = dic[f]
                del dic[f]

        for what, where in keytomapscalar:
            if what in dic:
                try:
                    ID = PH.ReadInt(dic[what])
                except:
                    print("id must be a int not : " + str(dic[what]))
                    raise

                try:
                    dic[what] = almanac.__dict__[where][ID]
                except:
                    print("Impossible to find object in group '" + str(where) + "'  with id " + str(ID))
                    raise

        for what, where in keytomapvector:
            if what in dic:
                z = PH.ReadInts(dic[what])
                dic[what] = [almanac.__dict__[where][x] for x in z]

        if "children" in dic:
            for _, dat in dic["children"]:
                self.ReplaceObjectFromAlmanac(dat, almanac)



def CheckIntegrity(GUI=False):
    return "ok"


if __name__ == "__main__":
    CheckIntegrity(GUI=True)
