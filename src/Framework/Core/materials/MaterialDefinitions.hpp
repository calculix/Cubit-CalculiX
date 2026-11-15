#pragma once
#include "MaterialsExport.hpp"
#include "MaterialCardDefinition.hpp"
#include "MaterialData.hpp"
#include <string>
#include <vector>
class CUBITSIM_MATERIALS_EXPORT MaterialDefinitions {
public:
  MaterialDefinitions();
  ~MaterialDefinitions();
  const std::vector<FrameworkMaterialCardDefinition>& cards() const noexcept;
  const FrameworkMaterialCardDefinition* card(const std::string& key) const noexcept;
  const MaterialModelDefinition* model(const FrameworkMaterialCardDefinition& card, const std::string& key) const noexcept;
  const MaterialModelDefinition* property(const std::string& property, const FrameworkMaterialCardDefinition*& card) const noexcept;
  std::vector<std::vector<std::string>> legacy_properties() const;
  bool validate(const MaterialData& data, bool requireActiveValues, std::string& error) const;
private:
  std::vector<FrameworkMaterialCardDefinition> cardDefinitions;
};
