#ifndef CORETOPOLOGYLEVELSETINTEGRATIONDIAGNOSTICS_HPP
#define CORETOPOLOGYLEVELSETINTEGRATIONDIAGNOSTICS_HPP

#include "CoreTopologyLevelSetStructs.hpp"
#include <array>
#include <string>
#include <vector>

/**
 * @brief Numerical refinement diagnostics and bounded, opt-in failure captures.
 * @note Successive estimates are compared, not certified error bounds. Volume
 * error is relative, fraction error absolute, and coefficient derivatives are
 * scaled by the dimensionless interface width. Capture I/O runs only after
 * workers finish, on the calling thread. No Cubit queries or output occur here.
 */
class CoreTopologyLevelSetIntegrationDiagnostics
{
public:
  struct Estimate
  {
    double volume = 0.0;
    double solid = 0.0;
    std::vector<double> derivatives;
  };

  struct Comparison
  {
    int subdivisions = 0;
    double relative_volume_error = 0.0;
    double solid_fraction_error = 0.0;
    double scaled_derivative_error = 0.0;
    /// Zero-based index in the supplied Cubit expanded-connectivity order.
    int derivative_node_index = -1;
    double maximum_error = 0.0;
    double tolerance = 0.0;
  };

  struct Result
  {
    bool refinement_limit_reached = false;
    std::vector<Comparison> comparisons;
  };

  struct Capture
  {
    int iteration_id = -1;
    int source_iteration = -1;
    std::string phase;
    int element_id = -1;
    std::string calculix_type;
    int cubit_type = -1;
    std::vector<int> nodes;
    std::vector<std::array<double, 3>> coordinates;
    std::vector<double> values;
    double volume = 0.0;
    bool preserved = false;
    TopologyLevelSetSettings settings{};
    Result diagnostics;
    std::string error;
  };

  /// Inputs are validated positive-volume estimates of the same element, with
  /// equal derivative counts. Acceptance remains maximum_error <= tolerance.
  static Comparison compare(const Estimate& previous, const Estimate& current, double width, int subdivisions, double tolerance);
  static std::string describe(const Result& result);

  /// A capture holds at most one supported element (20 nodes, five comparisons).
  /// JSON numbers retain double round-trip precision. Write uses a unique file
  /// and atomic commit; read rejects malformed/nonfinite/out-of-contract data.
  /// False returns an error without publishing a partial output or printing.
  static bool write_capture(const std::string& directory, const Capture& capture, std::string& path, std::string& error);
  static bool read_capture(const std::string& path, Capture& capture, std::string& error);
};

#endif
