Calculix Core Units source files
================================

Source navigation updated for the unit/material integration. These links describe
file ownership; they do not establish compilation or runtime validation.

.. list-table:: Owned files
   :header-rows: 1
   :widths: 68 32

   * - Source/download
     - Declaration hints
   * - :download:`src/CalculiX/Core/units/CalculiXPhysicalModelUnits.cpp <../../../src/CalculiX/Core/units/CalculiXPhysicalModelUnits.cpp>`
     - ``Implementation / integration``
   * - :download:`src/CalculiX/Core/units/CalculiXPhysicalModelUnits.hpp <../../../src/CalculiX/Core/units/CalculiXPhysicalModelUnits.hpp>`
     - ``CalculiXPhysicalModelUnits``
   * - :download:`src/CalculiX/Core/units/CalculiXUnitsInterface.cpp <../../../src/CalculiX/Core/units/CalculiXUnitsInterface.cpp>`
     - ``Implementation / integration``
   * - :download:`src/CalculiX/Core/units/CalculiXUnitsInterface.hpp <../../../src/CalculiX/Core/units/CalculiXUnitsInterface.hpp>`
     - ``CalculiXUnitsInterface``
   * - :download:`src/CalculiX/Core/units/CMakeLists.txt <../../../src/CalculiX/Core/units/CMakeLists.txt>`
     - ``Build registration``

   * - :download:`CalculiXDomainUnits.cpp <../../../src/CalculiX/Core/units/CalculiXDomainUnits.cpp>`
     - Dimensional fields for the eleven model domains
   * - :download:`CalculiXDomainUnits.hpp <../../../src/CalculiX/Core/units/CalculiXDomainUnits.hpp>`
     - ``CalculiXDomainUnits`` and conversion views

``CalculiXReportDomainSettings`` (``Core/reporting``) obtains detached settings
using Report preferences. Persistence writes canonical SI markers; temporary
converted views must never be persisted. Preset changes alter context only.
