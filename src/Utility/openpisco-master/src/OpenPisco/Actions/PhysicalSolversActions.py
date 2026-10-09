# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations
from typing import Dict

import Muscat.Helpers.ParserHelper as PH
from Muscat.IO.IOFactory import CreateWriter,InitAllWriters

from OpenPisco import RETURN_SUCCESS
from OpenPisco.Actions.ActionBase import ActionBase
from OpenPisco.Actions.ActionFactory import RegisterClass
import OpenPisco.PhysicalSolvers.FieldsNames as FN
from OpenPisco.CLApp.OpenPiscoCL import MainApp

InitAllWriters()


class SolveByLevelSetAction(ActionBase):
    """
    Solve physical problem with a levelset as an input.

    Parameters
    ----------
        ls: levelset
        problem: physical problem
        postFields: postprocessing fields (default "")
        on: posprocessing field location (default "")

    Example in XML:
    <Action type="SolveByLevelSet" ls="1" problem="1" />

    Example in python:
    action = SolveByLevelSetAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"problem":physicalProblemInstance)
    """
    def __init__(self):
        super(SolveByLevelSetAction,self).__init__()
    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        problem = ops['problem']
        extraFieldNames, on = PH.ReadStrings(ops.get("postFields","")), PH.ReadStrings(ops.get("on",""))
        assert len(extraFieldNames)==len(on)

        for extrafieldName,location in zip(extraFieldNames, on):
            problem.SetAuxiliaryFieldGeneration(extrafieldName,location)

        problem.SolveByLevelSet(ls)
        if 'filename' in ops:
            filename = ops['filename']
            writer = CreateWriter(filename )
            writer.SetFileName(filename )
            writer.SetTemporal()
            writer.SetBinary(False)
            writer.SetHdf5(True)
            writer.automaticOpen = True
            PointFields =  [ls.phi, problem.GetNodalSolution()]
            PointFieldsNames=["phi", "solutions"]

            for extrafieldName,location in zip(extraFieldNames, on):
                if location==FN.Nodes:
                   PointFields.append(problem.GetAuxiliaryField(extrafieldName,location))
                   PointFieldsNames.append(extrafieldName)

            writer.Write(ls.support,PointFields=PointFields,PointFieldsNames=PointFieldsNames)
            writer.Close()

        return RETURN_SUCCESS

RegisterClass("SolveByLevelSet", SolveByLevelSetAction)


def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
