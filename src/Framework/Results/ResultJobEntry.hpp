#ifndef SIMRESULTS_RESULTJOBENTRY_HPP
#define SIMRESULTS_RESULTJOBENTRY_HPP
#include "ResultSourceReference.hpp"
namespace SimResults
{
  struct ResultJobEntry
  {
    JobSourceReference reference;
    std::string command_prefix;
    std::string application_name;
    std::string job_name;
    bool available = false;
    std::string unavailable_reason;
  };
}
#endif
