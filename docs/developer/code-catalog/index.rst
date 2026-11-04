Project-owned source catalogue
===============================

Inventory prepared from the working tree on 4 November 2026. It includes 1500
owned source, interface, resource-registration and build files across 63 groups.
It excludes external source trees under ``src/Utility``, binary libraries, solver
distributions and generated wrapper/MOC/UI files. SVG/icon implementations are
assets and are explained through their resource owner rather than C++ declarations.

Every listed file is a local source download in the offline manual. The catalogue
does not establish compilation, runtime support or a complete branch-by-branch
audit. For behavior and contracts, start with :doc:`../code-documentation`.
The extracted declaration index is :doc:`../code-api-index`.

Framework
---------

.. toctree::
   :maxdepth: 1

   framework-commands
   framework-core
   framework-entry-points
   framework-gui
   framework-interfaces
   framework-results
   framework-storage
   framework-utility

CalculiX model domains
----------------------

.. toctree::
   :maxdepth: 1

   calculix-core-amplitudes
   calculix-core-application
   calculix-core-bcs
   calculix-core-drawing
   calculix-core-export
   calculix-core-gui
   calculix-core-interactions
   calculix-core-jobs
   calculix-core-loads
   calculix-core-materials
   calculix-core-mesh
   calculix-core-results
   calculix-core-sections
   calculix-core-steps
   calculix-core-topology

CalculiX commands
-----------------

.. toctree::
   :maxdepth: 1

   calculix-commands-amplitudes
   calculix-commands-application
   calculix-commands-bcs
   calculix-commands-drawing
   calculix-commands-export
   calculix-commands-interactions
   calculix-commands-jobs
   calculix-commands-loads
   calculix-commands-materials
   calculix-commands-mesh
   calculix-commands-results
   calculix-commands-sections
   calculix-commands-steps
   calculix-commands-topology

CalculiX GUI
------------

.. toctree::
   :maxdepth: 1

   calculix-gui-amplitudes
   calculix-gui-application
   calculix-gui-bcs
   calculix-gui-cgal
   calculix-gui-common
   calculix-gui-drawing
   calculix-gui-export
   calculix-gui-interactions
   calculix-gui-jobs
   calculix-gui-loads
   calculix-gui-materials
   calculix-gui-mesh
   calculix-gui-options
   calculix-gui-properties
   calculix-gui-sections
   calculix-gui-steps
   calculix-gui-topology

Module, adapters and build
--------------------------

.. toctree::
   :maxdepth: 1

   build-configuration
   calculix-commands
   calculix-core
   calculix-eventobservers
   calculix-gui
   calculix-module
   calculix-pythoninterface
   calculix-useroptions
   calculix-utility

Updating the catalogue
-----------------------

After an owned source move/addition/removal, update its catalogue entry, source
download target and CMake registration reference. Keep vendor roots out of both
the catalogue and Doxygen INPUT. New declarations do not become verified contracts
through automatic extraction. Validate local links before the next authorized
documentation build.
