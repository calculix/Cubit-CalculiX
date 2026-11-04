#ifndef CORERESULTSVTKELEMENTS_HPP
#define CORERESULTSVTKELEMENTS_HPP

#include "ElementIntegration.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace CoreResultsVtkElements
{
  enum class NodeOrder
  {
    Exodus,
    CalculiX,
    Frd,
    Vtk
  };

  struct Description
  {
    int frd_type = 0;
    int vtk_type = 0;
    std::size_t node_count = 0;
    std::string shape_type;
    // Empty means geometry is supported but the IP rule is not established.
    std::string integration_type;
  };

  bool describe(const std::string& calculix_type, Description& result, std::string& error);
  bool describe_frd(int frd_type, Description& result, std::string& error);

  /** Reorder LOCAL positions, preserving global IDs. Output is unchanged on error.
   * HEX20/WEDGE15 FRD follows Exodus, not CalculiX INP order (CCX 2.23 frd.c).
   * VTK topology follows its edge arrays; no face numbering is inferred here.
   * Only explicitly described topologies are accepted; unknown is never identity.
   */
  bool reorder(const Description& element, NodeOrder source, NodeOrder target, const std::vector<int>& input, std::vector<int>& output, std::string& error);

  // Convert one-based Exodus side IDs to CCX side labels, independently of
  // local node permutations and of Cubit enum numeric values.
  bool exodus_side(const std::string& calculix_type, int side, std::string& result, std::string& error);
  bool valid_side(const Description& element, const std::string& side);

  /** Extract an outward face (or a 2D edge) in VTK local node order.
   * Input connectivity is CalculiX ordered. SPOS/SNEG retain shell orientation;
   * numeric side IDs use CCX numbering, never VTK face numbering.
   */
  bool surface(const Description& element, const std::string& calculix_type, const std::string& side, const std::vector<int>& connectivity, int& vtk_type, std::vector<int>& nodes, std::string& error);

  /** Interpolate position or displacement from the supplied ordering. A precise
   * formulation is required; an FRD topology by itself never selects an IP rule.
   */
  bool integration_point(const std::string& calculix_type, NodeOrder order, int point_id, const std::vector<ElementIntegration::Coordinates>& values, ElementIntegration::Coordinates& result, std::string& error);
}

#endif // CORERESULTSVTKELEMENTS_HPP
