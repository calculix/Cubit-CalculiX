#ifndef NATIVEDISPLAYREFRESHOBSERVER_HPP
#define NATIVEDISPLAYREFRESHOBSERVER_HPP
#include "DrawingExport.hpp"
#include <QObject>
#include <string>
#include <set>
#include <functional>
class GCubitModelSet;
class QTimer;
class CUBITSIM_DRAWING_EXPORT NativeDisplayRefreshObserver : public QObject
{
public:
  NativeDisplayRefreshObserver(const std::string& workspace, QObject* parent);
  ~NativeDisplayRefreshObserver();
  void setActive(bool active);
  void reset();
  std::function<void()> displayChanged;
  std::function<bool()> shouldWatch;
private:
  void poll();
  std::string sentinelName;
  QTimer* timer = nullptr;
  bool active = false;
  std::set<GCubitModelSet*> armed;
};
#endif
