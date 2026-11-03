#ifndef NATIVEDISPLAYREFRESHOBSERVER_HPP
#define NATIVEDISPLAYREFRESHOBSERVER_HPP
#include <QObject>
#include <set>
#include <functional>
class GCubitModelSet;
class QTimer;
class NativeDisplayRefreshObserver : public QObject
{
public:
  explicit NativeDisplayRefreshObserver(QObject* parent);
  ~NativeDisplayRefreshObserver();
  void setActive(bool active);
  void reset();
  std::function<void()> displayChanged;
private:
  void poll();
  QTimer* timer = nullptr;
  bool active = false;
  std::set<GCubitModelSet*> armed;
};
#endif
