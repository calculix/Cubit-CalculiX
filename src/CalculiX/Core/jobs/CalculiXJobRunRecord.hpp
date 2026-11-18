#pragma once
#include "CalculiXJobInputBundle.hpp"
#include "CalculiXJobSetupSnapshot.hpp"
#include <memory>
#include "Framework/MeshQuality/MeshQualitySnapshot.hpp"
#include <map>

class CalculiXJobRunRecord
{
public:
  CalculiXJobRunRecord();
  // Parse archived root-deck provenance, or validate restored metadata when no
  // root deck is retained (topology/adjoint records).
  bool read_input_units(std::string& error);
  // Read root-deck provenance without retaining an input or typed setup archive.
  // Failure leaves the existing provenance unchanged.
  bool read_input_units(const std::string& filename, std::string& error);
  std::string materialProjectId;
  std::map<std::string, int> materialIds;
  std::string unitSystem;
  // Emitted solver-step ordinal -> established abscissa quantity.
  std::map<int, std::string> step_axis_quantities;
  std::string geometryLengthUnit;
  int jobId = -1;
  std::string launchId;
  std::string launchedAt;
  std::string workingDirectory;
  CalculiXJobInputBundle input;
  std::shared_ptr<const CalculiXJobSetupSnapshot> setup;
  std::string setupUnavailableReason;
  std::shared_ptr<const MeshQualitySnapshot> quality;
  std::string qualityUnavailableReason;
};
