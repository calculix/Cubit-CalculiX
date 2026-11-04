Framework API contracts
=======================

One Doxygen XML project supplies both Framework and workspace API pages.
Only project-owned sources are extracted; bundled third-party libraries are
excluded. The Framework owns module startup, workspace hosting and shared
storage. Modules borrow services through the provider interfaces.

Module and workspace contracts
------------------------------

.. doxygenclass:: ModuleInterface
   :members:

.. doxygenclass:: WorkspaceProviderInterface
   :members:

User options
------------

Defaults and validators belong to the workspace. The storage service borrows
option declarations during each operation; it does not retain validators.
See :doc:`user-options` for atomic-save and failure contracts.

.. doxygenstruct:: UserOption
   :members:

.. doxygenclass:: UserOptionsInterface
   :members:

.. doxygenclass:: UserOptionsStore
   :members:
