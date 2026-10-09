.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

OpenPiscoCL
===========

OpenPiscoCL is the command-line interface of the OpenPisco platform.

It provides a convenient way to define, and automate studies without
directly interacting with the Python API. Studies are described using the
OpenPisco Domain-Specific Language (DSL), an XML-based language designed to
describe the various components of a topology optimization workflow.

By relying on the DSL, users can focus on describing their problem rather than
implementing it. OpenPiscoCL is therefore particularly well suited for:

* automation and batch execution,
* reproducible computational studies,
* parameter sweeps and design exploration,
* integration into larger simulation workflows.

From a conceptual point of view, an OpenPisco study is assembled from a set of
independent components. These components describe the geometry, computational
meshes, level sets, physical analyses, optimization problems, optimization
algorithms, outputs, and the sequence of actions to be performed.

A typical study therefore follows the structure illustrated below:

::

   Study
   ├── Geometry (Zones)
   ├── Meshes
   ├── Level Sets
   ├── Physical Problems
   ├── Optimization Problems
   ├── Optimization Algorithms
   ├── Outputs
   └── Actions

This organization reflects the modular architecture of OpenPisco. Each
component can be defined, reused, or replaced independently, enabling users to
build workflows of varying complexity while maintaining readability and
flexibility.

The following sections introduce the main principles of the DSL and illustrate
its usage through practical examples. For a complete description of all
available tags, attributes, parameters, and actions, refer to the
:doc:`XML Reference Guide <XmlReference>`.