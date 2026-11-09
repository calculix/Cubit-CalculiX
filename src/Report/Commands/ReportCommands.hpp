#pragma once
#include "Framework/Interfaces/CommandProviderInterface.hpp"
#include <memory>
class ReportApplicationInterface;

class ReportCommands final : public CommandProviderInterface
{
public:
  explicit ReportCommands(std::weak_ptr<ReportApplicationInterface> application);
  ~ReportCommands() override;
  std::vector<std::string> command_keys() const override;
  CubitCommand* create_command(const std::string& key) override;

private:
  std::weak_ptr<ReportApplicationInterface> application;
};
