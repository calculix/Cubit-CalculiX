#ifndef POSTVIEWCAMERACOMMAND_HPP
#define POSTVIEWCAMERACOMMAND_HPP

#include "CubitCommandInterface.hpp"
#include <memory>
class PostprocessorApplicationInterface;

class PostViewCameraCommand : public CubitCommand
{
public:
  explicit PostViewCameraCommand(std::weak_ptr<PostprocessorApplicationInterface> application);
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;

private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
};
#endif
