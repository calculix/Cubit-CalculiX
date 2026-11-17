#pragma once
#include <string>
#include <QSizeF>
#include <QSize>
class QIODevice;
class QXmlStreamWriter;
class ReportDocument;

class ReportOdtStyles
{
public:
  ReportOdtStyles();
  static QSizeF figure_size(const ReportDocument& report, const QSize& pixels);
  static void write_content(QXmlStreamWriter& xml);
  static bool write_document(QIODevice& output, const ReportDocument& report, std::string& error);
};
