Topology: mathematics and implementation map
============================================

This page establishes the notation and links to the implementing code.
See :doc:`topology-algorithms` for the derivative derivations, integration
conventions, sensitivity filter, update algorithms, and convergence checks.

Linear model and density interpolation
--------------------------------------

For fixed loads, homogeneous prescribed displacements, and the supported
symmetric linear-elastic model:

.. math::

   K(\rho)u=f,\qquad
   K_e=s_eK_e^{\mathrm{base}},\qquad
   s_e=v+(1-v)\rho_e^p.

Preserved elements are fixed at :math:`\rho_e=s_e=1`. Their stored design
sensitivities are zero because their densities are not optimization variables.

Compliance
----------

.. math::

   C=f^T u=2\sum_e U_e,\qquad
   U_e=\tfrac12u_e^T K_eu_e,\qquad
   \frac{\partial C}{\partial\rho_e}
   =-\frac{2U_e}{s_e}(1-v)p\rho_e^{p-1}.

Displacement adjoint
--------------------

For a monitored node, the objective is the absolute selected component or the
vector norm. For a nodeset, the implementation selects the node attaining the
maximum. Away from a change in that node or a nondifferentiable zero:

.. math::

   K^T\lambda=\frac{\partial J}{\partial u},\qquad
   \frac{\partial J}{\partial\rho_e}
   =-\lambda_e^T\frac{\partial K_e}{\partial\rho_e}u_e.

The component adjoint load is a signed unit load; the magnitude load is
:math:`u/\lVert u\rVert` at the controlling node. The current implementation
uses the zero subgradient at zero monitored displacement and skips the adjoint.

Using physical stress and adjoint mechanical strain:

.. math::

   I_e=\sum_q
   \left(\sigma_e^{\mathrm{physical}}:
   \varepsilon_e^{\mathrm{adjoint}}\right)_q w_q\det J_q,
   \qquad
   \frac{\partial J}{\partial\rho_e}
   =-\frac{I_e}{s_e}(1-v)p\rho_e^{p-1}.

There is **no factor of one half** in this contraction: it is the bilinear
adjoint product :math:`\lambda_e^T K_eu_e`, not stored strain energy.
For tensor shear components, the contraction uses normal terms once and
the three shear terms twice. Both fields must use matching global coordinates
and CalculiX integration-point IDs.

Configuration and run lifecycle
-------------------------------

Defaults belong to ``CoreTopology``. Creation initializes a ``TopologyCase``
with those defaults, applies supplied ``TopologyOptions`` entries, validates
the configuration, and stores it. Each ``has_*`` flag distinguishes an omitted
option from an explicitly supplied value, including zero where allowed.
The GUI and commands must not introduce a competing set of defaults.

Modification rejects an active case, copies the existing configuration,
applies supplied options, validates the copy, and only then replaces the
stored configuration. Selections and iteration records are preserved.
Configuration acceptance does not establish support for running every enum
value: mass and level set remain unavailable in the iteration controller.

``CalculiXCore::begin_topology_run()`` validates the required cores, current
state, supported method/objective, analysis, and displacement selection.
It then calls ``reset_topology_run()`` before marking the case running.
Reset removes associated physical/adjoint job records and clears evaluations,
density designs, resolved preservation flags, selection, and prepared topology
material/section collections. The case settings and explicit preservation
selections remain. Remaining initialization and output preparation occur in
the controller, so a later preparation failure does not restore the old run.

``CoreTopologyOptimization`` owns the transient run context. Its phases are
``Preparing``, ``Starting``, ``Waiting``, ``StartingAdjoint``,
``WaitingAdjoint``, ``Evaluating``, and ``Finished``. The controller advances
these as jobs are polled. A nonzero displacement objective uses an adjoint;
compliance and the zero-displacement branch do not require that extra solve.
Each context has a serial so stale work can be distinguished from a new run.

With ``wait=false``, a successful call establishes startup/acceptance rather
than completion. With ``wait=true``, it enters the wait loop, polls jobs,
processes Qt events, and returns success only for convergence or the iteration
limit. Polling/event processing is not a separate result-reader worker.
Nested waits and waits without a current controller context are rejected.
Persisted case results do not recreate an active controller context on load.

Kill sets ``stop_requested`` and requests polling when possible. Subsequent
advancement terminates the active process and confirms that it is no longer
running before setting the terminal state. A true return from kill therefore
means request acceptance, not guaranteed immediate process termination.
Completed evaluations remain available. A failed run uses the corresponding
stop/termination path with the failed state rather than continuing updates.

Calls between domain cores use the domain-specific interfaces (see :doc:`interface-layout`). ``CalculiXCore``
coordinates its owned collections and Cubit queries; the optimization controller
does not include other domain-core headers to obtain their stored data.

See :doc:`../user/topology-settings`, :doc:`../user/topology-running`, and
:doc:`../commands/topology` for the corresponding public behavior.

Implementation entry points
---------------------------

* :cpp:class:`CoreTopologyOptimization` orchestrates physical/adjoint jobs and evaluation.
* :cpp:class:`CoreTopology` stores cases and performs objective/density calculations.
* :cpp:func:`ElementIntegration::calculate_weights` produces :math:`w_q\det J_q`
  using Cubit/Exodus coordinates and CalculiX integration rules.
* The shared node-order helper converts local connectivity positions; its
  element-type IDs must match the installed Cubit SDK. WEDGE15 swaps midside
  positions 10--12 with 13--15 when exported to C3D15.

Cross-core operations pass through the domain-specific interfaces (see :doc:`interface-layout`); geometry and
materials are supplied through the existing core and Cubit interfaces.

See :doc:`../tutorials/index` for verification scope and
:doc:`../user/topology` for the user workflow.
