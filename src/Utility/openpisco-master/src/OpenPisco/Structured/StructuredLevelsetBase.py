# -*- coding: utf-8 -*-
#
# This file is subject to the terms and conditions defined in
# file 'LICENSE', which is part of this source code package.
#

from __future__ import division
import numpy as np
import math

from Muscat.Helpers.Logger import Debug
from OpenPisco.LevelSetBase import LevelSetBase

class StructuredLevelsetBase(LevelSetBase):
    def __init__(self, other=None, support=None):
        super(StructuredLevelsetBase, self).__init__(other=other, support=support)

        if other is None:
            self.regularize_algo = '' # ("","skfmm")
            self.reinitialize_algo = '' # ("", "skfmm")

        else:
            self.regularize_algo = other.regularize_algo
            self.reinitialize_algo = other.reinitialize_algo

    def GetDxMin(self):
        return min(self.support.props.get("spacing"))

    def GetMonoIndexView(self, field):
        return field.ravel()

    def GetMultiIndexView(self, field):
        resField = field.view()
        resField.shape = tuple(x for x in self.support.props.get("dimensions"))
        return resField

    def InterfaceIntegral(self, field):
        trace = self.InterfaceRestriction(field).ravel()
        v = self.GetMassMatrix() * trace
        return v.sum()

    def GetElementsVolumicFractions(self,phi=None):
        """
        Get the element-wise volumic fractions located inside the domain
        implicitly defined by a levelset function.

        By convention, the inside corresponds to the nonpositive values of the levelset function.

        Parameters
        ----------
        levelset : np.ndarray
            A node-based levelset function interpreted as the signed distance
            close to the interface.

        Returns
        -------
        np.ndarray
            Element-based field.
        """
        import Muscat.MeshContainers.ElementsDescription as ED
        coon = self.support.GetElementsOfType( [ED.Point_1 ,ED.Bar_2 ,ED.Quadrangle_4 , ED.Hexahedron_8][self.support.GetElementsDimensionality()]).connectivity

        if phi is None:
            ravel_ls = self.phi.ravel()
        else:
            ravel_ls = phi.ravel()
        return self.GetVolumeFractionOperator()(ravel_ls[coon])

    def Regularize(self, field, lengthscaleParameter=1.0, extra = None, on=[]):
        Debug("Regularize")
        if extra is None:
            Debug("Regularize (using phi)")
        else:
            Debug("Regularize on surface of extra field (extra)")

        if self.regularize_algo == "skfmm" :
            import skfmm
            field2 = self.GetMultiIndexView(field)
            if extra is None:
                phi2 = self.GetMultiIndexView(self.phi)
            else:
                phi2 = self.GetMultiIndexView(extra)
            result  = skfmm.extension_velocities(phi2,field2,self.support.props.get("spacing"))[1].ravel()
        else:

            if extra is None:
                phi2 = self.phi
            else:
                phi2 = extra

            tt = self.ComputeInterfaceProjection(np.ones_like(field), phi = phi2)
            Rop = self.RegularizationOperator()(self.support, lengthscaleParameter, blockDofs = tt.ravel()  )
            self.rhs = self.ComputeInterfaceProjection(field.ravel())
            result = Rop.Solve(self.rhs).view()
            result = self.GetMultiIndexView(result)
        Debug("Regularize Done")
        return result

    def InterfaceRestriction(self, field,phi=None):
        if phi is None:
            phi = self.GetMultiIndexView(self.phi)
        else:
            phi = self.GetMultiIndexView(phi)
        field2 = self.GetMultiIndexView(field)
        return field2 * self.GetMultiIndexView(self.SurfaceDiracFunction(phi))

    def GetVolumeOfNegativePartbyElement(self,phi=None):
        """
        Get the element-wise volumic located inside the domain
        implicitly defined by a levelset function.

        By convention, the inside corresponds to the nonpositive values of the levelset function.

        Parameters
        ----------
        levelset : np.ndarray
            A node-based levelset function interpreted as the signed distance
            close to the interface.
        """
        if phi is None:
            phi = self.phi
        dv = np.prod(self.support.props["spacing"])
        return dv * self.GetElementsVolumicFractions(phi)

    def TransportOnce(self, velocity, cfl=0.5):
        """
        Apply a single transport time-step to the level set function.

        Solve the advection equation:

          d(phi)/dt + V \\|grad(phi)\\| = 0 with
          d(phi)/dn = 0 on the boundary

        where V is the normal velocity (scalar field).
        """
        dt = (cfl * self.GetDxMin()) / np.amax(np.fabs(velocity))
        self.ApplyTransportStep(velocity, dt)

    def ApplyTransportStep(self, velocity, dt):
        update = self.LevelsetUpdate(velocity, self.phi,
                self.GradientNeumannOrder2Extension, self.support)
        self.phi -= dt * update

    def TransportAndReinitialize(self, velocity, velocity_normalization=1.0, disableCFLTruncation = True):
        velocity_magnitude = np.amax(np.fabs(velocity))
        assert velocity_magnitude != 0,"velocity field equal to zero!!!"
        final_time = velocity_normalization / velocity_magnitude
        dx = self.GetDxMin()
        cfl = 0.5
        dt_cfl = (cfl * dx) / velocity_magnitude
        niter_cfl = int(math.ceil(final_time / dt_cfl))
        Debug("transport niter_cfl = " + str(niter_cfl))

        niter = max(10, niter_cfl)
        dt = final_time / niter
        swept_distance = dt * velocity_normalization * velocity_magnitude
        reinit_distance = 4.0 * swept_distance

        for _ in range(niter):
            self.ApplyTransportStep(velocity, dt)
            self.Reinitialize(length=reinit_distance)

    def ComputeInterfaceProjection(self,data,phi=None):

        res = self.InterfaceRestriction(data,phi=phi).ravel()
        return res* np.squeeze(np.array(np.sum(self.GetMassMatrix(),axis=1)) )

    def Reinitialize(self, length=None):
        """
        Reinitialize the levelset function as a signed distance.

        The levelset function guaranteed to be a signed distance function at
        least at a distance of `length` from the interface.

        Parameters
        ----------
        length : real, optional
            Sufficient length of redistanciation, expressed as an absolute
            distance from the interface (the default is `4 * self.GetDxMin()`)

        Notes
        -----
        Solve the evolution equation:
        d(phi)/dt + sign(phi0)(\\|grad(phi)\\|-1) = 0
        on a pseudo-time interval corresponding to the specified `length`,
        using the property that the velocity is uniformly equal to one.
        """

        if self.reinitialize_algo == "skfmm":
            import skfmm
            MIPhi = self.GetMultiIndexView(self.phi)
            self.phi[:] = skfmm.distance(MIPhi, dx=self.support.props.get("spacing")).ravel()
            return

        dx = self.GetDxMin()
        if length is None:
            length = dx*4

        cfl = 0.25
        #TODO for the moment I don't know why with a 0.5 clf the algo is not stable

        remaining_distance = length
        while remaining_distance > 0.0:
            dt = min(cfl * dx, remaining_distance)
            update = self.GradientNormVelocity(self.phi, self.support)
            update -= np.sign(self.phi)
            self.phi -= dt * update
            remaining_distance -= dt

    def GradientNormVelocity(self, phi, support):
        velocity_field = np.sign(phi)
        return self.LevelsetUpdate(velocity_field, phi, self.NeumannOrder2Extension, support)

    def SurfaceDiracFunction(self, phi=None):
        if phi is None:
            phi = self.phi
        #https://tel.archives-ouvertes.fr/tel-00189409v2/document
        m = 2
        h = self.GetDxMin()
        eps = (m+1)/(2)*(h)
        def psi_cos(x):
            return 0.5*(1+np.cos(x*np.pi) )

        res = 1/eps*(psi_cos(phi/eps))

        res[abs(phi)> eps] = 0
        return res


def CheckIntegrity(GUI=False):
    return "ok"

if __name__ == '__main__':# pragma: no cover
    print(CheckIntegrity())
