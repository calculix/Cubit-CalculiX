#include "ccxMaterialCommand.hpp"
#include "Framework/Core/materials/FrameworkMaterialsInterface.hpp"
#include "Framework/Core/materials/CoreFrameworkMaterialLibrary.hpp"
#include "Framework/Interfaces/ModelEditorInterface.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <cmath>

ccxMaterialCommand::ccxMaterialCommand() = default;
std::vector<std::string> ccxMaterialCommand::get_syntax() {
  const std::string id = "<value:label='material_id'>";
  const std::string key = "<string:type='unquoted',number='1',label='card_key'>";
  const std::string model = "<string:type='unquoted',number='1',label='model'>";
  const std::string unit = "[unit <string:type='quoted',number='1',label='unit'>]";
  std::vector<std::string> syntax = {"ccx create material <string:type='quoted',number='1',label='name'>", "ccx delete material " + id, "ccx list materials", "ccx open materialmanagement [material " + id + "]", "ccx close materialmanagement"};
  const std::vector<std::string> targets = {"ccx modify material " + id, "ccx modify materiallibrary material name <string:type='quoted',number='1',label='library_name'> groupname <string:type='quoted',number='1',label='library_group'>"};
  for (const auto& target : targets) {
    syntax.push_back(target + " name <string:type='quoted',number='1',label='name'>");
    syntax.push_back(target + " description <string:type='quoted',number='1',label='description'>");
    syntax.push_back(target + " add card " + key + " model " + model);
    syntax.push_back(target + " {activate|deactivate|remove} card " + key);
    syntax.push_back(target + " card " + key + " model " + model);
    syntax.push_back(target + " card " + key + " clear values");
    syntax.push_back(target + " card " + key + " row <value:label='row'> values <value:label='values'>... " + unit);
    syntax.push_back(target + " card " + key + " value <value:label='value'> " + unit);
    syntax.push_back(target + " card " + key + " reference temperature <value:label='temperature'> " + unit);
    syntax.push_back(target + " card " + key + " clear reference temperature");
  }
  syntax.push_back("ccx materiallibrary file [<string:type='quoted',number='1',label='library_file'>]");
  return syntax;
}
std::vector<std::string> ccxMaterialCommand::get_syntax_help() {
  return {"ccx create material \"Steel\"", "ccx modify material 1 add card elastic model isotropic", "ccx modify material 1 card elastic row 1 values 210000 0.3", "ccx modify material 1 activate card elastic", "ccx modify material 1 deactivate card plastic", "ccx open materialmanagement material 1"};
}
std::vector<std::string> ccxMaterialCommand::get_help() {
  return {"Materials belong only to the CalculiX workspace. Card rows use the displayed column units; unit \"SI\" selects canonical SI for every column. Rows start at one. Deactivation keeps values. Removal discards the card. Library edits use the same card grammar."};
}
bool ccxMaterialCommand::execute(CubitCommandData& command) {
  try {
    auto& core = framework_materials();
    auto& store = core.workspace("calculix");
    FrameworkMaterialsInterface display("calculix", MaterialValueSpace::Display);
    std::string error;
    double requestedId = 0.0;
    command.get_value("material_id", requestedId);
    if (!std::isfinite(requestedId) || requestedId < 0.0 || requestedId > 2147483646.0 || std::floor(requestedId) != requestedId) {
      PRINT_ERROR("A material ID must be a positive integer.\n");
      return false;
    }
    const int id = static_cast<int>(requestedId);
    bool accepted = false;
    if (command.find_keyword("MATERIALLIBRARY") && command.find_keyword("FILE")) {
      std::string filename;
      if (command.get_string("library_file", filename)) {
        accepted = core.library().set_file_option(filename, error);
      } else {
        PRINT_INFO("Shared material library: %s\n", core.library().filename().c_str());
        return true;
      }
    } else if (command.find_keyword("MATERIALMANAGEMENT")) {
      if (command.find_keyword("OPEN")) {
        accepted = ModelEditorInterface::open("calculix", "materials", id, error);
      } else {
        accepted = ModelEditorInterface::close("calculix", "materials", error);
      }
    } else if (command.find_keyword("LIST")) {
      for (const auto& entry : store.all()) {
        PRINT_INFO("CalculiX material %d: %s\n", entry.first, entry.second.data.name.c_str());
      }
      return true;
    } else if (command.find_keyword("CREATE")) {
      std::string name;
      int created = 0;
      accepted = command.get_string("name", name) && display.create_material(name, created, error);
      if (accepted) {
        PRINT_INFO("Created CalculiX material %d: %s\n", created, name.c_str());
      }
    } else if (command.find_keyword("DELETE")) {
      CalculiXSectionsInterface sections;
      for (const auto& row : sections.get_sections_tree_data()) {
        if (row.empty()) {
          continue;
        }
        SectionsSettings settings;
        if (!sections.get_section_settings(std::stoi(row.front()), settings, error)) {
          PRINT_ERROR("Cannot check material assignments: %s\n", error.c_str());
          return false;
        }
        if (settings.material == std::to_string(id)) {
          PRINT_ERROR("Material %d is assigned to section %d. Reassign or delete the section first.\n", id, settings.id);
          return false;
        }
      }
      accepted = display.delete_material(id, error);
    } else {
      std::string libraryName;
      std::string libraryGroup;
      const bool library = command.get_string("library_name", libraryName);
      MaterialData candidate;
      if (library) {
        if (!command.get_string("library_group", libraryGroup) || !core.library().read_material("calculix", libraryName, libraryGroup, candidate, error)) {
          PRINT_ERROR("Cannot read library material: %s\n", error.c_str());
          return false;
        }
      } else {
        const auto* original = store.find({"calculix", id});
        if (original == nullptr) {
          PRINT_ERROR("CalculiX material %d does not exist.\n", id);
          return false;
        }
        candidate = original->data;
      }
      std::string name;
      std::string description;
      std::string cardKey;
      if (command.get_string("name", name)) {
        candidate.name = name;
        accepted = true;
      } else if (command.get_string("description", description)) {
        candidate.description = description;
        accepted = true;
      } else if (command.get_string("card_key", cardKey)) {
        const auto* definition = core.definitions().card(cardKey);
        if (definition == nullptr) {
          error = "Unknown material card: " + cardKey;
        } else {
          cardKey = definition->key;
          auto found = candidate.cards.find(cardKey);
          std::string modelName;
          const bool hasModel = command.get_string("model", modelName);
          if (command.find_keyword("ADD")) {
            const auto* model = core.definitions().model(*definition, modelName);
            if (found != candidate.cards.end() || model == nullptr) {
              error = "Card already exists or its model is unknown.";
            } else {
              MaterialCard card;
              card.model = model->key;
              candidate.cards.emplace(cardKey, std::move(card));
              accepted = true;
            }
          } else if (found == candidate.cards.end()) {
            error = "Add the material card before editing it.";
          } else if (command.find_keyword("REMOVE")) {
            candidate.cards.erase(found);
            accepted = true;
          } else if (command.find_keyword("DEACTIVATE")) {
            found->second.state = MaterialCardState::Inactive;
            accepted = true;
          } else if (command.find_keyword("ACTIVATE")) {
            found->second.state = MaterialCardState::Active;
            accepted = core.definitions().validate(candidate, true, error);
          } else if (hasModel) {
            const auto* model = core.definitions().model(*definition, modelName);
            if (model == nullptr) {
              error = "Unknown material model: " + modelName;
            } else {
              if (found->second.model != model->key) {
                found->second.values = MaterialCardValues{};
              }
              found->second.model = model->key;
              accepted = true;
            }
          } else {
            const auto* model = core.definitions().model(*definition, found->second.model);
            std::string unit;
            command.get_string("unit", unit);
            if (!unit.empty() && unit != "SI") {
              error = "Use unit \"SI\" for canonical rows, or omit unit for workspace display units.";
            } else if (model == nullptr) {
              error = "The material card has no valid model.";
            } else {
              MaterialValueSpace space = MaterialValueSpace::Display;
              if (unit == "SI") {
                space = MaterialValueSpace::Canonical;
              }
              FrameworkMaterialsInterface values("calculix", space);
              if (command.find_keyword("REFERENCE")) {
                if (cardKey != "expansion") {
                  error = "Reference temperature belongs to the expansion card.";
                } else if (command.find_keyword("CLEAR")) {
                  found->second.values.referenceTemperature.reset();
                  accepted = true;
                } else {
                  double temperature = 0.0;
                  accepted = command.get_value("temperature", temperature) && values.write_scalar(candidate, "CCX_EXPANSION_ZERO", temperature, error);
                }
              } else if (command.find_keyword("CLEAR")) {
                found->second.values.rows.clear();
                found->second.values.scalar.reset();
                accepted = true;
              } else if (model->scalar) {
                double value = 0.0;
                accepted = command.get_value("value", value) && values.write_scalar(candidate, model->property, value, error);
              } else {
                std::vector<std::vector<double>> rows;
                std::vector<double> row;
                double rowIndex = 0.0;
                if (command.get_value("row", rowIndex) && command.get_values("values", row) && values.read_table(candidate, model->property, rows, error)) {
                  if (!std::isfinite(rowIndex) || rowIndex < 1.0 || rowIndex > static_cast<double>(rows.size() + 1) || std::floor(rowIndex) != rowIndex) {
                    error = "Row must replace an existing row or append the next row, starting at one.";
                  } else {
                    const auto index = static_cast<std::size_t>(rowIndex - 1.0);
                    if (index == rows.size()) {
                      rows.push_back(row);
                    } else {
                      rows[index] = row;
                    }
                    accepted = values.write_table(candidate, model->property, rows, error);
                  }
                }
              }
            }
          }
        }
      }
      if (accepted) {
        if (library) {
          accepted = core.library().replace_material("calculix", libraryName, libraryGroup, candidate, error);
        } else {
          accepted = store.replace({"calculix", id}, std::move(candidate), error);
        }
      }
    }
    if (!accepted) {
      if (error.empty()) {
        error = "Missing or invalid material command arguments.";
      }
      PRINT_ERROR("Cannot apply material command: %s\n", error.c_str());
    }
    return accepted;
  } catch (const std::exception& exception) {
    PRINT_ERROR("Cannot apply material command: %s\n", exception.what());
  } catch (...) {
    PRINT_ERROR("Cannot apply material command: unknown error.\n");
  }
  return false;
}
