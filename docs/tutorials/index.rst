Tutorials and journals
======================

.. toctree::
   :maxdepth: 1

   topology-beam

Beam workflow
-------------

The :doc:`topology-beam` tutorial uses
:download:`simple_beam_displacement.jou <../../examples/calculix/topology/simple_beam_displacement.jou>`
to prepare the beam and create its displacement topology case.
The journal does not start optimization; the tutorial shows the subsequent
run, inspection, export, and deletion operations.

Derivative verification
-----------------------

See :doc:`../developer/topology-verification` for the historical displacement
matrix, finite-difference procedure, acceptance criteria, evidence status and
remaining coverage. The former verification journals were deleted from
``examples/calculix/topology``; they are not offered here as current runnable downloads.
The historical material-editing procedure needs adaptation to grouped exports.
The beam workflow above is an example workflow, not a derivative regression test.
