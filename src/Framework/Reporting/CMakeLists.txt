add_library(cubitsim_reporting STATIC
  ../MeshQuality/MeshQualityMetric.cpp ../MeshQuality/MeshQualityMetric.hpp
  ../MeshQuality/MeshQualityDefaults.cpp ../MeshQuality/MeshQualityDefaults.hpp
  ../MeshQuality/MeshQualitySettings.cpp ../MeshQuality/MeshQualitySettings.hpp
  ../MeshQuality/MeshQualityBlock.cpp ../MeshQuality/MeshQualityBlock.hpp
  ../MeshQuality/MeshQualitySnapshot.cpp ../MeshQuality/MeshQualitySnapshot.hpp
  ../MeshQuality/MeshQualityStatistics.cpp ../MeshQuality/MeshQualityStatistics.hpp
  ../MeshQuality/MeshQualitySnapshotCodec.cpp ../MeshQuality/MeshQualitySnapshotCodec.hpp
  ReportMeshQualityContent.cpp ReportMeshQualityContent.hpp
  ../MeshQuality/MeshQualityPresentation.cpp ../MeshQuality/MeshQualityPresentation.hpp
  ReportContentGroup.cpp ReportContentGroup.hpp
  ReportProgress.cpp ReportProgress.hpp
  ReportQuantity.cpp ReportQuantity.hpp
  ReportSourceUnits.cpp ReportSourceUnits.hpp
  ReportUnitSettings.cpp ReportUnitSettings.hpp
  ReportConfiguration.cpp
  ReportConfiguration.hpp
  ReportDiagnostic.cpp
  ReportDiagnostic.hpp
  ReportDocument.cpp
  ReportDocument.hpp
  ReportImage.cpp
  ReportImage.hpp
  ReportImageProviderInterface.cpp
  ReportImageProviderInterface.hpp
  ReportImageRequest.cpp
  ReportImageRequest.hpp
  ReportJson.cpp
  ReportJson.hpp
  ReportModelProviderInterface.cpp
  ReportModelProviderInterface.hpp
  ReportOption.cpp
  ReportOption.hpp
  ReportProfile.cpp
  ReportProfile.hpp
  ReportProfileStoreInterface.cpp
  ReportProfileStoreInterface.hpp
  ReportProviderRegistryInterface.cpp
  ReportProviderRegistryInterface.hpp
  ReportRequest.cpp
  ReportRequest.hpp
  ReportResultProviderInterface.cpp
  ReportResultProviderInterface.hpp
  ReportResultRequest.cpp
  ReportResultRequest.hpp
  ReportSection.cpp
  ReportSection.hpp
  ReportSourceEntry.cpp
  ReportSourceEntry.hpp
  ReportSource.cpp
  ReportSource.hpp
  ReportTable.cpp
  ReportTable.hpp
  ReportTemplate.cpp
  ReportTemplate.hpp
  ReportTemplateProviderInterface.cpp
  ReportTemplateProviderInterface.hpp
)
target_compile_features(cubitsim_reporting PUBLIC cxx_std_17)
target_include_directories(cubitsim_reporting PUBLIC "${CUBITSIM_SOURCE_ROOT}")
target_link_libraries(cubitsim_reporting PUBLIC Qt6::Core)
set_target_properties(cubitsim_reporting PROPERTIES POSITION_INDEPENDENT_CODE ON)
