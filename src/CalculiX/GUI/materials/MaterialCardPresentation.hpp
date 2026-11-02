#ifndef MATERIALCARDPRESENTATION_HPP
#define MATERIALCARDPRESENTATION_HPP

#include "Core/materials/MaterialProperties.hpp"
#include <QString>
#include <QStringList>
#include <vector>

struct MaterialCardDefinition
{
  std::string property;
  QString title;
  QString model;
  std::string parentSwitch;
  std::string subtypeSwitch;
  QStringList columns;
};

class MaterialCardPresentation
{
public:
  static const MaterialCardDefinition* find(const std::string& property);
  static QStringList columns(const std::string& property);
  static bool enabled(const MaterialCardDefinition& definition, const MaterialProperties& material);
};

#endif
