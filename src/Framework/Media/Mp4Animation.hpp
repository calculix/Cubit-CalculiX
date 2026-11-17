#pragma once
#include "AnimationExportDefinition.hpp"
#include <atomic>
namespace Framework::Media
{
  class CUBITSIM_MEDIA_EXPORT Mp4Animation
  {
  public:
    static bool available(const std::atomic_bool& cancelled,QString& error);
    static bool write(const QString& frames,const QString& output,int fps,
      const std::atomic_bool& cancelled,std::atomic_int& completed,QString& error);
  };
}
