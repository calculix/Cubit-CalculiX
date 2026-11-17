#include "AnimationExportJob.hpp"
#include "PngSequenceWriter.hpp"
#include "GifAnimation.hpp"
#include "Mp4Animation.hpp"
#include <QCoreApplication>
#include <QThread>
#include <QFileInfo>
#include <QPainter>
#include <chrono>
#include <exception>
#include <utility>
namespace Framework::Media
{
  AnimationExportJob::AnimationExportJob(QObject* parent) : QObject(parent)
  {
    timer.setSingleShot(true);
    connect(&timer,&QTimer::timeout,this,[this]()
    {
      try
      {
        advance();
      }
      catch (const std::exception& exception)
      {
        failure = QString::fromUtf8(exception.what());
        cancel();
        if (!active())
        {
          return;
        }
        if (worker.valid())
        {
          timer.start(25);
        }
        else
        {
          try
          {
            encode();
          }
          catch (...)
          {
            finish("Cannot start the animation cleanup worker.");
          }
        }
      }
      catch (...)
      {
        failure = "Unexpected animation export failure.";
        cancel();
        if (!active())
        {
          return;
        }
        if (worker.valid())
        {
          timer.start(25);
        }
        else
        {
          try
          {
            encode();
          }
          catch (...)
          {
            finish("Cannot start the animation cleanup worker.");
          }
        }
      }
    });
  }
  AnimationExportJob::~AnimationExportJob()
  {
    timer.stop();
    cancel();
    if (worker.valid())
    {
      worker.wait();
    }
  }
  bool AnimationExportJob::active() const
  {
    return phase != Phase::Idle;
  }
  bool AnimationExportJob::start(const AnimationExportDefinition& requested,Producer frames,Finished done,QString& error)
  {
    error.clear();
    if (active() || !QCoreApplication::instance() || QThread::currentThread() != thread() || !frames ||
        !valid_animation_export(requested,error))
    {
      if (error.isEmpty())
      {
        error = "Animation export requires an idle job on its owner thread and a frame producer.";
      }
      return false;
    }
    auto normalized = requested;
    normalized.path = QFileInfo(requested.path).absoluteFilePath();
    auto staged = std::make_shared<AnimationOutput>();
    if (!staged->prepare(normalized,error))
    {
      return false;
    }
    definition = normalized;
    producer = std::move(frames);
    finished = std::move(done);
    output = std::move(staged);
    cancelled = std::make_shared<std::atomic_bool>(false);
    completed = std::make_shared<std::atomic_int>(0);
    published = std::make_shared<std::atomic_bool>(false);
    next = 0;
    failure.clear();
    lastStatus.clear();
    if (definition.format == AnimationFormat::Mp4)
    {
      const auto stop = cancelled;
      worker = std::async(std::launch::async,[stop]()
      {
        QString error;
        Mp4Animation::available(*stop,error);
        return error;
      });
      phase = Phase::Checking;
    }
    else
    {
      phase = Phase::Capturing;
    }
    timer.start(0);
    return true;
  }
  void AnimationExportJob::cancel()
  {
    if (cancelled)
    {
      cancelled->store(true);
    }
  }
  QString AnimationExportJob::status() const
  {
    if (!active())
    {
      return lastStatus.isEmpty() ? QString("No animation export is active.") : lastStatus;
    }
    if (cancelled->load())
    {
      return "Cancelling animation export and cleaning temporary files...";
    }
    if (phase == Phase::Checking)
    {
      return "Checking MP4 encoder...";
    }
    if (phase == Phase::Encoding)
    {
      return QString("Encoding %1: %2/%3 frames").arg(animation_format_name(definition.format))
        .arg(completed->load()).arg(static_cast<qulonglong>(definition.frames));
    }
    return QString("Rendering/writing frame %1/%2").arg(static_cast<qulonglong>(next+1))
      .arg(static_cast<qulonglong>(definition.frames));
  }
  void AnimationExportJob::advance()
  {
    if (!active())
    {
      return;
    }
    if (phase == Phase::Checking || phase == Phase::Writing || phase == Phase::Encoding)
    {
      if (worker.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
      {
        timer.start(25);
        return;
      }
      const auto result = worker.get();
      if (!result.isEmpty())
      {
        failure = result;
      }
      if (phase == Phase::Encoding)
      {
        finish(failure);
        return;
      }
      if (phase == Phase::Writing)
      {
        ++next;
      }
      phase = Phase::Capturing;
    }
    if (cancelled->load() || !failure.isEmpty() || next >= definition.frames)
    {
      encode();
      return;
    }
    QImage image;
    QString error;
    if (!producer(next,image,error) || image.isNull() || image.width() != definition.width || image.height() != definition.height)
    {
      failure = error.isEmpty() ? QString("The frame producer returned invalid image dimensions.") : error;
      encode();
      return;
    }
    if (cancelled->load())
    {
      encode();
      return;
    }
    const auto directory = output->frames_directory();
    const auto index = next;
    const auto stop = cancelled;
    const auto background = definition.background;
    const bool opaque = definition.format != AnimationFormat::Png;
    worker = std::async(std::launch::async,[image = std::move(image),directory,index,stop,background,opaque]() mutable
    {
      QString error;
      if (stop->load())
      {
        return error;
      }
      if (opaque && image.hasAlphaChannel())
      {
        QImage flattened(image.size(),QImage::Format_RGB32);
        flattened.fill(background);
        QPainter painter(&flattened);
        painter.drawImage(0,0,image);
        painter.end();
        image = std::move(flattened);
      }
      PngSequenceWriter::write_frame(directory,index,image,error);
      return error;
    });
    phase = Phase::Writing;
    timer.start(0);
  }
  void AnimationExportJob::encode()
  {
    phase = Phase::Encoding;
    producer = {};
    const auto staged = output;
    const auto settings = definition;
    const auto stop = cancelled;
    const auto progress = completed;
    const auto committed = published;
    const bool valid = failure.isEmpty();
    worker = std::async(std::launch::async,[staged,settings,stop,progress,committed,valid]()
    {
      QString error;
      try
      {
        bool accepted = valid && !stop->load();
        if (accepted && settings.format == AnimationFormat::Gif)
        {
          QStringList paths;
          for (std::size_t index = 0; index < settings.frames; ++index)
          {
            paths.push_back(PngSequenceWriter::frame_path(staged->frames_directory(),index));
          }
          accepted = GifAnimation::write(paths,staged->encoded_file(settings.format),settings.fps,error,*stop,*progress,
            settings.loop,settings.background);
        }
        else if (accepted && settings.format == AnimationFormat::Mp4)
        {
          accepted = Mp4Animation::write(staged->frames_directory(),staged->encoded_file(settings.format),settings.fps,*stop,*progress,error);
        }
        else if (accepted)
        {
          accepted = PngSequenceWriter::write_manifest(staged->frames_directory(),settings,error);
          progress->store(static_cast<int>(settings.frames));
        }
        if (accepted && !stop->load())
        {
          if (settings.format != AnimationFormat::Png && QFileInfo(staged->encoded_file(settings.format)).size() == 0)
          {
            error = "The animation encoder produced an empty file.";
          }
          else
          {
            if (staged->publish(settings,error))
            {
              committed->store(true);
            }
          }
        }
      }
      catch (const std::exception& exception)
      {
        error = QString::fromUtf8(exception.what());
      }
      catch (...)
      {
        error = "Unexpected animation encoder failure.";
      }
      if (!staged->cleanup() && error.isEmpty())
      {
        error = "Animation processing finished, but temporary-file cleanup failed.";
      }
      return error;
    });
    timer.start(25);
  }
  void AnimationExportJob::finish(const QString& error)
  {
    timer.stop();
    const bool succeeded = published && published->load();
    const bool aborted = !succeeded && cancelled->load();
    lastStatus = "Animation export cancelled.";
    if (succeeded)
    {
      lastStatus = "Animation saved to "+definition.path;
      if (!error.isEmpty())
      {
        lastStatus += " ("+error+")";
      }
    }
    else if (!error.isEmpty())
    {
      lastStatus = "Animation export failed: "+error;
    }
    phase = Phase::Idle;
    producer = {};
    output.reset();
    auto callback = std::move(finished);
    if (callback)
    {
      // Notifications belong to the owner thread and may not escape a Qt timer.
      try
      {
        callback(succeeded,aborted,error);
      }
      catch (...)
      {
        lastStatus += " (completion notification failed)";
      }
    }
  }
}
