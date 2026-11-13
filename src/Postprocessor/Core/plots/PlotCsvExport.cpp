#include "PlotData.hpp"
#include <utility>
#include <QDir>
#include <QLocale>
#include <QFile>
#include <cmath>
#include <algorithm>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTextStream>
#include <QStringConverter>
namespace
{
  QString quoted(const std::string& value)
  {
    auto text = QString::fromUtf8(value.c_str());
    text.replace("\"","\"\"");
    return QStringLiteral("\"")+text+"\"";
  }
}
namespace Postprocessor
{
  bool export_plot_csv(const PlotDefinition& definition, const PlotData& data,
    const std::string& path, std::string& error)
  {
    error.clear();
    if (std::none_of(data.curves.begin(),data.curves.end(),[](const auto& curve)
      {
        return !curve.samples.empty();
      }))
    {
      error = "The plot has no samples to export.";
      return false;
    }
    const QFileInfo target(QString::fromUtf8(path.c_str()));
    if (path.empty() || target.exists() || target.isSymLink() || !target.dir().exists())
    {
      error = "Choose a new CSV filename in an existing directory; existing files are never overwritten.";
      return false;
    }
    QTemporaryFile temporary(target.dir().filePath(".cubitsim-plot-XXXXXX.tmp"));
    if (!temporary.open())
    {
      error = temporary.errorString().toStdString();
      return false;
    }
    QTextStream stream(&temporary);
    stream.setEncoding(QStringConverter::Utf8);
    stream.setLocale(QLocale::c());
    stream.setRealNumberPrecision(17);
    stream << "Plot,Curve,Source,EntityId,IntegrationPoint,Frame,Step,Increment,Time,X,Y,Valid,Status\r\n";
    for (std::size_t index = 0; index < definition.curves.size() && index < data.curves.size(); ++index)
    {
      const auto& curve = definition.curves[index];
      const auto& values = data.curves[index];
      const auto* entity = std::get_if<PlotEntityReference>(&curve.input);
      for (const auto& sample : values.samples)
      {
        stream << quoted(definition.name) << ',' << quoted(curve.name) << ',' << quoted(values.source_name.empty() ? plot_curve_source(curve) : values.source_name) << ',';
        if (entity)
        {
          stream << QString::number(static_cast<qlonglong>(entity->entity.entity_id));
        }
        stream << ',';
        if (entity && entity->entity.integration_point)
        {
          stream << QString::number(static_cast<qlonglong>(*entity->entity.integration_point));
        }
        stream << ',';
        if (sample.frame)
        {
          stream << QString::number(static_cast<qulonglong>(*sample.frame+1));
        }
        stream << ',' << sample.step << ',' << sample.increment << ',';
        if (std::isfinite(sample.time))
        {
          stream << sample.time;
        }
        stream << ',';
        if (std::isfinite(sample.x))
        {
          stream << sample.x;
        }
        stream << ',';
        if (sample.valid)
        {
          stream << sample.y;
        }
        stream << ',' << (sample.valid ? 1 : 0) << ',' << quoted(values.status) << "\r\n";
      }
    }
    stream.flush();
    if (stream.status() != QTextStream::Ok || !temporary.flush())
    {
      error = "Cannot write complete plot CSV.";
      return false;
    }
    temporary.close();
    if (!QFile::rename(temporary.fileName(),target.absoluteFilePath()))
    {
      error = "Cannot publish the CSV; destination may already exist.";
      return false;
    }
    temporary.setAutoRemove(false);
    return true;
  }
}
