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
   :members: run_topology, wait_topology, kill_topology, prepare_topology_run, build_topology_adjoint_load, finish_topology_iteration

Case and objective operations
-----------------------------

.. doxygenclass:: CoreTopology
   :project: Cubit-CalculiX
   :members: create_topology, modify_topology, delete_topology, evaluate_compliance, evaluate_displacement, build_density_design, evaluate_density_volume, build_density_update, evaluate_compliance_oc_residual, set_density_update_diagnostics, check_density_convergence, filter_density_sensitivities, reset_preserved_sets, add_preserved_sets, remove_preserved_sets, resolve_preserved_elements

Preservation queries and coordination
-------------------------------------

.. doxygenfunction:: CalculiXCore::get_topology_set_element_ids
   :project: Cubit-CalculiX

.. doxygenfunction:: CalculiXCore::collect_topology_default_sets
   :project: Cubit-CalculiX

.. doxygenfunction:: CalculiXCore::resolve_topology_preservation
   :project: Cubit-CalculiX

Integration weights
-------------------

.. doxygenfunction:: ElementIntegration::get_rule
   :project: Cubit-CalculiX

.. doxygenfunction:: ElementIntegration::calculate_weights
   :project: Cubit-CalculiX
