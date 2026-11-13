#pragma once
#include <QByteArray>
#include <map>
#include <string>
#include <vector>

class CalculiXJobInputBundle
{
public:
  CalculiXJobInputBundle();
  std::string rootFile;
  std::vector<std::string> order;
  std::map<std::string, QByteArray> files;
  std::map<std::string, std::string> fingerprints;
  // Parent file -> literal INCLUDE spelling -> resolved archived file identity.
  std::map<std::string, std::map<std::string, std::string>> includes;
  std::string manifestFingerprint;
};
