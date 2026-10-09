.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

##########
Level Set
##########

Level Set Representations
-------------------------
OpenPisco supports several level set representations adapted to different
simulation and optimization workflows.

.. grid:: 3
   :gutter: 3

   .. grid-item-card::
      :link: StructuredLevelSet
      :link-type: doc

      🟦 Structured Level Sets

      Level sets defined on Cartesian grids.

   .. grid-item-card::
      :link: UnstructuredLevelSet
      :link-type: doc

      🔺 Unstructured Level Sets

      Level sets defined on simplicial meshes. 

   .. grid-item-card::
      :link: BodyFittedLevelSet
      :link-type: doc

      🧩 Body-Fitted Level Sets

      Lagrangian representation relying on mesh conformity to the interface.




Level Set Operations
--------------------

Regularization
^^^^^^^^^^^^^^

Purpose
~~~~~~~~

Extend and smooth a velocity field initially defined on the structural
interface so that it can be used reliably during level set transport.

Why is it needed?
~~~~~~~~~~~~~~~~~

Shape derivatives are naturally computed on the interface only. Regularization
extends this information to the surrounding domain while reducing numerical
oscillations and improving robustness.

Mathematical formulation
~~~~~~~~~~~~~~~~~~~~~~~~

The regularized velocity field :math:`V \in H^1(D)` is obtained by solving

.. math::

   \forall w\in H^{1}(D), \:\:
   \alpha \int_{D}{\nabla V \cdot \nabla w \:dx }
   + \int_{\partial\Omega}{Vw\:ds}
   =
   \int_{\partial\Omega}{v_{\partial\Omega}w\:ds}

where :math:`\alpha > 0` controls the amount of regularization and
:math:`v_{\partial\Omega}` denotes the input interface velocity.

Implementation
~~~~~~~~~~~~~~

Implemented by:

* :py:func:`OpenPisco.Unstructured.Levelset.Regularize`

The variational problem is solved using the finite element capabilities
provided by Muscat [1]_.

----

Transport
^^^^^^^^^

Purpose
~~~~~~~~

Move the level set interface according to a prescribed velocity field.

Why is it needed?
~~~~~~~~~~~~~~~~~

Transport is the main mechanism used to evolve the geometry during shape
and topology optimization iterations.

Mathematical formulation
~~~~~~~~~~~~~~~~~~~~~~~~

The level set function is transported by solving

.. math::

   \frac{\partial \phi}{\partial t}
   +
   \theta \cdot \nabla \phi
   =
   \frac{\partial \phi}{\partial t}
   -
   v |\nabla \phi|
   =
   0

where :math:`\theta = -vn` is the normal velocity field.

Implementation
~~~~~~~~~~~~~~

Implemented by:

* :py:func:`OpenPisco.Unstructured.Levelset.Transport`
* :py:class:`OpenPisco.Unstructured.Levelset.Advect`

The advection problem is solved through an interface to the open-source
Advection library [2]_.

----

Reinitialization
^^^^^^^^^^^^^^^^

Purpose
~~~~~~~~

Restore the signed-distance property of a level set function.

Why is it needed?
~~~~~~~~~~~~~~~~~

After several transport steps, the level set field progressively deviates
from a signed-distance function. Reinitialization improves numerical
stability while preserving the interface location.

Mathematical formulation
~~~~~~~~~~~~~~~~~~~~~~~~

The signed-distance function :math:`d_\Omega` is defined as

.. math::

   d_{\Omega}(x)=
   \begin{cases}
      -d(x,\partial\Omega) & x\in\Omega \\
       0 & x\in\partial\Omega \\
       d(x,\partial\Omega) & \text{otherwise}
   \end{cases}

where

.. math::

   d(x,\partial\Omega)=
   \min_{p\in\partial\Omega}|x-p|.

Implementation
~~~~~~~~~~~~~~

Implemented by:

* :py:class:`OpenPisco.Unstructured.Meshdist`

The signed-distance field is computed through an interface to the open-source
Mshdist library [3]_.







.. [1] https://gitlab.com/drti/muscat

.. [2] https://github.com/ISCDtoolbox/Advection/blob/master/Documentation/advectguide.pdf

.. [3] https://github.com/ISCDtoolbox/Mshdist/blob/master/documentation/mshdistguide.pdf


.. toctree::
   :hidden:

   BodyFittedLevelSet
   StructuredLevelSet
   UnstructuredLevelSet
