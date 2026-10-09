# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import division

import numpy as np
import numpy.testing as npt

def test_tetrahedron_volumic_fraction():
    from OpenPisco.Structured.Geometry3D \
            import tetrahedron_volumic_fraction

    cases = (([ 0.0,  1.0,  1.0,  1.0], 0.0),   \
             ([-2.0,  2.0,  2.0,  2.0], 0.125), \
             ([ 2.0, -2.0, -2.0, -2.0], 0.875), \
             ([ 1.0, -1.0, -1.0,  1.0], 0.5), \
             ([ 1.0,  1.0, -1.0,  2.0], 0.08333333))

    for phis, expected in cases:
        phis in np.array(phis)
        for shift in range(0, 4):
            rolled_phis = np.roll(phis, shift)
            for step in (1, -1):
                stepped_phis = rolled_phis[::step]
                for f, e in zip((1, -1), (expected, 1.0 - expected)):
                    p = f * stepped_phis
                    actual = tetrahedron_volumic_fraction(p)
                    npt.assert_allclose(actual, e)

def test_tetrahedron_volumic_fraction_dominated():
    from OpenPisco.Structured.Geometry3D \
            import tetrahedron_volumic_fraction_dominated

    cases = (( 0.0, [ 1.0,  1.0,  1.0], 0.0),   \
             (-2.0, [ 2.0,  2.0,  2.0], 0.125), \
             ( 2.0, [-2.0, -2.0, -2.0], 0.125))

    for phi_dominated, phi_others, expected in cases:
        actual = tetrahedron_volumic_fraction_dominated( \
                np.array(phi_dominated), np.array(phi_others))
        npt.assert_allclose(actual, expected)

def test_sort_by_sign():
    from OpenPisco.Structured.Geometry3D import sort_by_sign

    cases = (([ 1.0, -1.0,  1.0,  1.0], ([-1.0], [ 1.0,  1.0,  1.0])), \
             ([ 1.0,  0.0, -1.0, -1.0], ([ 0.0, -1.0, -1.0], [ 1.0])), \
             ([ 1.0,  2.0, -1.0, -1.0], ([-1.0, -1.0], [ 1.0,  2.0])), \
             ([ 1.0,  0.0,  0.0, -1.0], ([-1.0], [ 1.0,  0.0,  0.0])))

    for phis, expected in cases:
        actual = sort_by_sign(np.array(phis))
        for a, e in zip(actual, expected):
            npt.assert_allclose(a, np.array(e))

def test_cuboid_volumic_fraction():
    from OpenPisco.Structured.Geometry3D import cuboid_volumic_fraction

    cases = (([ 1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0], 0.0), \
             ([-1.0,  0.0,  1.0,  0.0, -1.0,  0.0,  1.0,  0.0], 0.5), \
             ([ 1.0, -1.0, -1.0,  1.0, -1.0,  1.0,  1.0, -1.0], 0.5), \
             ([-1.0, -1.0, -1.0, -1.0,  3.0,  3.0,  3.0,  3.0], 0.25), \
             ([-1.0,  1.0,  3.0,  1.0,  1.0,  3.0,  5.0,  3.0], 0.020833333))

    for phis, expected in cases:
        actual = cuboid_volumic_fraction(np.array(phis))
        npt.assert_allclose(actual, expected)

def test_cuboid_volumic_fractions():
    from OpenPisco.Structured.Geometry3D import cuboid_volumic_fractions

    phis = [[ 1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0], \
            [-1.0,  0.0,  1.0,  0.0, -1.0,  0.0,  1.0,  0.0], \
            [ 1.0, -1.0, -1.0,  1.0, -1.0,  1.0,  1.0, -1.0], \
            [-1.0, -1.0, -1.0, -1.0,  3.0,  3.0,  3.0,  3.0], \
            [-1.0,  1.0,  3.0,  1.0,  1.0,  3.0,  5.0,  3.0]]

    expected = [0.0, 0.5, 0.5, 0.25, 0.020833333]

    actual = cuboid_volumic_fractions(np.array(phis))
    npt.assert_allclose(actual, expected)

def test_tetrahedron_volumic_fractions():
    from OpenPisco.Structured.Geometry3D \
            import tetrahedron_volumic_fractions

    phis = [[ 0.0,  1.0,  1.0,  1.0], \
            [-2.0,  2.0,  2.0,  2.0], \
            [ 2.0, -2.0, -2.0, -2.0], \
            [ 1.0, -1.0, -1.0,  1.0], \
            [ 1.0,  1.0, -1.0,  2.0]]

    expected = [0.0, 0.125, 0.875, 0.5, 0.08333333]

    actual = tetrahedron_volumic_fractions(np.array(phis))
    npt.assert_allclose(actual, expected)

def test_strict_signs():
    from OpenPisco.Structured.Geometry3D import strict_signs

    phis = np.array( \
            [[[-2.0,  0.0,  3.0,  3.0], [ 3.0,  0.0,  0.0,  0.0]], \
             [[-2.0, -2.0, -2.0, -2.0], [-2.0,  3.0,  0.0,  0.0]]])

    expected = np.array( \
            [[[-1.0,  1.0,  1.0,  1.0], [ 1.0,  1.0,  1.0,  1.0]], \
             [[-1.0, -1.0, -1.0, -1.0], [-1.0,  1.0, -1.0, -1.0]]])

    actual = strict_signs(phis)
    npt.assert_allclose(actual, expected)

def test_tetrahedron_volumic_fractions_dominated():
    from OpenPisco.Structured.Geometry3D \
            import tetrahedron_volumic_fractions_dominated

    phis_dominated =  [0.0, -2.0,  2.0]
    phis_dominant = \
            [[ 1.0,  1.0,  1.0], \
             [ 2.0,  2.0,  2.0], \
             [-2.0, -2.0, -2.0]]

    expected = [0.0, 0.125, 0.125]

    actual = tetrahedron_volumic_fractions_dominated( \
            np.array(phis_dominated), np.array(phis_dominant))
    npt.assert_allclose(actual, expected)
