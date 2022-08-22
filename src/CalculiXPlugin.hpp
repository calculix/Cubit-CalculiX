#ifndef CALCULIXPLUGIN_HPP
#define CALCULIXPLUGIN_HPP

#include "CubitCommandInterface.hpp"
#include "CubitPluginExport.hpp"

/*!
 * \brief The CalculiXPlugin class derives from CubitCommandInterface
 * which gives access to the parser and allows you to implement
 * your own commands.
 */
class CalculiXPlugin : public CubitCommandInterface
{
public:
  CalculiXPlugin();
  ~CalculiXPlugin();

  //! Used to get a list of unique keys that identify the commands this plugin
  //! will create.
  std::vector<std::string> get_keys();

  //! Returns the command corresponding to 'key'.
  CubitCommand* create_command(const std::string &key);
};

#endif // CALCULIXPLUGIN_HPP
