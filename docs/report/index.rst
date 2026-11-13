Report workspace
================

.. toctree::
   :maxdepth: 2

   workspace
   setup-images

Postprocessor snapshots
-----------------------

``report image add postprocessor view|plot "name" [caption "text"]`` appends
a frozen PNG to the active generated Report. See :doc:`../postprocessor/capture`.
The Figures section and its image provenance are embedded in project storage and
preserved across report update/regeneration. Later source changes do not refresh
these user snapshots.
