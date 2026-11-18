#pragma once
#include <QObject>
#include <QElapsedTimer>
#include <QTimer>
#include <functional>
#include <memory>
#include <string>
class PostprocessorApplicationInterface;
class PostprocessorExplosionPlayback final : public QObject
{
public:
  using Dispatch = std::function<bool(const std::string&,std::string&)>;
  PostprocessorExplosionPlayback(std::weak_ptr<PostprocessorApplicationInterface> application,
    Dispatch silent,std::function<bool()> ready,std::function<bool()> paused,
    std::function<void()> redraw,QObject* parent);
  void pause_clock();
  void advance_for_render();
private:
  bool active();
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Dispatch silent;
  std::function<bool()> paused;
  QTimer timer;
  QElapsedTimer clock;
  std::string owner;
  std::string lastError;
};
