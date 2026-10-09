.. This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.

####################################################
OpenPisco Developer Quick Reference
####################################################

This document provides a high-level overview of the core components of the OpenPisco library, focusing on the main classes, their roles, and their interactions.

Core Concepts
====================================================

OpenPisco is a topology optimization platform built on top of the Muscat library. It uses the level set method to represent and evolve the geometry of the optimized shape. The core workflow involves:

* **Level Set**: An implicit representation of the shape.
* **Physical Solver**: A component that solves the physics (e.g., mechanics, thermal) for a given shape.
* **Criterion**: A function that evaluates the performance of the shape (e.g., volume, compliance).
* **Optimization Algorithm**: An algorithm that iteratively updates the level set to improve the criterion.
* **Actions**: A set of commands to control the optimization process.


LevelSetBase
====================================================

Abstract base class for all level set objects in OpenPisco:

-   **File**: `src/OpenPisco/LevelSetBase.py`
-   **Class**: `LevelSetBase`

It manages the level set function (`phi`) and its support mesh.

+----------------+-----------------------------------------------------------------------------------------------------+
| Key Attributes |                                                                                                     |
+----------------+-----------------------------------------------------------------------------------------------------+
| `support`      | Muscat `Mesh` object on which the level set is defined                                              | 
+----------------+-----------------------------------------------------------------------------------------------------+
| `phi`          | NumPy array representing the level set function                                                     | 
+----------------+-----------------------------------------------------------------------------------------------------+
| `conform`      | Boolean indicating if the level set is conforming (body-fitted mesh) or non-conforming (fixed mesh) | 
+----------------+-----------------------------------------------------------------------------------------------------+


+------------------------------------------+--------------------------------------------------------------------+
| Key Methods                              |                                                                    |
+------------------------------------------+--------------------------------------------------------------------+
| `SetSupport(support)`                    | Sets the support mesh for the level set.                           |
+------------------------------------------+--------------------------------------------------------------------+
|`Initialize(Function)`                    | : Initializes the level set function `phi` using a given function  |
+------------------------------------------+--------------------------------------------------------------------+
| `Transport(velocity, ...)`               | Evolves the level set based on a velocity field                    |
+------------------------------------------+--------------------------------------------------------------------+
| `Reinitialize()`                         | Reinitializes the level set function as a signed distance function |
+------------------------------------------+--------------------------------------------------------------------+
| `TransportAndReinitialize(velocity, ...)`| Evolves the level set based on a velocity field  and reinitializes |
+------------------------------------------+--------------------------------------------------------------------+
| `Regularize(velocity, ...)`              | regularization of an input velocity field                          |
+------------------------------------------+--------------------------------------------------------------------+



Optimization (`Optim` module)
====================================================

This module contains the core logic for the optimization process, including the algorithms and the criteria used to evaluate the shape.

OptimAlgoBase
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   **File**: `src/OpenPisco/Optim/Algorithms/OptimAlgoBase.py`
-   **Class**: `OptimAlgoBase`

This is the abstract base class for all optimization algorithms. It defines the main optimization loop and provides an interface to the optimization problem.

+----------------+----------------------------------------------+
| Key Attributes |                                              |
+----------------+----------------------------------------------+
| `optimProblem` | The optimization problem to be solved        |
+----------------+----------------------------------------------+
| `stepSize`     | The step size for the optimization algorithm |
+----------------+----------------------------------------------+
| `direction`    | The descent direction for the optimization   |
+----------------+----------------------------------------------+

+------------------------+--------------------------------------------------------------+
| Key Methods            |                                                              |
+------------------------+--------------------------------------------------------------+
| `RunOptimization()`    |The main optimization loop                                    |
+------------------------+--------------------------------------------------------------+
| `DoOneStep()`          | Performs a single optimization step                          |
+------------------------+--------------------------------------------------------------+
| `Advance(optimProblem)`| Advances the optimization problem in the computed direction  |
+------------------------+--------------------------------------------------------------+

CriteriaBase & PhysicalCriteriaBase
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   **File**: `src/OpenPisco/Optim/Criteria/Criteria.py`
-   **Classes**: `CriteriaBase`, `PhysicalCriteriaBase`

These are the base classes for all optimization criteria. `CriteriaBase` is the generic interface, while `PhysicalCriteriaBase` is a specialization for criteria that depend on a physical simulation.


+----------------------------------------+---------------------------------------------------------------+
| Key Attributes                         |                                                               |
+----------------------------------------+---------------------------------------------------------------+
| `upperBound`                           | The upper bound for the criterion (for constraints)           | 
+----------------------------------------+---------------------------------------------------------------+
| `targetValue`                          | The target value for the criterion (for equality constraints) | 
+----------------------------------------+---------------------------------------------------------------+
| `problem` (in `PhysicalCriteriaBase`)  | Sets the physical solver for the criterion.                   | 
+----------------------------------------+---------------------------------------------------------------+


+--------------------------------------------------+----------------------------------------------------+
| Key Methods                                      |                                                    |
+--------------------------------------------------+----------------------------------------------------+
|`UpdateValues(point, phi)`                        | Updates the value and sensitivity of the criterion | 
+--------------------------------------------------+----------------------------------------------------+
|`GetValue()`                                      | Returns the current value of the criterion         | 
+--------------------------------------------------+----------------------------------------------------+
|`GetSensitivity()`:                               | Returns the current sensitivity of the criterion   | 
+--------------------------------------------------+----------------------------------------------------+
|`SetProblem(problem)` (in `PhysicalCriteriaBase`) | Sets the physical solver for the criterion         | 
+--------------------------------------------------+----------------------------------------------------+


PhysicalSolvers
====================================================

-   **File**: `src/OpenPisco/PhysicalSolvers/SolverBase.py`
-   **Class**: `SolverBase`

This is the base class for all physical solvers. It provides a generic interface for solving physical problems on a given domain.


+-------------------------+------------------------------------------------------------------------+
| Key Attributes          |                                                                        |
+-------------------------+------------------------------------------------------------------------+
| `originalSupport`       | The original mesh of the design domain                                 | 
+-------------------------+------------------------------------------------------------------------+
| `cleanMesh`             | The computational mesh (which can be a subset of the original support) | 
+-------------------------+------------------------------------------------------------------------+


+----------------------------------------+------------------------------------------------------------------------+
| Key Methods                            |                                                                        |
+----------------------------------------+------------------------------------------------------------------------+
|`SetSupport(support)`                   | Sets the support mesh for the solver                                   | 
+----------------------------------------+------------------------------------------------------------------------+
|`PrepareComputationalSupport(levelset)` | Prepares the computational mesh based on the level set.                | 
+----------------------------------------+------------------------------------------------------------------------+
|`SolveByLevelSet(levelset)`             | Solves the physical problem for a given level set.                     | 
+----------------------------------------+------------------------------------------------------------------------+
|`GetAuxiliaryField(name, on)`           | Retrieves a field (e.g., stress) from the solution                     | 
+----------------------------------------+------------------------------------------------------------------------+
|`GetAuxiliaryScalar(name)`              | Retrieves a scalar (e.g., integral of strain energy) from the solution | 
+----------------------------------------+------------------------------------------------------------------------+

Actions
=====================

-   **File**: `src/OpenPisco/Actions/ActionBase.py`
-   **Class**: `ActionBase`

This is the base class for all actions that can be executed in the OpenPisco application. Actions are used to control the optimization workflow, such as starting or stopping the optimization, saving results, etc.

+--------------------+------------------------------------------------------------------------+
| Key Methods        |                                                                        |
+--------------------+------------------------------------------------------------------------+
|`Apply(app, ops)`   | Applies the action to the application.                                 | 
+--------------------+------------------------------------------------------------------------+


Factory Pattern
=========================

OpenPisco makes extensive use of the factory pattern to create instances of algorithms, criteria, solvers, and actions. Each of these components has a corresponding factory module:

-   `OptimAlgoFactory.py`
-   `CriteriaFactory.py`
-   `PhysicalSolverFactory.py`
-   `ActionFactory.py`

These factories provide a `Create(name, ops)` function to instantiate objects by name, and a `RegisterClass(...)` function to register new components. This makes the framework highly extensible.