#pragma once
#include <QString>
#include <string>

namespace ReportCommandText
{
  bool quote(const QString& value, std::string& output, std::string& error);
}
