#ifndef POSTPROCESSOR_VTKFILESOURCEPROVIDER_HPP
#define POSTPROCESSOR_VTKFILESOURCEPROVIDER_HPP
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
namespace Postprocessor
{
  class VtkFileSourceProvider : public ResultSourceProviderInterface
  {
  public:
    std::string command_prefix() const override;
    bool restore(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override;
    bool open(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override;
    bool refresh(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override;
  };
}
#endif
