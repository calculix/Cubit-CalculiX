# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#
from __future__ import division

import numpy as np
import numpy.testing as npt

import OpenPisco.Unstructured.Levelset as ls

def test_triangle_3d_areas():
    coords = np.array([
        [[0., 0., 0.], [1., 0., 0.], [0., 1., 0.]],
        [[0., 0., 0.], [1., 0., 0.], [0., .5, 0.]],
        [[0., 0., 0.], [.5, 0., 0.], [0., .5, 0.]],
        ])

    expected = [0.5, 0.25, 0.125]
    actual = ls.triangle_3d_areas(coords)
    npt.assert_allclose(actual, expected)

def test_triangle_3d_lumped_mass_matrix():
    nodal_coords = np.array([[0., 0., 0.], [1., 0., 0.], [0., 1., 0.], [.5, 0., 0.], [0., .5, 0.]])
    elem_conn = np.array([[0, 1, 2], [0, 1, 4], [0, 3, 4]])

    expected = np.matrix(np.diag([
        0.291666666667, 0.25, 0.166666666667, 0.0416666666667, 0.125]))
    actual = ls. triangle_3d_lumped_mass_matrix(nodal_coords, elem_conn).todense()
    npt.assert_allclose(actual, expected)

def test_interface_lumped_mass_matrix():
    nodal_coords = np.array([[0., 0., 0.], [1., 0., 0.], [0., 1., 0.], [.5, 0., 0.], [0., .5, 0.]])
    elem_conn = np.array([[0, 1, 2], [0, 1, 4], [0, 3, 4]])

    import Muscat.MeshTools.MeshCreationTools as UMCT
    import OpenPisco.TopoZones as TZ
    mesh = UMCT.CreateMeshOfTriangles(nodal_coords, elem_conn)
    for i in range(mesh.GetNumberOfElements()):
        mesh.AddElementToTag(i, TZ.InterSurf)

    expected = np.matrix(np.diag([
        0.291666666667, 0.25, 0.166666666667, 0.0416666666667, 0.125]))
    actual = ls.interface_lumped_mass_matrix(mesh).todense()
    npt.assert_allclose(actual, expected)

def test_interface_lumped_mass_matrix_advanced():
    def create_test_mesh():
        import Muscat.MeshTools.MeshCreationTools as UMCT
        import Muscat.MeshContainers.ElementsDescription as ED
        import OpenPisco.TopoZones as TZ
        res = UMCT.CreateCube([5, 5, 2], [0., 0., 0.], [0.25, 0.25, 0.25], True)
        tri_elems = res.GetElementsOfType(ED.Triangle_3)
        face_tag = tri_elems.tags["Z0"]
        face_ids = face_tag.GetIds()
        interface_tag = tri_elems.GetTag(TZ.InterSurf)
        interface_tag.SetIds(face_ids)
        return res

    mesh = create_test_mesh()
    levelset = ls.LevelSet(support=mesh)

    expected = 1.0
    actual = levelset.InterfaceIntegral(np.ones((mesh.GetNumberOfNodes(),)))

    npt.assert_allclose(actual, expected)
