#include "ReportTestProviders.hpp"
#include "ReportTestProgress.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/Document/ReportPdfExporter.hpp"
#include "Report/Document/ReportOdtExporter.hpp"
#include "Report/Document/ReportOdtPackage.hpp"
#include "Report/Document/ReportOdtStyles.hpp"
#include "Report/Document/ReportPageDecoration.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QImage>
#include <QXmlStreamReader>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>

namespace
{
QByteArray contents(const QString& filename)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
  {
    return {};
  }
  return file.readAll();
}

bool fixture(const QString& filename, const QByteArray& bytes)
{
  QFile file(filename);
  return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
}

bool entries(const QByteArray& bytes, std::map<QByteArray, QByteArray>& output)
{
  QBuffer buffer;
  buffer.setData(bytes);
  if (!buffer.open(QIODevice::ReadOnly))
  {
    return false;
  }
  QDataStream stream(&buffer);
  stream.setByteOrder(QDataStream::LittleEndian);
  while (!buffer.atEnd())
  {
    quint32 signature = 0;
    stream >> signature;
    if (signature == 0x02014b50)
    {
      return !output.empty();
    }
    if (signature != 0x04034b50)
    {
      return false;
    }
    quint16 version = 0, flags = 0, method = 0, time = 0, date = 0, nameSize = 0, extraSize = 0;
    quint32 crc = 0, packed = 0, unpacked = 0;
    stream >> version >> flags >> method >> time >> date >> crc >> packed >> unpacked >> nameSize >> extraSize;
    if (stream.status() != QDataStream::Ok || flags != 0 || method != 0 || packed != unpacked)
    {
      return false;
    }
    const auto name = buffer.read(nameSize);
    if (name.size() != nameSize || !buffer.seek(buffer.pos() + extraSize))
    {
      return false;
    }
    const auto data = buffer.read(unpacked);
    if (static_cast<quint64>(data.size()) != unpacked || !output.emplace(name, data).second)
    {
      return false;
    }
  }
  return false;
}

ReportImage picture(const std::string& id, const QSize& size)
{
  QImage pixels(size, QImage::Format_RGB32);
  pixels.fill(Qt::blue);
  QByteArray png;
  QBuffer buffer(&png);
  buffer.open(QIODevice::WriteOnly);
  pixels.save(&buffer, "PNG");
  ReportImage image;
  image.id = id;
  image.caption = id;
  image.media_type = "image/png";
  image.base64_data = png.toBase64().toStdString();
  return image;
}
}

int reportExportTests(const QString& directory)
{
  int failures = 0;
  const auto check = [&](bool condition, const char* message)
  {
    if (!condition)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  ReportTestProviders providers;
  ReportApplicationInterface application(providers);
  check(application.generate("test", 1, "", error), "Generate export fixture without a GUI preview");
  const auto original = application.document();
  check(original && !application.prepared_layout(), "Export fixture has no prepared layout");
  if (!original)
  {
    return failures + 1;
  }
  const auto frozen = original->to_json();
  ReportTestProgress direct;
  bool observedBusy = false;
  direct.onUpdate = [&]()
  {
    observedBusy = application.busy();
    std::string local;
    check(!application.configure("author", "must not change", local), "Export blocks reentrant configuration");
  };
  const auto directOdt = directory + "/direct-document.odt";
  check(application.export_document("odt", directOdt.toStdString(), error, true, &direct) && direct.finished,
    "ODT export completes without preparing a preview");
  check(observedBusy && !application.busy() && !application.prepared_layout() && original->to_json() == frozen,
    "ODT export preserves the report and cache and releases its busy guard");
  check(std::find(direct.phases.begin(), direct.phases.end(), ReportProgressStage::PreviewContent) == direct.phases.end(),
    "ODT export never enters Qt preview preparation");

  ReportDocument large = *original;
  auto& table = large.sections.front().tables.front();
  table.rows.clear();
  table.cell_quantities.clear();
  table.units = {"", "mm"};
  for (int index = 0; index < 600; ++index)
  {
    table.rows.push_back({"Row " + QString::number(index), 1.25 + index});
  }
  large.sections.front().images = {picture("portrait", QSize(100, 800)), picture("landscape", QSize(800, 100))};
  ReportSection child;
  child.id = "export.child";
  child.title = "Block 1 / Stress / XX & YY";
  child.paragraphs = {"Nested component result"};
  large.sections.front().children.push_back(child);
  const auto semantic = large.to_json();
  ReportDocumentLayout layout;
  check(layout.prepare(large, error) && layout.page_count() > 2, "Prepare multi-page export fixture");
  const auto pdfPath = directory + "/cancel-existing.pdf";
  check(fixture(pdfPath, "old PDF"), "Create existing PDF fixture");
  ReportTestProgress pdfCancel;
  pdfCancel.cancelStage = "Writing PDF page";
  pdfCancel.cancelAfter = 2;
  check(!ReportPdfExporter().write(layout, pdfPath.toStdString(), error, &pdfCancel) &&
    !pdfCancel.finished && error.find("regression sink") != std::string::npos && contents(pdfPath) == "old PDF", "PDF page cancellation preserves the existing file");

  const auto odtPath = directory + "/cancel-existing.odt";
  for (const std::string stage : {"Writing ODT tables", "Preparing ODT image", "Writing ODT styles", "Checking ODT entry", "Writing ODT entry", "Publishing ODT"})
  {
    check(fixture(odtPath, "old ODT"), "Create existing ODT fixture");
    ReportTestProgress cancelled;
    cancelled.cancelStage = stage;
    check(!ReportOdtExporter().write(large, odtPath.toStdString(), error, &cancelled) &&
      !cancelled.finished && error.find("regression sink") != std::string::npos && contents(odtPath) == "old ODT", "ODT cancellation preserves the destination at every export phase");
  }
  const auto base = directory + "/paired-cancellation";
  check(fixture(base + ".pdf", "old paired PDF") && fixture(base + ".odt", "old paired ODT"),
    "Create paired export fixtures");
  ReportTestProgress pairCancel;
  pairCancel.cancelStage = "Publishing exported report";
  check(!application.export_document("both", base.toStdString(), error, true, &pairCancel) &&
    !pairCancel.finished && error.find("regression sink") != std::string::npos && contents(base + ".pdf") == "old paired PDF" &&
    contents(base + ".odt") == "old paired ODT" && !application.prepared_layout(),
    "Cancellation after both encoders finish preserves both outputs and the previous cache");
  ReportTestProgress pairSuccess;
  check(application.export_document("both", base.toStdString(), error, true, &pairSuccess) &&
    pairSuccess.finished && contents(base + ".pdf").startsWith("%PDF-") && contents(base + ".odt").startsWith("PK"),
    "Paired export publishes both outputs before completion");

  QByteArray bytes;
  QBuffer destination(&bytes);
  check(destination.open(QIODevice::WriteOnly), "Open ODT memory output");
  ReportTestProgress progress;
  check(ReportOdtExporter().encode(large, destination, error, &progress) && !progress.finished,
    "Encoding does not claim completion before publication");
  std::map<QByteArray, QByteArray> members;
  check(entries(bytes, members) && bytes.mid(30, 8) == "mimetype" &&
    members["mimetype"] == "application/vnd.oasis.opendocument.text", "ODT stores mimetype first and has readable ZIP entries");
  check(members.count("Pictures/image1.png") == 1 && members.count("Pictures/image2.png") == 1 &&
    members["content.xml"].contains("Value [mm]") && members["content.xml"].contains("office:value="1.25""),
    "ODT preserves images, numeric values and stored unit labels");
  std::set<QString> anchors, links;
  std::vector<QString> parents;
  int frames = 0;
  QXmlStreamReader xml(members["content.xml"]);
  while (!xml.atEnd())
  {
    xml.readNext();
    if (xml.isStartElement())
    {
      const auto name = xml.name().toString();
      if (name == "bookmark")
      {
        check(!parents.empty() && parents.back() == "h", "Contents bookmarks are inside their section headings");
        anchors.insert(xml.attributes().value("urn:oasis:names:tc:opendocument:xmlns:text:1.0", "name").toString());
      }
      if (name == "a")
      {
        auto target = xml.attributes().value("http://www.w3.org/1999/xlink", "href").toString();
        if (target.startsWith('#'))
        {
          links.insert(target.mid(1));
        }
      }
      if (name == "frame")
      {
        const auto width = xml.attributes().value("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0", "width").toString();
        const auto height = xml.attributes().value("urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0", "height").toString();
        const double w = width.left(width.size() - 2).toDouble();
        const double h = height.left(height.size() - 2).toDouble();
        double ratio = 0.125;
        if (frames == 1)
        {
          ratio = 8.0;
        }
        check(w > 0 && h > 0 && std::abs(w / h - ratio) < 1e-9 && h <= ReportUserOptions::figure_max_height() + 1e-8,
          "Portrait and landscape figures preserve their aspect ratio");
        ++frames;
      }
      parents.push_back(name);
    }
    else if (xml.isEndElement() && !parents.empty())
    {
      parents.pop_back();
    }
  }
  check(!xml.hasError() && frames == 2 && links.size() >= 2 &&
    std::all_of(links.begin(), links.end(), [&](const QString& link)
    {
      return anchors.count(link) != 0;
    }), "ODT cached contents link to nested section bookmarks");
  QXmlStreamReader styles(members["styles.xml"]);
  while (!styles.atEnd())
  {
    styles.readNext();
  }
  check(!styles.hasError() && members["content.xml"].contains("table:table-header-rows") &&
    members["content.xml"].contains("ReportHeaderCell") && members["content.xml"].contains("FigureCaption"),
    "ODT style and repeated-header XML is well formed");
  check(large.to_json() == semantic, "Export leaves the semantic report unchanged");

  const std::vector<ReportProgressStage> order = {ReportProgressStage::ExportOdt,
    ReportProgressStage::ExportPackage, ReportProgressStage::ExportCommit};
  check(ReportProgress::fraction(ReportProgressStage::ExportOdt, 0, 1, order) == 0 &&
    ReportProgress::fraction(ReportProgressStage::ExportPackage, 0, 1, order) >=
      ReportProgress::fraction(ReportProgressStage::ExportOdt, 1, 1, order) &&
    ReportProgress::fraction(ReportProgressStage::ExportCommit, 1, 1, order) < 1,
    "Export workflow percentages advance monotonically and reserve 100 percent for publication");
  check(ReportProgress::fraction(ReportProgressStage::Contents, 1, 1) == 0.99,
    "Export phases do not change generation progress weighting");
  check(ReportOdtPackage::supports_entry(1, 9, 0, 8) &&
    !ReportOdtPackage::supports_entry(65535, 9, 0, 8) &&
    !ReportOdtPackage::supports_entry(1, 0xffffffffull, 0, 8) &&
    !ReportOdtPackage::supports_entry(1, 9, 0xfffffff0ull, 8),
    "Classic ZIP limits reject ZIP64 sentinel values without large allocations");
  QByteArray sample;
  QBuffer zip(&sample), input;
  zip.open(QIODevice::WriteOnly);
  input.setData("123456789");
  input.open(QIODevice::ReadOnly);
  ReportOdtPackage package(zip);
  check(package.begin(9, 54, error) && package.add_file("mimetype", input, nullptr, error) && package.finish(nullptr, error),
    "Encode known CRC-32 fixture");
  QBuffer crcInput;
  crcInput.setData(sample.mid(14, 4));
  crcInput.open(QIODevice::ReadOnly);
  QDataStream crcStream(&crcInput);
  crcStream.setByteOrder(QDataStream::LittleEndian);
  quint32 crc = 0;
  crcStream >> crc;
  check(crc == 0xcbf43926u, "ZIP CRC-32 matches the standard check vector");
  return failures;
}
