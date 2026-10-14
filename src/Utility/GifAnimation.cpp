#include "GifAnimation.hpp"
#include "CubitMessage.hpp"

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
                         const QString& output_path, int fps)
{
  if (frame_paths.isEmpty() || fps < 1 || fps > 100)
  {
    PRINT_ERROR("Invalid GIF frames or frame rate.\n");
    return false;
  }

  // GIF frame delays have a resolution of one hundredth of a second.
  const uint32_t delay = static_cast<uint32_t>(
      std::max(1L, std::lround(100.0 / fps)));

  GifWriterGuard guard;
  QSize frame_size;

  for (const QString& frame_path : frame_paths)
  {
    const QImage loaded(frame_path);
    if (loaded.isNull())
    {
      PRINT_ERROR("Unable to load GIF frame %s.\n",
                  frame_path.toUtf8().constData());
      return false;
    }

    const QImage rgba = loaded.convertToFormat(QImage::Format_RGBA8888);
    if (rgba.isNull())
    {
      PRINT_ERROR("Unable to convert GIF frame to RGBA.\n");
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
        PRINT_ERROR("GIF frame dimensions are too large.\n");
        return false;
      }

      FILE* file = open_gif_file(output_path);
      if (!file)
      {
        PRINT_ERROR("Unable to open GIF output file.\n");
        return false;
      }

      // GifBeginFile takes ownership of the file, including on failure.
      if (!GifBeginFile(&guard.writer, file,
                        static_cast<uint32_t>(rgba.width()),
                        static_cast<uint32_t>(rgba.height()), delay))
      {
        PRINT_ERROR("Unable to initialize GIF encoder.\n");
        return false;
      }
    }
    else if (rgba.size() != frame_size)
    {
      PRINT_ERROR("GIF frame dimensions changed.\n");
      return false;
    }

    if (!GifWriteFrame(&guard.writer, rgba.constBits(),
                       static_cast<uint32_t>(rgba.width()),
                       static_cast<uint32_t>(rgba.height()), delay, 8, false) ||
        std::ferror(guard.writer.f) != 0)
    {
      PRINT_ERROR("Unable to encode or write GIF frame.\n");
      return false;
    }
  }

  if (!GifEnd(&guard.writer))
  {
    PRINT_ERROR("Unable to finish GIF output.\n");
    return false;
  }
  return true;
}
