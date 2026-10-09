****************************************************
Developer Guide
****************************************************

This guide provides a comprehensive overview of the development standards, architectural principles and best practices for contributing to the OpenPisco project. Adhering to these guidelines is essential for maintaining code quality, readability, and consistency across the platform.


Core Architectural Principles
====================================================

*   **Modularity**: Components are self-contained and loosely coupled.
*   **Extensibility**: New features are integrated, for instance via the **Factory design pattern**.
*   **Interoperability**: Components can be swapped easily (e.g., changing physical solvers).

---

Software Architecture
====================================================

Macroscopic Components
----------------------------------------------------
1.  **Optimization Engine**: Drives the optimization iterations.
#.  **Level-set Engine**: Manages the evolution of the level-set function and interfaces with meshing tools.
#.  **Criteria Engine**: Computes the value and sensitivity of objectives and constraints by interfacing with physical solvers.

Package Structure
----------------------------------------------------
*   ``OpenPisco.Actions``: Reusable operations for the DSL.
*   ``OpenPisco.Optim``: Core optimization logic (Algorithms, Criteria, Problems).
*   ``OpenPisco.PhysicalSolvers``: Interfaces to PDE solvers.
*   ``OpenPisco.ExternalTools``: Scripts and templates for external solvers.
*   ``OpenPisco.Unstructured`` / ``OpenPisco.Structured``: Level-set and mesh handling tools.
*   ``OpenPisco.CLApp`` / ``OpenPisco.QtApp``: Command-line and GUI applications.

---

XML-based Language
----------------------------------------------------
OpenPisco provides a higher-level Language, whose syntax is losely related to XML. Most of the platform capabilities are covered by such a language. 
In order to use it for new develoment, one has to rely on a specific data structure based on the factory pattern described thereafter for each module of OpenPisco.
The RegisterClass and Create are respectively used to register a class and a constructor using the string name and create a instance of a class associated to the key name.

Namely, for the associated module, we refer to:

*   **Actions**: ``OpenPisco.Actions.ActionFactory``
*   **Optimization Algorithms**: ``OpenPisco.Optim.Algorithms.OptimAlgoFactory``
*   **Optimization Criteria**: ``OpenPisco.Optim.Criteria.CriteriaFactory``
*   **PhysicalSolvers**: ``OpenPisco.PhysicalSolvers.PhysicalSolverFactory``

It covers most of the likely case where one would seek to extend the XML-based langage.

Development Standards
====================================================

The Factory Pattern
----------------------------------------------------
New components **must be registered** with the corresponding factory (e.g., ``CriteriaFactory``, ``PhysicalSolverFactory``) to be accessible by the application.

Naming Conventions
----------------------------------------------------

*   **Modules, Packages, Classes, Functions, Methods**: ``PascalCase``
*   **Variables & Arguments**: ``camelCase``

Code Formatting
----------------------------------------------------

*   **Indentation**: 4 spaces.
*   **Line Length**: Max 88 characters.
*   **Quotes**: Double quotes (``"``).

Error Handling
----------------------------------------------------

*   Use specific exceptions, not generic ``Exception``.
*   Use status returns (``RETURN_SUCCESS``) for operations that can fail gracefully.

---

Muscat Best Practices
====================================================

Muscat is the C++/Python foundation for our finite element computations. Correct usage is critical to avoid errors and ensure performance.

Initialization
----------------------------------------------------
Several Muscat modules require explicit initialization before use. Failure to do so will lead to runtime errors. Always initialize factories when needed:

.. code-block:: python

   from Muscat.IO.IOFactory import InitAllReaders, InitAllWriters
   InitAllReaders()
   InitAllWriters()

   from Muscat.FE.Spaces.FESpaces import InitAllSpaces
   InitAllSpaces()

Typical FEA Workflow
----------------------------------------------------

1.  **Mesh Loading/Creation**: (``ReadMesh``, ``CreateCube``)
#.  **Space and Numbering**: (``LagrangeSpaceP1``, ``ComputeDofNumbering``)
#. **Field Creation**: (``FEField``)
#.  **Weak Form Definition**: (``SymWeakForm``)
#.  **Integration**: (``IntegrateGeneral``)
#.  **Solve**: (``LinearSolver``)

Important Developer Notes
----------------------------------------------------

*   **``CheckIntegrity()`` Functions**: Use these self-contained tests to understand and debug modules.
*   **Object Lifetimes**: Be mindful of Python's garbage collector. Ensure Python objects wrapping C++ data (meshes, fields) remain in scope as long as they are needed by the C++ core to prevent segmentation faults.
*   **In-place Operations**: Be aware of functions that modify objects in-place. Copy objects if you need to preserve the original.
*   **Data Types**: Ensure NumPy arrays have the correct ``dtype`` (``MuscatFloat`` or ``MuscatIndex``).

---

Common Development Workflows
====================================================

Interfacing a New Physical Solver
----------------------------------------------------

1.  Inherit from ``OpenPisco.PhysicalSolvers.SolverBase``.
#.  Implement ``SolveByLevelSet(levelset)`` and ``GetNodalSolution()``.
#.  Register with the ``PhysicalSolverFactory``.

See also :ref:`Interface your own physical solver`.

Creating a New Optimization Criterion
----------------------------------------------------

This is one of the most common and powerful ways to extend OpenPisco. The process involves understanding the core concepts and then following a clear implementation path.

**Core Concepts**
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

*   **Geometrical vs. Physical Criteria**:

    *   *Geometrical Criteria* inherit from ``CriteriaBase`` and depend only on the shape's geometry (e.g., volume, perimeter). They are self-contained.
    *   *Physical Criteria* inherit from ``PhysicalCriteriaBase`` and depend on the results of a PDE simulation. They are decoupled from specific solvers via a generic ``problem`` object.

*   **The "Auxiliary Quantities" Data Contract**: This is a critical concept for physical criteria. Instead of accessing solver-specific results, a criterion requests the data it needs (e.g., "stress", "potential_energy") using standardized string names. This allows a single criterion to work with any solver that can provide the requested data.
*   **Sensitivity and the Adjoint Method**: For complex physical criteria, calculating the sensitivity efficiently requires solving a second "adjoint" problem. The criterion itself is responsible for defining and triggering the solution of this adjoint problem, typically using a second, internal solver instance stored in ``self.adjointProblem``.

**Practical Implementation Steps**
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



1.  **Choose the Right Base Class**

    * For geometry-only criteria: ``OpenPisco.Optim.Criteria.Criteria.CriteriaBase``
    * For PDE-dependent criteria: ``OpenPisco.Optim.Criteria.Criteria.PhysicalCriteriaBase``

#.  **Implement the Core Criterion Class**: Create a new Python file in ``src/OpenPisco/Optim/Criteria/`` and implement the essential methods:

    *   ``__init__()``: Call ``super()``, set a default name with ``self.SetName()``, and initialize parameters.
    *   ``UpdateValues(levelSet)``: This is the main entry point. Its job is to compute the criterion's value and sensitivity and store them in ``self.f_val`` and ``self.fSensitivity_val``. For physical criteria, this is where you interact with ``self.problem`` (and ``self.adjointProblem`` if needed) to run simulations and retrieve auxiliary quantities.
    *   ``GetValue()``: Return ``self.f_val``.
    *   ``GetSensitivity()``: Return ``self.fSensitivity_val``.

#.  **Implement Data Export (for Debugging)**: To visualize fields in the GUI, implement:

    *   ``GetNumberOfSolutions()``: Return the number of fields to export.
    *   ``GetSolution(i)``: Return the NumPy array for the i-th solution.
    *   ``GetSolutionName(i)``: Return the name for the i-th solution.

#.  **Register with the Factory**: Make the criterion available to the application by registering it with the ``CriteriaFactory``. This is typically done at the bottom of the file where your new criterion is defined.

    *   **Simple Case**: ``RegisterCriteriaClass("MyCriterionName", MyCriterionClass)``
    *   **Complex Case (with XML parameters)**: Provide a dedicated constructor function and register it: ``RegisterCriteriaClass("MyCriterionName", MyCriterionClass, CreateMyCriterionFunction)``

See also :ref:`Create your own optimization criterion`.

Interfacing a New Optimization Algorithm
----------------------------------------------------

1.  Inherit from ``OpenPisco.Optim.Algorithms.OptimAlgoBase``.
#.  Implement ``DoOneStep()`` to compute a descent direction and call ``TryToAdvance()``.
#.  Register with the ``OptimAlgoFactory``.

See also :ref:`Interface your own optimization algorithm`.