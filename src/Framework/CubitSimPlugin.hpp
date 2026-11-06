#ifndef CUBITSIMPLUGIN_HPP
#define CUBITSIMPLUGIN_HPP

#include "CubitCommandInterface.hpp"

class CubitSimPlugin : public CubitCommandInterface
{
public:
  CubitSimPlugin();
  ~CubitSimPlugin() override;

  std::vector<std::string> get_keys() override;
  CubitCommand* create_command(const std::string& key) override;

private:
  bool acquired = false;
};

#endif // CUBITSIMPLUGIN_HPP
