#include "GifAnimation.hpp"

// gif.h contains definitions: include it in this translation unit only.
#include "Utility/gif.h"

#include <QFile>
#include <QImage>
#include <QPainter>
#include <memory>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

namespace
{
FILE* open_gif_file(const QString& path)
{
#ifdef _WIN32
  return _wfopen(reinterpret_cast<const wchar_t*>(path.utf16()), L"wb");
#else
  const QByteArray native_path = QFile::encodeName(path);
  return std::fopen(native_path.constData(), "wb");
#endif
}

}

bool GifAnimation::write(const QStringList& frame_paths,
                         const QString& output_path, int fps, QString& error,
                         const std::atomic_bool& cancelled, std::atomic_int& completed_frames,
                         bool loop,const QColor& background)
{
  error.clear();
  completed_frames.store(0);
  if (frame_paths.isEmpty() || fps < 1 || fps > 100 || !background.isValid() || background.alpha() != 255)
  {
    error = "Invalid GIF frames or frame rate.";
    return false;
  }

  GifWriter writer{};
  const auto close = [](GifWriter* value)
  {
    if (value->f)
    {
      GifEnd(value);
    }
  };
  const std::unique_ptr<GifWriter,decltype(close)> guard(&writer,close);
  QSize frame_size;
  std::size_t index = 0;

  for (const QString& frame_path : frame_paths)
  {
    if (cancelled.load())
    {
      return false;
    }
    const QImage loaded(frame_path);
    if (loaded.isNull())
    {
      error = QString("Unable to load GIF frame: %1").arg(frame_path);
      return false;
    }

    QImage opaque(loaded.size(),QImage::Format_ARGB32_Premultiplied);
    if (opaque.isNull())
    {
      error = "Cannot allocate GIF background pixels.";
      return false;
    }
    opaque.fill(background);
    {
      QPainter painter(&opaque);
      painter.drawImage(0,0,loaded);
    }
    const QImage rgba = opaque.convertToFormat(QImage::Format_RGBA8888);
    if (rgba.isNull())
    {
      error = "Unable to convert GIF frame to RGBA.";
      return false;
    }

    // Distribute centisecond rounding instead of rounding every frame equally.
    const uint32_t delay = static_cast<uint32_t>(std::max(1LL,
      std::llround((index+1)*100.0/fps)-std::llround(index*100.0/fps)));
    if (!writer.f)
    {
      frame_size = rgba.size();
      const qint64 pixel_count =
          static_cast<qint64>(rgba.width()) * rgba.height();
      if (rgba.width() > 65535 || rgba.height() > 65535 ||
          pixel_count > std::numeric_limits<int>::max() / 4)
      {
        error = "GIF frame dimensions are too large.";
        return false;
      }

      FILE* file = open_gif_file(output_path);
      if (!file)
      {
        error = "Unable to open GIF output file.";
        return false;
      }

      // GifBeginFile takes ownership of the file, including on failure.
      if (!GifBeginFile(&writer, file,
                        static_cast<uint32_t>(rgba.width()),
                        static_cast<uint32_t>(rgba.height()), loop ? delay : 0))
      {
        error = "Unable to initialize GIF encoder.";
        return false;
      }
    }
    else if (rgba.size() != frame_size)
    {
      error = "GIF frame dimensions changed.";
      return false;
    }

    if (!GifWriteFrame(&writer, rgba.constBits(),
                       static_cast<uint32_t>(rgba.width()),
                       static_cast<uint32_t>(rgba.height()), delay, 8, false) ||
        std::ferror(writer.f) != 0)
    {
      error = "Unable to encode or write GIF frame.";
      return false;
    }
    completed_frames.fetch_add(1);
    ++index;
  }

  if (!GifEnd(&writer))
  {
    error = "Unable to finish GIF output.";
    return false;
  }
  return true;
}
