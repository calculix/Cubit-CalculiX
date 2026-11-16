#include "ReportFigureCaption.hpp"
#include "Framework/Reporting/ReportConfiguration.hpp"
#include "Framework/Reporting/ReportImage.hpp"
#include "Framework/Reporting/ReportJson.hpp"
#include "Report/Core/ReportUnitValidation.hpp"
#include <QStringList>

ReportFigureCaption::ReportFigureCaption() = default;

QString ReportFigureCaption::text(const ReportImage& image, const ReportConfiguration& configuration)
{
  QStringList lines;
  if (!image.caption.empty())
  {
    lines.append(QString::fromStdString(image.caption));
  }
  const auto captured = image.provenance.value("presentation_units");
  if (captured.isObject())
  {
    ReportUnitSettings units;
    std::string error;
    ReportConfiguration captureConfiguration;
    const bool parsed = ReportUnitSettings::from_json(captured.toObject(), units, error);
    captureConfiguration.units = units;
    if (parsed && ReportUnitValidation::configuration(captureConfiguration, error))
    {
      QStringList details;
      details.append(QString::fromStdString(units.system));
      for (const auto& entry : units.display_units)
      {
        details.append(QString::fromStdString(entry.first + ": " + entry.second));
      }
      lines.append("Capture preferences: " + details.join("; ") + ".");
      if (configuration.units && configuration.units->to_json() != units.to_json())
      {
        lines.append("The picture retains its capture units; generated tables use Report units.");
      }
    }
    else
    {
      lines.append("Capture unit preferences are unavailable or invalid.");
    }
  }
  else if (image.id.compare(0, 13, "postprocessor") == 0 || image.provenance.contains("presentation_units"))
  {
    lines.append("Capture unit preferences are unknown.");
  }
  return lines.join('\n');
}
