# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations
from typing import Dict

from Muscat.IO.IOFactory import CreateWriter,InitAllWriters

from OpenPisco import RETURN_SUCCESS
from OpenPisco.Actions.ActionBase import ActionBase
from OpenPisco.Actions.ActionFactory import RegisterClass
from OpenPisco.CLApp.OpenPiscoCL import MainApp

InitAllWriters()

class UpdateCriteriaValuesAction(ActionBase):
    """
    Update optimization criteria values (function and sensitivity)

    Parameters
    ----------
        ls: levelset
        optimProblem: optimization problem
        saveToFile: string, export path for mesh endowed with each respective criteria sensitivity of the optimization problem

    Example in XML:
    <Action type="UpdateCriteriaValues" ls="1" optimProblem="1" saveToFile="path/to/result.xmf"/>

    Example in python:
    action = UpdateCriteriaValuesAction()
    action.Apply(app=None, ops={"ls":levelsetObject,"optimProblem":optimProblem,"saveToFile":"path/to/result.xmf")
    """
    def __init__(self):
        super(UpdateCriteriaValuesAction,self).__init__()
    def Apply(self, app:MainApp, ops:Dict):
        ls = ops['ls']
        optimProblem = ops['optimProblem']
        optimProblem.UpdateValues()

        if 'saveToFile' in ops:
            filename = ops['saveToFile']
            writer = CreateWriter(filename )
            writer.SetFileName(filename )
            writer.SetTemporal()
            writer.SetBinary(False)
            writer.SetHdf5(True)
            writer.automaticOpen = True
            pointFields =  [ls.phi,optimProblem.objectiveCriterion.GetSensitivity()]
            pointFieldsNames=['phi',"O_"+optimProblem.objectiveCriterion.GetName()]

            for criterion in optimProblem.GetConstraintsCriteria():
                pointFields.append(criterion.GetSensitivity())
                pointFieldsNames.append("C_"+criterion.GetName())

            writer.Write(ls.support,PointFields=pointFields,PointFieldsNames=pointFieldsNames)
            writer.Close()

        return RETURN_SUCCESS

RegisterClass("UpdateCriteriaValues",UpdateCriteriaValuesAction)


def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
