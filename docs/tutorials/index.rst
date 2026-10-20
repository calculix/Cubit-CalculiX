Tutorials and journals
======================

Beam workflow
-------------

:download:`simple_beam_displacement.jou <../../examples/topology/simple_beam_displacement.jou>`
creates the model, material, section, rigid-body constraint, force, fixed
boundary condition, step, and mesh, then runs one displacement iteration.

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

Recorded coverage
-----------------

The one-solver-thread run on 20 October 2026 met that criterion for all
36 sampled derivatives (six element types, two objectives, three elements).
The result reader used 20 threads. Preserved-element sensitivities were zero.
Some smallest perturbations exceeded tolerance for weak derivatives, consistent
with limited result precision.

This supports the tested straight-sided C3D4, C3D6, C3D8, C3D10, C3D15, and
C3D20 elements at a single monitored node. It does not verify curved elements,
switching maxima, arbitrary analyses, or every model configuration. Earlier
multithreaded solver runs produced inconsistent raw solver outputs; their cause
was not investigated. Use one solver thread to reproduce the recorded checks.
