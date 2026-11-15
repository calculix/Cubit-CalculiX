#pragma once
#include "MaterialsExport.hpp"
#include "CoreFrameworkMaterials.hpp"
#include <string>
#include <vector>
class HDF5Tool;
enum class MaterialValueSpace { Canonical, Display, Solver };
class CUBITSIM_MATERIALS_EXPORT FrameworkMaterialsInterface {
public:
  using Material = int;
  using Property = std::string;
  using PropertyGroup = std::string;
  explicit FrameworkMaterialsInterface(std::string workspace = "calculix", MaterialValueSpace space = MaterialValueSpace::Display);
  ~FrameworkMaterialsInterface();
  int get_material(int id) const;
  int get_material(const std::string& name) const;
  int get_material_id(int material) const;
  std::string get_material_name(int material) const;
  std::string get_material_property_group(int material) const;
  std::string get_group_name(const std::string& group) const;
  std::string get_property(const std::string& property) const;
  std::vector<int> get_material_ids() const;
  std::vector<std::vector<std::string>> get_group_properties() const;
  int get_property_size(const std::string& property) const;
  bool get_material_property_value(int material, const std::string& property, double& value) const;
  bool get_material_property_value(int material, const std::string& property, std::vector<std::vector<double>>& values) const;
  bool get_material_property_value(int material, const std::string& property, std::vector<double>& values) const;
  bool read_scalar(const MaterialData& material, const std::string& property, double& value, std::string& error) const;
  bool read_table(const MaterialData& material, const std::string& property, std::vector<std::vector<double>>& values, std::string& error) const;
  bool write_scalar(MaterialData& material, const std::string& property, double value, std::string& error) const;
  bool write_table(MaterialData& material, const std::string& property, const std::vector<std::vector<double>>& values, std::string& error) const;
  bool set_scalar(int material, const std::string& property, double value, std::string& error);
  bool set_table(int material, const std::string& property, const std::vector<std::vector<double>>& values, std::string& error);
  bool create_material(const std::string& name, int& id, std::string& error, int requestedId = 0);
  bool delete_material(int id, std::string& error);
  bool rename_material(int id, const std::string& name, std::string& error);
  bool set_card_state(int id, const std::string& card, MaterialCardState state, std::string& error);
  bool set_model(int id, const std::string& card, const std::string& model, std::string& error);
  bool validate_materials(std::string& error) const;
  bool save(HDF5Tool& file, const std::string& group, std::string& error) const;
  bool load(HDF5Tool& file, const std::string& group, std::string& error);
  void reset();
  const MaterialInstance* find(int id) const;
  const std::string& workspace() const noexcept;
  MaterialValueSpace value_space() const noexcept;
  std::vector<std::vector<std::string>> get_material_tree_data() const;
  std::vector<std::vector<std::string>> get_material_group_properties() const;
  bool create_materiallibrary_group(const std::string& name);
  bool modify_materiallibrary_group(const std::string& name, const std::string& data, int mode);
  bool delete_materiallibrary_group(const std::string& name);
  bool create_materiallibrary_material(const std::string& name, const std::string& group);
  bool modify_materiallibrary_material(const std::string& name, const std::string& group, const std::string& data, int mode, const std::vector<double>& values);
  bool delete_materiallibrary_material(const std::string& name, const std::string& group);
  bool export_materiallibrary_material(const std::string& name, const std::string& group, const std::string& materialName);
  bool import_materiallibrary_material(const std::string& name, const std::string& group, const std::string& materialName);
  std::vector<std::vector<std::string>> get_materiallibrary_tree_data() const;
  std::vector<std::vector<std::string>> get_materiallibrary_material_properties(const std::string& name, const std::string& group) const;
  std::vector<std::vector<double>> get_materiallibrary_material_values(const std::string& name, const std::string& group, const std::string& property) const;
private:
  bool convert(double input, const MaterialModelDefinition& model, std::size_t column, const std::vector<double>& row, bool toCanonical, double& output, std::string& error) const;
  std::string workspaceId;
  MaterialValueSpace space;
};
