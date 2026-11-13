#ifndef POSTPROCESSOR_CAPTUREDEFINITION_HPP
#define POSTPROCESSOR_CAPTUREDEFINITION_HPP
#include "../session/PostprocessorSession.hpp"
#include "../options/PostprocessorUserOptions.hpp"
#include <QJsonObject>
#include <QByteArray>
#include <cstdint>
namespace Postprocessor
{
  namespace CaptureDefaults
  {
    inline constexpr bool marker_visible = true;
    inline constexpr int width = 1920;
    inline constexpr int height = 1080;
    inline constexpr int minimum_side = 64;
    inline constexpr int maximum_side = 8192;
    inline constexpr std::int64_t maximum_pixels = 32000000;
    inline constexpr int maximum_png_bytes = 32*1024*1024;
    inline constexpr int maximum_manifest_bytes = 4*1024*1024;
  }
  enum class CaptureTarget
  {
    View, Plot
  };
  enum class CaptureBackground
  {
    Current, White, Transparent
  };
  struct CaptureDefinition
  {
    CaptureTarget target = CaptureTarget::View;
    std::string name;
    int width = CaptureDefaults::width;
    int height = CaptureDefaults::height;
    CaptureBackground background = CaptureBackground::Current;
    std::optional<bool> labels, legend, triad, marker;
  };
  bool valid_capture(const CaptureDefinition& request,std::string& error);
  struct CaptureSnapshot
  {
    CaptureDefinition request;
    PostprocessorSession session;
    UserOptions options;
    std::map<std::string,std::shared_ptr<const SimResults::VtkResultsData>> sources;
    std::string target_id;
    // Logical widget dimensions, used only for presentation scaling.
    std::array<int,2> reference_size = {CaptureDefaults::width,CaptureDefaults::height};
    QJsonObject provenance;
  };
  bool write_capture_png(const std::string& path,const QByteArray& png,std::string& error);
}
#endif
