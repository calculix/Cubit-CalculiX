#ifndef FEAPLUGIN_HPP
#define FEAPLUGIN_HPP

#include "CubitCommandInterface.hpp"

class FEAPlugin : public CubitCommandInterface
{
public:
  FEAPlugin();
  ~FEAPlugin() override;

  std::vector<std::string> get_keys() override;
  CubitCommand* create_command(const std::string& key) override;

private:
  bool acquired = false;
};

#endif // FEAPLUGIN_HPP
