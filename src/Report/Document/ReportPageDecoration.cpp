#include <type_traits>
#include "ReportPageDecoration.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include <QPainter>
#include <QImage>
#include <QRegularExpression>
#include <QXmlStreamWriter>
#include <algorithm>
#include <set>

QString ReportPageDecoration::value(const ReportDocument& report, const std::string& key)
{
  const auto found = report.configuration.values.find(key);
  if (found != report.configuration.values.end())
  {
    return QString::fromStdString(found->second);
  }
  for (const auto& option : ReportUserOptions::definitions())
  {
    if (option.key == key)
    {
      return QString::fromStdString(option.default_value);
    }
  }
  return {};
}

bool ReportPageDecoration::validate_templates(const ReportConfiguration& configuration, std::string& error)
{
  const std::set<QString> tokens = {"company_name", "company_details", "title", "project", "author", "revision", "page", "pages"};
  const QRegularExpression token("\\{([^{}]*)\\}");
  for (const std::string key : {"header_left", "header_center", "header_right", "footer_left", "footer_center", "footer_right"})
  {
    const auto found = configuration.values.find(key);
    if (found == configuration.values.end())
    {
      continue;
    }
    QString rest = QString::fromStdString(found->second);
    auto matches = token.globalMatch(rest);
    while (matches.hasNext())
    {
      if (tokens.count(matches.next().captured(1)) == 0)
      {
        error = "Unknown header/footer placeholder in " + key + ".";
        return false;
      }
    }
    rest.remove(token);
    if (rest.contains('{') || rest.contains('}'))
    {
      error = "Unbalanced header/footer placeholder in " + key + ".";
      return false;
    }
  }
  return true;
}

QString ReportPageDecoration::expand(const QString& pattern, const ReportDocument& report, int page, int pages)
{
  const QRegularExpression token("\\{([^{}]*)\\}");
  auto matches = token.globalMatch(pattern);
  QString result;
  qsizetype offset = 0;
  while (matches.hasNext())
  {
    const auto match = matches.next();
    result += pattern.mid(offset, match.capturedStart() - offset);
    const auto key = match.captured(1);
    if (key == "page") {
      result += QString::number(page);
    } else {
      if (key == "pages") {
        result += QString::number(pages);
      } else {
        if (key == "title") {
          result += QString::fromStdString(report.title);
        } else {
          result += value(report, key.toStdString());
        }
      }
    }
    offset = match.capturedEnd();
  }
  return result + pattern.mid(offset);
}

QRectF ReportPageDecoration::body_rect(const ReportDocument&, const QSizeF& paper)
{
  const double band = 96;
  return QRectF(44, band, paper.width() - 88, paper.height() - 2 * band);
}

QSizeF ReportPageDecoration::logo_size(const ReportDocument& report, double columnWidth)
{
  if (!report.configuration.company_logo)
  {
    return {};
  }
  const auto image = QImage::fromData(QByteArray::fromBase64(QByteArray::fromStdString(report.configuration.company_logo->base64_data)));
  const double requested = value(report, "logo_width_mm").toDouble() * 72.0 / 25.4;
  QSizeF size(image.size());
  size.scale(QSizeF(std::min(requested, columnWidth), 36), Qt::KeepAspectRatio);
  return size;
}

void ReportPageDecoration::paint(QPainter& painter, const ReportDocument& report, const QSizeF& paper, int page, int pages)
{
  if (page == 1 && value(report, "cover") == "true" && value(report, "decoration_cover") == "false")
  {
    return;
  }
  const auto body = body_rect(report, paper);
  painter.save();
  painter.setPen(QColor("#18334c"));
  painter.setFont(QFont("Arial", 8));
  const double column = body.width() / 3;
  for (const std::string band : {"header", "footer"})
  {
    const double top = [&]() -> double {
      if (band == "header") {
        return 20;
      } else {
        return body.bottom() + 16;
      }
    }();
    int index = 0;
    for (const std::string position : {"left", "center", "right"})
    {
      const std::string key = band + "_" + position;
      QRectF slot(body.left() + index * column + 4, top, column - 8, 60);
      const int alignment = [&]() -> int {
        if (index == 0) {
          return Qt::AlignLeft;
        } else {
          if (index == 1) {
            return Qt::AlignHCenter;
          } else {
            return Qt::AlignRight;
          }
        }
      }();
      painter.save();
      painter.setClipRect(slot);
      if (report.configuration.company_logo && value(report, "logo_placement") == QString::fromStdString(key))
      {
        const auto size = logo_size(report, slot.width());
        const double x = [&]() -> double {
          if (index == 0) {
            return slot.left();
          } else {
            if (index == 1) {
              return slot.center().x() - size.width() / 2;
            } else {
              return slot.right() - size.width();
            }
          }
        }();
        const auto pixels = QImage::fromData(QByteArray::fromBase64(QByteArray::fromStdString(report.configuration.company_logo->base64_data)));
        painter.drawImage(QRectF(QPointF(x, top), size), pixels);
        slot.setTop(top + size.height() + 4);
      }
      const auto label = expand(value(report, key), report, page, pages);
      painter.drawText(slot, alignment | Qt::AlignTop | Qt::TextWordWrap, label);
      painter.restore();
      ++index;
    }
    if (value(report, band + "_line") == "true")
    {
      const double y = [&]() -> double {
        if (band == "header") {
          return body.top() - 10;
        } else {
          return body.bottom() + 8;
        }
      }();
      painter.drawLine(QPointF(body.left(), y), QPointF(body.right(), y));
    }
  }
  painter.restore();
}

void ReportPageDecoration::write_odt(QXmlStreamWriter& xml, const ReportDocument& report, bool header)
{
  const std::string band = [&]() -> std::string {
    if (header) {
      return "header";
    } else {
      return "footer";
    }
  }();
  xml.writeStartElement([&]() -> const char* {
    if (header) {
      return "style:header";
    } else {
      return "style:footer";
    }
  }());
  xml.writeStartElement("table:table");
  xml.writeAttribute("table:name", QString::fromStdString(band));
  xml.writeAttribute("table:style-name", "DecorationTable");
  xml.writeStartElement("table:table-column");
  xml.writeAttribute("table:number-columns-repeated", "3");
  xml.writeAttribute("table:style-name", "DecorationColumn");
  xml.writeEndElement();
  xml.writeStartElement("table:table-row");
  for (const std::string position : {"left", "center", "right"})
  {
    const auto key = band + "_" + position;
    xml.writeStartElement("table:table-cell");
    xml.writeAttribute("table:style-name", "DecorationCell");
    xml.writeStartElement("text:p");
    xml.writeAttribute("text:style-name", QString::fromStdString("Decoration_" + position));
    if (report.configuration.company_logo && value(report, "logo_placement") == QString::fromStdString(key))
    {
      const auto size = logo_size(report, ([&]() -> std::common_type_t<decltype(612.0 - 88), decltype(595.28 - 88)> {
        if (value(report, "page_size") == "Letter") {
          return 612.0 - 88;
        } else {
          return 595.28 - 88;
        }
      }()) / 3 - 8);
      xml.writeStartElement("draw:frame");
      xml.writeAttribute("draw:name", "CompanyLogo");
      xml.writeAttribute("text:anchor-type", "as-char");
      xml.writeAttribute("svg:width", QString::number(size.width()) + "pt");
      xml.writeAttribute("svg:height", QString::number(size.height()) + "pt");
      xml.writeStartElement("draw:image");
      xml.writeAttribute("xlink:href", "Pictures/company-logo.png");
      xml.writeAttribute("xlink:type", "simple");
      xml.writeAttribute("xlink:show", "embed");
      xml.writeAttribute("xlink:actuate", "onLoad");
      xml.writeEndElement();
      xml.writeEndElement();
      xml.writeEmptyElement("text:line-break");
    }
    // Expand metadata once. Page placeholders become native ODF fields so they
    // follow the office editor's pagination rather than the PDF page count.
    const auto pattern = value(report, key);
    auto matches = QRegularExpression("\\{([^{}]*)\\}").globalMatch(pattern);
    qsizetype offset = 0;
    while (matches.hasNext())
    {
      const auto match = matches.next();
      xml.writeCharacters(pattern.mid(offset, match.capturedStart() - offset));
      const auto token = match.captured(1);
      if (token == "page" || token == "pages")
      {
        xml.writeStartElement([&]() -> const char* {
          if (token == "page") {
            return "text:page-number";
          } else {
            return "text:page-count";
          }
        }());
        if (token == "page")
        {
          xml.writeAttribute("text:select-page", "current");
        }
        xml.writeCharacters("1");
        xml.writeEndElement();
      }
      else
      {
        xml.writeCharacters(expand(match.captured(), report, 1, 1));
      }
      offset = match.capturedEnd();
    }
    xml.writeCharacters(pattern.mid(offset));
    xml.writeEndElement();
    xml.writeEndElement();
  }
  xml.writeEndElement();
  xml.writeEndElement();
  xml.writeEndElement();
}
