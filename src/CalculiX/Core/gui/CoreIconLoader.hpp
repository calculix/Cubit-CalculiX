#ifndef COREICONLOADER_HPP
#define COREICONLOADER_HPP

#include <QIcon>
#include <QString>
#include <string>

class CoreIconLoader
{
public:
  static QIcon load(const QString& directory, const std::string& name);
};

#endif
