# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import division

import numpy as np

from Muscat.Types import MuscatFloat

def triangle_lob_fraction(index, phis):
    div = phis-phis[index]
    div[index] = phis[index]
    val = phis[index]/(div)
    return np.prod(val)

def triangle_volumic_fraction2(phis):
    signs = extract_signs(phis)
    minuses, pluses = split_signs(signs)
    # we c
    if minuses.size == 3:
        return 1.0

    if minuses.size == 2:
        return 1.0 - triangle_lob_fraction(pluses[0], phis)

    if minuses.size == 1:
        return  triangle_lob_fraction(minuses[0], phis)

    if pluses.size == 3 :
        return 0.0

def triangle_volumic_fractions(phis):
    nb = phis.shape[0]
    res = np.empty(nb, dtype=MuscatFloat)

    for i in range(nb):
        res[i] = triangle_volumic_fraction2(phis[i,:])
    return res

def quadrangle_volumic_fractions(phis):
    nb = phis.shape[0]
    res = np.empty(nb, dtype=MuscatFloat)

    for i in range(nb):
        res[i] = quadrangle_volumic_fraction(phis[i,:])
    return res

def quadrangle_volumic_fraction(phis):
    r"""
    3-------2
    | \   / |
    |   c   |
    | /   \ |
    0-------1
    """
    cc = np.sum(phis)/4
    f = triangle_volumic_fraction2(np.array([phis[0], phis[1], cc]))
    f += triangle_volumic_fraction2(np.array([phis[1], phis[2], cc]))
    f += triangle_volumic_fraction2(np.array([phis[2], phis[3], cc]))
    f += triangle_volumic_fraction2(np.array([phis[3], phis[0], cc]))
    return f/4

def extract_signs(phis):
    signs = np.sign(phis)
    dominant_sign = np.sign(np.sum(signs))
    if dominant_sign == -1:
        signs[signs != 1] = -1
    else:
        signs[signs != -1] = 1
    return signs

def split_signs(signs):
    minuses = np.nonzero(signs == -1)[0]
    pluses = np.nonzero(signs == 1)[0]
    return minuses, pluses

    ########################### OLD STUFF #####################################

def convex_quandrangle_volume(coords):# pragma: no cover
    assert coords.shape == (4, 2)
    return 0.5 * np.cross(coords[2, :] - coords[0, :], coords[3, :] - coords[1, :])

def quadrangle_volumic_fraction_old(phis):# pragma: no cover
    assert phis.size == 4
    assert np.count_nonzero(phis) > 0

    signs = extract_signs(phis)
    minuses, pluses = split_signs(signs)
    if minuses.size == 4:
        return 1.0
    if minuses.size == 3:
        return 1.0 - lobe_volumic_fraction_from_index(pluses[0], phis)
    if minuses.size == 2:
        if np.sum(minuses) % 2 == 0:
            sign_at_centroid = np.sign(np.sum(phis))
            if sign_at_centroid == 1:
                return lobe_volumic_fraction_from_index(minuses[0], phis) + \
                        lobe_volumic_fraction_from_index(minuses[1], phis)
            else:
                void_volumic_fraction = \
                        lobe_volumic_fraction_from_index(minuses[0], phis) + \
                        lobe_volumic_fraction_from_index(minuses[1], phis)
                return 1.0 - void_volumic_fraction
        else:
            if minuses[0] == 1:
                pluses = np.roll(pluses, 1)
            elif pluses[0] == 1:
                minuses = np.roll(minuses, 1)
            return slab_volumic_fraction(minuses, pluses)
    if minuses.size == 1:
        return lobe_volumic_fraction_from_index(minuses[0], phis)
    if minuses.size == 0:
        return 0.0



def lobe_volumic_fraction_from_index(corner_index, phis, tol=None):# pragma: no cover
    roll = np.array([i % 4 for i in range(1, 7)])
    return lobe_volumic_fraction( \
            phis[corner_index], phis[roll[corner_index:corner_index+3]], tol)

def lobe_volumic_fraction(phi_in, phis_out, tol=None):# pragma: no cover
    if tol is None:
        tol = 1.0e-6
    denom = phi_in - phis_out[0] + phis_out[1] - phis_out[2]
    prod = (phi_in - phis_out[0]) * (phi_in - phis_out[2])
    base = (phi_in**2) / prod
    var = (phi_in * denom) / prod
    if np.abs(var) < tol:
        return 0.5 * base
    if np.abs(1.0 - var) < tol:
        return base
    else:
        return base * ((1.0 + ((1.0 / var) - 1.0) * np.log(1.0 - var)) / var)

def slab_volumic_fraction(phis_in, phis_out, tol=None):# pragma: no cover
    if tol is None:
        tol = 1.0e-6
    denom = phis_in[0] - phis_in[1] + phis_out[0] - phis_out[1]
    var = denom / (phis_in[0] - phis_out[1])
    if np.abs(var) < tol:
        return 0.5 * (phis_in[0] / (phis_in[0] - phis_out[1]) + \
                phis_in[1] / (phis_in[1] - phis_out[0]))
    else:
        return ((phis_in[0] - phis_in[1]) + \
                ((phis_in[1] * phis_out[1] - phis_in[0] * phis_out[0]) * \
                np.log((phis_in[1] - phis_out[0]) / (phis_in[0] - phis_out[1])) \
                / denom)) / denom

def CheckIntegrity():
     x = quadrangle_volumic_fraction(np.array([ 0.45105652, -1.08778525, -0.33203568,  1.20680609]))
     assert x>=0 and x<= 1, "Failure"
     x =  quadrangle_volumic_fraction(np.array([ 0.25574957,  0.84353483, -0.90203619, -1.48982144]))
     assert x>=0 and x<= 1, "Failure"
     x =  quadrangle_volumic_fraction(np.array([ -1,  -1, -1, -1]))
     assert x==1, "Failure"
     x =  quadrangle_volumic_fraction(np.array([ 1,  1, 1, 1]))
     assert x==0, "Failure"
     return "ok"


if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity())

