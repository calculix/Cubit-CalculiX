CGAL alpha wrapping
===================

In the CalculiX workspace, choose **CCX > CGAL > Alpha wrap**. CGAL is a sibling of
Topology in navigation and uses the alpha-envelope icon.

Choose Blocks to wrap one or more selected mesh blocks. Choose Topology iteration
to select a case, a completed iteration, and a density threshold. For topology,
preserved elements are always retained; other elements are retained at densities
greater than or equal to the threshold. Wrapping requires an idle topology case
and the stored elements to remain available in the current mesh.

Alpha and offset use model length units. Empty alpha uses the input bounding-box
diagonal divided by 40; empty offset uses the resolved alpha divided by 30. Smaller
alpha retains finer detail, and smaller offset gives a tighter fit. These defaults
are resolved by the wrap command, not by the panel.

Choose an STL filename, whether to import the generated STL into Cubit, and whether
to overwrite an existing file. With an empty filename the panel chooses a name
based on its selected source. Wrap submits one of the existing commands::

   ccx wrap block 1 2 file "blocks.stl" no_import
   ccx wrap topology 1 iteration 4 threshold 0.5 file "beam.stl" overwrite

The result is a wrapped STL surface, not a new volume analysis mesh. Higher-order
elements use the existing corner-node approximation. The panel and Topology
Management have independent controls and use the same command backend. Wrap
execution retains the existing synchronous command behavior.
