#include <iterator>
#include "ReportCore.hpp"
#include "Framework/Reporting/ReportProviderRegistryInterface.hpp"
#include "Framework/Reporting/ReportImageProviderInterface.hpp"
#include <QByteArray>
#include <QImageReader>
#include <QBuffer>
#include <QJsonDocument>
#include <algorithm>
#include <memory>

bool ReportCore::add_image(const ReportImageRequest& requested,const std::string& caption,std::string& error)
{
  error.clear();
  const auto current = document();
  if (!current || state.pending_source || caption.size() > 65536)
  {
    error = "Select a generated report before adding a figure; captions are limited to 64 KiB.";
    return false;
  }
  auto* provider = registry.report_images(requested.provider);
  if (!provider)
  {
    error = "The requested report image provider is unavailable.";
    return false;
  }
  auto request = requested;
  request.source = current->source;
  ReportImage image;
  if (!provider->render(request,image,error))
  {
    return false;
  }
  // Verify the encoded payload before publishing a new immutable document.
  const auto bytes = QByteArray::fromBase64(QByteArray::fromStdString(image.base64_data),QByteArray::AbortOnBase64DecodingErrors);
  QBuffer buffer;
  buffer.setData(bytes);
  buffer.open(QIODevice::ReadOnly);
  QImageReader reader(&buffer,"PNG");
  const auto size = reader.size();
  if (image.media_type != "image/png" || bytes.isEmpty() || bytes.size() > 32*1024*1024 ||
      size.width() < 64 || size.height() < 64 || size.width() > 8192 || size.height() > 8192 ||
      static_cast<qint64>(size.width())*size.height() > 32000000 || reader.read().isNull())
  {
    error = "The image provider returned an invalid or oversized PNG.";
    return false;
  }
  auto candidate = std::make_shared<ReportDocument>(*current);
  auto section = std::find_if(candidate->sections.begin(),candidate->sections.end(),[](const auto& value)
  {
    return value.id == "user.figures";
  });
  if (section == candidate->sections.end())
  {
    ReportSection figures;
    figures.id = "user.figures";
    figures.title = "Figures";
    candidate->sections.push_back(std::move(figures));
    section = std::prev(candidate->sections.end());
  }
  if (section->images.size() >= 64)
  {
    error = "A report supports at most 64 user figures.";
    return false;
  }
  std::size_t number = 1;
  while (std::any_of(section->images.begin(),section->images.end(),[&](const auto& value)
    {
      return value.id == "user.figure_"+std::to_string(number);
    }))
  {
    ++number;
  }
  image.id = "user.figure_"+std::to_string(number);
  if (!caption.empty())
  {
    image.caption = caption;
  }
  section->images.push_back(std::move(image));
  if (!candidate->validate(error))
  {
    return false;
  }
  auto staged = state;
  staged.documents[state.active_id] = candidate;
  if (QJsonDocument(staged.to_json()).toJson(QJsonDocument::Compact).size() > 64*1024*1024)
  {
    error = "Report figures would exceed the 64 MiB project report limit.";
    return false;
  }
  if (document() != current)
  {
    error = "The active report changed during capture. Retry the insertion.";
    return false;
  }
  state.documents[state.active_id] = std::move(candidate);
  return true;
}
