#include "ReportTestImages.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include "Framework/Reporting/ReportImageRequest.hpp"
#include <QBuffer>
#include <QImage>
#include <algorithm>
#include <utility>

ReportTestImages::ReportTestImages() = default;
ReportTestImages::~ReportTestImages() = default;

bool ReportTestImages::render(const ReportImageRequest&, ReportImage& output, std::string& error)
{
  if (fail)
  {
    error = "Image fixture failure.";
    return false;
  }
  QImage pixels(192, 108, QImage::Format_RGB32);
  pixels.fill(Qt::blue);
  QByteArray png;
  QBuffer buffer(&png);
  if (!buffer.open(QIODevice::WriteOnly) || !pixels.save(&buffer, "PNG"))
  {
    error = "Cannot encode image fixture.";
    return false;
  }
  ReportImage image;
  image.id = "test.images.model";
  image.caption = "Setup capture fixture";
  image.media_type = "image/png";
  image.base64_data = png.toBase64().toStdString();
  output = std::move(image);
  error.clear();
  return true;
}

bool ReportTestImages::collect(const ReportImageRequest& request, ReportDocument& output, std::string& error)
{
  ReportImage image;
  if (!render(request, image, error))
  {
    return false;
  }
  ReportDocument candidate = output;
  candidate.sections.erase(std::remove_if(candidate.sections.begin(), candidate.sections.end(), [](const ReportSection& section)
  {
    return section.id == "test.images";
  }), candidate.sections.end());
  ReportSection section;
  section.id = "test.images";
  section.title = "Setup images";
  section.images.push_back(std::move(image));
  candidate.sections.push_back(std::move(section));
  output = std::move(candidate);
  error.clear();
  return true;
}
