# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import division

import numpy as np
import math


# Non-vectorized implementation

def tetrahedron_volumic_fraction(phis):
    assert phis.size == 4
    assert np.count_nonzero(phis) > 0
    minuses, pluses = sort_by_sign(phis)
    if minuses.size == 0:
        return 0.0
    elif minuses.size == 1:
        return tetrahedron_volumic_fraction_dominated(minuses, pluses)
    elif minuses.size == 2:
        return tetrahedron_volumic_fraction_nondominated(minuses, pluses)
    elif minuses.size == 3:
        return 1.0 - tetrahedron_volumic_fraction_dominated(pluses, minuses)
    else:
        return 1.0

def tetrahedron_volumic_fraction_dominated(phi_in, phis_out):
    return np.prod(phi_in / (phi_in - phis_out))

def tetrahedron_volumic_fraction_nondominated(phis_in, phis_out):
    phis_in_r = np.reshape(phis_in, (2, 1))
    phis_out_r = np.reshape(phis_out, (1, 2))
    ratios = phis_in_r / (phis_in_r - phis_out_r)
    result = ratios[0, 0] * ratios[0, 1] * (1.0 - ratios[1, 0]) + \
             ratios[1, 0] * ratios[1, 1] * (1.0 - ratios[0, 1]) + \
             ratios[1, 0] * ratios[0, 1]
    return result

def sort_by_sign(phis):
    signs = np.sign(phis)
    dominant_sign = np.sign(np.sum(signs))
    if dominant_sign == -1:
        minuses = phis[signs != 1]
        pluses = phis[signs == 1]
    else:
        minuses = phis[signs == -1]
        pluses = phis[signs != -1]
    return minuses, pluses

hexahedron_faces = np.array(
        [[0, 1, 2, 3], \
         [7, 6, 5, 4], \
         [0, 3, 7, 4], \
         [1, 5, 6, 2], \
         [0, 4, 5, 1], \
         [2, 6, 7, 3]])

def cuboid_volumic_fraction(phis):
    assert phis.shape[-1] == 8
    assert phis.size == 8
    face_phis = phis[hexahedron_faces]
    face_averages = np.mean(face_phis, axis=1)
    cuboid_average = np.mean(face_averages)
    tetra_phis = np.empty((24, 4), dtype=cuboid_average.dtype)
    tetra_phis[:, 0] = np.ravel(face_phis)
    tetra_phis[:, 1] = np.ravel(np.roll(face_phis, -1, axis=1))
    tetra_phis[:, 2] = np.repeat(face_averages, 4)
    tetra_phis[:, 3] = cuboid_average

    return math.fsum(tetrahedron_volumic_fraction(t) for t in tetra_phis) / 24.0

def thetrahedron_p2_volumic_fraction(phis):
    assert phis.shape[-1] == 10
    assert phis.size == 10
    centerphi =  np.sum(phis[4:])/6.
    # the corners
    s = 0
    s += tetrahedron_volumic_fraction(phis[[0, 4, 6, 7]])
    s += tetrahedron_volumic_fraction(phis[[1, 5, 4, 8]])
    s += tetrahedron_volumic_fraction(phis[[2, 6, 5, 9]])
    s += tetrahedron_volumic_fraction(phis[[7, 8, 9, 3]])
    s /= 4.
    # center a regular octaedral
    phis2 = np.append(phis,centerphi)
    s2 =0
    s2 += tetrahedron_volumic_fraction(phis2[[4, 7, 6, 10]])
    s2 += tetrahedron_volumic_fraction(phis2[[4, 8, 7, 10]])
    s2 += tetrahedron_volumic_fraction(phis2[[4, 5, 8, 10]])
    s2 += tetrahedron_volumic_fraction(phis2[[5, 9, 8, 10]])
    s2 += tetrahedron_volumic_fraction(phis2[[5, 6, 9, 10]])
    s2 += tetrahedron_volumic_fraction(phis2[[6, 7, 9, 10]])
    s2 += tetrahedron_volumic_fraction(phis2[[7, 8, 9, 10]])
    s2 += tetrahedron_volumic_fraction(phis2[[6, 4, 5, 10]])
    s2 /= 8.
    return (s + s2)/2.



# Vectorized implementation
def cuboid_volumic_fractions(phis):
    """
    Element-wise fractions of volume located inside the domain implicitly
    defined by a levelset function for an arbitrary number of cuboids.

    The implementation of the function is vectorized for computational
    efficiency.

    Parameters
    ----------
    phis : array_like
        The nodal values of the levelset function for an arbitrary number of
        cuboid elements. The trailing dimension is interpreted as the node
        index therefore its size must be exactly 8.

    Returns
    -------
    np.ndarray
        The result. The shape matches the one of `phis` with the
        trailing dimension removed.
    """
    assert phis.shape[-1] == 8

    face_phis = phis[..., hexahedron_faces]
    face_averages = np.mean(face_phis, axis=-1)
    cuboid_average = np.mean(face_averages, axis=-1, keepdims=True)

    tetra_phis_shape = phis.shape[:-1] + (24, 4)
    tetra_phis = np.empty(tetra_phis_shape, dtype=cuboid_average.dtype)
    tetra_phis[..., 0] = np.reshape(face_phis, tetra_phis.shape[:-1])
    tetra_phis[..., 1] = np.reshape( \
            np.roll(face_phis, -1, axis=-1), tetra_phis.shape[:-1])
    tetra_phis[..., 2] = np.repeat(face_averages, 4, axis=-1)
    tetra_phis[..., 3] = cuboid_average

    tetra_phis.sort(axis=-1)
    tetra_fracs = tetrahedron_volumic_fractions_impl(tetra_phis)
    return np.mean(tetra_fracs, axis=-1)

def tetrahedron_volumic_fractions(phis):
    """
    Element-wise fractions of volume located inside the domain implicitly
    defined by a levelset function for an arbitrary number of tetrahedra.

    The implementation of the function is vectorized for computational
    efficiency.

    Parameters
    ----------
    phis : array_like
        The nodal values of the levelset function for an arbitrary number of
        tetrahedric elements. The trailing dimension is interpreted as the node
        index therefore its size must be exactly 4.

    Returns
    -------
    np.ndarray
        The result. The shape matches the one of `phis` with the
        trailing dimension removed.
    """
    assert phis.shape[-1] == 4
    if phis.dtype.kind == 'f':
        phis_sorted = np.sort(phis, axis=-1)
    else:
        phis_sorted = np.asfarray(phis)
        phis_sorted.sort(axis=-1)
    return tetrahedron_volumic_fractions_impl(phis_sorted)

def tetrahedron_volumic_fractions_impl(phis_sorted):
    signs = strict_signs(phis_sorted)
    sums = np.sum(signs, axis=-1, keepdims=False)
    # sums holds values in set {-4, -2, 0, 2, 4}
    indices = np.argsort(sums, axis=None)
    counts = np.searchsorted(np.ravel(sums), \
            np.arange(-4, 4, 2), side='right', sorter=indices)

    partition = np.array_split(indices, counts)

    result = np.empty(indices.shape, dtype=phis_sorted.dtype)
    phis_reshaped = np.reshape(phis_sorted, indices.shape + (4,))
    phis_parts = dict((i, phis_reshaped[partition[i], :]) for i in range(1,4))
    result[partition[0]] = 1.0
    result[partition[1]] = 1.0 - tetrahedron_volumic_fractions_dominated( \
            phis_parts[1][..., -1:], phis_parts[1][..., :-1])
    result[partition[2]] = tetrahedron_volumic_fractions_nondominated( \
            phis_parts[2][..., :2], phis_parts[2][..., 2:])
    result[partition[3]] = tetrahedron_volumic_fractions_dominated( \
            phis_parts[3][..., :1], phis_parts[3][..., 1:])
    result[partition[4]] = 0.0
    return np.reshape(result, phis_sorted.shape[:-1])

def strict_signs(phis, default_negative=True):
    signs = np.empty(phis.shape, dtype=np.double)
    np.sign(phis, out=signs)
    zeros = signs == 0
    if np.any(zeros):
        # Fixup zeros
        default_signs = np.sign(np.sum(signs, axis=-1, keepdims=True))
        default_value = -1 if default_negative else 1
        np.putmask(default_signs, default_signs == 0, default_value)
        np.putmask(signs, zeros, np.broadcast_arrays(default_signs, signs)[0])
    return signs

def tetrahedron_volumic_fractions_dominated(phis_in, phis_out):
    if phis_in.ndim == phis_out.ndim - 1:
        phis_in = np.expand_dims(phis_in, axis=-1)
    return np.prod(phis_in / (phis_in - phis_out), axis=-1)

def tetrahedron_volumic_fractions_nondominated(phis_in, phis_out):
    phis_in_r = np.expand_dims(phis_in, axis=-1)
    phis_out_r = np.expand_dims(phis_out, axis=-2)
    ratios = phis_in_r / (phis_in_r - phis_out_r)
    result = ratios[..., 0, 0] * ratios[..., 0, 1] * (1.0 - ratios[..., 1, 0]) + \
             ratios[..., 1, 0] * ratios[..., 1, 1] * (1.0 - ratios[..., 0, 1]) + \
             ratios[..., 1, 0] * ratios[..., 0, 1]
    return result


def CheckIntegrity():
    import OpenPisco.Structured.test_geometry3d as tests
    test_functions = \
            (t for n, t in tests.__dict__.items() if n.startswith("test"))
    for f in test_functions:
        f()
    return "ok"


if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity())
