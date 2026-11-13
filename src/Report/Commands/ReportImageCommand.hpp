#pragma once
#include "CubitCommandInterface.hpp"
#include <memory>
class ReportApplicationInterface;
class ReportImageCommand final : public CubitCommand
{
public:
  explicit ReportImageCommand(std::weak_ptr<ReportApplicationInterface> application);
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;
private:
  std::weak_ptr<ReportApplicationInterface> application;
};
