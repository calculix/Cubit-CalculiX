Complete project-owned declaration index
=========================================

The Doxygen input covers ``src/Framework`` and ``src/CalculiX``, including module
entry points, commands, observers, GUI, project-owned adapters and utilities.
External ``src/Utility`` implementations, bundled binaries/solvers and generated
wrapper/MOC/UI files are excluded. Private/static extraction is enabled for
developer implementation navigation; extraction does not make an internal method
a supported public API.

This index is generated from the current source when documentation is built.
Existing comments are shown where present. Undocumented declarations are not
invented contracts. Read :doc:`code-documentation` and its domain chapters for
ownership, state transitions, identifier domains, errors and numerical limits.
The :doc:`code-catalog/index` links every included owned source/build file
independently of Doxygen parsing.

No Doxygen/Sphinx build was run during the documentation-only pass. Any SDK
macro/parser issues in extraction remain to be checked at the next authorized
documentation build.

.. doxygenindex::
   :project: Cubit FEA
   :outline:
