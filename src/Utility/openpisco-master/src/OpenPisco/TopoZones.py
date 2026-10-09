# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#



#to use this file
#import OpenPisco.TopoZones as TZ

import Muscat.IO.MeshTools as MT

Corners           = MT.Corners
Ridges            = MT.Ridges
RequiredVertices  = MT.RequiredVertices
RequiredEdges     = MT.RequiredEdges
RequiredTriangles = MT.RequiredTriangles

MMG_ISO = 10

MG_PLUS  = 2
MG_MINUS = 3

MMG5_Notype = 0;# < Undefined type (unusable)
MMG5_Scalar = 1;# < Scalar solution
MMG5_Vector = 2;# < Vectorial solution
MMG5_Tensor = 3;# < Tensorial solution


Bulk = "Bulk"
ALWAYS  = "Always"
NEVER  = "Never"

Inside3D = "Inside3D"
Outside3D = "Outside3D"

InterSurf = "InterfaceSurf"
InterBars = "InterfaceBars"

ExterBars = "ExteriorBars"
ExterSurf = "ExteriorSurf"

isoNegative = "isoNegative"
isoPositive = "isoPositive"
isoZero = "isoZero"

AllZones =  [Inside3D, Outside3D, InterSurf, InterBars, ExterBars ,ExterSurf ]

InterZoneByDim = {2:InterBars, 3:InterSurf}


def CheckIntegrity():
    return "OK"

