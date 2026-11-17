#include "Framework/Media/AnimationExportJob.hpp"
#include "Framework/Media/GifAnimation.hpp"
#include "Framework/Media/Mp4Animation.hpp"
#include "Framework/Media/PngSequenceWriter.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QElapsedTimer>
#include <QThread>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <iostream>
int main(int argc,char** argv)
{
  QCoreApplication app(argc,argv);
  using namespace Framework::Media;
  int failures = 0;
  const auto check = [&](bool value,const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  QTemporaryDir directory;
  AnimationExportDefinition definition;
  definition.format = AnimationFormat::Png;
  definition.path = directory.filePath("sequence");
  definition.width = definition.height = 64;
  definition.fps = 60;
  definition.frames = 3;
  QString error;
  check(valid_animation_export(definition,error),"Valid PNG sequence was rejected");
  AnimationExportJob job;
  bool done = false, succeeded = false, aborted = false;
  const auto finished = [&](bool success,bool cancelled,const QString& failure)
  {
    done = true;
    succeeded = success;
    aborted = cancelled;
    if (!failure.isEmpty())
    {
      std::cerr << failure.toStdString() << '\n';
    }
  };
  const auto wait = [&]()
  {
    QElapsedTimer clock;
    clock.start();
    while (job.active() && clock.elapsed() < 15000)
    {
      QCoreApplication::processEvents();
      QThread::msleep(1);
    }
    check(!job.active(),"Media job did not finish within the test limit");
  };
  check(job.start(definition,[](std::size_t index,QImage& image,QString&)
  {
    image = QImage(64,64,QImage::Format_ARGB32);
    image.fill(QColor(static_cast<int>(index),20,40,128));
    return true;
  },finished,error),"Cannot start reusable PNG sequence job");
  wait();
  check(done && succeeded && !aborted,"PNG sequence job failed");
  const auto firstPath = PngSequenceWriter::frame_path(definition.path,0);
  const QImage first(firstPath);
  check(!first.isNull() && first.pixelColor(0,0).alpha() == 128,"PNG sequence lost alpha");
  QFile manifest(QDir(definition.path).filePath("animation.json"));
  check(manifest.open(QIODevice::ReadOnly),"PNG sequence manifest is missing");
  const auto metadata = QJsonDocument::fromJson(manifest.readAll()).object();
  check(metadata.value("fps").toInt() == 60 && metadata.value("frames").toInt() == 3,
    "PNG sequence manifest lost timing");
  check(!valid_animation_export(definition,error),"Existing sequence directory was accepted for replacement");
  QStringList frames;
  for (std::size_t index = 0; index < 3; ++index)
  {
    frames.push_back(PngSequenceWriter::frame_path(definition.path,index));
  }
  std::atomic_bool cancel(false);
  std::atomic_int completed(0);
  const auto gif = directory.filePath("sample.gif");
  check(GifAnimation::write(frames,gif,60,error,cancel,completed,true) && completed.load() == 3,
    "Shared GIF helper failed to encode ordered PNGs");
  QFile encoded(gif);
  check(encoded.open(QIODevice::ReadOnly),"GIF file is missing");
  const auto bytes = encoded.readAll();
  check(bytes.startsWith("GIF89a") && bytes.contains("NETSCAPE2.0"),"GIF header/loop extension is invalid");
  const auto gifDelays = [](const QByteArray& stream)
  {
    const auto byte = [&](qsizetype index)
    {
      return static_cast<unsigned char>(stream.at(index));
    };
    if (stream.size() < 13)
    {
      return -1;
    }
    qsizetype position = 13;
    if ((byte(10)&128) != 0)
    {
      position += 3*(1 << ((byte(10)&7)+1));
    }
    int delay = 0;
    while (position < stream.size())
    {
      const int tag = byte(position++);
      if (tag == 0x3b)
      {
        return delay;
      }
      if (tag == 0x21 && position < stream.size())
      {
        const int label = byte(position++);
        if (label == 0xf9 && position+4 < stream.size() && byte(position) == 4)
        {
          delay += byte(position+2)+256*byte(position+3);
        }
      }
      else if (tag == 0x2c && position+9 < stream.size())
      {
        const int flags = byte(position+8);
        position += 9;
        if ((flags&128) != 0)
        {
          position += 3*(1 << ((flags&7)+1));
        }
        ++position; // LZW minimum code size.
      }
      else
      {
        return -1;
      }
      while (position < stream.size())
      {
        const int length = byte(position++);
        if (length == 0)
        {
          break;
        }
        position += length;
      }
    }
    return -1;
  };
  check(gifDelays(bytes) == 5,"GIF centisecond timing accumulated a constant-delay speed error");
  const auto once = directory.filePath("once.gif");
  check(GifAnimation::write(frames,once,30,error,cancel,completed,false),"Non-looping GIF failed");
  QFile single(once);
  check(single.open(QIODevice::ReadOnly) && !single.readAll().contains("NETSCAPE2.0"),"Non-looping GIF retained infinite loop metadata");
  cancel.store(true);
  check(!GifAnimation::write(frames,directory.filePath("cancel.gif"),30,error,cancel,completed),
    "GIF encoder ignored cancellation");
  cancel.store(false);
  const auto previousEncoder = qgetenv("CUBITSIM_FFMPEG");
  const bool hadEncoder = qEnvironmentVariableIsSet("CUBITSIM_FFMPEG");
  qputenv("CUBITSIM_FFMPEG",directory.filePath("missing-encoder").toUtf8());
  check(!Mp4Animation::available(cancel,error) && !error.isEmpty(),"Missing MP4 encoder was not reported");
  if (hadEncoder)
  {
    qputenv("CUBITSIM_FFMPEG",previousEncoder);
  }
  else
  {
    qunsetenv("CUBITSIM_FFMPEG");
  }
  definition.format = AnimationFormat::Mp4;
  definition.path = directory.filePath("odd.mp4");
  definition.width = 65;
  check(!valid_animation_export(definition,error),"MP4 accepted odd H.264 dimensions");
  definition.format = AnimationFormat::Png;
  definition.width = 64;
  definition.path = directory.filePath("cancelled-sequence");
  done = false;
  check(job.start(definition,[&](std::size_t,QImage& image,QString&)
  {
    image = QImage(64,64,QImage::Format_RGB32);
    image.fill(Qt::white);
    job.cancel();
    return true;
  },finished,error),"Cannot start cancellation fixture");
  wait();
  check(done && aborted && !succeeded && !QFileInfo::exists(definition.path),
    "Cancelled sequence was published or incorrectly reported successful");
  definition.path = directory.filePath("raced-sequence");
  done = false;
  check(job.start(definition,[&](std::size_t index,QImage& image,QString&)
  {
    if (index == 0)
    {
      QDir().mkdir(definition.path); // Simulate an output created while export runs.
    }
    image = QImage(64,64,QImage::Format_RGB32);
    image.fill(Qt::white);
    return true;
  },finished,error),"Cannot start output-publication race fixture");
  wait();
  check(done && !succeeded && !aborted && QDir(definition.path).entryList(QDir::Files).isEmpty(),
    "Publication replaced a destination created after export started");
  check(QDir(directory.path()).entryList({".cubitsim-animation-*"},QDir::Dirs|QDir::Hidden|QDir::NoDotAndDotDot).isEmpty(),
    "Media job left staging directories behind");
  return failures;
}
