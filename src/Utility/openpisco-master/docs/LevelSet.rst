##########
Level Set
##########

Level Set Representations
-------------------------

OpenPisco supports several level set representations adapted to different
simulation and optimization workflows.

.. grid:: 3

   .. grid-item-card:: Structured Level Sets
      :link: StructuredLevelSet
      :link-type: doc

      .. image:: images/StructLevelset.png
         :width: 100%

      Cartesian grid representation.

   .. grid-item-card:: Unstructured Level Sets
      :link: UnstructuredLevelSet
      :link-type: doc

      .. image:: images/UnstructLevelset.png
         :width: 100%

      Simplex mesh representation.

   .. grid-item-card:: Body-Fitted Level Sets
      :link: BodyFittedLevelSet
      :link-type: doc

      .. image:: images/ConformLevelset.png
         :width: 100%

      Mesh conforms to the interface.
``

Structured Level Set
=======================

In the structured setting the support mesh is a structured uniform tetrahedral grid.

.. figure:: images/StructLevelset.png
    :width: 600px
    :align: center
    :alt: Structured Level Set
    :figclass: align-center

    Structured Level Set



Unstructured Level Set
=======================

In the unstructured setting the support mesh is an unstructured tetrahedral mesh.
The following module contains a derived class describing unstructured level sets

* :py:func:`OpenPisco.Unstructured.Levelset`

Utilitary functions helping the manipulation of unstructured level sets can be found in

* :py:func:`OpenPisco.Unstructured.LevelsetTools`

Body-fitted vs non conformal level sets
----------------------------------------

OpenPisco support two modes :

* **conformal** : the background mesh is dynamically adapted to fit the 0 isovalue of the level set field
* **non conformal** : the background mesh is not adapted to the 0 isovalue of the level set field


.. figure:: images/UnstructLevelset.png
    :width: 600px
    :align: center
    :alt: Unstructured non conformal Level Set
    :figclass: align-center

    Unstructured non conformal Level Set


.. figure:: images/ConformLevelset.png
    :width: 600px
    :align: center
    :alt: Unstructured conformal Level Set
    :figclass: align-center

    Unstructured conformal Level Set


In the conformal, or body fitted, case the support mesh contains the negative part of the level set as a submesh and a triangulation of the 0 isovalue is available.
In the non conformal case the 0 isovalue is not explicitly discretized. The background mesh is fixed or adapted to the problem at stake.

The boolean attribute :py:attr:`OpenPisco.Unstructured.Levelset.conform` allows to switch from the non conformal (default) to the body-fitted mode.

Regularization
-------------------
The function :py:func:`OpenPisco.Unstructured.Levelset.Regularize` allows to extend and regularize an input velocity field defined on the structural interface in both body-fitted and non conformal settings.
The regularized velocity field is the solution :math:`V \in H^1(D)` of the following variational problem in the body-fitted setting.

.. math::
	\forall w\in H^{1}(D), \:\: \alpha \int_{D}{\nabla V \cdot \nabla w \:dx } + \int_{\partial\Omega}{ V w \:ds }  = \int_{\partial \Omega}{ v_{\partial \Omega} w \:ds },

where :math:`\alpha>0` is a small parameter tuning the regularization level and :math:`v_{\partial \Omega}` is the input velocity field. This equation is solved using the finite elements framework of the library Muscat [1]_.


Transport
-------------------
The function :py:func:`OpenPisco.Unstructured.Levelset.Transport` allows to transport an initial level set field along a given velocity field by solving the following level set transport equation

.. math::
    \frac{\partial \phi}{\partial t } + \theta \cdot \nabla \phi  = \frac{\partial \phi}{\partial t } -v \lvert \nabla \phi \lvert   =  0 \text{ in } D.

where :math:`\theta = -v n` is a normal velocity field. The transport equation is numerically solved using the class :py:class:`OpenPisco.Unstructured.Levelset.Advect`. This class implements a python interface to the open source library Advection [2]_.


Reinitialization
-------------------
The redistanciation consists in evaluating the signed distance function at the vertices of the computation domain. This function :math:`d_{\Omega}` is defined by

.. math::

    \begin{equation}
        d_{\Omega}(x)=
        \begin{aligned}
            \begin{cases}
            -d(x,\partial \Omega)\quad \text{if}\quad x\in\Omega\\
            0\quad \text{if} \quad x\in\partial\Omega\\
            d(x,\partial \Omega) \quad \text{otherwise}\\
            \end{cases}
        \end{aligned}
    \end{equation}

where :math:`d(x,\partial \Omega) = \min_{p\in \partial \Omega}|x-p|`

It is computed using the class :py:class:`OpenPisco.Unstructured.Meshdist`. This class implements a python interface to the open source library Mshdist [3]_.



.. [1] https://gitlab.com/drti/muscat

.. [2] https://github.com/ISCDtoolbox/Advection/blob/master/Documentation/advectguide.pdf

.. [3] https://github.com/ISCDtoolbox/Mshdist/blob/master/documentation/mshdistguide.pdf
