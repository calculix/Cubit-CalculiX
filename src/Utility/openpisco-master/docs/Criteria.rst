.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

########
Criteria
########

Criteria define the objective functions and constraints appearing in an
:doc:`optimization problem <OptimizationProblem>`.

A criterion is responsible for evaluating both:

* its value;
* its sensitivity with respect to the current optimization state.

All criteria derive from a common interface and implement:

* :py:func:`OpenPisco.Optim.Criteria.Criteria.UpdateValues`
* :py:func:`OpenPisco.Optim.Criteria.Criteria.GetValue`
* :py:func:`OpenPisco.Optim.Criteria.Criteria.GetSensitivity`

Overview
########

OpenPisco provides two families of criteria:

* **Physical criteria**, which rely on the solution of a physical problem.
* **Geometrical criteria**, which depend only on the geometry represented by
  the level-set function.

In addition, criteria can be combined using algebraic operations such as
weighted sums, powers and ratios.

Documentation
#############

.. grid:: 3
   :gutter: 3

   .. grid-item-card::
      :link: PhysicalCriteria
      :link-type: doc

      ⚙️ Physical Criteria

      Compliance, stresses, temperatures, eigenfrequencies,
      buckling, aerodynamic forces and more.

   .. grid-item-card::
      :link: GeometricalCriteria
      :link-type: doc

      📐 Geometrical Criteria

      Volume, surface, mass, thickness and center-of-mass measures.

   .. grid-item-card::
      :link: CriteriaOperations
      :link-type: doc

      ➕ Criteria Operations

      Weighted criteria, ratios, powers and other combinations.

.. toctree::
   :hidden:

   PhysicalCriteria
   GeometricalCriteria
   CriteriaOperations



.. [1] https://gitlab.com/drti/muscat
.. [2] G. Allaire, Ch. Dapogny, A linearized approach to worst-case design in parametric and geometric shape optimization, M3AS Vol. 24, No. 11 (2014) 2199-2257
.. [3] G. Allaire, F. Jouve, G. Michailidis, Thickness control in structural optimization via a level set method, SMO 53, 1349-1382 (2016)
.. [4] G. Michailidis, Manufacturing constraints and multi-phase shape and topology optimization via a level-set method. Diss. Ecole Polytechnique X (2014)
