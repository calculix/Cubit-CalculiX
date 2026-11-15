#ifndef CUBITSIM_VTKRESULTSMETADATACODEC_HPP
#define CUBITSIM_VTKRESULTSMETADATACODEC_HPP
#include "VtkResultsData.hpp"
namespace SimResults {
class VtkResultsMetadataCodec {
public:
  static bool write(VtkResultsData& result, std::string& error);
  static bool read(VtkResultsData& result, std::string& error);
  static bool write_frame(vtkDataObject& target, const VtkResultsData& result, std::size_t frame, std::string& error);
  static bool write_table(vtkTable& target, const VtkResultsData& result, const std::optional<std::size_t>& frame, std::size_t table, std::string& error);
};
}
#endif
