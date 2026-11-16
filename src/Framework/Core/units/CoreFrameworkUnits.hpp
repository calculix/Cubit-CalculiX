#pragma once
#include "Framework/ModelExport.hpp"
#include "UnitDefinition.hpp"
#include "UnitSystem.hpp"
#include "QuantityUnit.hpp"
#include "WorkspaceUnitContext.hpp"
#include "SourceUnitContext.hpp"
#include <map>
#include <cstdint>
#include <string>
#include <vector>

class UserOptionsInterface;

namespace SimUnits {
class CUBITSIM_MODEL_EXPORT CoreFrameworkUnits {
public:
  CoreFrameworkUnits();
  bool initialize(UserOptionsInterface& options, std::string& error);
  void shutdown() noexcept;
  void reset_project();
  void swap_project_state(CoreFrameworkUnits& other) noexcept;
  bool reset_workspace(const std::string& workspace, std::string& error);
  std::uint64_t generation() const noexcept;
  bool restore_project(const std::vector<WorkspaceUnitContext>& contexts, const std::string& geometryUnit, std::string& error);
  std::vector<std::string> workspace_ids() const;
  std::vector<std::string> system_ids() const;
  std::vector<std::string> quantity_ids() const;
  bool unit(const std::string& expression, UnitDefinition& output, std::string& error) const;
  bool quantity(const std::string& id, QuantityUnit& output, std::string& error) const;
  bool system_unit(const std::string& system, const std::string& quantity, QuantityUnit& output, std::string& error) const;
  bool workspace_context(const std::string& workspace, WorkspaceUnitContext& output, std::string& error) const;
  bool validate_workspace_context(const WorkspaceUnitContext& context, std::string& error) const;
  bool set_workspace_context(const WorkspaceUnitContext& context, std::string& error);
  bool set_workspace_system(const std::string& workspace, const std::string& system, std::string& error);
  bool display_unit(const std::string& workspace, const std::string& quantity, std::string& output, std::string& error) const;
  bool set_display_unit(const std::string& workspace, const std::string& quantity, const std::string& unit, std::string& error);
  bool save_defaults(const std::string& workspace, std::string& error);
  bool validate_source(const SourceUnitContext& context, std::string& error) const;
  bool declare_geometry_unit(const std::string& unit, std::string& error);
  const std::string& geometry_unit() const noexcept;
  bool geometry_to_si(double value, double& output, std::string& error) const;
  bool convert(double value, const std::string& from, const std::string& to, double& output, std::string& error) const;
  bool read(double value, const std::string& unit, const Dimension& dimension, UnitRole role, double& output, std::string& error) const;
  bool to_si(double value, const std::string& unit, const Dimension& dimension, UnitRole role, double& output, std::string& error) const;
  bool from_si(double value, const std::string& unit, const Dimension& dimension, UnitRole role, double& output, std::string& error) const;
  bool to_solver(double value, const Dimension& dimension, const std::string& workspace, double& output, std::string& error) const;
  bool from_solver(double value, const Dimension& dimension, const std::string& workspace, double& output, std::string& error) const;
  bool system_to_si(double value, const Dimension& dimension, const std::string& system, double& output, std::string& error) const;
  bool system_from_si(double value, const Dimension& dimension, const std::string& system, double& output, std::string& error) const;
  static bool compatible(const UnitDefinition& source, const UnitDefinition& target);
private:
  static bool apply(double value, const UnitDefinition& source, const UnitDefinition& target, double& output, std::string& error);
  void add_unit(UnitDefinition unit);
  void add_quantity(QuantityUnit quantity);
  std::map<std::string, UnitDefinition> definitions;
  std::map<std::string, QuantityUnit> quantities;
  std::map<std::string, UnitSystem> systems;
  std::map<std::string, WorkspaceUnitContext> contexts;
  std::map<std::string, WorkspaceUnitContext> defaults;
  std::string geometryUnit;
  UserOptionsInterface* options = nullptr;
  std::uint64_t changeGeneration = 0;
};
}

CUBITSIM_MODEL_EXPORT SimUnits::CoreFrameworkUnits& framework_units();
