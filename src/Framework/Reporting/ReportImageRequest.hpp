#pragma once
#include <QJsonObject>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "ReportSource.hpp"
#include "ReportConfiguration.hpp"

enum class ReportVisualKind
{
  View, Plot
};
struct ReportVisualReference
{
  ReportVisualKind kind = ReportVisualKind::View;
  std::string reference;
};

class ReportImageRequest
{
public:
  ReportImageRequest();
  std::string provider;
  // Opaque provider-defined purpose and scope, e.g. setup / step:17.
  std::string purpose;
  std::string context;
  std::optional<ReportVisualReference> visual;
  ReportSource source;
  ReportConfiguration settings;
  int width = 0;
  int height = 0;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportImageRequest& output, std::string& error);
};
