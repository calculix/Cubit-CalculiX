#ifndef POSTPROCESSORICONLOADER_HPP
#define POSTPROCESSORICONLOADER_HPP

#include <QIcon>
#include <string>

class PostprocessorIconLoader
{
public:
  // Load an SVG by basename from the active Core-configured directory.
  // Missing assets return an empty icon.
  static QIcon load(const std::string& directory, const std::string& name);
};

#endif
