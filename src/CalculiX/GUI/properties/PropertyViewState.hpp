#ifndef PROPERTYVIEWSTATE_HPP
#define PROPERTYVIEWSTATE_HPP

#include <QByteArray>
#include <QMap>
#include <QString>

// Runtime GUI preferences, retained per feature across selection changes and model reset.
struct PropertyViewState
{
  QMap<QString, bool> expanded;
  QByteArray header;
  int verticalScroll = 0;
  int horizontalScroll = 0;
};

enum class PropertySection
{
  General = 0,
  Assignment = 10,
  Classification = 20,
  Data = 30,
  Settings = 40,
  Context = 90
};

#endif
