#ifndef CORECSVWRITER_HPP
#define CORECSVWRITER_HPP

#include <string>
#include <vector>

class CoreCsvWriter
{
public:
  static bool write(
    const std::string& path,
    const std::vector<std::string>& header,
    const std::vector<std::vector<double>>& data,
    bool overwrite);
};

#endif
