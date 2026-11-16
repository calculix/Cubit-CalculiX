#include "Core/results/CoreResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include "Framework/Results/VtkResultsMetadataCodec.hpp"
#include <QCoreApplication>
#include <vtkDataObject.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVariant.h>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <utility>

namespace
{
  int failures = 0;
  void check(bool accepted,const char* message)
  {
    if (!accepted)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  }
  struct Fixture
  {
    SimResults::VtkResultsData result;
    vtkUnstructuredGrid* mesh;
    Fixture()
    {
      mesh = vtkUnstructuredGrid::New();
      result.increments.emplace_back();
      result.increments[0].dataset = std::shared_ptr<vtkDataObject>(mesh,[](vtkDataObject* value)
      {
        value->Delete();
      });
      vtkNew<vtkPoints> points;
      points->InsertNextPoint(0,0,0);
      mesh->SetPoints(points);
      result.increments[0].step = 1;
    }
    std::size_t field(const std::string& source,const std::string& name,const std::vector<std::string>& components)
    {
      auto& fields = result.increments[0].fields;
      SimResults::Field field;
      field.name = name;
      field.source = source;
      field.components = components;
      field.array_name = "values"+std::to_string(fields.size());
      if (name == "IntegrationPointDisplacement")
      {
        field.role = SimResults::FieldRole::Displacement;
      }
      vtkNew<vtkDoubleArray> array;
      array->SetName(field.array_name.c_str());
      array->SetNumberOfComponents(static_cast<int>(components.size()));
      array->SetNumberOfTuples(1);
      for (int i = 0; i < static_cast<int>(components.size()); ++i)
      {
        array->SetComponent(0,i,42.0+i);
      }
      mesh->GetPointData()->AddArray(array);
      fields.push_back(std::move(field));
      return fields.size()-1;
    }
    const SimUnits::QuantityUnit& quantity(std::size_t field,std::size_t component = 0) const
    {
      return result.increments[0].fields.at(field).component_units.at(component);
    }
    void source(const std::string& system)
    {
      result.source_units.system = system;
      result.source_units.coordinate_unit = "mm";
      result.source_units.time_unit = "s";
    }
  };
  std::shared_ptr<vtkTable> table(const char* key,const std::vector<std::string>& columns)
  {
    auto* data = vtkTable::New();
    std::shared_ptr<vtkTable> owner(data,[](vtkTable* value)
    {
      value->Delete();
    });
    vtkNew<vtkStringArray> field;
    field->SetName("Field");
    field->InsertNextValue(key);
    data->GetFieldData()->AddArray(field);
    for (const auto& name : columns)
    {
      vtkNew<vtkDoubleArray> array;
      array->SetName(name.c_str());
      array->InsertNextValue(7);
      data->AddColumn(array);
    }
    return owner;
  }
  void mixed_outputs()
  {
    Fixture fixture;
    fixture.source("N-mm-s");
    const auto contact = fixture.field("FRD","CONTACT",{"COPEN","CSLIP1","CPRESS","CSHEAR2","Unrecognized"});
    const auto phase = fixture.field("FRD","PCONTAC",{"MAGO","MAGP","PHAO","PHAP"});
    const auto temperature = fixture.field("FRD","PNDTEMP",{"MAG1","PHA1"});
    const auto errorField = fixture.field("FRD","ERROR",{"STR(%)"});
    const auto mass = fixture.field("DAT","mass",{"mass","xx","yz"});
    const auto fracture = fixture.field("FRD","CT3D-MIS",{"DeltaKEQ","PHI(DEG)","CRLENGTH","DADN","DOM_SLIP"});
    const auto turbulence = fixture.field("FRD","TURB3DF",{"K","OM","NUT","Y+","U+"});
    const auto conservative = fixture.field("DAT","turbulence conservative variables",{"turbulence1","turbulence2"});
    auto* resident = fixture.mesh->GetPointData()->GetArray("values0");
    const double* buffer = vtkDoubleArray::SafeDownCast(resident)->GetPointer(0);
    std::string error;
    if (!framework_units().set_workspace_system("postprocessor","SI",error) || !CoreResultsUnits::annotate(fixture.result,error,{{1,"frequency"}}))
    {
      check(false,error.c_str());
      return;
    }
    check(fixture.quantity(contact,0).unit == "mm" && fixture.quantity(contact,1).quantity == "length" &&
      fixture.quantity(contact,2).quantity == "stress" && fixture.quantity(contact,2).unit == "N/mm^2" &&
      fixture.quantity(contact,3).dimension == SimUnits::Stress,"CONTACT lost mixed native quantities");
    check(fixture.quantity(contact,4).quantity.empty(),"Unknown contact component inherited a field-wide unit");
    check(fixture.quantity(phase,0).quantity == "length" && fixture.quantity(phase,1).quantity == "stress" &&
      fixture.quantity(phase,2).unit == "deg" && fixture.quantity(phase,3).role == SimUnits::UnitRole::Angle,"Contact phase units are incorrect");
    check(fixture.quantity(temperature,0).role == SimUnits::UnitRole::TemperatureDifference &&
      fixture.quantity(temperature,1).unit == "deg","Temperature amplitude acquired an absolute offset or phase lost degrees");
    check(fixture.quantity(errorField).unit == "%","Native percentage scale was lost");
    check(fixture.quantity(mass,0).quantity == "mass" && fixture.quantity(mass,1).quantity == "mass_moment" &&
      fixture.quantity(mass,2).quantity == "mass_moment","Mass and inertia were assigned one quantity");
    check(fixture.quantity(fracture,0).quantity == "stress_intensity" && fixture.quantity(fracture,1).unit == "deg" &&
      fixture.quantity(fracture,2).quantity == "length" && fixture.quantity(fracture,3).quantity == "crack_growth_per_cycle" &&
      fixture.quantity(fracture,4).unit.empty(),"Fracture components were not classified individually");
    check(fixture.quantity(turbulence,0).quantity == "turbulent_kinetic_energy" &&
      fixture.quantity(turbulence,1).quantity == "inverse_time" && fixture.quantity(turbulence,2).quantity == "kinematic_viscosity" &&
      fixture.quantity(turbulence,3).unit == "1" && fixture.quantity(turbulence,4).unit == "1","Primitive turbulence components were not classified individually");
    check(fixture.quantity(conservative,0).quantity == "energy_density" &&
      fixture.quantity(conservative,1).quantity == "turbulence_density_rate","DAT conservative turbulence was confused with FRD primitive variables");
    double canonical = 0;
    check(framework_units().to_si(1,fixture.quantity(fracture,0).unit,fixture.quantity(fracture,0).dimension,
      fixture.quantity(fracture,0).role,canonical,error) && std::abs(canonical-std::pow(1000.0,1.5)) < 1e-6,"Fracture native length scale is incorrect");
    check(buffer == vtkDoubleArray::SafeDownCast(resident)->GetPointer(0) && resident->GetComponent(0,0) == 42 &&
      resident->GetComponent(0,3) == 45,"Unit annotation modified resident result values or replaced arrays");
    check(fixture.result.increments[0].independent_variable.quantity == "frequency","Result-axis provenance was lost");
    auto restored = fixture.result;
    restored.increments[0].fields.clear();
    check(SimResults::VtkResultsMetadataCodec::read(restored,error) && !restored.increments[0].fields.empty() &&
      restored.increments[0].fields[contact].component_units[2].quantity == "stress","Component units did not survive VTK metadata serialization");
    double displayed = 0;
    std::string label;
    check(SimResults::ResultsUnits::display_value(5,fixture.quantity(errorField),"postprocessor",displayed,label,error) &&
      displayed == 5 && label == "%","Default presentation reinterpreted percentage values");
    check(framework_units().set_display_unit("postprocessor","relative_error","1",error) &&
      SimResults::ResultsUnits::display_value(5,fixture.quantity(errorField),"postprocessor",displayed,label,error) &&
      std::abs(displayed-0.05) < 1e-12,"Percentage-to-fraction conversion changed the wrong numeric scale");
    check(framework_units().set_display_unit("postprocessor","temperature_difference","delta_degC",error) &&
      SimResults::ResultsUnits::display_value(10,fixture.quantity(temperature,0),"postprocessor",displayed,label,error) &&
      displayed == 10,"Temperature oscillation amplitude received an affine Celsius offset");
    check(framework_units().set_display_unit("postprocessor","phase_angle","rad",error) &&
      SimResults::ResultsUnits::display_value(90,fixture.quantity(phase,2),"postprocessor",displayed,label,error) &&
      std::abs(displayed-std::acos(-1.0)/2) < 1e-12,"Native phase degrees were interpreted as radians");
  }
  void aliases_and_missing_sources()
  {
    Fixture fixture;
    const std::vector<std::pair<std::string,std::string>> aliases =
    {
      {"DISPI","length"},{"STRESSI","stress"},{"TOSTRAII","strain"},{"MESTRAII","strain"},
      {"PE","strain"},{"MSTRAIN","strain"},{"THSTRAIN","strain"},{"ZZSTRI","stress"},
      {"STRMID","stress"},{"STRNEG","stress"},{"STRPOS","stress"},{"VSTRES","stress"},
      {"STTEMP","temperature"},{"TOTEMP","temperature"},{"TS3DF","temperature"},{"TT3DF","temperature"},
      {"STPRES","pressure"},{"TOPRES","pressure"},{"PS3DF","pressure"},{"PT3DF","pressure"},
      {"MAFLOW","mass_flow"},{"V3DF","velocity"},{"DTIMF","time"},{"HCRIT","length"},
      {"ELPOT","electric_potential"},{"CURR","electric_current_density"},{"EMFB","magnetic_flux_density"},{"EMFE","electric_field"}
    };
    for (const auto& alias : aliases)
    {
      fixture.field("FRD",alias.first,{"value"});
    }
    const std::vector<std::pair<std::string,std::string>> datAliases =
    {
      {"static temperatures","temperature"},{"total temperatures","temperature"},
      {"static pressures","pressure"},{"total pressures","pressure"},{"network pressures","pressure"},
      {"pressure coefficients","dimensionless"},{"change in depth","length"},{"mass flows","mass_flow"},
      {"heat generation","power"},{"viscous stresses","stress"},{"internal energy density","energy_density"},
      {"heat flux","heat_flux"},{"global coordinates","length"},{"internal energy","energy"},
      {"kinetic energy","energy"},{"volume","volume"},{"body heating","power"},
      {"centrifugal force","angular_velocity_squared"}
    };
    const auto datBegin = fixture.result.increments[0].fields.size();
    for (const auto& alias : datAliases)
    {
      fixture.field("DAT",alias.first,{"value"});
    }
    const auto unknown = fixture.field("FRD","SDV",{"SXX"});
    const auto strain = fixture.field("DAT","equivalent plastic strain",{"pe"});
    const auto mach = fixture.field("DAT","mach number",{"mach"});
    const auto phase = fixture.field("FRD","PDISP",{"MAG1","PHA1"});
    const auto errorField = fixture.field("FRD","HERRORI",{"TEM(%)"});
    std::string error;
    check(CoreResultsUnits::annotate(fixture.result,error),"Cannot annotate outputs without dimensional source provenance");
    for (std::size_t i = 0; i < aliases.size(); ++i)
    {
      check(fixture.quantity(i).quantity == aliases[i].second,"A documented alias has no physical quantity");
    }
    for (std::size_t i = 0; i < datAliases.size(); ++i)
    {
      check(fixture.quantity(datBegin+i).quantity == datAliases[i].second,"A descriptive DAT field name has no physical quantity");
    }
    check(fixture.quantity(unknown).quantity.empty() && fixture.quantity(unknown).unit.empty(),"State-variable names were guessed as physical stresses");
    check(fixture.quantity(strain).unit == "1" && fixture.quantity(mach).unit == "1","Dimensionless units were lost without a source system");
    check(fixture.quantity(phase,0).unit.empty() && fixture.quantity(phase,1).unit == "deg" &&
      fixture.quantity(errorField).unit == "%","Missing source provenance was guessed or intrinsic scales were lost");
    check(fixture.quantity(1).dimension == SimUnits::Stress && fixture.quantity(1).unit.empty(),"Known stress quantity lost its dimension or gained guessed units");
    fixture.result.source_units.coordinate_unit = "mm";
    check(CoreResultsUnits::annotate(fixture.result,error) && fixture.quantity(0).unit == "mm" &&
      fixture.quantity(1).unit.empty(),"Partial coordinate provenance was ignored or used to invent stress units");
    fixture.source("N-mm-s");
    check(CoreResultsUnits::annotate(fixture.result,error),"Cannot reannotate aliases with source provenance");
    for (std::size_t i = 0; i < aliases.size(); ++i)
    {
      check(!fixture.quantity(i).unit.empty(),"A documented alias did not resolve its native unit system");
    }
  }
  void quantity_presets()
  {
    const std::vector<std::string> ids =
    {
      "relative_error","phase_angle","mass_flow","angular_velocity_squared","electric_potential",
      "electric_field","electric_current_density","magnetic_flux_density","stress_intensity",
      "turbulent_kinetic_energy","turbulence_density_rate","crack_growth_per_cycle"
    };
    std::string error;
    for (const auto& system : framework_units().system_ids())
    {
      for (const auto& id : ids)
      {
        SimUnits::QuantityUnit quantity;
        double canonical = 0;
        check(framework_units().system_unit(system,id,quantity,error) &&
          framework_units().to_si(1,quantity.unit,quantity.dimension,quantity.role,canonical,error) &&
          std::isfinite(canonical) && canonical > 0,"A new quantity cannot be represented in a supported unit preset");
      }
    }
  }
  void table_units()
  {
    Fixture fixture;
    fixture.source("N-mm-s");
    fixture.result.increments[0].tables =
    {
      table("CSTR",{"SlaveElementId","Pressure","Tangential1","Mystery"}),
      table("CDIS",{"ReportedNodeId","Normal","Tangential2"}),
      table("EMAS",{"ElementId","Mass","Ixx"}),
      table("FREQUENCY",{"Mode","Eigenvalue","AngularFrequencyReal","Frequency"}),
      table("ENER",{"Energy"})
    };
    fixture.result.tables = {table("ELSE",{"Energy","Valid"})};
    std::string error;
    check(CoreResultsUnits::annotate(fixture.result,error),"Cannot annotate DAT tables");
    const auto& metadata = fixture.result.increments[0].table_metadata;
    check(metadata[0].columns[0].unit == "1" && metadata[0].columns[1].quantity == "stress" &&
      metadata[0].columns[2].quantity == "stress" && metadata[0].columns[3].quantity.empty(),"Contact table identifiers or unknown columns inherited stress units");
    check(metadata[1].columns[1].quantity == "length" && metadata[1].columns[2].quantity == "length","Contact displacement table lost length units");
    check(metadata[2].columns[1].quantity == "mass" && metadata[2].columns[2].quantity == "mass_moment","Mass table conflated mass and inertia");
    check(metadata[3].columns[0].unit == "1" && metadata[3].columns[1].quantity == "frequency_squared" &&
      metadata[3].columns[2].quantity == "angular_velocity" && metadata[3].columns[3].quantity == "frequency","Frequency table units are inconsistent");
    check(metadata[4].columns[0].quantity == "energy_density" && fixture.result.table_metadata[0].columns[0].quantity == "energy",
      "Energy density and integrated energy share an incorrect table unit");
    check(fixture.result.increments[0].tables[0]->GetValue(0,1).ToDouble() == 7,"Table unit annotation changed source numbers");
  }
}
int main(int argc,char** argv)
{
  QCoreApplication application(argc,argv);
  mixed_outputs();
  aliases_and_missing_sources();
  table_units();
  quantity_presets();
  if (failures != 0)
  {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
