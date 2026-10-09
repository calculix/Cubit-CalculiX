.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

####################
Geometrical Criteria
####################

This page presents the geometrical criteria currently supported by OpenPisco and
their corresponding mathematical formulations.

Overview
"""""""""

Geometrical criteria only depend on the current geometry represented by the
level-set function.

Most geometrical criteria do not require a physical solver.

The corresponding classes are collected in:

* :py:meth:`OpenPisco.Optim.Criteria.GeoCriteria`
* :py:meth:`OpenPisco.Optim.Criteria.GeoThicknessCriteria`

Supported Criteria
""""""""""""""""""

* Volume
* Surface
* Mass
* Minimal thickness
* Maximal thickness
* Center of mass deviation

Volume
======

.. math::

    J(\Omega) = \int_{\Omega}{ dx}

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.GeoCriteria.TopoCriteriaVolume`

Surface
=======

.. math::

    J(\Omega) = \int_{\partial\Omega}{ dx}

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.GeoCriteria.TopoCriteriaSurface`

Mass
====

.. math::

    J(\Omega) = \rho \int_{\Omega}{ dx}

where :math:`\rho` is the constant density.

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.GeoCriteria.TopoCriteriaMass`

Minimal Thickness
=================

.. math::

     J(\Omega) = \int_{ \partial \Omega}{ \int_{0}^{d_{min}}{ \left[ ( d_{\Omega}(s - z n(s)))^+ \right]^2 } \:dz \:dx},

where :math:`d_{\Omega}` is the signed distance function, :math:`d_{min}` is
the desired minimal thickness and :math:`n` is the normal vector.

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.GeoThicknessCriteria.TopoCriteriaMinThickness`

Maximal Thickness
=================

.. math::

    J(\Omega) = \int_{\Omega}{\left[ \left( d_{\Omega} (x) + \frac{d_{max}}{2} \right)^{-}\right]^2 \:dx}.

where :math:`d_{\Omega}` is the signed distance function and :math:`d_{max}` is
the desired maximal thickness.

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.GeoThicknessCriteria.TopoCriteriaMaxThickness`

Center of Mass Deviation
========================

.. math::

    J(\Omega) = \| (\int_\Omega x dx) / |\Omega| - c\|^2

where :math:`c` is the reference center of mass.

Associated Implementation
^^^^^^^^^^^^^^^^^^^^^^^^^

* :py:class:`OpenPisco.Optim.Criteria.GeoCriteria.TopoCriteriaCenterOfMassDeviation`


.. [3] G. Allaire, F. Jouve, G. Michailidis, Thickness control in structural optimization via a level set method, SMO 53, 1349-1382 (2016)