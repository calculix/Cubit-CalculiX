#pragma once
#include "CubitInterface.hpp"
#include <memory>
class ReportApplicationInterface;

class ReportModelObserver final : public CubitInterface::ObserverBase
{
public:
  explicit ReportModelObserver(std::weak_ptr<ReportApplicationInterface> application);
  ~ReportModelObserver() override;
  bool project_coordinated = false;
  void notify_model_reset() override;
  void notify_cub_file_read(std::string filename) override;
  void notify_cub_file_saved(std::string filename, bool temporary) override;

private:
  std::weak_ptr<ReportApplicationInterface> application;
  bool updating = false;
};
