#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Core/CalculiXCore.hpp"
#include "CoreContactPairs.hpp"
#include "CoreCubitContactImport.hpp"
#include "CoreInteractionsTreeData.hpp"

std::vector<std::string> CalculiXCore::get_contactpair_master_slave(int contactpair_id) {
  return contactpairs->get_contactpair_master_slave(*me_iface, contactpair_id);
}

bool CalculiXCore::check_contactpair_exists(int contactpair_id) {
  return contactpairs->check_contactpair_exists(contactpair_id);
}

bool CalculiXCore::create_contactpair(std::vector<std::string> options) {
  const bool accepted = contactpairs->create_contactpair(options);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXCore::modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker) {
  const bool accepted = contactpairs->modify_contactpair(contactpair_id, options, options_marker);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXCore::delete_contactpair(int contactpair_id) {
  return contactpairs->delete_contactpair(contactpair_id);
}

bool CalculiXCore::create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust, std::string adjust_nodeset) {
  const bool accepted = CoreCubitContactImport::create_pairs(*contactpairs, *surfaceinteractions, *me_iface, surfaceinteraction_id, contactpairtype, adjust, adjust_nodeset);
  if (!accepted) {
    return false;
  }
  return true;
}

std::string CalculiXCore::get_contactpair_export_data() {
  return contactpairs->get_contactpair_export();
}

std::vector<std::vector<std::string>> CalculiXCore::get_contactpairs_tree_data() {
  return CoreInteractionsTreeData::get_contactpairs_tree_data(*contactpairs);
}
