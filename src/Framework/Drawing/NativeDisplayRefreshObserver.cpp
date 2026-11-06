#include "NativeDisplayRefreshObserver.hpp"
#include "GCubitModelSet.hpp"
#include "GWindowFactory.hpp"
#include "GView.hpp"
#include "CubitMessage.hpp"
#include <QTimer>
#include <QString>
#include <QByteArray>
#include <QCoreApplication>

namespace
{
std::set<GCubitModelSet*> currentModels()
{
  std::set<GCubitModelSet*> result;
  auto* factory = GWindowFactory::get_window_factory();
  if (factory != nullptr)
  {
    for (const auto& view : factory->views())
    {
      auto* models = view == nullptr ? nullptr : dynamic_cast<GCubitModelSet*>(view->get_model_set());
      if (models != nullptr)
      {
        result.insert(models);
      }
    }
  }
  return result;
}
}

NativeDisplayRefreshObserver::NativeDisplayRefreshObserver(const std::string& workspace, QObject* parent) : QObject(parent)
  , sentinelName("cubitsim_refresh_" + QString::fromStdString(workspace).toUtf8().toHex().toStdString())
{
  timer = new QTimer(this);
  timer->setInterval(100);
  connect(timer, &QTimer::timeout, this, [this]() { poll(); });
  connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, [this]()
  {
    active = false;
    timer->stop();
    armed.clear();
  });
}

NativeDisplayRefreshObserver::~NativeDisplayRefreshObserver()
{
  timer->stop();
  reset();
}

void NativeDisplayRefreshObserver::setActive(bool enabled)
{
  timer->stop();
  reset();
  active = enabled && !QCoreApplication::closingDown();

  if (active)
  {
    timer->start();
  }
}

void NativeDisplayRefreshObserver::reset()
{
  if (armed.empty())
  {
    return;
  }

  if (QCoreApplication::closingDown())
  {
    armed.clear();
    return;
  }

  // Re-enumerate live viewports; never dereference a remembered model-set pointer.
  for (auto* models : currentModels())
  {
    if (auto* set = models->get_drawing_set(sentinelName.c_str()))
    {
      models->delete_drawing_set(set);
    }
  }
  armed.clear();
}

void NativeDisplayRefreshObserver::poll()
{
  if (!active || QCoreApplication::closingDown())
  {
    return;
  }

  if (!shouldWatch || !shouldWatch())
  {
    reset();
    return;
  }
  const auto models = currentModels();
  bool cleared = false;
  for (auto* model : models)
  {
    if (armed.count(model) != 0 && model->get_drawing_set(sentinelName.c_str()) == nullptr)
    {
      cleared = true;
    }
  }
  if (cleared)
  {
    reset();
    // Native display lifecycle notification, not a user command. Avoid nested
    // commands while Cubit is finishing its own Refresh Display operation.
    if (displayChanged)
    {
      displayChanged();
    }
    return;
  }
  armed.clear();
  for (auto* model : models)
  {
    // Cubit deletes these sets on Display/Refresh Display, not on repaint or
    // camera motion. Empty sentinel geometry never appears in the viewport.
    auto* set = model->get_drawing_set(sentinelName.c_str());
    if (set == nullptr)
    {
      set = model->create_drawing_set(sentinelName.c_str(), true);
    }
    if (set != nullptr)
    {
      armed.insert(model);
    }
  }
}
