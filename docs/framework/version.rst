Application version
===================

Run the framework command::

   sim version

It reports the cubit-sim release version, Cubit version and build, graphics
runtime, ACIS geometry runtime, Exodus version and Cubit revision date.
It does not require an active CalculiX workspace or query CalculiX Core.
The framework plugin registers it even if module initialization fails.

The application release number is generated in
``src/Framework/CubitSimVersion.cmake`` using the existing year.month scheme.
Consumers read ``CubitSim::Version`` from ``Framework/Core/CubitSimVersion.hpp``.
CalculiX uses that shared release number; it does not define a separate version.
