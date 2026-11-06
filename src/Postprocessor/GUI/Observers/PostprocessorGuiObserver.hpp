#ifndef POSTPROCESSORGUIOBSERVER_HPP
#define POSTPROCESSORGUIOBSERVER_HPP

#include "CubitInterface.hpp"
#include <QObject>
#include <functional>

class PostprocessorGuiObserver : public QObject, public CubitInterface::ObserverBase
{
public:
  explicit PostprocessorGuiObserver(std::function<void()> refresh);
  ~PostprocessorGuiObserver() override;
  void notify_command_complete() override;

private:
  std::function<void()> refresh;
  bool pending = false;
};

#endif // POSTPROCESSORGUIOBSERVER_HPP
