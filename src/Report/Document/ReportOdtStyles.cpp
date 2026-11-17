#include "ReportOdtStyles.hpp"
#include "ReportPageDecoration.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include <algorithm>
#include <QIODevice>
#include <QXmlStreamWriter>
#include <QString>
#include <type_traits>

namespace
{
void namespaces(QXmlStreamWriter& xml)
{
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:office:1.0", "office");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:text:1.0", "text");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:style:1.0", "style");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:table:1.0", "table");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0", "fo");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", "draw");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0", "svg");
  xml.writeNamespace("http://www.w3.org/1999/xlink", "xlink");
  xml.writeAttribute("office:version", "1.2");
}
}

ReportOdtStyles::ReportOdtStyles() = default;

void ReportOdtStyles::write_content(QXmlStreamWriter& xml)
{
  xml.writeStartElement("office:automatic-styles");
  for (int level = 1; level <= 10; ++level)
  {
    xml.writeStartElement("style:style");
    xml.writeAttribute("style:name", "Heading" + QString::number(level));
    xml.writeAttribute("style:family", "paragraph");
    xml.writeAttribute("style:master-page-name", "Standard");
    xml.writeStartElement("style:paragraph-properties");
    xml.writeAttribute("fo:keep-with-next", "always");
    xml.writeAttribute("fo:margin-top", "0.4cm");
    if (level == 1)
    {
      xml.writeAttribute("fo:break-before", "page");
    }
    xml.writeEndElement();
    xml.writeStartElement("style:text-properties");
    QString size = "13pt";
    if (level == 1)
    {
      size = "20pt";
    }
    xml.writeAttribute("fo:font-size", size);
    xml.writeAttribute("fo:font-weight", "bold");
    xml.writeAttribute("fo:color", "#18334c");
    xml.writeEndElement();
    xml.writeEndElement();
  }
  for (const QString name : {QString("TableTitle"), QString("Figure"), QString("FigureCaption"), QString("Number"), QString("ContentsTitle")})
  {
    xml.writeStartElement("style:style");
    xml.writeAttribute("style:name", name);
    xml.writeAttribute("style:family", "paragraph");
    if (name == "ContentsTitle")
    {
      xml.writeAttribute("style:master-page-name", "Standard");
    }
    xml.writeStartElement("style:paragraph-properties");
    if (name == "ContentsTitle")
    {
      xml.writeAttribute("fo:break-before", "page");
    }
    if (name == "TableTitle" || name == "Figure")
    {
      xml.writeAttribute("fo:keep-with-next", "always");
    }
    if (name == "FigureCaption")
    {
      xml.writeAttribute("fo:margin-bottom", "0.2cm");
    }
    if (name == "Number")
    {
      xml.writeAttribute("fo:text-align", "right");
    }
    xml.writeEndElement();
    xml.writeStartElement("style:text-properties");
    if (name == "TableTitle" || name == "ContentsTitle")
    {
      xml.writeAttribute("fo:font-weight", "bold");
    }
    if (name == "ContentsTitle")
    {
      xml.writeAttribute("fo:font-size", "20pt");
    }
    if (name == "FigureCaption")
    {
      xml.writeAttribute("fo:font-size", "8pt");
    }
    xml.writeEndElement();
    xml.writeEndElement();
  }
  xml.writeStartElement("style:style");
  xml.writeAttribute("style:name", "ReportTable");
  xml.writeAttribute("style:family", "table");
  xml.writeStartElement("style:table-properties");
  xml.writeAttribute("table:align", "margins");
  xml.writeEndElement();
  xml.writeEndElement();
  xml.writeStartElement("style:style");
  xml.writeAttribute("style:name", "ReportColumn");
  xml.writeAttribute("style:family", "table-column");
  xml.writeStartElement("style:table-column-properties");
  xml.writeAttribute("style:rel-column-width", "1*");
  xml.writeEndElement();
  xml.writeEndElement();
  for (const QString name : {QString("ReportCell"), QString("ReportHeaderCell")})
  {
    xml.writeStartElement("style:style");
    xml.writeAttribute("style:name", name);
    xml.writeAttribute("style:family", "table-cell");
    xml.writeStartElement("style:table-cell-properties");
    xml.writeAttribute("fo:padding", "4pt");
    xml.writeAttribute("fo:border", "0.3pt solid #c9d6df");
    if (name == "ReportHeaderCell")
    {
      xml.writeAttribute("fo:background-color", "#edf3f7");
    }
    xml.writeEndElement();
    if (name == "ReportHeaderCell")
    {
      xml.writeStartElement("style:text-properties");
      xml.writeAttribute("fo:font-weight", "bold");
      xml.writeEndElement();
    }
    xml.writeEndElement();
  }
  for (int level = 1; level <= 10; ++level)
  {
    xml.writeStartElement("style:style");
    xml.writeAttribute("style:name", "Contents" + QString::number(level));
    xml.writeAttribute("style:family", "paragraph");
    xml.writeStartElement("style:paragraph-properties");
    xml.writeAttribute("fo:margin-left", QString::number((level - 1) * 10) + "pt");
    xml.writeEndElement();
    xml.writeEndElement();
  }
  xml.writeEndElement();
}

bool ReportOdtStyles::write_document(QIODevice& output, const ReportDocument& report, std::string& error)
{
  QXmlStreamWriter sx(&output);
  sx.writeStartDocument();
  sx.writeStartElement("office:document-styles");
  namespaces(sx);
  sx.writeStartElement("office:styles");
  sx.writeStartElement("style:default-style");
  sx.writeAttribute("style:family", "paragraph");
  sx.writeStartElement("style:text-properties");
  sx.writeAttribute("fo:font-size", "9pt");
  sx.writeAttribute("fo:font-family", "Arial");
  sx.writeEndElement();
  sx.writeEndElement();
  sx.writeStartElement("style:style");
  sx.writeAttribute("style:name", "CoverText");
  sx.writeAttribute("style:family", "paragraph");
  sx.writeAttribute("style:master-page-name", [&]() -> const char*
  {
    if (ReportPageDecoration::value(report, "decoration_cover") == "false")
    {
      return "Cover";
    }
    else
    {
      return "Standard";
    }
  }());
  sx.writeEndElement();
  for (const QString position : {QString("left"), QString("center"), QString("right")})
  {
    sx.writeStartElement("style:style");
    sx.writeAttribute("style:name", "Decoration_" + position);
    sx.writeAttribute("style:family", "paragraph");
    sx.writeStartElement("style:paragraph-properties");
    sx.writeAttribute("fo:text-align", position);
    sx.writeAttribute("fo:margin", "0pt");
    sx.writeEndElement();
    sx.writeStartElement("style:text-properties");
    sx.writeAttribute("fo:font-size", "8pt");
    sx.writeAttribute("fo:color", "#18334c");
    sx.writeEndElement();
    sx.writeEndElement();
  }
  sx.writeStartElement("style:style");
  sx.writeAttribute("style:name", "DecorationTable");
  sx.writeAttribute("style:family", "table");
  sx.writeStartElement("style:table-properties");
  sx.writeAttribute("table:align", "margins");
  sx.writeEndElement();
  sx.writeEndElement();
  sx.writeStartElement("style:style");
  sx.writeAttribute("style:name", "DecorationColumn");
  sx.writeAttribute("style:family", "table-column");
  sx.writeStartElement("style:table-column-properties");
  sx.writeAttribute("style:rel-column-width", "1*");
  sx.writeEndElement();
  sx.writeEndElement();
  sx.writeStartElement("style:style");
  sx.writeAttribute("style:name", "DecorationCell");
  sx.writeAttribute("style:family", "table-cell");
  sx.writeStartElement("style:table-cell-properties");
  sx.writeAttribute("fo:padding", "4pt");
  sx.writeAttribute("fo:border", "none");
  sx.writeAttribute("style:vertical-align", "top");
  sx.writeEndElement();
  sx.writeEndElement();
  sx.writeEndElement();
  sx.writeStartElement("office:automatic-styles");
  sx.writeStartElement("style:page-layout");
  sx.writeAttribute("style:name", "ReportPage");
  sx.writeStartElement("style:page-layout-properties");
  sx.writeAttribute("fo:page-width", QString::number(ReportPageDecoration::paper_size(report).width(), 'g', 12) + "pt");
  sx.writeAttribute("fo:page-height", QString::number(ReportPageDecoration::paper_size(report).height(), 'g', 12) + "pt");
  sx.writeAttribute("fo:margin-left", "44pt");
  sx.writeAttribute("fo:margin-right", "44pt");
  sx.writeAttribute("fo:margin-top", "20pt");
  sx.writeAttribute("fo:margin-bottom", "20pt");
  sx.writeEndElement();
  for (const std::string band : {"header", "footer"})
  {
    sx.writeStartElement(QString::fromStdString("style:" + band + "-style"));
    sx.writeStartElement("style:header-footer-properties");
    sx.writeAttribute("fo:min-height", "60pt");
    sx.writeAttribute([&]() -> const char*
    {
      if (band == "header")
      {
        return "fo:margin-bottom";
      }
      else
      {
        return "fo:margin-top";
      }
    }(), "16pt");
    if (ReportPageDecoration::value(report, band + "_line") == "true")
    {
      sx.writeAttribute([&]() -> const char*
      {
        if (band == "header")
        {
          return "fo:border-bottom";
        }
        else
        {
          return "fo:border-top";
        }
      }(), "0.5pt solid #18334c");
    }
    sx.writeEndElement();
    sx.writeEndElement();
  }
  sx.writeEndElement();
  sx.writeStartElement("style:page-layout");
  sx.writeAttribute("style:name", "CoverPage");
  sx.writeStartElement("style:page-layout-properties");
  sx.writeAttribute("fo:page-width", QString::number(ReportPageDecoration::paper_size(report).width(), 'g', 12) + "pt");
  sx.writeAttribute("fo:page-height", QString::number(ReportPageDecoration::paper_size(report).height(), 'g', 12) + "pt");
  sx.writeAttribute("fo:margin-left", "44pt");
  sx.writeAttribute("fo:margin-right", "44pt");
  sx.writeAttribute("fo:margin-top", "96pt");
  sx.writeAttribute("fo:margin-bottom", "96pt");
  sx.writeEndElement();
  sx.writeEndElement();
  sx.writeEndElement();
  sx.writeStartElement("office:master-styles");
  sx.writeStartElement("style:master-page");
  sx.writeAttribute("style:name", "Standard");
  sx.writeAttribute("style:page-layout-name", "ReportPage");
  ReportPageDecoration::write_odt(sx, report, true);
  ReportPageDecoration::write_odt(sx, report, false);
  sx.writeEndElement();
  sx.writeStartElement("style:master-page");
  sx.writeAttribute("style:name", "Cover");
  sx.writeAttribute("style:page-layout-name", "CoverPage");
  sx.writeAttribute("style:next-style-name", "Standard");
  sx.writeEndElement();
  sx.writeEndElement();
  sx.writeEndElement();
  sx.writeEndDocument();
  if (sx.hasError())
  {
    error = "Cannot write ODT document styles.";
    return false;
  }
  error.clear();
  return true;
}

QSizeF ReportOdtStyles::figure_size(const ReportDocument& report, const QSize& pixels)
{
  if (pixels.width() <= 0 || pixels.height() <= 0)
  {
    return {};
  }
  const auto paper = ReportPageDecoration::paper_size(report);
  const auto body = ReportPageDecoration::body_rect(report, paper).size();
  // Uniform scaling preserves pixel aspect ratio; use the same height limit
  // as the PDF preview and reserve the remaining body for heading/caption.
  const double scale = (std::min)(body.width() / pixels.width(),
    (std::min)(body.height(), ReportUserOptions::figure_max_height()) / pixels.height());
  return QSizeF(pixels.width() * scale, pixels.height() * scale);
}
