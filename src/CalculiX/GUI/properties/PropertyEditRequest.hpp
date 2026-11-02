#ifndef PROPERTYEDITREQUEST_HPP
#define PROPERTYEDITREQUEST_HPP

#include <QString>
#include <QtCore/Qt>

enum class PropertyEditorKind { None, Name, Choice };
enum PropertyEditRole
{
  PropertyEditorRole = Qt::UserRole + 1300,
  PropertyChoicesRole,
  PropertyFieldRole,
  PropertyEntityRole,
  PropertyRevisionRole,
  PropertyOriginalRole
};

struct PropertyEditRequest
{
  int entityId = -1;
  quint64 revision = 0;
  QString field;
  QString original;
  QString value;
};

#endif
