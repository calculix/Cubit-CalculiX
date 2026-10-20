C++ API reference: initial topology coverage
============================================

Doxygen extracts declarations from project-owned Core, Commands, GUI,
PythonInterface, UserOptions, and the selected topology utilities. Bundled
Eigen, CGAL, HDF5, and OpenPisco sources are not included.

This initial page exposes selected topology entry points. Many declarations
do not yet have complete developer comments; API extraction is not a substitute
for documenting contracts, ownership, error returns, and mathematics.

Run orchestration
-----------------

.. doxygenclass:: CoreTopologyOptimization
   :project: Cubit-CalculiX
   :members: run_topology, wait_topology, kill_topology, prepare_topology_run, finish_topology_iteration

Case and objective operations
-----------------------------

.. doxygenclass:: CoreTopology
   :project: Cubit-CalculiX
   :members: create_topology, modify_topology, delete_topology, evaluate_compliance, evaluate_displacement

Integration weights
-------------------

.. doxygenfunction:: ElementIntegration::calculate_weights
   :project: Cubit-CalculiX
