#pragma once
#include "CubitCommandInterface.hpp"
class ImportMaterialCommand : public CubitCommand {
public:
  ImportMaterialCommand();
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;
};
