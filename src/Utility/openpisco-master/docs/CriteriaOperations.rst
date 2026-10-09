.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

###################
Criteria Operations
###################

OpenPisco provides operators allowing existing criteria to be combined
or transformed.

Overview
########

Criteria operations can be used to scale, combine or modify existing criteria
without changing their implementation.

Unary Operations
################

The following unary operators are available:

* :py:class:`OpenPisco.Optim.Criteria.Criteria.CriteriaFactor`
* :py:class:`OpenPisco.Optim.Criteria.Criteria.Weighted`
* :py:class:`OpenPisco.Optim.Criteria.Criteria.PolynomialVanishingCriteria`
* :py:class:`OpenPisco.Optim.Criteria.Criteria.CriteriaPower`

Binary Operations
#################

The following binary operators are available:

* :py:class:`OpenPisco.Optim.Criteria.Criteria.CriteriaRatio`