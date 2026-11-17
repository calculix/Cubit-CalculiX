#include "ReportOdtExporter.hpp"
#include "ReportOdtPackage.hpp"
#include "ReportOdtStyles.hpp"
#include "ReportFigureCaption.hpp"
#include "ReportPageDecoration.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include "Report/Core/ReportUnitValidation.hpp"
#include "Report/Core/ReportLogo.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
#include <QXmlStreamWriter>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QBuffer>
#include <QImageReader>
#include <QCryptographicHash>
#include <QStringList>
#include <functional>
#include <set>
#include <algorithm>
#include <limits>

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


void paragraph(QXmlStreamWriter& xml, const QString& value, const QString& name = QString())
{
  xml.writeStartElement("text:p");
  if (!name.isEmpty())
  {
    xml.writeAttribute("text:style-name", name);
  }
  const auto lines = value.split('\n');
  for (qsizetype index = 0; index < lines.size(); ++index)
  {
    if (index > 0)
    {
      xml.writeEmptyElement("text:line-break");
    }
    xml.writeCharacters(lines[index]);
  }
  xml.writeEndElement();
}

QString bookmark(const std::string& id)
{
  return "Section_" + QString::fromLatin1(QCryptographicHash::hash(
    QByteArray::fromStdString(id), QCryptographicHash::Sha256).toHex());
}

bool image_payload(const ReportImage& image, QByteArray& bytes, QSize& pixels, std::string& error)
{
  const auto maximum = static_cast<std::size_t>(ReportUserOptions::export_image_max_bytes());
  if (image.base64_data.size() > ((maximum + 2) / 3) * 4)
  {
    error = "ODT encoded image exceeds the image limit: " + image.id;
    return false;
  }
  bytes = QByteArray::fromBase64(QByteArray::fromStdString(image.base64_data), QByteArray::AbortOnBase64DecodingErrors);
  if (bytes.isEmpty() || bytes.size() > ReportUserOptions::export_image_max_bytes())
  {
    error = "ODT image payload is invalid or exceeds the image limit: " + image.id;
    return false;
  }
  QBuffer buffer(&bytes);
  if (!buffer.open(QIODevice::ReadOnly))
  {
    error = "Cannot inspect ODT image: " + image.id;
    return false;
  }
  QImageReader reader(&buffer);
  pixels = reader.size();
  const auto format = reader.format();
  if ((image.media_type == "image/png" && format != "png") ||
      (image.media_type == "image/jpeg" && format != "jpeg" && format != "jpg") ||
      pixels.width() <= 0 || pixels.height() <= 0 ||
      pixels.width() > ReportUserOptions::export_image_max_side() ||
      pixels.height() > ReportUserOptions::export_image_max_side() ||
      static_cast<qint64>(pixels.width()) * pixels.height() > ReportUserOptions::export_image_max_pixels() ||
      reader.read().isNull())
  {
    error = "ODT image is invalid or oversized: " + image.id;
    return false;
  }
  return true;
}
}

ReportOdtExporter::ReportOdtExporter() = default;

bool ReportOdtExporter::encode(const ReportDocument& report, QIODevice& output,
  std::string& error, ReportProgress* progress) const
{
  if (!output.isWritable() || output.pos() != 0 || !ReportUnitValidation::document(report, error) ||
      !ReportPageDecoration::validate_templates(report.configuration, error))
  {
    if (error.empty())
    {
      error = "ODT export requires a valid report and writable output.";
    }
    return false;
  }
  std::size_t totalWork = 0, completedWork = 0, lastCheckpoint = 0;
  const auto add = [&](std::size_t amount)
  {
    if (amount > (std::numeric_limits<std::size_t>::max)() - totalWork)
    {
      error = "ODT content exceeds the supported work count.";
      return false;
    }
    totalWork += amount;
    return true;
  };
  std::function<bool(const ReportSection&)> count = [&](const ReportSection& item)
  {
    if (!ReportProgress::advance(progress, ReportProgressStage::ExportOdt,
          "Planning ODT content", 0, 0, error) || !add(1) ||
        !add(item.paragraphs.size()) || !add(item.images.size()))
    {
      return false;
    }
    for (const auto& table : item.tables)
    {
      const auto columns = table.columns.size();
      if (table.rows.size() == (std::numeric_limits<std::size_t>::max)() ||
          columns > (std::numeric_limits<std::size_t>::max)() / (table.rows.size() + 1) ||
          !add((table.rows.size() + 1) * columns))
      {
        error = "ODT table exceeds the supported work count.";
        return false;
      }
    }
    for (const auto& child : item.children)
    {
      if (!count(child))
      {
        return false;
      }
    }
    return true;
  };
  for (const auto& item : report.sections)
  {
    if (!count(item))
    {
      return false;
    }
  }
  QTemporaryFile content;
  QTemporaryFile styles;
  QTemporaryFile manifest;
  if (!content.open() || !styles.open() || !manifest.open())
  {
    error = "Cannot stage ODT XML files.";
    return false;
  }
  std::vector<std::pair<QByteArray, const std::string*>> assets;
  quint64 assetBytes = 0;
  if (report.configuration.company_logo)
  {
    const auto& logo = *report.configuration.company_logo;
    if (!ReportLogo::validate(logo, error))
    {
      return false;
    }
    const auto bytes = QByteArray::fromBase64(QByteArray::fromStdString(logo.base64_data), QByteArray::AbortOnBase64DecodingErrors);
    assets.push_back({"Pictures/company-logo.png", &logo.base64_data});
    assetBytes += static_cast<quint64>(bytes.size());
  }
  QXmlStreamWriter xml(&content);
  const auto tick = [&](std::size_t amount, const std::string& stage, bool force)
  {
    if (amount > totalWork - completedWork)
    {
      error = "ODT content work count changed unexpectedly.";
      return false;
    }
    completedWork += amount;
    if (xml.hasError() || static_cast<quint64>(content.size()) >= 0xffffffffull)
    {
      error = "ODT content write failed or exceeds classic ZIP limits.";
      return false;
    }
    if (force || completedWork == totalWork ||
        completedWork - lastCheckpoint >= static_cast<std::size_t>(ReportUserOptions::export_checkpoint_cells()))
    {
      lastCheckpoint = completedWork;
      return ReportProgress::advance(progress, ReportProgressStage::ExportOdt,
        stage, completedWork, totalWork, error);
    }
    return true;
  };
  xml.writeStartDocument();
  xml.writeStartElement("office:document-content");
  namespaces(xml);
  ReportOdtStyles::write_content(xml);
  xml.writeStartElement("office:body");
  xml.writeStartElement("office:text");
  auto heading = [&](const QString& value, int level, const QString& anchor = QString())
  {
    xml.writeStartElement("text:h");
    xml.writeAttribute("text:outline-level", QString::number(level));
    xml.writeAttribute("text:style-name", "Heading" + QString::number(level));
    if (!anchor.isEmpty())
    {
      xml.writeEmptyElement("text:bookmark");
      xml.writeAttribute("text:name", anchor);
    }
    xml.writeCharacters(value);
    xml.writeEndElement();
  };
  const auto value = [&](const std::string& key)
  {
    return ReportPageDecoration::value(report, key).toStdString();
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
    paragraph(xml, "Source: " + QString::fromStdString(report.source.workspace_id) + ([&]() -> QString
    {
      if (report.source.kind == "model")
      {
        return " / current model";
      }
      else
      {
        return " / job " + QString::number(report.source.job_id);
      }
    }()));
    paragraph(xml, QString::fromStdString(report.source.provenance));
  }
  if (value("contents") == "true")
  {
    paragraph(xml, "Contents", "ContentsTitle");
    // A native index is regenerated by the editor; cached entries deliberately
    // omit PDF page numbers because ODT pagination is editor-dependent.
    xml.writeStartElement("text:table-of-content");
    xml.writeAttribute("text:name", "Contents");
    xml.writeStartElement("text:table-of-content-source");
    xml.writeAttribute("text:outline-level", "10");
    xml.writeEndElement();
    xml.writeStartElement("text:index-body");
    std::function<bool(const ReportSection&, int)> entry = [&](const ReportSection& item, int level)
    {
      if (!tick(0, "Writing ODT contents", true))
      {
        return false;
      }
      xml.writeStartElement("text:p");
      xml.writeAttribute("text:style-name", "Contents" + QString::number(std::clamp(level, 1, 10)));
      xml.writeStartElement("text:a");
      xml.writeAttribute("xlink:href", "#" + bookmark(item.id));
      xml.writeAttribute("xlink:type", "simple");
      xml.writeCharacters(QString::fromStdString(item.title));
      xml.writeEndElement();
      xml.writeEndElement();
      for (const auto& child : item.children)
      {
        if (!entry(child, level + 1))
        {
          return false;
        }
      }
      return true;
    };
    for (const auto& item : report.sections)
    {
      if (!entry(item, 1))
      {
        return false;
      }
    }
    ReportSection diagnosticsEntry;
    diagnosticsEntry.id = "report.diagnostics";
    diagnosticsEntry.title = "Sources and diagnostics";
    if (!entry(diagnosticsEntry, 1))
    {
      return false;
    }
    xml.writeEndElement();
    xml.writeEndElement();
  }
  int imageNumber = 0, tableNumber = 0;
  std::set<std::string> renderedNotes;
  const int precision = QString::fromStdString(value("precision")).toInt();
  std::function<bool(const ReportSection&, int)> section = [&](const ReportSection& item, int level)
  {
    if (!tick(1, "Writing ODT sections", true))
    {
      return false;
    }
    heading(QString::fromStdString(item.title), std::clamp(level + 1, 1, 10), bookmark(item.id));
    for (const auto& line : item.paragraphs)
    {
      paragraph(xml, QString::fromStdString(line));
      if (!tick(1, "Writing ODT paragraphs", false))
      {
        return false;
      }
    }
    for (const auto& table : item.tables)
    {
      paragraph(xml, QString::fromStdString(table.title), "TableTitle");
      xml.writeStartElement("table:table");
      xml.writeAttribute("table:name", "Table" + QString::number(++tableNumber));
      xml.writeAttribute("table:style-name", "ReportTable");
      xml.writeEmptyElement("table:table-column");
      xml.writeAttribute("table:number-columns-repeated", QString::number(table.columns.size()));
      xml.writeAttribute("table:style-name", "ReportColumn");
      auto row = [&](const std::vector<QJsonValue>& cells, bool header)
      {
        xml.writeStartElement("table:table-row");
        for (const auto& cell : cells)
        {
          xml.writeStartElement("table:table-cell");
          QString cellStyle = "ReportCell";
          if (header)
          {
            cellStyle = "ReportHeaderCell";
          }
          xml.writeAttribute("table:style-name", cellStyle);
          xml.writeAttribute("office:value-type", [&]() -> const char*
          {
            if (cell.isDouble())
            {
              return "float";
            }
            else
            {
              return "string";
            }
          }());
          if (cell.isDouble())
          {
            xml.writeAttribute("office:value", QString::number(cell.toDouble(), 'g', 17));
          }
          QString display = cell.toString();
          QString paragraphStyle;
          if (cell.isDouble())
          {
            display = QString::number(cell.toDouble(), 'g', precision);
            paragraphStyle = "Number";
          }
          paragraph(xml, display, paragraphStyle);
          xml.writeEndElement();
          if (!tick(1, "Writing ODT tables", false))
          {
            return false;
          }
        }
        xml.writeEndElement();
        if (xml.hasError())
        {
          error = "Cannot write ODT content XML.";
          return false;
        }
        return true;
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
      if (!row(labels, true))
      {
        return false;
      }
      xml.writeEndElement();
      for (const auto& cells : table.rows)
      {
        if (!row(cells, false))
        {
          return false;
        }
      }
      xml.writeEndElement();
    }
    for (const auto& image : item.images)
    {
      const QByteArray path = "Pictures/image" + QByteArray::number(++imageNumber) + ([&]() -> const char*
      {
        if (image.media_type == "image/png")
        {
          return ".png";
        }
        else
        {
          return ".jpg";
        }
      }());
      if (!tick(0, "Preparing ODT image " + std::to_string(imageNumber), true))
      {
        return false;
      }
      QByteArray bytes;
      QSize pixels;
      if (!image_payload(image, bytes, pixels, error))
      {
        return false;
      }
      assets.push_back({path, &image.base64_data});
      assetBytes += static_cast<quint64>(bytes.size());
      xml.writeStartElement("text:p");
      xml.writeAttribute("text:style-name", "Figure");
      xml.writeStartElement("draw:frame");
      xml.writeAttribute("draw:name", "Image" + QString::number(imageNumber));
      xml.writeAttribute("text:anchor-type", "as-char");
      const auto size = ReportOdtStyles::figure_size(report, pixels);
      xml.writeAttribute("svg:width", QString::number(size.width(), 'g', 12) + "pt");
      xml.writeAttribute("svg:height", QString::number(size.height(), 'g', 12) + "pt");
      xml.writeStartElement("draw:image");
      xml.writeAttribute("xlink:href", QString::fromUtf8(path));
      xml.writeAttribute("xlink:type", "simple");
      xml.writeEndElement();
      xml.writeEndElement();
      xml.writeEndElement();
      paragraph(xml, ReportFigureCaption::text(image, report.configuration), "FigureCaption");
      if (!tick(1, "Writing ODT images", true))
      {
        return false;
      }
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
      if (!section(child, level + 1))
      {
        return false;
      }
    }
    if (xml.hasError())
    {
      error = "Cannot write ODT content XML.";
      return false;
    }
    return true;
  };
  for (const auto& item : report.sections)
  {
    if (!section(item, 0))
    {
      return false;
    }
  }
  for (const auto& note : report.notes)
  {
    if (renderedNotes.count(note.first) == 0 && !note.second.empty())
    {
      heading("Retained note / " + QString::fromStdString(note.first), 2);
      paragraph(xml, QString::fromStdString(note.second));
    }
  }
  heading("Sources and diagnostics", 1, bookmark("report.diagnostics"));
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
  if (xml.hasError() || !content.flush() ||
      !tick(0, "Writing ODT styles", true) ||
      !ReportOdtStyles::write_document(styles, report, error) || !styles.flush())
  {
    if (error.empty())
    {
      error = "Cannot finalize ODT content or styles.";
    }
    return false;
  }
  QXmlStreamWriter mx(&manifest);
  mx.writeStartDocument();
  mx.writeStartElement("manifest:manifest");
  mx.writeNamespace("urn:oasis:names:tc:opendocument:xmlns:manifest:1.0", "manifest");
  mx.writeAttribute("manifest:version", "1.2");
  const auto entry = [&](const QString& path, const QString& type)
  {
    mx.writeStartElement("manifest:file-entry");
    mx.writeAttribute("manifest:full-path", path);
    mx.writeAttribute("manifest:media-type", type);
    mx.writeEndElement();
  };
  entry("/", "application/vnd.oasis.opendocument.text");
  for (const QString name : {QString("content.xml"), QString("styles.xml")})
  {
    entry(name, "text/xml");
  }
  for (const auto& asset : assets)
  {
    if (!tick(0, "Writing ODT manifest", true))
    {
      return false;
    }
    QString type = "image/png";
    if (asset.first.endsWith(".jpg"))
    {
      type = "image/jpeg";
    }
    entry(QString::fromUtf8(asset.first), type);
  }
  mx.writeEndElement();
  mx.writeEndDocument();
  if (mx.hasError() || !manifest.flush() || completedWork != totalWork)
  {
    error = "Cannot finalize the ODT manifest or content work count.";
    return false;
  }
  const QByteArray mime = "application/vnd.oasis.opendocument.text";
  const std::vector<QByteArray> names = {"mimetype", "content.xml", "styles.xml", "META-INF/manifest.xml"};
  quint64 directoryBytes = 0;
  for (const auto& name : names)
  {
    directoryBytes += 46 + static_cast<quint64>(name.size());
  }
  for (const auto& asset : assets)
  {
    directoryBytes += 46 + static_cast<quint64>(asset.first.size());
  }
  const auto payloadBytes = assetBytes + static_cast<quint64>(mime.size()) +
    static_cast<quint64>(content.size()) + static_cast<quint64>(styles.size()) + static_cast<quint64>(manifest.size());
  ReportOdtPackage package(output);
  if (!package.begin(payloadBytes, directoryBytes, error))
  {
    return false;
  }
  QBuffer mimeInput;
  mimeInput.setData(mime);
  if (!mimeInput.open(QIODevice::ReadOnly) || !package.add_file("mimetype", mimeInput, progress, error) ||
      !package.add_file("content.xml", content, progress, error) ||
      !package.add_file("styles.xml", styles, progress, error))
  {
    return false;
  }
  for (const auto& asset : assets)
  {
    if (!ReportProgress::advance(progress, ReportProgressStage::ExportPackage,
          "Preparing ODT image payload", 0, 0, error))
    {
      return false;
    }
    const auto bytes = QByteArray::fromBase64(QByteArray::fromStdString(*asset.second), QByteArray::AbortOnBase64DecodingErrors);
    QBuffer input;
    input.setData(bytes);
    if (bytes.isEmpty() || !input.open(QIODevice::ReadOnly) || !package.add_file(asset.first, input, progress, error))
    {
      if (error.empty())
      {
        error = "Cannot package ODT image: " + asset.first.toStdString();
      }
      return false;
    }
  }
  if (!package.add_file("META-INF/manifest.xml", manifest, progress, error) || !package.finish(progress, error))
  {
    return false;
  }
  error.clear();
  return true;
}

bool ReportOdtExporter::write(const ReportDocument& report, const std::string& filename,
  std::string& error, ReportProgress* progress) const
{
  QSaveFile output(QString::fromStdString(filename));
  output.setDirectWriteFallback(false);
  if (!output.open(QIODevice::WriteOnly))
  {
    error = "Cannot open ODT output: " + output.errorString().toStdString();
    return false;
  }
  if (!encode(report, output, error, progress) ||
      !ReportProgress::advance(progress, ReportProgressStage::ExportCommit, "Publishing ODT", 0, 1, error))
  {
    return false;
  }
  if (!output.commit())
  {
    error = "Cannot commit ODT output: " + output.errorString().toStdString();
    return false;
  }
  if (progress)
  {
    progress->finish();
  }
  error.clear();
  return true;
}
