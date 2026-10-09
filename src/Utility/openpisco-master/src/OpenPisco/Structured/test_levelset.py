# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import division

import numpy as np
import numpy.testing as npt

from OpenPisco.Structured.Levelset3D import SurfaceDiracFunction,LevelSet3D

def unit_spaced_support(shape):
    from OpenPisco.Structured.OptimConstantRectilinearMesh \
            import OptimConstantRectilinearMesh
    dim = len(shape)
    result = OptimConstantRectilinearMesh(dim)
    result.SetDimensions(shape)
    result.SetSpacing([1.0] * dim)
    return result

def test_surface_dirac():
    cases = [(
        [-3.5, -2.5, -1.5, -0.5, 0.5, 1.5, 2.5, 3.5],
        [0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0]), (
        [-4.0, -3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0],
        [0.0, 0.0, 0.0, 0.25, 0.5, 0.25, 0.0, 0.0, 0.0])]

    def to_base(a):
        return np.broadcast_to(a, (2, 2, len(a)))

    def to_final(a, axis):
        return np.swapaxes(a, -1, axis)

    for p, e in cases:
        base_phi = to_base(p)
        base_exp = to_base(e)
        for axis in range(3):
            phi = to_final(base_phi, axis)
            expected = to_final(base_exp, axis)
            support = unit_spaced_support(phi.shape)
            actual = SurfaceDiracFunction(phi, support)
            npt.assert_allclose(actual, expected, atol=2e-3)

def test_levelset3d_interface_integral():
    shape = (2, 2, 7)
    dim = len(shape)

    from OpenPisco.Structured.OptimConstantRectilinearMesh \
            import OptimConstantRectilinearMesh as OCRM
    support = OCRM(dim=dim)
    support.SetDimensions(shape)
    support.SetSpacing([1.0] * dim)
    support.SetOrigin([0., 0., -3.5])

    ls = LevelSet3D(support=support)
    get_z_coords = lambda coords: coords[:, -1]
    ls.Initialize(get_z_coords)

    constant = 1.0
    field = np.full(support.GetNumberOfNodes(), constant)
    actual = ls.InterfaceIntegral(field)
    expected = constant
    npt.assert_allclose(actual, expected, atol=2e-3)
