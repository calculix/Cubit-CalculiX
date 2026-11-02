#ifndef CALCULIX_CONSTRAINTSSETTINGS_HPP
#define CALCULIX_CONSTRAINTSSETTINGS_HPP

#include <string>
#include <vector>

struct ConstraintsSettings
{
  std::vector<std::vector<double>> terms;
  std::vector<std::vector<double>> equationPairs;
  int id = -1;
  int type = 0;
  std::string name;
  std::string block;
  std::string nodeset;
  std::string reference;
  std::string rotation;
  std::string master;
  std::string slave;
  std::string tolerance;
};

#endif
