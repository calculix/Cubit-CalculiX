CalculiX topology C++ API contracts
===================================

Doxygen extracts project-owned declarations from ``src/CalculiX`` and
``src/Framework``, including commands, GUI, observers, providers and owned utility
adapters. External ``src/Utility`` implementations, bundled binaries/solvers and
generated wrapper/MOC/UI files are excluded. The input list is maintained in
``docs/Doxyfile``; extraction does not compile the plugin. See
:doc:`code-api-index` for the full declaration index and :doc:`code-documentation`
for domain contracts beyond this topology-specific API page.

This page exposes topology entry points, stored records and helper contracts.
Shared provider and storage declarations are in :doc:`framework-api`.
References and pointers below borrow existing domain objects; creating a helper
or calling a static method does not create another model. The mathematical
derivations, units and exact-versus-surrogate distinction are in
:doc:`topology-algorithms`; :doc:`topology` maps the source files. Output-clearing,
partial-commit and availability behavior is documented per operation rather
than promised as a single transaction across all run stages.

Run orchestration
-----------------

.. doxygenclass:: CoreTopologyOptimization
   :project: cubit-sim
   :members: run_topology, wait_topology, kill_topology, run_topology_iteration, start_topology_iteration, create_topology_adjoint_job, prepare_topology_run, build_topology_adjoint_load, finish_topology_iteration, prepare_topology_density_iteration, get_topology_run_info, update, cancel_topology_runs, forget_topology_run

Case and objective operations
-----------------------------

.. doxygenclass:: CoreTopology
   :project: cubit-sim
   :members: create_topology, modify_topology, delete_topology, evaluate_compliance, evaluate_displacement, build_density_design, evaluate_density_volume, build_density_update, evaluate_compliance_oc_residual, evaluate_displacement_projected_residual, set_density_update_diagnostics, check_density_convergence, filter_density_sensitivities, reset_preserved_sets, add_preserved_sets, remove_preserved_sets, resolve_preserved_elements

Stored records and configuration
--------------------------------

Availability flags determine whether diagnostic values are meaningful. Stored
iteration IDs are one-based; job IDs and global mesh entity IDs are separate
identifier domains. ``TopologyCase`` owns case/run records, while the material
and job cores own their associated data. GUI browsing need not change the
stored ``selected_iteration``.

.. doxygenstruct:: TopologyRunInfo
   :project: cubit-sim
   :members:

.. doxygenstruct:: TopologyIterationEvaluation
   :project: cubit-sim
   :members:

.. doxygenstruct:: TopologyVolumeEvaluation
   :project: cubit-sim
   :members:

.. doxygenstruct:: TopologyDensityDesign
   :project: cubit-sim
   :members:

.. doxygenstruct:: TopologyPreservedSetInfo
   :project: cubit-sim
   :members:

.. doxygenstruct:: TopologyCase
   :project: cubit-sim
   :members:

.. doxygenstruct:: TopologyOptions
   :project: cubit-sim
   :members:

Method, objective and run-state identifiers
-------------------------------------------

An enum value being recognized does not establish that the current iteration
controller implements it. ``IterationLimit`` is distinct from convergence.

.. doxygenenum:: TopologyMethod
   :project: cubit-sim

.. doxygenenum:: TopologyObjective
   :project: cubit-sim

.. doxygenenum:: TopologyRunState
   :project: cubit-sim

.. doxygenfunction:: topology_objective_name
   :project: cubit-sim

Density storage and evaluation coordination
-------------------------------------------

.. doxygenclass:: CoreTopologyDensity
   :project: cubit-sim
   :members: set_densities

.. doxygenclass:: CoreTopologyEvaluation
   :project: cubit-sim
   :members: evaluate_compliance, evaluate_displacement

Preservation queries and coordination
-------------------------------------

.. doxygenfunction:: CalculiXCore::get_topology_set_element_ids
   :project: cubit-sim

.. doxygenfunction:: CalculiXCore::collect_topology_default_sets
   :project: cubit-sim

.. doxygenfunction:: CalculiXCore::resolve_topology_preservation
   :project: cubit-sim

FRD displacement inspection and DAT energies
--------------------------------------------

``CoreTopologyResults`` reads final-frame FRD displacements for topology job
inspection/coverage and DAT whole-element energies for compliance. It does not
supply the DAT nodeset measurement used to construct the displacement objective.

.. doxygenclass:: CoreTopologyResults
   :project: cubit-sim
   :members: read_final_displacements, read_job_displacements, read_final_element_energies, select_maximum_displacement, read_iteration_energies

DAT displacement objective measurement
--------------------------------------

The objective path reads the monitored nodeset at the final DAT parser frame.
A missing final-frame quantity is an error rather than an earlier-frame fallback.

.. doxygenclass:: CoreTopologyDisplacementResults
   :project: cubit-sim
   :members: read_final, measure_iteration

DAT integration-point tensors
-----------------------------

These readers expect global-coordinate ``S`` and tensor-shear ``ME`` values.
Reading every required element does not by itself establish coverage of its
expected quadrature-point set; the objective evaluator checks that separately.

.. doxygenclass:: CoreTopologyTensorResults
   :project: cubit-sim
   :members: read_final, read_job

Cubit geometry and connectivity
-------------------------------

.. doxygenclass:: CoreTopologyGeometry
   :project: cubit-sim
   :members: collect_element_volumes, collect_integration_weights, collect_face_owners, append_node_elements, collect_element_nodes

Integration weights
-------------------

.. doxygenfunction:: ElementIntegration::get_rule
   :project: cubit-sim

.. doxygenfunction:: ElementIntegration::calculate_weights
   :project: cubit-sim

Reference integration-point data
--------------------------------

.. doxygenstruct:: ElementIntegration::IntegrationPoint
   :project: cubit-sim
   :members:

Local node-order conversion
---------------------------

.. doxygenfunction:: ElementNodeOrdering::cubit_source_index
   :project: cubit-sim
