# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations
from typing import Dict

import Muscat.Helpers.ParserHelper as PH

from OpenPisco.Actions.ActionFactory import RegisterClass
from OpenPisco.Actions.ActionBase import ActionBase
from OpenPisco.CLApp.OpenPiscoCL import MainApp
from OpenPisco import RETURN_SUCCESS

class RunAlgorithm(ActionBase):
    """
    Run topology optimisation problem

    Parameters
    ----------
        TopoOp: topology optimization problem

    Example in XML:
    <Action type="RunTopoOp" TopoOp="1" />

    Example in python:
    action = RunAlgorithm()
    action.Apply(app=None, ops={"TopoOp":topologyOptimizationObject)
    """
    def __init__(self):
       super(RunAlgorithm,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):
       return app.optimAlgos[PH.ReadInt(ops['TopoOp'])].Start()


RegisterClass("RunTopoOp",RunAlgorithm)


class ResetAlgorithm(ActionBase):
    def __init__(self):
       super(ResetAlgorithm,self).__init__()

    def Apply(self, app:MainApp, ops:Dict):

       app.optimAlgos[int(ops['TopoOp'])].ResetState()

       return RETURN_SUCCESS


RegisterClass("ResetTopoOp",ResetAlgorithm)

def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())
