****************************************
Why OpenPisco
****************************************

OpenPisco is a research and development platform dedicated to topology optimization based on the level set method. 
It has been designed to support both industrial and academic research by providing a flexible environment for developing topology optimization methodologies.

The platform pursues several objectives:

* Improve the understanding of the numerical challenges inherent to level set-based topology optimization.
* Enable rapid prototyping of new developments driven by industrial and research needs.
* Provide a unified interface to the various external tools commonly used in topology optimization workflows.

To achieve these goals, OpenPisco has been designed around three core principles:

* **Modularity:** OpenPisco is built from weakly coupled, highly cohesive
  components. Individual modules can be used independently of the topology
  optimization framework. For example, physical solvers or remeshing tools may
  be employed as standalone components without any dependency on topology
  optimization features.

* **Interoperability:** The architecture makes it easy to replace one
  component with another, whether it is a physical solver, an optimization
  algorithm, or an optimization criterion. This flexibility allows users to
  tailor the platform to their specific needs.

* **Extensibility:** The platform has been designed with future developments in
  mind. A plugin-based infrastructure enables users to integrate their own
  functionalities, such as mesh readers, optimization algorithms, or physics
  solvers, without modifying the existing source code.


Given Python's widespread adoption within the scientific community, and the availability of powerful numerical libraries, OpenPisco is implemented almost entirely in Python. 
The Python package distributed with the platform is also named OpenPisco.
Nevertheless, OpenPisco is designed to interface with external tools that may be written in other languages and do not necessarily provide a Python API. 
This includes, for example, finite element and finite volume solvers.

Features and capabilities
"""""""""""""""""""""""""

OpenPisco provides the following components:

* An algorithmic toolbox specialized in level set methods.
* A generic interface to several well-known PDE solvers (finite element and finite volume), supporting various types of physical analyses.
* Algorithms dedicated to the solution of constrained optimization problems.
* A collection of physical and geometrical optimization criteria.
* A toolbox for solving generic topology optimization problems.
* An advanced interface to the remeshing software MMG3D.
* A graphical user interface (GUI).

Although topology optimization is the primary application of OpenPisco, many of its components can be used independently within other workflows. For example, the solver interfaces, optimization criteria, and remeshing tools are fully self-contained and may be employed in contexts unrelated to topology optimization.

OpenPisco is built on top of the Muscat library, which constitutes one of its core dependencies. 
Muscat provides capabilities such as mesh handling, Finite Elements and linear algebra operations.

