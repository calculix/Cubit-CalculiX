#ifndef POSTLABELSCOMMAND_HPP
#define POSTLABELSCOMMAND_HPP
#include "CubitCommandInterface.hpp"
#include <memory>
class PostprocessorApplicationInterface;
class PostLabelsCommand : public CubitCommand
{
public:
  explicit PostLabelsCommand(std::weak_ptr<PostprocessorApplicationInterface> application);
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;
private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
};
#endif
