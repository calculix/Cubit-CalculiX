#include "GifAnimation.hpp"

// gif.h contains definitions: include it in this translation unit only.
#include "gif.h"

#include <QFile>
#include <QImage>

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

class GifWriterGuard
{
public:
  GifWriter writer{};

  ~GifWriterGuard()
  {
    if (writer.f)
    {
      GifEnd(&writer);
    }
  }

  GifWriterGuard() = default;
  GifWriterGuard(const GifWriterGuard&) = delete;
  GifWriterGuard& operator=(const GifWriterGuard&) = delete;
};
}

bool GifAnimation::write(const QStringList& frame_paths,
                         const QString& output_path, int fps, QString& error,
                         const std::atomic_bool& cancelled, std::atomic_int& completed_frames)
{
  error.clear();
  completed_frames.store(0);
  if (frame_paths.isEmpty() || fps < 1 || fps > 100)
  {
    error = "Invalid GIF frames or frame rate.";
    return false;
  }

  // GIF frame delays have a resolution of one hundredth of a second.
  const uint32_t delay = static_cast<uint32_t>(
      std::max(1L, std::lround(100.0 / fps)));

  GifWriterGuard guard;
  QSize frame_size;

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

    const QImage rgba = loaded.convertToFormat(QImage::Format_RGBA8888);
    if (rgba.isNull())
    {
      error = "Unable to convert GIF frame to RGBA.";
      return false;
    }

    if (!guard.writer.f)
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
      if (!GifBeginFile(&guard.writer, file,
                        static_cast<uint32_t>(rgba.width()),
                        static_cast<uint32_t>(rgba.height()), delay))
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

    if (!GifWriteFrame(&guard.writer, rgba.constBits(),
                       static_cast<uint32_t>(rgba.width()),
                       static_cast<uint32_t>(rgba.height()), delay, 8, false) ||
        std::ferror(guard.writer.f) != 0)
    {
      error = "Unable to encode or write GIF frame.";
      return false;
    }
    completed_frames.fetch_add(1);
  }

  if (!GifEnd(&guard.writer))
  {
    error = "Unable to finish GIF output.";
    return false;
  }
  return true;
}
