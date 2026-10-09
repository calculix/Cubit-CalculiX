# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#


from Muscat.Helpers.Logger import Info
import Muscat.MeshContainers.ElementsDescription as ED
MEDName = {}

try:
    import med.medmesh
    import med.medfamily as medfamily
    import med.medfield
    import med.medlocalization


    MEDName[ED.Point_1] = medfamily.MED_POINT1
    MEDName[ED.Bar_2]         = medfamily.MED_SEG2
    MEDName[ED.Bar_3]         = medfamily.MED_SEG3
    MEDName[ED.Triangle_3]    = medfamily.MED_TRIA3
    MEDName[ED.Triangle_6] = medfamily.MED_TRIA6
    MEDName[ED.Quadrangle_4]  = medfamily.MED_QUAD4
    MEDName[ED.Quadrangle_8]  = medfamily.MED_QUAD8
    MEDName[ED.Tetrahedron_4] = medfamily.MED_TETRA4
    MEDName[ED.Tetrahedron_10] = medfamily.MED_TETRA10
    MEDName[ED.Hexahedron_8]   = medfamily.MED_HEXA8
    MEDName[ED.Hexahedron_20]   = medfamily.MED_HEXA20
    MEDAvailable = True
except Exception as e:
    Info("Unable to import med library")
    Info(e)
    MEDAvailable = False

ElementNamesbyMedName = {v:k for k,v in MEDName.items()}

class MedLocalizationBase(object):
    def __init__(self,name="",refcoordinates=None,rule=None):
        self.name= name
        self.refcoordinates = refcoordinates
        self.rule = rule
    def GetLocalizationName(self):
        return self.name
    def GetNodesCoordinates(self):
        return self.refcoordinates
    def GetRule(self):
        return self.rule

class MedLocalizationTetrahedronLinear(MedLocalizationBase):
    def __init__(self):
        super(MedLocalizationTetrahedronLinear,self).__init__()
        self.name= "TE4_____FPG1"
        self.refcoordinates = [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0]
        from Muscat.FE.IntegrationRules import LagrangeP1Quadrature
        self.rule = LagrangeP1Quadrature[ED.Tetrahedron_4]

class MedLocalizationTriangleLinear(MedLocalizationBase):
    def __init__(self):
        super(MedLocalizationTriangleLinear,self).__init__()
        self.name= "TR3_____FPG1"
        self.refcoordinates = [0.0,0.0, 1.0, 0.0, 0.0, 1.0]
        from OpenPisco.MuscatExtentions.AsterIntegrationsRules import AsterLagrangeP1
        self.rule = AsterLagrangeP1[ED.Triangle_3]

class MedLocalizationTetrahedron4PointsLinear(MedLocalizationBase):
    def __init__(self):
        super(MedLocalizationTetrahedron4PointsLinear,self).__init__()
        self.name= "TE4_____FPG4"
        self.refcoordinates = [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0]
        from OpenPisco.MuscatExtentions.AsterIntegrationsRules import AsterTetrahedron4PointsLagrangeP1
        self.rule = AsterTetrahedron4PointsLagrangeP1[ED.Tetrahedron_4]

class MedLocalizationTetrahedronQuadratic(MedLocalizationBase):
    def __init__(self):
        super(MedLocalizationTetrahedronQuadratic,self).__init__()
        self.name= "T10_____FPG5"
        self.refcoordinates = [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5,  0.0,0.5, 0.5, 0.5, 0.5, 0.0, 0.5, 0.0, 0.0, 0.5, 0.0, 0.5]
        from OpenPisco.MuscatExtentions.AsterIntegrationsRules import AsterLagrangeP2
        self.rule = AsterLagrangeP2[ED.Tetrahedron_10]

class MedLocalizationTriangleQuadratic(MedLocalizationBase):
    def __init__(self):
        super(MedLocalizationTriangleQuadratic,self).__init__()
        self.name= "TR6_____FPG3"
        self.refcoordinates = [0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.5, 0.0, 0.5, 0.5, 0.0, 0.5]
        from OpenPisco.MuscatExtentions.AsterIntegrationsRules import AsterLagrangeP2
        self.rule = AsterLagrangeP2[ED.Triangle_6]

from collections import defaultdict
def OptionObjGenerator():
    res = dict()
    for opt in [1,4,5]:
        res[opt]  = None
    return res

MEDLocalizationbyElementName = defaultdict(OptionObjGenerator)
MEDLocalizationbyElementName[ED.Tetrahedron_4][1] = MedLocalizationTetrahedronLinear()
MEDLocalizationbyElementName[ED.Tetrahedron_4][4] = MedLocalizationTetrahedron4PointsLinear()
MEDLocalizationbyElementName[ED.Tetrahedron_10][5] = MedLocalizationTetrahedronQuadratic()
MEDLocalizationbyElementName[ED.Triangle_3][1] = MedLocalizationTriangleLinear()
MEDLocalizationbyElementName[ED.Triangle_6][3] = MedLocalizationTriangleQuadratic()



MEDComponentsNames = [ "_xx", "_yy", "_zz", "_xy","_xz","_yz"]


def CheckIntegrity():
    return "ok"
