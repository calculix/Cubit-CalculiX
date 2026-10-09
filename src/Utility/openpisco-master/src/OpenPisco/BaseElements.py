# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

import Muscat.MeshContainers.ElementsDescription as ED

elementsTypeSupportedForDimension=[ED.Point_1,ED.Bar_2,ED.Triangle_3,ED.Tetrahedron_4]

complexElementsTypesSupportedForDimension = [ED.Point_1 ,ED.Bar_2 ,ED.Quadrangle_4 , ED.Hexahedron_8]

def CheckIntegrity():
    return "OK"

