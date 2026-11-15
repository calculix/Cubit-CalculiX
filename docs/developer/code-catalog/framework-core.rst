Framework Core source files
===========================

Source navigation updated for the unit/material integration. These links describe
file ownership; they do not establish compilation or runtime validation.

.. list-table:: Owned files
   :header-rows: 1
   :widths: 68 32

   * - Source/download
     - Declaration hints
   * - :download:`src/Framework/Core/CMakeLists.txt <../../../src/Framework/Core/CMakeLists.txt>`
     - ``Build registration``
   * - :download:`src/Framework/Core/CubitSimVersion.hpp <../../../src/Framework/Core/CubitSimVersion.hpp>`
     - ``Interface / value declarations``
   * - :download:`src/Framework/Core/FrameworkContext.cpp <../../../src/Framework/Core/FrameworkContext.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/FrameworkContext.hpp <../../../src/Framework/Core/FrameworkContext.hpp>`
     - ``FrameworkContext``
   * - :download:`src/Framework/Core/FrameworkModules.cpp <../../../src/Framework/Core/FrameworkModules.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/FrameworkModules.hpp <../../../src/Framework/Core/FrameworkModules.hpp>`
     - ``Interface / value declarations``
   * - :download:`src/Framework/Core/materials/CMakeLists.txt <../../../src/Framework/Core/materials/CMakeLists.txt>`
     - ``Build registration``
   * - :download:`src/Framework/Core/materials/CoreFrameworkMaterialLibrary.cpp <../../../src/Framework/Core/materials/CoreFrameworkMaterialLibrary.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/CoreFrameworkMaterialLibrary.hpp <../../../src/Framework/Core/materials/CoreFrameworkMaterialLibrary.hpp>`
     - ``CoreFrameworkMaterialLibrary``
   * - :download:`src/Framework/Core/materials/CoreFrameworkMaterials.cpp <../../../src/Framework/Core/materials/CoreFrameworkMaterials.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/CoreFrameworkMaterials.hpp <../../../src/Framework/Core/materials/CoreFrameworkMaterials.hpp>`
     - ``CoreFrameworkMaterials``
   * - :download:`src/Framework/Core/materials/FrameworkMaterialsInterface.cpp <../../../src/Framework/Core/materials/FrameworkMaterialsInterface.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/FrameworkMaterialsInterface.hpp <../../../src/Framework/Core/materials/FrameworkMaterialsInterface.hpp>`
     - ``FrameworkMaterialsInterface``
   * - :download:`src/Framework/Core/materials/FrameworkMaterialsInterfaceLibrary.cpp <../../../src/Framework/Core/materials/FrameworkMaterialsInterfaceLibrary.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialCard.cpp <../../../src/Framework/Core/materials/MaterialCard.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialCard.hpp <../../../src/Framework/Core/materials/MaterialCard.hpp>`
     - ``MaterialCard``
   * - :download:`src/Framework/Core/materials/MaterialCardDefinition.cpp <../../../src/Framework/Core/materials/MaterialCardDefinition.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialCardDefinition.hpp <../../../src/Framework/Core/materials/MaterialCardDefinition.hpp>`
     - ``FrameworkMaterialCardDefinition``
   * - :download:`src/Framework/Core/materials/MaterialCardValues.cpp <../../../src/Framework/Core/materials/MaterialCardValues.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialCardValues.hpp <../../../src/Framework/Core/materials/MaterialCardValues.hpp>`
     - ``MaterialCardValues``
   * - :download:`src/Framework/Core/materials/MaterialColumnDefinition.cpp <../../../src/Framework/Core/materials/MaterialColumnDefinition.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialColumnDefinition.hpp <../../../src/Framework/Core/materials/MaterialColumnDefinition.hpp>`
     - ``MaterialColumnDefinition``
   * - :download:`src/Framework/Core/materials/MaterialData.cpp <../../../src/Framework/Core/materials/MaterialData.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialData.hpp <../../../src/Framework/Core/materials/MaterialData.hpp>`
     - ``MaterialData``
   * - :download:`src/Framework/Core/materials/MaterialDefinitions.cpp <../../../src/Framework/Core/materials/MaterialDefinitions.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialDefinitions.hpp <../../../src/Framework/Core/materials/MaterialDefinitions.hpp>`
     - ``MaterialDefinitions``
   * - :download:`src/Framework/Core/materials/MaterialInstance.cpp <../../../src/Framework/Core/materials/MaterialInstance.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialInstance.hpp <../../../src/Framework/Core/materials/MaterialInstance.hpp>`
     - ``MaterialInstance``
   * - :download:`src/Framework/Core/materials/MaterialLibraryEntry.cpp <../../../src/Framework/Core/materials/MaterialLibraryEntry.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialLibraryEntry.hpp <../../../src/Framework/Core/materials/MaterialLibraryEntry.hpp>`
     - ``MaterialLibraryEntry``
   * - :download:`src/Framework/Core/materials/MaterialModelDefinition.cpp <../../../src/Framework/Core/materials/MaterialModelDefinition.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialModelDefinition.hpp <../../../src/Framework/Core/materials/MaterialModelDefinition.hpp>`
     - ``MaterialModelDefinition``
   * - :download:`src/Framework/Core/materials/MaterialProjectPersistence.cpp <../../../src/Framework/Core/materials/MaterialProjectPersistence.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialProjectPersistence.hpp <../../../src/Framework/Core/materials/MaterialProjectPersistence.hpp>`
     - ``MaterialProjectPersistence``
   * - :download:`src/Framework/Core/materials/MaterialRef.cpp <../../../src/Framework/Core/materials/MaterialRef.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialRef.hpp <../../../src/Framework/Core/materials/MaterialRef.hpp>`
     - ``MaterialRef``
   * - :download:`src/Framework/Core/materials/MaterialsExport.hpp <../../../src/Framework/Core/materials/MaterialsExport.hpp>`
     - ``Interface / value declarations``
   * - :download:`src/Framework/Core/materials/MaterialTableRow.cpp <../../../src/Framework/Core/materials/MaterialTableRow.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/MaterialTableRow.hpp <../../../src/Framework/Core/materials/MaterialTableRow.hpp>`
     - ``MaterialTableRow``
   * - :download:`src/Framework/Core/materials/WorkspaceMaterialStore.cpp <../../../src/Framework/Core/materials/WorkspaceMaterialStore.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/materials/WorkspaceMaterialStore.hpp <../../../src/Framework/Core/materials/WorkspaceMaterialStore.hpp>`
     - ``WorkspaceMaterialStore``
   * - :download:`src/Framework/Core/ModuleManager.cpp <../../../src/Framework/Core/ModuleManager.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/ModuleManager.hpp <../../../src/Framework/Core/ModuleManager.hpp>`
     - ``ModuleManager``
   * - :download:`src/Framework/Core/Monitoring/FrameworkUserOptions.cpp <../../../src/Framework/Core/Monitoring/FrameworkUserOptions.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/Monitoring/FrameworkUserOptions.hpp <../../../src/Framework/Core/Monitoring/FrameworkUserOptions.hpp>`
     - ``FrameworkOptions, FrameworkUserOptions``
   * - :download:`src/Framework/Core/Monitoring/ResourceAttribution.cpp <../../../src/Framework/Core/Monitoring/ResourceAttribution.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/Monitoring/ResourceAttribution.hpp <../../../src/Framework/Core/Monitoring/ResourceAttribution.hpp>`
     - ``ResourceAttribution``
   * - :download:`src/Framework/Core/Monitoring/ResourceMonitor.cpp <../../../src/Framework/Core/Monitoring/ResourceMonitor.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/Monitoring/ResourceMonitor.hpp <../../../src/Framework/Core/Monitoring/ResourceMonitor.hpp>`
     - ``ProcessResourceSample, JobResourceSample, ResourceSnapshot, ResourceMonitor``
   * - :download:`src/Framework/Core/ProjectStateCoordinator.cpp <../../../src/Framework/Core/ProjectStateCoordinator.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/ProjectStateCoordinator.hpp <../../../src/Framework/Core/ProjectStateCoordinator.hpp>`
     - ``ProjectStateCoordinator``
   * - :download:`src/Framework/Core/tests/CMakeLists.txt <../../../src/Framework/Core/tests/CMakeLists.txt>`
     - ``Build registration``
   * - :download:`src/Framework/Core/tests/ModelServicesTests.cpp <../../../src/Framework/Core/tests/ModelServicesTests.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/units/CMakeLists.txt <../../../src/Framework/Core/units/CMakeLists.txt>`
     - ``Build registration``
   * - :download:`src/Framework/Core/units/CoreFrameworkUnits.cpp <../../../src/Framework/Core/units/CoreFrameworkUnits.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/units/CoreFrameworkUnits.hpp <../../../src/Framework/Core/units/CoreFrameworkUnits.hpp>`
     - ``CoreFrameworkUnits``
   * - :download:`src/Framework/Core/units/CoreFrameworkUnitsPersistence.cpp <../../../src/Framework/Core/units/CoreFrameworkUnitsPersistence.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/units/CoreFrameworkUnitsPersistence.hpp <../../../src/Framework/Core/units/CoreFrameworkUnitsPersistence.hpp>`
     - ``CoreFrameworkUnitsPersistence``
   * - :download:`src/Framework/Core/units/FrameworkUnitsInterface.cpp <../../../src/Framework/Core/units/FrameworkUnitsInterface.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/units/FrameworkUnitsInterface.hpp <../../../src/Framework/Core/units/FrameworkUnitsInterface.hpp>`
     - ``FrameworkUnitsInterface``
   * - :download:`src/Framework/Core/units/QuantityUnit.cpp <../../../src/Framework/Core/units/QuantityUnit.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/units/QuantityUnit.hpp <../../../src/Framework/Core/units/QuantityUnit.hpp>`
     - ``QuantityUnit``
   * - :download:`src/Framework/Core/units/SourceUnitContext.cpp <../../../src/Framework/Core/units/SourceUnitContext.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/units/SourceUnitContext.hpp <../../../src/Framework/Core/units/SourceUnitContext.hpp>`
     - ``SourceUnitContext``
   * - :download:`src/Framework/Core/units/UnitDefinition.cpp <../../../src/Framework/Core/units/UnitDefinition.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/units/UnitDefinition.hpp <../../../src/Framework/Core/units/UnitDefinition.hpp>`
     - ``UnitDefinition``
   * - :download:`src/Framework/Core/units/UnitSystem.cpp <../../../src/Framework/Core/units/UnitSystem.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/units/UnitSystem.hpp <../../../src/Framework/Core/units/UnitSystem.hpp>`
     - ``UnitSystem``
   * - :download:`src/Framework/Core/units/UnitTypes.hpp <../../../src/Framework/Core/units/UnitTypes.hpp>`
     - ``Interface / value declarations``
   * - :download:`src/Framework/Core/units/WorkspaceUnitContext.cpp <../../../src/Framework/Core/units/WorkspaceUnitContext.cpp>`
     - ``Implementation / integration``
   * - :download:`src/Framework/Core/units/WorkspaceUnitContext.hpp <../../../src/Framework/Core/units/WorkspaceUnitContext.hpp>`
     - ``WorkspaceUnitContext``
