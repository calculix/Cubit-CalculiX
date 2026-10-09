# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

"""AsterIntegrationRules

Code\_Aster provides details regarding the specific integration rules for its analysis. The aim is to make them available in OpenPisco by relying on Muscat features.
"""
import numpy as np
import Muscat.MeshContainers.ElementsDescription as ED
from Muscat.FE.IntegrationRules import LagrangeP2Quadrature, LagrangeP1Quadrature, RegisterIntegrationRule, MeshQuadrature, ElementaryQuadrature

# from https://www.code-aster.org/V2/doc/v14/fr/man_r/r3/r3.01.01.pdf


AsterLagrangeP1 = MeshQuadrature("AsterLagrangeP1")

AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Point_1])
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Bar_2])
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Bar_2].CopyFor(ED.Bar_3))
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Quadrangle_4])
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Quadrangle_4].CopyFor(ED.Quadrangle_8))
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Quadrangle_4].CopyFor(ED.Quadrangle_9))
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Hexahedron_8])
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Hexahedron_8].CopyFor(ED.Hexahedron_20))
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Hexahedron_8].CopyFor(ED.Hexahedron_27))
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Tetrahedron_4])
AsterLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Tetrahedron_4].CopyFor(ED.Tetrahedron_10))

p0 = 1./3.
p = np.array([[p0,p0,0]])
w = np.array([0.5])

AsterLagrangeP1.AddElementaryQuadrature(ElementaryQuadrature(ED.Triangle_3, points=p, weights=w))
AsterLagrangeP1.AddElementaryQuadrature(AsterLagrangeP1[ED.Triangle_3].CopyFor(ED.Triangle_6))

RegisterIntegrationRule(AsterLagrangeP1)

AsterLagrangeP2 = MeshQuadrature("AsterLagrangeP2")

AsterLagrangeP2.AddElementaryQuadrature(LagrangeP2Quadrature[ED.Point_1])
AsterLagrangeP2.AddElementaryQuadrature(LagrangeP2Quadrature[ED.Bar_2])
AsterLagrangeP2.AddElementaryQuadrature(LagrangeP2Quadrature[ED.Bar_2].CopyFor(ED.Bar_3))
AsterLagrangeP2.AddElementaryQuadrature(LagrangeP2Quadrature[ED.Quadrangle_4])
AsterLagrangeP2.AddElementaryQuadrature(LagrangeP2Quadrature[ED.Quadrangle_4].CopyFor(ED.Quadrangle_8))
AsterLagrangeP2.AddElementaryQuadrature(LagrangeP2Quadrature[ED.Quadrangle_4].CopyFor(ED.Quadrangle_9))
AsterLagrangeP2.AddElementaryQuadrature(LagrangeP2Quadrature[ED.Hexahedron_8])
AsterLagrangeP2.AddElementaryQuadrature(LagrangeP2Quadrature[ED.Hexahedron_8].CopyFor(ED.Hexahedron_20))
AsterLagrangeP2.AddElementaryQuadrature(LagrangeP2Quadrature[ED.Hexahedron_8].CopyFor(ED.Hexahedron_27))

p0 = 0.25
p1 = 1./6.
p2 = 0.5
p=np.array([[p0,p0,p0],
            [p1,p1,p1],
            [p1,p1,p2],
            [p1,p2,p1],
            [p2,p1,p1]])
w0 = -2./15
w1 = 3./40
w= np.array([ w0, w1, w1, w1, w1])

AsterLagrangeP2.AddElementaryQuadrature(ElementaryQuadrature(ED.Tetrahedron_4, points=p, weights=w))
AsterLagrangeP2.AddElementaryQuadrature(ElementaryQuadrature(ED.Tetrahedron_10, points=p, weights=w))

p0 = 0.445948490915965
p1 = 0.091576213509771

p = np.array([[p1,p1,0],
[1.-2.*p1,p1,0],
[p1,1.-2.*p1,0],
[p0,1.-2.*p0,0],
[p0,p0,0],
[1.-2.*p0,p0,0]])

w0 = 0.0549758718227661
w1 = 0.11169079483905
w = np.array([w0 ,w0 ,w0 ,w1 ,w1 ,w1])

AsterLagrangeP2.AddElementaryQuadrature(ElementaryQuadrature(ED.Triangle_3, points=p, weights=w))
AsterLagrangeP2.AddElementaryQuadrature(ElementaryQuadrature(ED.Triangle_6, points=p, weights=w))


RegisterIntegrationRule(AsterLagrangeP2)

# integration rules used in P1 thermal analysis
AsterTetrahedron4PointsLagrangeP1 = MeshQuadrature("AsterTetrahedron4PointsLagrangeP1")

AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Point_1])
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Bar_2])
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Bar_2].CopyFor(ED.Bar_3))
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Quadrangle_4])
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Quadrangle_4].CopyFor(ED.Quadrangle_8))
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Quadrangle_4].CopyFor(ED.Quadrangle_9))
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature( LagrangeP1Quadrature[ED.Hexahedron_8])
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Hexahedron_8].CopyFor(ED.Hexahedron_20))
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Hexahedron_8].CopyFor(ED.Hexahedron_27))

p0 = (5.-np.sqrt(5.))/20.# 0.1381966
p1 = (5.+3*np.sqrt(5.))/20.#0.5854102
p=np.array([[p0,p0,p0],
            [p0,p0,p1],
            [p0,p1,p0],
            [p1,p0,p0]])

w0 = 1./24.#0.04166667
w= np.array([ w0, w0, w0, w0])
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(ElementaryQuadrature(ED.Tetrahedron_4, points=p, weights=w))
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(ElementaryQuadrature(ED.Tetrahedron_10, points=p, weights=w))
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Triangle_3])
AsterTetrahedron4PointsLagrangeP1.AddElementaryQuadrature(LagrangeP1Quadrature[ED.Triangle_6])

RegisterIntegrationRule(AsterTetrahedron4PointsLagrangeP1)


def CheckIntegrity(GUI=False):
    rules = {"AsterLagrangeP2":AsterLagrangeP2,
             "AsterTetrahedron4PointsLagrangeP1":AsterTetrahedron4PointsLagrangeP1,
             "AsterLagrangeP1":AsterLagrangeP1}
    for rulename,rule in rules.items():
        for elemName, data in rule.items():
            lw = data.GetNumberOfPoints()
            print(rulename + " " + str(elemName) + " has " + str(lw) + " integration points")
    return "ok"

if __name__ == '__main__':
    print(CheckIntegrity(True))# pragma: no cover





