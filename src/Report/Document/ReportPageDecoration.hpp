#pragma once
#include "Framework/Reporting/ReportDocument.hpp"
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <string>
class QPainter;
class QXmlStreamWriter;

// Page geometry uses typographic points (72 points per inch). Logo dimensions
// are maxima: aspect ratio is preserved within its column and a 36-point height.
class ReportPageDecoration
{
public:
  static QString value(const ReportDocument& report, const std::string& key);
  static bool validate_templates(const ReportConfiguration& configuration, std::string& error);
  static QString expand(const QString& pattern, const ReportDocument& report, int page, int pages);
  static QRectF body_rect(const ReportDocument& report, const QSizeF& paper);
  static QSizeF logo_size(const ReportDocument& report, double columnWidth);
  static void paint(QPainter& painter, const ReportDocument& report, const QSizeF& paper, int page, int pages);
  static void write_odt(QXmlStreamWriter& xml, const ReportDocument& report, bool header);
};
