#ifndef GIFANIMATION_HPP
#define GIFANIMATION_HPP

#include <QString>
#include <QStringList>
#include <atomic>
#include <QColor>
#include "MediaExport.hpp"

class CUBITSIM_MEDIA_EXPORT GifAnimation
{
public:
  static bool write(const QStringList& frame_paths,
                    const QString& output_path, int fps, QString& error,
                    const std::atomic_bool& cancelled, std::atomic_int& completed_frames,
                    bool loop = true,const QColor& background = Qt::white);
};

#endif
