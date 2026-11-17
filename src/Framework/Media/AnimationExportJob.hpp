#pragma once
#include "AnimationOutput.hpp"
#include <QObject>
#include <QTimer>
#include <QImage>
#include <atomic>
#include <functional>
#include <future>
namespace Framework::Media
{
  class CUBITSIM_MEDIA_EXPORT AnimationExportJob : public QObject
  {
  public:
    using Producer = std::function<bool(std::size_t,QImage&,QString&)>;
    using Finished = std::function<void(bool,bool,const QString&)>;
    explicit AnimationExportJob(QObject* parent = nullptr);
    ~AnimationExportJob() override;
    bool start(const AnimationExportDefinition& definition,Producer producer,Finished finished,QString& error);
    void cancel();
    bool active() const;
    QString status() const;
  private:
    enum class Phase { Idle, Checking, Capturing, Writing, Encoding };
    void advance();
    void encode();
    void finish(const QString& error);
    QTimer timer;
    Phase phase = Phase::Idle;
    AnimationExportDefinition definition;
    Producer producer;
    Finished finished;
    std::shared_ptr<AnimationOutput> output;
    std::shared_ptr<std::atomic_bool> cancelled;
    std::shared_ptr<std::atomic_int> completed;
    std::shared_ptr<std::atomic_bool> published;
    std::future<QString> worker;
    std::size_t next = 0;
    QString failure, lastStatus;
  };
}
