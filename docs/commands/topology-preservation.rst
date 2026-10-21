Topology preservation commands
==============================

These commands edit case preservation rules. They do not edit the contents of
Cubit sets or remove analysis definitions. The case must exist and must not
be running. Changes affect subsequent runs, not the resolved flags of the
current stored design; see :doc:`../user/topology-preservation`.

Add selections
--------------

.. code-block:: text

   ccx topology <id> add preserved
     [block <ids...>] [nodeset <ids...>] [sideset <ids...>]

The wrapped syntax represents one command. Supply at least one set ID across
the optional lists, using existing positive block/nodeset/sideset IDs.
The command validates all specified IDs before applying the lists.

Add records an inclusion and cancels an exclusion for the same set kind/ID.
Repeated additions do not duplicate the stored selection.

.. code-block:: text

   ccx topology 1 add preserved block 2 nodeset 5 6 sideset 7

Remove selections
-----------------

.. code-block:: text

   ccx topology <id> remove preserved
     [block <ids...>] [nodeset <ids...>] [sideset <ids...>]

Supply at least one set ID, with the same existence requirements as add.
Remove records an exclusion and cancels the set's explicit inclusion. This
can exclude an automatically selected set even if it was never explicitly
added to the case. It does not remove direct automatic element selections.

.. code-block:: text

   ccx topology 1 remove preserved nodeset 5

The excluded set can still overlap other effective sets. Its elements are not
necessarily free after resolution.

Reset overrides
---------------

.. code-block:: text

   ccx topology <id> reset preserved

Clear explicit inclusions and exclusions, restoring automatic preservation
rules for the next run. This does not clear all preservation. Existing resolved
flags and stored results remain unchanged immediately after the command.

.. code-block:: text

   ccx topology 1 reset preserved

Overlap example
---------------

Assume case 1, block 2, and nodeset 5 already exist:

.. code-block:: text

   ccx topology 1 add preserved block 2 nodeset 5
   ccx topology 1 remove preserved nodeset 5

Block 2 remains included. Its participating elements stay preserved even if
they also touch nodeset 5. A direct selection or another effective set can
likewise preserve an element despite that nodeset exclusion.

All snippets require the stated existing case and sets. They are not standalone
model journals. See :doc:`topology` for creation and run-control commands.
