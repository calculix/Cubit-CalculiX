#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include <QString>
#include <vector>
#include <string>

class ConfigFile
{
  public:
    ConfigFile();
    ~ConfigFile();

  std::string componentpath;
  std::string filename;
  std::string filepath;

  void clear();
  void read_entry(std::string option, QString &value);
  void read_num_entry(std::string option, int &value);
  void write_entry(std::string option, QString value);
  void write_num_entry(std::string option, int value);
  QString standard_entry(std::string option);
  int standard_num_entry(std::string option);
};

#endif // CONFIGFILE_HPP