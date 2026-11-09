#include "ReportLogo.hpp"
#include <QFile>
#include <QImageReader>
#include <QBuffer>
#include <QSvgRenderer>
#include <QXmlStreamReader>
#include <QPainter>
#include <algorithm>
#include <utility>

namespace
{
constexpr int maxFileBytes = 5 * 1024 * 1024;
constexpr int maxLogoBytes = 512 * 1024;

bool validSize(const QSize& size)
{
  return size.width() > 0 && size.height() > 0 && size.width() <= 4096 && size.height() <= 4096;
}
}

bool ReportLogo::load(const std::string& filename, ReportImage& output, std::string& error)
{
  QFile file(QString::fromStdString(filename));
  if (!file.open(QIODevice::ReadOnly) || file.size() <= 0 || file.size() > maxFileBytes)
  {
    error = "Cannot read logo: use an SVG, PNG or JPEG file no larger than 5 MiB.";
    return false;
  }
  const QByteArray bytes = file.read(maxFileBytes + 1);
  QImage pixels;
  if (filename.size() >= 4 && QString::fromStdString(filename).endsWith(".svg", Qt::CaseInsensitive))
  {
    QXmlStreamReader xml(bytes);
    while (!xml.atEnd())
    {
      xml.readNext();
      if (xml.isDTD() || (xml.isStartElement() && (xml.name() == QLatin1String("script") || xml.name() == QLatin1String("foreignObject"))))
      {
        error = "Logo SVG must contain only self-contained graphics.";
        return false;
      }
      if (xml.isStartElement())
      {
        for (const auto& attribute : xml.attributes())
        {
          if (attribute.name() == QLatin1String("href") && !attribute.value().startsWith('#'))
          {
            error = "Logo SVG cannot reference external images or fonts.";
            return false;
          }
        }
      }
    }
    QSvgRenderer renderer(bytes);
    if (xml.hasError() || !renderer.isValid() || !validSize(renderer.defaultSize()))
    {
      error = "Invalid SVG logo or unsupported dimensions (maximum 4096 pixels).";
      return false;
    }
    QSize size = renderer.defaultSize();
    size.scale(QSize(1024, 512), Qt::KeepAspectRatio);
    pixels = QImage(size, QImage::Format_ARGB32_Premultiplied);
    pixels.fill(Qt::transparent);
    QPainter painter(&pixels);
    renderer.render(&painter);
  }
  else
  {
    QBuffer input;
    input.setData(bytes);
    input.open(QIODevice::ReadOnly);
    QImageReader reader(&input);
    const auto format = reader.format().toLower();
    if ((format != "png" && format != "jpeg" && format != "jpg") || !validSize(reader.size()))
    {
      error = "Logo must be PNG, JPEG or SVG, with dimensions no larger than 4096 pixels.";
      return false;
    }
    reader.setAutoTransform(true);
    pixels = reader.read();
    if (!pixels.isNull())
    {
      pixels = pixels.scaled(QSize(1024, 512), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
  }
  QByteArray png;
  QBuffer encoded(&png);
  encoded.open(QIODevice::WriteOnly);
  if (pixels.isNull() || !pixels.save(&encoded, "PNG") || png.size() > maxLogoBytes)
  {
    error = "Cannot decode logo or its normalized PNG exceeds 512 KiB.";
    return false;
  }
  ReportImage candidate;
  candidate.id = "company-logo";
  candidate.caption = "Company logo";
  candidate.media_type = "image/png";
  candidate.base64_data = png.toBase64().toStdString();
  output = std::move(candidate);
  error.clear();
  return true;
}

bool ReportLogo::validate(const ReportImage& image, std::string& error)
{
  if (image.media_type != "image/png" || image.base64_data.size() > ((maxLogoBytes + 2) / 3) * 4)
  {
    error = "Embedded company logo must be a PNG no larger than 512 KiB.";
    return false;
  }
  const auto decoded = QByteArray::fromBase64Encoding(QByteArray::fromStdString(image.base64_data), QByteArray::AbortOnBase64DecodingErrors);
  QBuffer input;
  input.setData(decoded.decoded);
  input.open(QIODevice::ReadOnly);
  QImageReader reader(&input, "PNG");
  const auto size = reader.size();
  if (!decoded || !validSize(size) || size.width() > 1024 || size.height() > 512 || reader.read().isNull())
  {
    error = "Embedded company logo has invalid image data or dimensions.";
    return false;
  }
  error.clear();
  return true;
}
