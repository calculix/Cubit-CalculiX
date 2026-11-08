#ifndef POSTVIEWZOOMCOMMAND_HPP
#define POSTVIEWZOOMCOMMAND_HPP

#include "CubitCommandInterface.hpp"
#include <memory>
class PostprocessorApplicationInterface;

class PostViewZoomCommand : public CubitCommand
{
public:
  explicit PostViewZoomCommand(std::weak_ptr<PostprocessorApplicationInterface> application);
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;

private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
};
#endif
