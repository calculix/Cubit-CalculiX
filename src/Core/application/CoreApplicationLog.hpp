#ifndef COREAPPLICATIONLOG_HPP
#define COREAPPLICATIONLOG_HPP

#include <string>

class CoreApplicationLog
{
public:
  static bool append(const std::string& message);
};

#endif
