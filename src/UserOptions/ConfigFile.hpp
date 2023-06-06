#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include <QString>

class ConfigFile
{
  public:
    ConfigFile();
    ~ConfigFile();

  std::string filename = "ccx.cfg";

  void clear();
  void read_entry(std::string option, QString &value);
  void read_num_entry(std::string option, int &value);  
  void write_entry(std::string option, QString value);
  void write_num_entry(std::string option, int value);
};

#endif // CONFIGFILE_HPP