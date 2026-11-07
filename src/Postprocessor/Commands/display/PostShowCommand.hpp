#ifndef POSTSHOWCOMMAND_HPP
#define POSTSHOWCOMMAND_HPP
#include "CubitCommandInterface.hpp"
#include <memory>
class PostprocessorApplicationInterface;

class PostShowCommand : public CubitCommand
{
public:
  explicit PostShowCommand(std::weak_ptr<PostprocessorApplicationInterface> application);
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;
private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
};
#endif
