#include "CoreResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Results/VtkResultsMetadataCodec.hpp"
#include <vtkTable.h>
#include <vtkFieldData.h>
#include <vtkStringArray.h>
#include <algorithm>
#include <cctype>
#include <map>
#include <set>

namespace
{
  std::string normalized(std::string value)
  {
    value.erase(std::remove_if(value.begin(),value.end(),[](unsigned char character)
    {
      return std::isspace(character) != 0;
    }),value.end());
    std::transform(value.begin(),value.end(),value.begin(),[](unsigned char character)
    {
      return static_cast<char>(std::toupper(character));
    });
    return value;
  }

  std::string lookup(const std::map<std::string,std::string>& quantities,const std::string& name)
  {
    const auto found = quantities.find(name);
    if (found != quantities.end())
    {
      return found->second;
    }
    return {};
  }

  // Only ordinary single-quantity CCX outputs belong here. Mixed outputs and
  // model-defined state/sensitivity variables must never inherit a field unit.
  std::string uniform_quantity(const SimResults::Field& field)
  {
    const auto name = normalized(field.name);
    if (name == "INTEGRATIONPOINTDISPLACEMENT" && field.role == SimResults::FieldRole::Displacement)
    {
      return "length";
    }
    if (field.source == "FRD")
    {
      static const std::map<std::string,std::string> quantities =
      {
        {"DISP","length"},{"DISPI","length"},{"MDISP","length"},
        {"DEPTH","length"},{"HCRIT","length"},{"DTIMF","time"},
        {"STRESS","stress"},{"STRESSI","stress"},{"MSTRESS","stress"},
        {"STRMID","stress"},{"STRNEG","stress"},{"STRPOS","stress"},
        {"VSTRES","stress"},{"ZZSTR","stress"},{"ZZSTRI","stress"},
        {"TOSTRAIN","strain"},{"TOSTRAII","strain"},
        {"MESTRAIN","strain"},{"MESTRAII","strain"},
        {"MESTRAI","strain"},{"MSTRAIN","strain"},{"THSTRAIN","strain"},
        {"PE","strain"},{"PEEQ","strain"},{"CEEQ","strain"},
        {"FORC","force"},{"FORCI","force"},
        {"NDTEMP","temperature"},{"STTEMP","temperature"},
        {"TOTEMP","temperature"},{"TS3DF","temperature"},{"TT3DF","temperature"},
        {"STPRES","pressure"},{"TOPRES","pressure"},
        {"PS3DF","pressure"},{"PT3DF","pressure"},
        {"CP3DF","dimensionless"},{"M3DF","dimensionless"},
        {"MAFLOW","mass_flow"},{"VELO","velocity"},{"V3DF","velocity"},
        {"ACCEL","acceleration"},{"RFL","power"},{"FLUX","heat_flux"},
        {"ENER","energy_density"},{"ELPOT","electric_potential"},
        {"CURR","electric_current_density"},{"EMFB","magnetic_flux_density"},
        {"EMFE","electric_field"}
      };
      return lookup(quantities,name);
    }
    if (field.source == "DAT")
    {
      static const std::map<std::string,std::string> quantities =
      {
        {"DISPLACEMENTS","length"},{"U","length"},
        {"TEMPERATURES","temperature"},{"NT","temperature"},{"TS","temperature"},
        {"STATICTEMPERATURES","temperature"},{"TSF","temperature"},
        {"TOTALTEMPERATURES","temperature"},{"TTF","temperature"},
        {"STATICPRESSURES","pressure"},{"PSF","pressure"},
        {"TOTALPRESSURES","pressure"},{"PTF","pressure"},
        {"NETWORKPRESSURES","pressure"},{"PN","pressure"},
        {"PRESSURECOEFFICIENTS","dimensionless"},{"CP","dimensionless"},
        {"MACHNUMBER","dimensionless"},{"MACH","dimensionless"},
        {"CHANGEINDEPTH","length"},{"DEPF","length"},
        {"MASSFLOWS","mass_flow"},{"MF","mass_flow"},
        {"FORCES","force"},{"RF","force"},
        {"HEATGENERATION","power"},{"RFL","power"},
        {"VELOCITIES","velocity"},{"V","velocity"},{"VF","velocity"},
        {"ACCELERATIONS","acceleration"},
        {"STRESSES","stress"},{"S","stress"},{"VISCOUSSTRESSES","stress"},{"SVF","stress"},
        {"STRAINS","strain"},{"E","strain"},{"MECHANICALSTRAINS","strain"},{"ME","strain"},
        {"ELASTICSTRAINS","strain"},{"PLASTICSTRAINS","strain"},
        {"EQUIVALENTPLASTICSTRAIN","strain"},{"PEEQ","strain"},{"CEEQ","strain"},
        {"INTERNALENERGYDENSITY","energy_density"},{"ENER","energy_density"},
        {"HEATFLUX","heat_flux"},{"HFL","heat_flux"},{"HFLF","heat_flux"},
        {"GLOBALCOORDINATES","length"},{"COORD","length"},
        {"INTERNALENERGY","energy"},{"ELSE","energy"},
        {"KINETICENERGY","energy"},{"ELKE","energy"},
        {"VOLUME","volume"},{"EVOL","volume"},
        {"BODYHEATING","power"},{"EBHE","power"},
        {"CENTRIFUGALFORCE","angular_velocity_squared"},{"CENT","angular_velocity_squared"}
      };
      return lookup(quantities,name);
    }
    return {};
  }

  // Names/layouts follow ccx 2.23 frd.c, crackfrd.c, frdfluidfem.f and
  // printout.f. Unknown components remain unresolved, never guessed by index.
  std::string component_quantity(const SimResults::Field& field,const std::string& component)
  {
    const auto name = normalized(field.name);
    const auto column = normalized(component);
    if (field.source == "FRD")
    {
      if (name == "CONTACT" || name == "CONTACTI")
      {
        static const std::map<std::string,std::string> contact =
        {
          {"COPEN","length"},{"CSLIP1","length"},{"CSLIP2","length"},
          {"CPRESS","stress"},{"CSHEAR1","stress"},{"CSHEAR2","stress"}
        };
        return lookup(contact,column);
      }
      if (name == "ERROR" || name == "ERRORI" || name == "HERROR" || name == "HERRORI")
      {
        return "relative_error";
      }
      if (name == "DUCT")
      {
        if (column == "DUCT")
        {
          return "dimensionless";
        }
        return {};
      }
      if (name == "PDISP" || name == "PFORC" || name == "PNDTEMP")
      {
        if (column == "PHA1" || ((name == "PDISP" || name == "PFORC") && (column == "PHA2" || column == "PHA3")))
        {
          return "phase_angle";
        }
        if (column == "MAG1" || ((name == "PDISP" || name == "PFORC") && (column == "MAG2" || column == "MAG3")))
        {
          if (name == "PDISP")
          {
            return "length";
          }
          if (name == "PFORC")
          {
            return "force";
          }
          // Harmonic amplitudes are temperature differences: no affine offset.
          return "temperature_difference";
        }
        return {};
      }
      if (name == "PSTRESS")
      {
        static const std::set<std::string> suffixes = {"XX","YY","ZZ","XY","YZ","ZX"};
        if (column.size() > 3 && suffixes.count(column.substr(3)) != 0)
        {
          if (column.compare(0,3,"MAG") == 0)
          {
            return "stress";
          }
          if (column.compare(0,3,"PHA") == 0)
          {
            return "phase_angle";
          }
        }
        return {};
      }
      if (name == "PCONTAC")
      {
        static const std::map<std::string,std::string> contact =
        {
          {"MAGO","length"},{"MAGSL1","length"},{"MAGSL2","length"},
          {"MAGP","stress"},{"MAGSH1","stress"},{"MAGSH2","stress"},
          {"PHAO","phase_angle"},{"PHASL1","phase_angle"},{"PHASL2","phase_angle"},
          {"PHAP","phase_angle"},{"PHASH1","phase_angle"},{"PHASH2","phase_angle"}
        };
        return lookup(contact,column);
      }
      if (name == "TURB3DF")
      {
        // frdfluidfem.f writes primitive vold(5)=k, vold(6)=omega,
        // k/omega and wall ratios, not the DAT conservative rho*k/rho*omega.
        static const std::map<std::string,std::string> turbulence =
        {
          {"K","turbulent_kinetic_energy"},{"OM","inverse_time"},
          {"NUT","kinematic_viscosity"},{"Y+","dimensionless"},{"U+","dimensionless"}
        };
        return lookup(turbulence,column);
      }
      if (name == "CT3D-MIS")
      {
        static const std::map<std::string,std::string> fracture =
        {
          {"DELTAKEQ","stress_intensity"},{"KEQMIN","stress_intensity"},
          {"KEQMAX","stress_intensity"},{"K1WORST","stress_intensity"},
          {"K2WORST","stress_intensity"},{"K3WORST","stress_intensity"},
          {"KTH","stress_intensity"},{"PHI(DEG)","phase_angle"},
          {"R","dimensionless"},{"INC","dimensionless"},{"CYCLES","dimensionless"},
          {"DADN","crack_growth_per_cycle"},{"CRLENGTH","length"}
        };
        return lookup(fracture,column);
      }
    }
    if (field.source == "DAT")
    {
      if (name == "MASS" || name == "EMAS")
      {
        static const std::map<std::string,std::string> mass =
        {
          {"MASS","mass"},{"XX","mass_moment"},{"YY","mass_moment"},
          {"ZZ","mass_moment"},{"XY","mass_moment"},{"XZ","mass_moment"},{"YZ","mass_moment"}
        };
        return lookup(mass,column);
      }
      if (name == "TURBULENCECONSERVATIVEVARIABLES" || name == "TURB")
      {
        static const std::map<std::string,std::string> turbulence =
        {
          {"TURBULENCE1","energy_density"},{"TURBULENCE2","turbulence_density_rate"}
        };
        return lookup(turbulence,column);
      }
    }
    return uniform_quantity(field);
  }

  bool resolve(const std::string& id,const SimUnits::SourceUnitContext& source,SimUnits::QuantityUnit& output,std::string& error)
  {
    error.clear();
    output = {};
    if (id.empty())
    {
      return true;
    }
    SimUnits::QuantityUnit quantity;
    if (!framework_units().quantity(id,quantity,error))
    {
      return false;
    }
    if ((quantity.dimension == SimUnits::Dimensionless && quantity.role == SimUnits::UnitRole::Linear) || id == "phase_angle")
    {
      // The solver writes percentages and phase degrees regardless of the
      // dimensional source system. Keep their native scale on resident arrays.
      output = quantity;
      return true;
    }
    if (!source.system.empty())
    {
      return framework_units().system_unit(source.system,id,output,error);
    }
    quantity.unit.clear();
    if (id == "length")
    {
      quantity.unit = source.coordinate_unit;
    }
    else if (id == "time")
    {
      quantity.unit = source.time_unit;
    }
    output = quantity;
    return true;
  }

  std::string table_text(vtkTable& table,const char* key)
  {
    auto* value = vtkStringArray::SafeDownCast(table.GetFieldData()->GetAbstractArray(key));
    if (value && value->GetNumberOfValues() > 0)
    {
      return value->GetValue(0);
    }
    return {};
  }

  std::string table_quantity(const std::string& field,const std::string& name,const std::string& column)
  {
    static const std::set<std::string> identifiers =
    {
      "RecordIndex","Valid","NodeId","ReportedNodeId","SlaveElementId","SlaveFace",
      "ElementId","Face","IntegrationPoint","Count","Mode","BucklingFactor"
    };
    if (identifiers.count(column) != 0)
    {
      return "dimensionless";
    }
    if (column == "Time")
    {
      return "time";
    }
    if (field == "PointMassHistory" || field == "SpringHistory")
    {
      if (column.find("force") != std::string::npos)
      {
        return "force";
      }
      return "length";
    }
    if (field == "DistributingHistory")
    {
      if (column.find("Mean displacement ") == 0)
      {
        return "length";
      }
      if (column.find("Force ") == 0)
      {
        return "force";
      }
      if (column.find("Moment ") == 0)
      {
        return "moment";
      }
      return "dimensionless";
    }
    if (field == "JointHistory")
    {
      if (column.find("Rotation") != std::string::npos || column == "Relative axial angle")
      {
        return "angle";
      }
      if (column.find("Translation") != std::string::npos || column == "Relative axial travel")
      {
        return "length";
      }
      if (column.find("Moment") != std::string::npos)
      {
        return "moment";
      }
      if (column.find("Force") != std::string::npos)
      {
        return "force";
      }
    }
    if (field == "FREQUENCY")
    {
      if (column == "Eigenvalue")
      {
        return "frequency_squared";
      }
      if (column == "Frequency")
      {
        return "frequency";
      }
      if (column == "AngularFrequencyReal" || column == "AngularFrequencyImaginary")
      {
        return "angular_velocity";
      }
    }
    if ((field == "ENER" || normalized(field) == "INTERNALENERGYDENSITY") && column == "Energy")
    {
      return "energy_density";
    }
    static const std::map<std::string,std::string> columns =
    {
      {"X","length"},{"Y","length"},{"Z","length"},
      {"CenterX","length"},{"CenterY","length"},{"CenterZ","length"},
      {"Fx","force"},{"Fy","force"},{"Fz","force"},
      {"NormalForce","force"},{"ShearForce","force"},
      {"MxOrigin","moment"},{"MyOrigin","moment"},{"MzOrigin","moment"},
      {"MxCenter","moment"},{"MyCenter","moment"},{"MzCenter","moment"},
      {"Torque","moment"},{"BendingMoment","moment"},
      {"NormalX","dimensionless"},{"NormalY","dimensionless"},{"NormalZ","dimensionless"},
      {"Area","area"},{"Energy","energy"},{"CELS","energy"},
      {"Volume","volume"},{"Mass","mass"},
      {"Ixx","mass_moment"},{"Iyy","mass_moment"},{"Izz","mass_moment"},
      {"Ixy","mass_moment"},{"Ixz","mass_moment"},{"Iyz","mass_moment"},
      {"Heat","power"},{"Heating","power"}
    };
    const auto known = lookup(columns,column);
    if (!known.empty())
    {
      return known;
    }
    if (field == "FLUX" && column == "Flux")
    {
      if (name == "total surface flux")
      {
        return "power";
      }
      return "heat_flux";
    }
    if ((field == "CSTR" && (column == "Pressure" || column == "Tangential1" || column == "Tangential2")) ||
        (field == "DRAG" && (column == "Tx" || column == "Ty" || column == "Tz" || column == "NormalStress" || column == "ShearStress")))
    {
      return "stress";
    }
    if (field == "CDIS" && (column == "Normal" || column == "Tangential1" || column == "Tangential2"))
    {
      return "length";
    }
    // Reuse physical component rules for field-style table columns, while
    // keeping table identifiers and integrated totals distinct above.
    SimResults::Field metadata;
    metadata.name = field;
    metadata.source = "DAT";
    return component_quantity(metadata,column);
  }

  bool tables(const std::vector<std::shared_ptr<vtkTable>>& inputs,std::vector<SimResults::ResultsTableMetadata>& metadata,
    const SimUnits::SourceUnitContext& source,std::string& error,const std::string& axis_quantity = {})
  {
    metadata.resize(inputs.size());
    for (std::size_t i = 0; i < inputs.size(); ++i)
    {
      if (!inputs[i])
      {
        continue;
      }
      metadata[i].name = table_text(*inputs[i],"TableName");
      const auto field = table_text(*inputs[i],"Field");
      metadata[i].columns.assign(static_cast<std::size_t>(inputs[i]->GetNumberOfColumns()),SimUnits::QuantityUnit());
      for (vtkIdType column = 0; column < inputs[i]->GetNumberOfColumns(); ++column)
      {
        const auto* rawName = inputs[i]->GetColumnName(column);
        if (!rawName)
        {
          continue;
        }
        const std::string columnName(rawName);
        auto quantity = table_quantity(field,metadata[i].name,columnName);
        if (table_text(*inputs[i],"ReferencePointRole") == "Rotation")
        {
          if (quantity == "length")
          {
            quantity = "angle";
          }
          else if (quantity == "force")
          {
            quantity = "moment";
          }
        }
        if (columnName == "Time" && (field != "DistributingHistory" || !axis_quantity.empty()))
        {
          quantity = axis_quantity;
        }
        if (!resolve(quantity,source,metadata[i].columns[static_cast<std::size_t>(column)],error))
        {
          return false;
        }
      }
    }
    return true;
  }
}

bool CoreResultsUnits::annotate(SimResults::VtkResultsData& result,std::string& error,const std::map<int,std::string>& step_axis_quantities)
{
  error.clear();
  if (!framework_units().validate_source(result.source_units,error))
  {
    return false;
  }
  for (auto& frame : result.increments)
  {
    frame.coordinate_unit = result.source_units.coordinate_unit;
    frame.independent_variable = {};
    const auto axis = step_axis_quantities.find(frame.step);
    if (axis != step_axis_quantities.end())
    {
      if ((axis->second != "time" && axis->second != "frequency") ||
          !resolve(axis->second,result.source_units,frame.independent_variable,error))
      {
        error = "Invalid result-axis quantity: " + axis->second;
        return false;
      }
    }
    for (auto& field : frame.fields)
    {
      if (field.categorical)
      {
        field.component_units.assign(field.components.size(),SimUnits::QuantityUnit("","1",SimUnits::Dimensionless));
        continue;
      }
      std::vector<SimUnits::QuantityUnit> units(field.components.size());
      for (std::size_t index = 0; index < field.components.size(); ++index)
      {
        if (!resolve(component_quantity(field,field.components[index]),result.source_units,units[index],error))
        {
          return false;
        }
      }
      field.component_units.swap(units);
    }
    if (!tables(frame.tables,frame.table_metadata,result.source_units,error,frame.independent_variable.quantity))
    {
      return false;
    }
  }
  if (!tables(result.tables,result.table_metadata,result.source_units,error))
  {
    return false;
  }
  return SimResults::VtkResultsMetadataCodec::write(result,error);
}
