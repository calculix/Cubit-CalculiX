Topology run control
====================

Start
-----

.. code-block:: text

   ccx run topology <id> [wait]

Without ``wait``, the run returns control while jobs progress. With ``wait``,
the command waits until the topology run finishes or fails. Starting a case
resets the run data belonging to that case. Review or save earlier evidence
before starting the same case again.

Wait
----

.. code-block:: text

   ccx wait topology <id>

Wait for an already started topology run.

Kill
----

.. code-block:: text

   ccx kill topology <id>

Request termination of the topology run and its active solver job.

Example
-------

.. code-block:: text

   ccx run topology 1
   ccx wait topology 1

Creation, modification, preservation selections, preview/projection, animation,
and wrapping will receive complete parameter references in later chapters.
See :doc:`../user/topology` for a first workflow.
