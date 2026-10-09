********************************************************************************
User interfaces 
********************************************************************************


OpenPisco provides three complementary user interfaces tailored to different workflows and levels of expertise. 
Despite these differences, all interfaces rely on the same computational core and are built upon one another in a hierarchical architecture:



.. code-block:: text

   OpenPisco GUI
         │
         ▼
    OpenPiscoCL
         │
         ▼
      Python API

At its core, the **Python API** provides direct access to all OpenPisco
functionalities and is primarily intended for developers, researchers, and
advanced users. Built on top of this API, **OpenPiscoCL** offers a command-line
interface based on a Domain-Specific Language (DSL), enabling the definition
and execution of studies without writing Python code. Finally, the
**OpenPisco GUI** provides an interactive graphical environment for creating,
running, and visualizing optimization studies.

Together, these interfaces allow users to choose the level of interaction best
suited to their expertise and workflow while relying on the same computational
core.

.. grid:: 3
   :gutter: 3

   .. grid-item-card::
      :link: openpiscoQuickRef
      :link-type: doc

      🐍 Python API

      Direct programmatic access to all OpenPisco functionalities.

      **Best suited for:**
      developers, advanced users.

   .. grid-item-card::
      :link: OpenPiscoCL
      :link-type: doc

      ⚙️ OpenPiscoCL

      Command-line interface driven by the OpenPisco DSL.

      **Best suited for:**
      automation, batch processing, HPC execution.

   .. grid-item-card::
      :link: OpenPiscoGUI
      :link-type: doc

      🖥️ OpenPisco GUI

      Graphical environment for modifying and running studies.

      **Best suited for:**
      visualization.

Overview
--------

Python API
^^^^^^^^^^

The Python API is the foundation of OpenPisco and exposes all platform
capabilities through Python. It provides maximum flexibility for developing
new algorithms, integrating custom tools, and building application-specific
workflows.

OpenPiscoCL
^^^^^^^^^^^

OpenPiscoCL is built on top of the Python API and executes studies described
using the OpenPisco Domain-Specific Language (DSL).

The DSL follows a few key principles:

* **Declarative**: describe what should be executed rather than how.
* **Readable**: study definitions remain easy to understand and review.
* **Modular**: workflows are assembled from independent building blocks.
* **Reproducible**: simulation and optimization settings are explicitly stored
  in portable text files.
* **Accessible**: common studies can be created without writing Python code.

OpenPisco GUI
^^^^^^^^^^^^^

The OpenPisco GUI provides the most
user-friendly experience. It allows users to configure studies, launch
simulations, and visualize results from a single application without relying
on external visualization tools.
