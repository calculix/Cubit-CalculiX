# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import division

import numpy.testing as npt

from OpenPisco.Structured.Laplacian3D import ConstantMassMatrix


def test_unit_mass_matrix():
    from OpenPisco.Structured.OptimConstantRectilinearMesh \
            import OptimConstantRectilinearMesh as OCRM
    shape = (3, 3, 3)
    dim = len(shape)
    support = OCRM(dim=3)
    support.SetDimensions(shape)
    support.SetSpacing([1.0] * dim)

    sparse_matrix = ConstantMassMatrix(support)
    actual_total_mass = sparse_matrix.sum()
    expected_total_mass = support.GetTotalVolume()
    npt.assert_allclose(actual_total_mass, expected_total_mass)
