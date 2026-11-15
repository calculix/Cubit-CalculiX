#pragma once
#include "Framework/Core/materials/MaterialData.hpp"
#include "Framework/Core/materials/MaterialModelDefinition.hpp"
#include <QStringList>
#include <string>
class MaterialCardPresentation {
public:
  static const MaterialModelDefinition* find(const std::string& property);
  static QStringList columns(const std::string& property, const std::string& workspace = "calculix");
  static bool enabled(const MaterialModelDefinition& definition, const MaterialData& material);
};
