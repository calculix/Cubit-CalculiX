Tutorials and journals
======================

.. toctree::
   :maxdepth: 1

   topology-beam

Beam workflow
-------------

The :doc:`topology-beam` tutorial uses
:download:`simple_beam_displacement.jou <../../examples/topology/simple_beam_displacement.jou>`
to prepare the beam and create its displacement topology case.
The journal does not start optimization; the tutorial shows the subsequent
run, inspection, export, and deletion operations.

Derivative verification
-----------------------

* :download:`Single beam sensitivity test <../../examples/topology/check_displacement_sensitivities.jou>`
* :download:`Element-type and objective matrix <../../examples/topology/check_displacement_element_types.jou>`

These journals reset the model. They retain inputs, outputs, model snapshots,
and CSV evidence in the working directory. The matrix tests Y displacement
and magnitude for six element types, with three sampled free elements per
case and three density perturbation sizes. The default matrix runs 288 solves.

The matrix uses the existing Python interface for iteration and result queries.
It reads unfiltered C++ sensitivities from the saved model using Cubit's bundled
``h5py`` and compares them with central finite differences. Inspect all
perturbations in the detailed CSVs: the summary passes a sampled element when
at least one perturbation satisfies the configured tolerance.

These checks verify the unfiltered objective derivatives. They do not establish
that filtered sensitivities are derivatives of a separately defined filtered
objective. See :doc:`../developer/topology-algorithms` for that distinction.

Recorded coverage
-----------------

The one-solver-thread run on 20 October 2026 met that criterion for all
36 sampled derivatives (six element types, two objectives, three elements).
The result reader used 20 threads. Preserved-element sensitivities were zero.
Some smallest perturbations exceeded tolerance for weak derivatives, consistent
with limited result precision.

The recorded matrix used density perturbations 0.01, 0.003, and 0.001. A sampled
element passed when at least one perturbation met
:math:`|g_{\mathrm{FD}}-g_{\mathrm{C++}}|\le 10^{-6}+0.01|g_{\mathrm{C++}}|`.
The table reports the largest, across three sampled elements, of each element's
best relative error over those perturbations. It does not mean every
perturbation passed.

.. list-table:: Recorded best-perturbation relative errors (percent)
   :header-rows: 1

   * - Element
     - Absolute Y objective
     - Magnitude objective
   * - C3D4
     - 0.679080
     - 0.230021
   * - C3D6
     - 0.013755
     - 0.005285
   * - C3D8
     - 0.011261
     - 0.016488
   * - C3D10
     - 0.023172
     - 0.072523
   * - C3D15
     - 0.002176
     - 0.001340
   * - C3D20
     - 0.019265
     - 0.017486

The local evidence run is identified by
``displacement_matrix_20261020_171632_915539_summary.csv`` and the associated
per-case ``_finite_differences.csv`` files in the Cubit working directory.
These generated evidence files are not bundled with the manual. Retain the
detailed CSVs when repeating the journals; small perturbations of weak
derivatives can amplify output rounding. A positive derivative in the C3D10
magnitude case also exercised the signed displacement sensitivity path.

This supports the tested straight-sided C3D4, C3D6, C3D8, C3D10, C3D15, and
C3D20 elements at a single monitored node. It does not verify curved elements,
switching maxima, arbitrary analyses, or every model configuration. Earlier
multithreaded solver runs produced inconsistent raw solver outputs; their cause
was not investigated. Use one solver thread to reproduce the recorded checks.
