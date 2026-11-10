#ifndef CORERESULTSEXTERNALSOURCE_HPP
#define CORERESULTSEXTERNALSOURCE_HPP
#include "Framework/Results/PublishedResultSource.hpp"
class CoreResultsExternalSource
{
public:
  // Host thread; fresh detached readers, synchronous conversion, atomic output.
  // restore=true requires the saved content identity before reading result arrays.
  bool open(const SimResults::FileSourceReference& request, SimResults::PublishedResultSource& output,
    std::string& error, bool restore = false) const;
};
#endif
