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
from OpenPisco import RETURN_SUCCESS, RETURN_FAIL
from OpenPisco.CLApp.OpenPiscoCL import MainApp

class RepeateAction(ActionBase):
    def __init__(self):
       super(RepeateAction,self).__init__()

    def Apply(self,app:MainApp, ops:Dict):
        times = PH.ReadInt(ops.get('times',1))
        for _ in range(times):
            for chil in ops["children"]:
                assert chil[0] == "Action","Don't know how to treat this object" + str(chil[0])
                retVal = app.RunAction(chil[1])
                if retVal != RETURN_SUCCESS:
                    return retVal
        return RETURN_SUCCESS

RegisterClass("Repeate",RepeateAction)

class TryAction(ActionBase):
    def __init__(self):
        super(TryAction,self).__init__()

    def Apply(self,app:MainApp, ops:Dict):
        for _, chil in ops["children"]:
            if app.RunAction(chil) == RETURN_SUCCESS:
                return RETURN_SUCCESS
        return RETURN_FAIL

RegisterClass("Try",TryAction)

class SetAction(ActionBase):
    def __init__(self):

        super(SetAction,self).__init__()

    def Apply(self,app:MainApp, ops:Dict):
        if "exec" in ops:
            exec(ops["exec"],{'mainApp':app})
            return RETURN_SUCCESS
        else:
            on = ops['on']
            oid = int(ops['id'])
            var = ops['var']
            val = ops['val']
            app.__dict__[on][oid].__dict__[var] = PH.Read(val, app.__dict__[on][oid].__dict__[var])
            return RETURN_SUCCESS

RegisterClass("Set",SetAction)

def CheckIntegrity():
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity())

