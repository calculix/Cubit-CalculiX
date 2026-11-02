#ifndef PROPERTYCOMMANDTEXT_HPP
#define PROPERTYCOMMANDTEXT_HPP

#include <QString>

class PropertyCommandText
{
public:
  static bool quoteName(const QString& name, QString& quoted, QString& error);
};

#endif
