#ifndef GIFANIMATION_HPP
#define GIFANIMATION_HPP

#include <QString>
#include <QStringList>

class GifAnimation
{
public:
  static bool write(const QStringList& frame_paths,
                    const QString& output_path, int fps);
};

#endif
