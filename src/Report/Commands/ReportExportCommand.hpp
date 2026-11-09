#pragma once
#include "CubitCommandInterface.hpp"
#include <memory>
class ReportApplicationInterface;

class ReportExportCommand final : public CubitCommand
{
public:
  explicit ReportExportCommand(std::weak_ptr<ReportApplicationInterface> application);
  ~ReportExportCommand() override;
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;

private:
  bool run(CubitCommandData& data, const std::shared_ptr<ReportApplicationInterface>& active, std::string& error);
  std::weak_ptr<ReportApplicationInterface> application;
};
