#ifndef GIFANIMATION_HPP
#define GIFANIMATION_HPP

#include <QString>
#include <QStringList>
#include <atomic>

class GifAnimation
{
public:
  static bool write(const QStringList& frame_paths,
                    const QString& output_path, int fps, QString& error,
                    const std::atomic_bool& cancelled, std::atomic_int& completed_frames);
};

#endif
