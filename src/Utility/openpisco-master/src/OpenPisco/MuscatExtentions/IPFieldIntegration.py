# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import annotations

from Muscat.FE.Spaces.FESpaces import ConstantSpaceGlobal
from Muscat.FE.DofNumbering import ComputeDofNumbering
from Muscat.FE.Fields.FEField import FEField
from Muscat.FE.Fields.IPField import IPField
from Muscat.FE.SymWeakForm import GetField
from Muscat.FE.SymWeakForm import GetTestField
from Muscat.FE.Integration import IntegrateGeneral
from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter

def IPFieldIntegration(ipfield:IPField, ff:ElementFilter)->float:
    """Compte integral of a field over a region

    Parameters
    ----------
    ipfield : IPField
        Field to be integrated
    ff : ElementFilter
        integrand (region)

    Returns
    -------
    float
        field integrated over region
    """
    mesh = ipfield.mesh
    space = ConstantSpaceGlobal
    ipfield_name = ipfield.name
    numbering = ComputeDofNumbering(mesh,space)
    testfield = FEField("Testfunc",mesh=mesh,space=space,numbering=numbering)

    Tt = GetTestField("Testfunc",1,sdim=mesh.GetPointsDimensionality())
    rho = GetField(ipfield_name,1,sdim=mesh.GetPointsDimensionality())
    wf = rho.T*Tt
    constants = {}
    fields = [ipfield]
    ### the unknownFields (the order is important for the order in the matrix )
    ### only test field is given  -> galerkin projection test functions are the
    ### same as the trial functions

    unknownFields = [ testfield ]
    _,F = IntegrateGeneral(mesh=mesh,
                    wform=wf,
                    constants=constants,
                    fields=fields,
                    unknownFields=unknownFields,
                    integrationRule=ipfield.rule,
                    elementFilter=ff)

    res = F[0]
    return res

def CheckIntegrity():
    import Muscat.MeshContainers.ElementsDescription as ED
    from Muscat.MeshTools.MeshCreationTools import CreateCube
    mesh = CreateCube([2.,3.,4.],[-1.0,-1.0,-1.0],[2./10, 2./10,2./10])
    from Muscat.FE.Fields.IPField import IPField
    from Muscat.FE.IntegrationRules import GetIntegrationRuleByName
    LP1 = GetIntegrationRuleByName("LagrangeP1Quadrature")
    rho_field = IPField("rho",mesh=mesh,rule=LP1)
    factor = 1
    rho_field.Allocate(factor)
    assert list(set(rho_field.GetDataFor(ED.Hexahedron_8).flatten()))[0]==1 , "Field should only contains 1"
    from Muscat.MeshContainers.Filters.FilterObjects import ElementFilter

    dimensions = mesh.ComputeBoundingBox()[1]- mesh.ComputeBoundingBox()[0]
    volume = dimensions[0]*dimensions[1]*dimensions[2]
    ff = ElementFilter(dimensionality=3)
    assert 100* (IPFieldIntegration(rho_field,ff )-volume)/volume <1 , "Should be almost equal to mesh volume"
    surface = 2* (dimensions[0]*dimensions[1]+dimensions[0]*dimensions[2]+dimensions[1]*dimensions[2])
    ff = ElementFilter(dimensionality=2)
    assert 100* (IPFieldIntegration(rho_field,ff )-surface)/surface <1 , "Should be almost equal to mesh surface"
    return "ok"

if __name__ == "__main__":# pragma: no cover
    print(CheckIntegrity())
