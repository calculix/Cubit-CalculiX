Postprocessor
=============

The Postprocessor is an independent application module inside cubit-sim.
The current source implements job-result loading and source management through
commands, source panels and a workspace with its own empty VTK viewport. Loading
results does not yet create result actors or switch workspaces automatically.

This implementation has not been compiled or verified in a native Cubit session.
See :doc:`../developer/postprocessor-verification` for the acceptance checklist.

.. toctree::
   :maxdepth: 1

   commands
   workspace
   ../developer/postprocessor
