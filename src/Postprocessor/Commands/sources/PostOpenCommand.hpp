#ifndef POSTOPENCOMMAND_HPP
#define POSTOPENCOMMAND_HPP

#include "CubitCommandInterface.hpp"

#include <memory>

class PostprocessorApplicationInterface;

// Cubit owns command objects. A weak application handle makes commands retained
// across module shutdown fail safely rather than retaining application state.
class PostOpenCommand : public CubitCommand
{
public:
  explicit PostOpenCommand(std::weak_ptr<PostprocessorApplicationInterface> application);
  std::vector<std::string> get_syntax() override;
  std::vector<std::string> get_syntax_help() override;
  std::vector<std::string> get_help() override;
  bool execute(CubitCommandData& data) override;

private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
};

#endif // POSTOPENCOMMAND_HPP
