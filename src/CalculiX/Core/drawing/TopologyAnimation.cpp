#include "TopologyAnimation.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "CubitMessage.hpp"
#include "GifAnimation.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QThread>
#include <chrono>
#include <cmath>
#include <exception>
#include <set>
#include <sstream>
#include <utility>

namespace
{
// File operations only: called by the encoding worker, never calls Cubit.
bool publish_gif(const QString& source_path, const QString& destination_path,
                 bool overwrite, const std::atomic_bool& cancelled, QString& error)
{
  if (cancelled.load())
  {
    return false;
  }
  if (!overwrite)
  {
    if (!QFile::copy(source_path, destination_path))
    {
      error = "Unable to save GIF; the destination may already exist.";
      return false;
    }
    return true;
  }

  QFile source(source_path);
  QSaveFile destination(destination_path);
  destination.setDirectWriteFallback(false);
  if (!source.open(QIODevice::ReadOnly) || !destination.open(QIODevice::WriteOnly))
  {
    error = "Unable to open animation files for replacement.";
    return false;
  }
  while (!source.atEnd())
  {
    if (cancelled.load())
    {
      destination.cancelWriting();
      return false;
    }
    const QByteArray chunk = source.read(1024 * 1024);
    if (source.error() != QFileDevice::NoError || chunk.isEmpty() || destination.write(chunk) != chunk.size())
    {
      error = "Unable to copy encoded GIF to its destination.";
      return false;
    }
  }
  if (cancelled.load())
  {
    destination.cancelWriting();
    return false;
  }
  if (!destination.commit())
  {
    error = "Unable to replace animation output.";
    return false;
  }
  return true;
}
}

TopologyAnimation::TopologyAnimation(QObject* parent) : QObject(parent)
{
  timer.setSingleShot(true);
  QObject::connect(&timer, &QTimer::timeout, this, [this]() { advance(); });
}

TopologyAnimation::~TopologyAnimation()
{
  timer.stop();
  cancel();
  if (encoding.valid())
  {
    // Join before the plugin code can be unloaded. Normal cancellation never waits.
    encoding.wait();
  }
}

bool TopologyAnimation::active() const
{
  return phase != Phase::Idle;
}

void TopologyAnimation::cancel()
{
  if (encoding_state)
  {
    encoding_state->cancelled.store(true);
  }
}

bool TopologyAnimation::start(int id, double density_threshold, int rate, const QString& filename, bool replace)
{
  auto* application = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
  if (!application || QThread::currentThread() != application->thread() || thread() != application->thread())
  {
    PRINT_ERROR("Topology animation requires the GUI main thread and an active graphics window.\n");
    return false;
  }
  if (active())
  {
    PRINT_ERROR("A topology animation export is already active.\n");
    return false;
  }
  topology_id = id;
  threshold = density_threshold;
  fps = rate;
  overwrite = replace;
  output = filename;
  iterations.clear();
  frame_paths.clear();
  next_frame = 0;
  reported_frames = -1;
  capture_error.clear();
  busy_wait.invalidate();
  frame_size = QSize();
  encoding_state = std::make_shared<EncodingState>();
  if (!prepare())
  {
    temporary.reset();
    encoding_state.reset();
    return false;
  }
  phase = Phase::Capturing;
  PRINT_INFO("Topology %d animation: export started (%zu frames).\n", topology_id, iterations.size());
  timer.start(0);
  return true;
}

bool TopologyAnimation::prepare()
{
  CalculiXTopologyInterface topology;
  if (!std::isfinite(threshold) || threshold < 0.0 || threshold > 1.0 || fps < 1 || fps > 100)
  {
    PRINT_ERROR("Animation requires threshold 0..1 and fps 1..100.\n");
    return false;
  }
  const std::string state = topology.get_topology_run_state(topology_id);
  if (state.empty() || state == "running")
  {
    PRINT_ERROR("Topology animation requires an existing idle case.\n");
    return false;
  }
  std::set<int> completed;
  for (const auto& row : topology.get_topology_iteration_data(topology_id))
  {
    if (row.size() < 6 || row[2] != "2" || row[3].empty() || row[4].empty())
    {
      continue;
    }
    int id = -1;
    std::istringstream stream(row[0]);
    if (!(stream >> id) || id < 1 || !(stream >> std::ws).eof())
    {
      PRINT_ERROR("Invalid topology animation iteration ID.\n");
      return false;
    }
    completed.insert(id);
  }
  iterations.assign(completed.begin(), completed.end());
  if (iterations.empty())
  {
    PRINT_ERROR("Topology %d has no completed iterations to animate.\n", topology_id);
    return false;
  }
  if (output.isEmpty())
  {
    output = QString("topo_%1_animation.gif").arg(topology_id);
  }
  output = QDir::current().absoluteFilePath(output);
  const QFileInfo destination(output);
  if (destination.suffix().compare("gif", Qt::CaseInsensitive) != 0 || !destination.dir().exists() || destination.isDir())
  {
    PRINT_ERROR("Animation requires a .gif filename in an existing directory.\n");
    return false;
  }
  if (destination.exists() && !overwrite)
  {
    PRINT_ERROR("Animation output exists; use overwrite.\n");
    return false;
  }
  temporary = std::make_unique<QTemporaryDir>();
  if (!temporary->isValid() || temporary->path().contains('\'') || temporary->path().contains('\n') || temporary->path().contains('\r'))
  {
    PRINT_ERROR("Unable to create a usable animation temporary directory.\n");
    return false;
  }
  return true;
}

bool TopologyAnimation::capture_next_frame()
{
  CalculiXApplicationInterface application;
  CalculiXTopologyInterface topology;
  const std::string state = topology.get_topology_run_state(topology_id);
  if (state.empty() || state == "running")
  {
    capture_error = "The topology case changed during animation capture.";
    return false;
  }
  const int id = iterations[next_frame];
  const QString preview = QString("ccx result project topology %1 iteration %2 threshold %3 preview")
    .arg(topology_id).arg(id).arg(QString::number(threshold, 'f', 6));
  if (!application.silent_cmd(preview.toStdString()))
  {
    capture_error = QString("Unable to preview iteration %1.").arg(id);
    return false;
  }
  const QString path = QDir(temporary->path()).filePath(QString("frame_%1.png").arg(static_cast<qulonglong>(next_frame), 6, 10, QChar('0')));
  const std::string command = "hardcopy '" + QDir::fromNativeSeparators(path).toStdString() + "' png";
  if (!application.silent_cmd(command))
  {
    capture_error = "Unable to capture topology animation frame.";
    return false;
  }
  const QImage image(path);
  if (image.isNull() || (next_frame != 0 && image.size() != frame_size))
  {
    capture_error = "Animation frame is missing or its dimensions changed.";
    return false;
  }
  frame_size = image.size();
  frame_paths.push_back(path);
  ++next_frame;
  PRINT_INFO("Topology %d animation: captured iteration %d (%zu/%zu).\n", topology_id, id, next_frame, iterations.size());
  return true;
}

void TopologyAnimation::start_encoding()
{
  phase = Phase::Encoding;
  if (capture_error.isEmpty() && !encoding_state->cancelled.load())
  {
    PRINT_INFO("Topology %d animation: encoding %zu frames.\n", topology_id, iterations.size());
  }
  try
  {
    encoding = std::async(std::launch::async,
      [paths = frame_paths, destination = output, rate = fps, replace = overwrite,
       state = encoding_state, initial_error = capture_error, directory = std::move(temporary)]() mutable
    {
      EncodingResult result;
      result.error = initial_error;
      try
      {
        if (result.error.isEmpty() && !state->cancelled.load())
        {
          const QString encoded = QDir(directory->path()).filePath("animation.gif");
          if (GifAnimation::write(paths, encoded, rate, result.error, state->cancelled, state->completed_frames))
          {
            if (QFileInfo(encoded).size() == 0)
            {
              result.error = "GIF encoder produced an empty file.";
            }
            else
            {
              result.succeeded = publish_gif(encoded, destination, replace, state->cancelled, result.error);
            }
          }
        }
      }
      catch (const std::exception& exception)
      {
        result.error = QString("Animation worker failed: %1").arg(QString::fromUtf8(exception.what()));
      }
      catch (...)
      {
        result.error = "Animation worker failed with an unexpected error.";
      }
      result.cancelled = !result.succeeded && result.error.isEmpty() && state->cancelled.load();
      // Delete PNGs on the worker before the main thread announces completion.
      if (!directory->remove())
      {
        result.cleanup_warning = QString("Unable to remove animation temporary directory: %1").arg(directory->path());
        directory->setAutoRemove(false);
      }
      directory.reset();
      return result;
    });
    timer.start(50);
  }
  catch (const std::exception& exception)
  {
    EncodingResult result;
    result.error = QString("Unable to start GIF encoding: %1").arg(QString::fromUtf8(exception.what()));
    finish(result);
  }
}

void TopologyAnimation::advance()
{
  if (phase == Phase::Capturing)
  {
    if (encoding_state->cancelled.load())
    {
      start_encoding(); // Cleanup captured files on the worker even on cancellation.
      return;
    }
    try
    {
      if (!capture_next_frame())
      {
        start_encoding();
        return;
      }
    }
    catch (const std::exception& exception)
    {
      const QString message = QString::fromUtf8(exception.what()).trimmed();
      if (message == QStringLiteral("Cubit is busy"))
      {
        if (!busy_wait.isValid())
        {
          busy_wait.start();
        }
        if (busy_wait.elapsed() < 30000)
        {
          // A timer callback can run before the outer Cubit command has returned.
          // Retry this frame without advancing its ID or blocking the event loop.
          timer.start(100);
          return;
        }
        capture_error = "Cubit remained busy for 30 seconds during animation capture.";
      }
      else
      {
        capture_error = QString("Animation capture failed: %1").arg(message);
      }
      start_encoding();
      return;
    }
    catch (...)
    {
      capture_error = "Animation capture failed with an unexpected error.";
      start_encoding();
      return;
    }
    busy_wait.invalidate();
    if (next_frame == iterations.size())
    {
      start_encoding();
    }
    else
    {
      timer.start(10);
    }
    return;
  }
  if (phase == Phase::Encoding)
  {
    const int count = encoding_state->completed_frames.load();
    if (count != reported_frames && count > 0)
    {
      PRINT_INFO("Topology %d animation: encoded %d/%zu frames.\n", topology_id, count, iterations.size());
      reported_frames = count;
    }
    if (encoding.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
    {
      timer.start(50);
      return;
    }
    try
    {
      const EncodingResult result = encoding.get();
      finish(result);
    }
    catch (const std::exception& exception)
    {
      EncodingResult result;
      result.error = QString("Animation completion failed: %1").arg(QString::fromUtf8(exception.what()));
      finish(result);
    }
  }
}

void TopologyAnimation::finish(const EncodingResult& result)
{
  timer.stop();
  phase = Phase::Idle;
  temporary.reset();
  frame_paths.clear();
  iterations.clear();
  encoding_state.reset();
  if (!result.cleanup_warning.isEmpty())
  {
    PRINT_WARNING("%s\n", result.cleanup_warning.toUtf8().constData());
  }
  if (result.succeeded)
  {
    PRINT_INFO("Topology %d animation saved to %s.\n", topology_id, output.toUtf8().constData());
  }
  else if (result.cancelled)
  {
    PRINT_INFO("Topology %d animation cancelled.\n", topology_id);
  }
  else
  {
    PRINT_ERROR("Topology %d animation failed: %s\n", topology_id, result.error.toUtf8().constData());
  }
}
