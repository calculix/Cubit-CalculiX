#include "ReportOdtExporter.hpp"
#include "ReportDocumentLayout.hpp"
#include "ReportPageDecoration.hpp"
#include <QXmlStreamWriter>
#include <QDataStream>
#include <QSaveFile>
#include <QDateTime>
#include <functional>
#include <set>
#include <QImage>
#include <algorithm>

namespace
{
const QString office = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
const QString text = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
const QString style = "urn:oasis:names:tc:opendocument:xmlns:style:1.0";

void namespaces(QXmlStreamWriter& xml)
{
  xml.writeNamespace(office, "office");
  xml.writeNamespace(text, "text");
  xml.writeNamespace(style, "style");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:table:1.0", "table");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0", "fo");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:drawing:1.0", "draw");
  xml.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0", "svg");
  xml.writeNamespace("http://www.w3.org/1999/xlink", "xlink");
  xml.writeAttribute("office:version", "1.2");
}

void paragraph(QXmlStreamWriter& xml, const QString& value)
{
  xml.writeTextElement("text:p", value);
}

quint32 crc32(const QByteArray& bytes)
{
  quint32 crc = 0xffffffffu;
  for (unsigned char byte : bytes)
  {
    crc ^= byte;
    for (int bit = 0; bit < 8; ++bit)
    {
      crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
  }
  return crc ^ 0xffffffffu;
}

// ODF requires mimetype to be the first, uncompressed ZIP member. Store all
// entries without compression to avoid private Qt ZIP APIs or external tools.
bool package(QIODevice& output, const std::vector<std::pair<QByteArray, QByteArray>>& entries)
{
  QDataStream stream(&output);
  stream.setByteOrder(QDataStream::LittleEndian);
  QByteArray central;
  QDataStream directory(&central, QIODevice::WriteOnly);
  directory.setByteOrder(QDataStream::LittleEndian);
  for (const auto& entry : entries)
  {
    const quint32 offset = static_cast<quint32>(output.pos());
    const quint32 size = static_cast<quint32>(entry.second.size());
    const quint32 crc = crc32(entry.second);
    const quint16 nameSize = static_cast<quint16>(entry.first.size());
    stream << quint32(0x04034b50) << quint16(20) << quint16(0) << quint16(0) << quint16(0) << quint16(0)
      << crc << size << size << nameSize << quint16(0);
    stream.writeRawData(entry.first.constData(), entry.first.size());
    stream.writeRawData(entry.second.constData(), entry.second.size());
    directory << quint32(0x02014b50) << quint16(20) << quint16(20) << quint16(0) << quint16(0)
      << quint16(0) << quint16(0) << crc << size << size << nameSize << quint16(0) << quint16(0)
      << quint16(0) << quint16(0) << quint32(0) << offset;
    directory.writeRawData(entry.first.constData(), entry.first.size());
  }
  const quint32 start = static_cast<quint32>(output.pos());
  stream.writeRawData(central.constData(), central.size());
  stream << quint32(0x06054b50) << quint16(0) << quint16(0) << quint16(entries.size()) << quint16(entries.size())
    << quint32(central.size()) << start << quint16(0);
  return stream.status() == QDataStream::Ok && directory.status() == QDataStream::Ok;
}
}

ReportOdtExporter::ReportOdtExporter() = default;

bool ReportOdtExporter::write(const ReportDocumentLayout& layout, const std::string& filename, std::string& error) const
{
  const auto& report = layout.report();
  if (!report.validate(error))
  {
    return false;
  }
  std::vector<std::pair<QByteArray, QByteArray>> files;
  files.push_back({"mimetype", "application/vnd.oasis.opendocument.text"});
  if (report.configuration.company_logo)
  {
    files.push_back({"Pictures/company-logo.png", QByteArray::fromBase64(QByteArray::fromStdString(report.configuration.company_logo->base64_data))});
  }
  QByteArray content;
  QXmlStreamWriter xml(&content);
  xml.writeStartDocument();
  xml.writeStartElement("office:document-content");
  namespaces(xml);
  xml.writeStartElement("office:automatic-styles");
  for (int level = 1; level <= 2; ++level)
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
    xml.writeAttribute("fo:font-size", level == 1 ? "20pt" : "13pt");
    xml.writeAttribute("fo:font-weight", "bold");
    xml.writeAttribute("fo:color", "#18334c");
    xml.writeEndElement();
    xml.writeEndElement();
  }
  xml.writeEndElement();
  xml.writeStartElement("office:body");
  xml.writeStartElement("office:text");
  auto heading = [&](const QString& value, int level)
  {
    xml.writeStartElement("text:h");
    xml.writeAttribute("text:outline-level", QString::number(level));
    xml.writeAttribute("text:style-name", "Heading" + QString::number(level));
    xml.writeCharacters(value);
    xml.writeEndElement();
  };
  const auto value = [&](const std::string& key)
  {
    const auto found = report.configuration.values.find(key);
    return found == report.configuration.values.end() ? std::string() : found->second;
  };
  if (value("cover") == "true")
  {
    xml.writeStartElement("text:p");
    xml.writeAttribute("text:style-name", "CoverText");
    xml.writeCharacters("ENGINEERING REPORT");
    xml.writeEndElement();
    paragraph(xml, QString::fromStdString(report.title));
    paragraph(xml, "Project: " + QString::fromStdString(value("project")));
    paragraph(xml, "Author: " + QString::fromStdString(value("author")));
    paragraph(xml, "Revision: " + QString::fromStdString(value("revision")));
    paragraph(xml, "Generated: " + QString::fromStdString(report.generated_at));
    paragraph(xml, "Source: " + QString::fromStdString(report.source.workspace_id) + (report.source.kind == "model" ? " / current model" : " / job " + QString::number(report.source.job_id)));
    paragraph(xml, QString::fromStdString(report.source.provenance));
  }
  if (value("contents") == "true")
  {
    heading("Contents", 1);
    // A native index is regenerated by the editor; cached entries deliberately
    // omit PDF page numbers because ODT pagination is editor-dependent.
    xml.writeStartElement("text:table-of-content");
    xml.writeAttribute("text:name", "Contents");
    xml.writeStartElement("text:table-of-content-source");
    xml.writeAttribute("text:outline-level", "2");
    xml.writeEndElement();
    xml.writeStartElement("text:index-body");
    for (const auto& section : report.sections)
    {
      paragraph(xml, QString::fromStdString(section.title));
    }
    xml.writeEndElement();
    xml.writeEndElement();
  }
  int imageNumber = 0, tableNumber = 0;
  std::set<std::string> renderedNotes;
  const int precision = QString::fromStdString(value("precision")).toInt();
  std::function<void(const ReportSection&, int)> section = [&](const ReportSection& item, int level)
  {
    heading(QString::fromStdString(item.title), level == 0 ? 1 : 2);
    for (const auto& line : item.paragraphs)
    {
      paragraph(xml, QString::fromStdString(line));
    }
    for (const auto& table : item.tables)
    {
      heading(QString::fromStdString(table.title), 2);
      xml.writeStartElement("table:table");
      xml.writeAttribute("table:name", "Table" + QString::number(++tableNumber));
      auto row = [&](const std::vector<QJsonValue>& cells)
      {
        xml.writeStartElement("table:table-row");
        for (const auto& cell : cells)
        {
          xml.writeStartElement("table:table-cell");
          xml.writeAttribute("office:value-type", cell.isDouble() ? "float" : "string");
          if (cell.isDouble())
          {
            xml.writeAttribute("office:value", QString::number(cell.toDouble(), 'g', 17));
          }
          paragraph(xml, cell.isDouble() ? QString::number(cell.toDouble(), 'g', precision) : cell.toString());
          xml.writeEndElement();
        }
        xml.writeEndElement();
      };
      std::vector<QJsonValue> labels;
      for (std::size_t i = 0; i < table.columns.size(); ++i)
      {
        QString label = QString::fromStdString(table.columns[i]);
        if (i < table.units.size() && !table.units[i].empty())
        {
          label += " [" + QString::fromStdString(table.units[i]) + "]";
        }
        labels.push_back(label);
      }
      xml.writeStartElement("table:table-header-rows");
      row(labels);
      xml.writeEndElement();
      for (const auto& cells : table.rows)
      {
        row(cells);
      }
      xml.writeEndElement();
    }
    for (const auto& image : item.images)
    {
      const QByteArray path = "Pictures/image" + QByteArray::number(++imageNumber) + (image.media_type == "image/png" ? ".png" : ".jpg");
      const auto bytes = QByteArray::fromBase64(QByteArray::fromStdString(image.base64_data));
      const auto pixels = QImage::fromData(bytes);
      files.push_back({path, bytes});
      xml.writeStartElement("text:p");
      xml.writeStartElement("draw:frame");
      xml.writeAttribute("draw:name", "Image" + QString::number(imageNumber));
      xml.writeAttribute("text:anchor-type", "as-char");
      xml.writeAttribute("svg:width", "16cm");
      const double height = pixels.width() > 0 ? std::min(20.0, 16.0 * pixels.height() / pixels.width()) : 9.0;
      xml.writeAttribute("svg:height", QString::number(height) + "cm");
      xml.writeStartElement("draw:image");
      xml.writeAttribute("xlink:href", QString::fromUtf8(path));
      xml.writeAttribute("xlink:type", "simple");
      xml.writeEndElement();
      xml.writeEndElement();
      xml.writeEndElement();
      paragraph(xml, QString::fromStdString(image.caption));
    }
    const auto note = report.notes.find(item.id);
    if (note != report.notes.end())
    {
      renderedNotes.insert(note->first);
      heading("Notes", 2);
      paragraph(xml, QString::fromStdString(note->second));
    }
    for (const auto& child : item.children)
    {
      section(child, level + 1);
    }
  };
  for (const auto& item : report.sections)
  {
    section(item, 0);
  }
  for (const auto& note : report.notes)
  {
    if (renderedNotes.count(note.first) == 0 && !note.second.empty())
    {
      heading("Retained note / " + QString::fromStdString(note.first), 2);
      paragraph(xml, QString::fromStdString(note.second));
    }
  }
  heading("Sources and diagnostics", 1);
  paragraph(xml, QString::fromStdString(report.source.provenance));
  if (!report.source.snapshot_fingerprint.empty())
  {
    paragraph(xml, "Reporting snapshot SHA-256: " + QString::fromStdString(report.source.snapshot_fingerprint));
  }
  for (std::size_t i = 0; i < report.source.input_files.size(); ++i)
  {
    paragraph(xml, QString::fromStdString(report.source.input_files[i]));
    paragraph(xml, "SHA-256: " + QString::fromStdString(report.source.fingerprints[i]));
  }
  for (const auto& issue : report.diagnostics)
  {
    paragraph(xml, QString::fromStdString(issue.severity + " / " + issue.section_id + ": " + issue.message));
  }
  xml.writeEndElement();
  xml.writeEndElement();
  xml.writeEndElement();
  xml.writeEndDocument();
  files.push_back({"content.xml", content});

  QByteArray styles;
  QXmlStreamWriter sx(&styles);
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
  sx.writeAttribute("style:master-page-name", ReportPageDecoration::value(report, "decoration_cover") == "false" ? "Cover" : "Standard");
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
  sx.writeAttribute("fo:page-width", value("page_size") == "Letter" ? "21.59cm" : "21cm");
  sx.writeAttribute("fo:page-height", value("page_size") == "Letter" ? "27.94cm" : "29.7cm");
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
    sx.writeAttribute(band == "header" ? "fo:margin-bottom" : "fo:margin-top", "16pt");
    if (ReportPageDecoration::value(report, band + "_line") == "true")
    {
      sx.writeAttribute(band == "header" ? "fo:border-bottom" : "fo:border-top", "0.5pt solid #18334c");
    }
    sx.writeEndElement();
    sx.writeEndElement();
  }
  sx.writeEndElement();
  sx.writeStartElement("style:page-layout");
  sx.writeAttribute("style:name", "CoverPage");
  sx.writeStartElement("style:page-layout-properties");
  sx.writeAttribute("fo:page-width", value("page_size") == "Letter" ? "21.59cm" : "21cm");
  sx.writeAttribute("fo:page-height", value("page_size") == "Letter" ? "27.94cm" : "29.7cm");
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
  files.push_back({"styles.xml", styles});

  QByteArray manifest;
  QXmlStreamWriter mx(&manifest);
  mx.writeStartDocument();
  mx.writeStartElement("manifest:manifest");
  mx.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0", "manifest");
  mx.writeAttribute("manifest:version", "1.2");
  auto entry = [&](const QString& path, const QString& type)
  {
    mx.writeStartElement("manifest:file-entry");
    mx.writeAttribute("manifest:full-path", path);
    mx.writeAttribute("manifest:media-type", type);
    mx.writeEndElement();
  };
  entry("/", "application/vnd.oasis.opendocument.text");
  for (const auto& file : files)
  {
    if (file.first != "mimetype")
    {
      entry(QString::fromUtf8(file.first), file.first.endsWith(".png") ? "image/png" : file.first.endsWith(".jpg") ? "image/jpeg" : "text/xml");
    }
  }
  mx.writeEndElement();
  mx.writeEndDocument();
  files.push_back({"META-INF/manifest.xml", manifest});
  QSaveFile output(QString::fromStdString(filename));
  output.setDirectWriteFallback(false);
  if (xml.hasError() || sx.hasError() || mx.hasError() || !output.open(QIODevice::WriteOnly) || !package(output, files) || !output.commit())
  {
    error = "Cannot create or commit the ODT document.";
    return false;
  }
  error.clear();
  return true;
}
