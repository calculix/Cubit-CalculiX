# -*- coding: utf-8 -*-

from Muscat.ImplicitGeometry.ImplicitGeometryBase import ImplicitGeometryBase
from Muscat.ImplicitGeometry.ImplicitGeometryFactory import RegisterClass

class DummyImplicitGeometry(ImplicitGeometryBase):
    def __init__(self):
        super(DummyImplicitGeometry,self).__init__()

    def GetDistanceToPoint(self, pos):
        return 0.

RegisterClass("Dummmy",DummyImplicitGeometry)

def MyLevelSetCreationFunctions(mainapp, data):
    userFunctionName = data['function']
    CDATA = data['CDATA']
    userData = {k:v for k,v in data.items() if k not in ['function', 'CDATA']}
    if userFunctionName == "GridHomeMade":
        dict_data = mainapp.TextToDict(CDATA)
        for i in range(int(userData['someUserArg'])):
            dict_copy = dict_data.copy()
            dict_copy["id"] = 2+i
            mainapp.ReadGrid("Grid",dict_copy)

from Muscat.Helpers.LocalVariables import AddToGlobalDictionary
AddToGlobalDictionary("GridHomeMade",MyLevelSetCreationFunctions)
AddToGlobalDictionary("MyFunc",MyLevelSetCreationFunctions)
AddToGlobalDictionary("HomeMadePhysicalProblems",MyLevelSetCreationFunctions)
