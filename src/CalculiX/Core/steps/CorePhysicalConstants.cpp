#include "Core/units/CalculiXDomainUnits.hpp"
#include <utility>
#include "Core/application/SettingsRead.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include <string>
#include <vector>
#include "Core/export/CalculiXExportInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

CorePhysicalConstants::CorePhysicalConstants()
{}

CorePhysicalConstants::~CorePhysicalConstants()
{}

bool CorePhysicalConstants::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    physicalconstants_data.push_back("");
    physicalconstants_data.push_back("");
    physicalconstants_data.push_back("");

    is_initialized = true;
    return true;
  }
}

bool CorePhysicalConstants::update()
{
  return true;
}

bool CorePhysicalConstants::reset()
{
  physicalconstants_data.clear();
  physicalconstants_data.push_back("");
  physicalconstants_data.push_back("");
  physicalconstants_data.push_back("");

  init();
  return true;
}

bool CorePhysicalConstants::check_initialized()
{
  return is_initialized;
}

bool CorePhysicalConstants::modify_physicalconstants(std::vector<std::string> options, std::vector<int> options_marker)
{
  if (!domain_unit_view) {
    auto candidate = *this;
    std::string unitError;
    if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Display, unitError)) {
      PRINT_ERROR("%s\n", unitError.c_str());
      return false;
    }
    if (!candidate.modify_physicalconstants(options, options_marker)) {
      return false;
    }
    if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Input, unitError)) {
      PRINT_ERROR("%s\n", unitError.c_str());
      return false;
    }
    *this = std::move(candidate);
    return true;
  }

  for (size_t i = 0; i < options.size(); i++)
  {
    if (options_marker[i]==1)
    {
      physicalconstants_data[i] = options[i];
    }
  }
  return true;
}

bool CorePhysicalConstants::delete_physicalconstants(bool delete_absolute_zero, bool delete_stefan_boltzmann, bool delete_newton_gravity)
{
  if (delete_absolute_zero)
  {
    physicalconstants_data[0] = "";
  }
  if (delete_stefan_boltzmann)
  {
    physicalconstants_data[1] = "";
  }
  if (delete_newton_gravity)
  {
    physicalconstants_data[2] = "";
  }

  return true;
}


std::string CorePhysicalConstants::get_physicalconstants_export_data()
{
  if (!domain_unit_view) {
    auto candidate = *this;
    std::string unitError;
    if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Solver, unitError)) {
      PRINT_ERROR("%s\n", unitError.c_str());
      return {};
    }
    return candidate.get_physicalconstants_export_data();
  }

  CalculiXExportInterface ccx_export_iface;

  std::string str_temp = "";

  // CUSTOMLINE START
  std::vector<std::string> customline = ccx_export_iface.get_customline_data("BEFORE","PHYSICALCONSTANTS",0);
  for (size_t icl = 0; icl < customline.size(); icl++)
  {
    str_temp.append(customline[icl] + "\n");
  }
  // CUSTOMLINE END

  if ((physicalconstants_data[0]!="")||(physicalconstants_data[1]!="")||(physicalconstants_data[2]!=""))
  {
    str_temp.append("*PHYSICAL CONSTANTS");
  }
  if (physicalconstants_data[0]!="")
  {
    str_temp.append(",ABSOLUTE ZERO=" + physicalconstants_data[0]);
  }
  if (physicalconstants_data[1]!="")
  {
    str_temp.append(",STEFAN BOLTZMANN=" + physicalconstants_data[1]);
  }
  if (physicalconstants_data[2]!="")
  {
    str_temp.append(",NEWTON GRAVITY=" + physicalconstants_data[2]);
  }
  str_temp.append("\n");

  // CUSTOMLINE START
  customline = ccx_export_iface.get_customline_data("AFTER","PHYSICALCONSTANTS",0);
  for (size_t icl = 0; icl < customline.size(); icl++)
  {
    str_temp.append(customline[icl] + "\n");
  }
  // CUSTOMLINE END

  return str_temp;
}


std::string CorePhysicalConstants::print_data()
{
  std::string str_return;
  str_return = "\n CorePhysicalConstants physicalconstants_data: \n";
  str_return.append("absolute zero, stefan boltzmann, newton gravity \n");

  str_return.append(physicalconstants_data[0] + " " + physicalconstants_data[1] + " " + physicalconstants_data[2] + " \n");

  return str_return;
}

std::vector<std::vector<std::string>> CorePhysicalConstants::get_tree_data() const
{
  if (!domain_unit_view) {
    auto candidate = *this;
    std::string error;
    if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Display, error)) {
      PRINT_ERROR("%s\n", error.c_str());
      return {};
    }
    return candidate.get_tree_data();
  }

  if (physicalconstants_data.size() != 3)
  {
    return {};
  }

  return {
    {"1", "ABSOLUTE ZERO = " + physicalconstants_data[0]},
    {"2", "STEFAN BOLTZMANN = " + physicalconstants_data[1]},
    {"3", "NEWTON GRAVITY = " + physicalconstants_data[2]}
  };
}


bool CorePhysicalConstants::get_settings(int id, PhysicalConstantsSettings& settings, std::string& error) const
{
  if (!domain_unit_view) {
    auto candidate = *this;
    std::string unitError;
    if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Display, unitError)) {
      error = unitError;
      return false;
    }
    return candidate.get_settings(id, settings, error);
  }

  settings = {};
  error.clear();
  settings.id = id;
  if (physicalconstants_data.size() > 0) {
    settings.absoluteZero = physicalconstants_data[0];
  } else {
    settings.absoluteZero = std::string();
  }
  if (physicalconstants_data.size() > 1) {
    settings.stefanBoltzmann = physicalconstants_data[1];
  } else {
    settings.stefanBoltzmann = std::string();
  }
  if (physicalconstants_data.size() > 2) {
    settings.newtonGravity = physicalconstants_data[2];
  } else {
    settings.newtonGravity = std::string();
  }

  if (!error.empty())
  {
    settings = {};
    return false;
  }

  return true;
}
