#pragma once
#include "CubitInterface.hpp"
#include <QObject>
#include <functional>

class ReportGuiObserver final : public QObject, public CubitInterface::ObserverBase
{
public:
  explicit ReportGuiObserver(std::function<void()> refresh);
  ~ReportGuiObserver() override;
  void notify_command_complete() override;

private:
  std::function<void()> refresh;
  bool pending = false;
};
