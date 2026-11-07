#ifndef CORERESULTSDATFORMAT_HPP
#define CORERESULTSDATFORMAT_HPP

#include "Core/results/dat/CoreResultsDatStructs.hpp"
#include "Core/results/ResultReadNumbers.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

// CCX 2.23 printout*.f formats. Recognition is independent of GUI and VTK.
namespace CoreResultsDatFormat
{
  enum class Kind
  {
    Unknown,
    Field,
    Table,
    Section,
    ContactStatistics
  };

  struct Header
  {
    Kind kind = Kind::Unknown;
    int association = 0;
    bool variable = false;
    DatResultTable table;
    std::vector<std::string> components;
  };

  inline std::string normalize(std::string_view line)
  {
    std::string result;
    for (const char c : ResultReadNumbers::trim(line))
    {
      if (ResultReadNumbers::whitespace(c))
      {
        if (!result.empty() && result.back() != ' ')
        {
          result.push_back(' ');
        }
      }
      else
      {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
      }
    }
    return result;
  }

  inline bool numeric(std::string_view line, std::vector<double>& values)
  {
    values.clear();
    std::size_t position = 0;
    std::string_view token;
    while (ResultReadNumbers::next_field(line, position, token))
    {
      double value = 0.0;
      if (!ResultReadNumbers::result_value(token, value))
      {
        values.clear();
        return false;
      }
      values.push_back(value);
    }
    return !values.empty();
  }

  inline bool local_vector_type(std::string_view type)
  {
    return type == "displacements" || type == "forces" || type == "velocities";
  }

  inline bool frequency_title(std::string_view source)
  {
    std::string compact;
    for (char c : normalize(source))
    {
      if (!ResultReadNumbers::whitespace(c))
      {
        compact.push_back(c);
      }
    }
    return compact == "eigenvalueoutput";
  }

  inline std::vector<std::string> frequency_columns()
  {
    return {"Mode", "Eigenvalue", "AngularFrequencyReal", "Frequency", "AngularFrequencyImaginary"};
  }

  // Standard symmetric CCX 2.23 writeev.f layout. Complex/cyclic variants have
  // different columns and must not be inferred from numeric record width.
  inline bool frequency_heading(DatReadState::FrequencyState state, std::string_view source)
  {
    const auto line = normalize(source);
    if (state == DatReadState::FrequencyState::Columns)
    {
      return line == "mode no eigenvalue frequency";
    }
    if (state == DatReadState::FrequencyState::Parts)
    {
      return line == "real part imaginary part";
    }
    std::string units;
    for (char c : line)
    {
      if (c != '(' && c != ')')
      {
        units.push_back(c);
      }
    }
    return state == DatReadState::FrequencyState::Units && normalize(units) == "rad/time cycles/time rad/time";
  }

  inline bool frequency_row(std::string_view source, std::vector<double>& values)
  {
    return numeric(source, values) && values.size() == 5 &&
      std::all_of(values.begin(), values.end(), [](double value)
      {
        return std::isfinite(value);
      }) &&
      values[0] >= 1 && values[0] <= std::numeric_limits<int>::max() && values[0] == std::floor(values[0]);
  }

  inline std::vector<std::string> section_columns()
  {
    return {"Fx", "Fy", "Fz", "MxOrigin", "MyOrigin", "MzOrigin",
      "CenterX", "CenterY", "CenterZ", "NormalX", "NormalY", "NormalZ",
      "MxCenter", "MyCenter", "MzCenter", "Area", "NormalForce", "ShearForce", "Torque", "BendingMoment"};
  }

  // CCX 2.23 contact statistics have four records of 6, 6, 3 and 3 values.
  // Section statistics additionally report torque and bending moment.
  inline std::vector<std::string> contact_statistics_columns()
  {
    return {"Fx", "Fy", "Fz", "MxOrigin", "MyOrigin", "MzOrigin",
      "CenterX", "CenterY", "CenterZ", "NormalX", "NormalY", "NormalZ",
      "MxCenter", "MyCenter", "MzCenter", "Area", "NormalForce", "ShearForce"};
  }

  inline bool identifiers(const std::vector<std::string>& columns, const std::vector<double>& values)
  {
    if (columns.size() != values.size())
    {
      return false;
    }
    for (std::size_t i = 0; i < columns.size(); ++i)
    {
      const auto& name = columns[i];
      const bool id = name == "NodeId" || name == "ReportedNodeId" || name == "SlaveElementId" || name == "SlaveFace" ||
        name == "ElementId" || name == "Face" || name == "IntegrationPoint";
      if ((id || name == "Count") && (!std::isfinite(values[i]) || values[i] != std::floor(values[i]) ||
        values[i] < (id ? 1 : 0) || values[i] > std::numeric_limits<int>::max()))
      {
        return false;
      }
      const bool coordinate = name == "X" || name == "Y" || name == "Z" ||
        name == "CenterX" || name == "CenterY" || name == "CenterZ";
      if (coordinate && !std::isfinite(values[i]))
      {
        return false;
      }
    }
    return true;
  }

  /** Recognize by header, never by numeric row width. Set/pair names preserve
   * their original spelling. A variable field fixes its component count from
   * its first row and requires that same count on subsequent rows.
   */
  inline Header header(std::string_view source)
  {
    Header result;
    const std::string original(ResultReadNumbers::trim(source));
    const std::string line = normalize(source);
    const auto time = line.rfind("time ");
    if (time == std::string::npos || !ResultReadNumbers::real(line.substr(time + 5), result.table.time))
    {
      return result;
    }
    const auto set = line.find("for set ");
    const auto and_time = line.rfind(" and time ");
    if (set != std::string::npos && and_time != std::string::npos && set + 8 <= and_time)
    {
      // Positions refer to normalized whitespace, but original case is retained
      // by finding the same delimiters in a whitespace-normalized original.
      std::string spaced;
      for (char c : ResultReadNumbers::trim(source))
      {
        if (ResultReadNumbers::whitespace(c))
        {
          if (!spaced.empty() && spaced.back() != ' ')
          {
            spaced.push_back(' ');
          }
        }
        else
        {
          spaced.push_back(c);
        }
      }
      result.table.set_name = spaced.substr(set + 8, and_time - set - 8);
    }
    auto name = line.substr(0, set == std::string::npos ? time : set);
    const auto open = name.find('(');
    if (open != std::string::npos)
    {
      name.resize(open);
    }
    name = std::string(ResultReadNumbers::trim(name));
    result.table.name = name;

    if (line.find("statistics for surface set ") == 0)
    {
      result.kind = Kind::Section;
      return result;
    }
    if (line.find("statistics for slave set ") == 0)
    {
      result.kind = Kind::ContactStatistics;
      result.table.key = "CF";
      result.table.columns = contact_statistics_columns();
      const auto master = line.find(", master set ");
      if (master != std::string::npos && and_time != std::string::npos)
      {
        // Pair identifiers are read from the original text with normalized spaces.
        const auto slave_begin = original.find("set ");
        const auto master_begin = original.find(", master set ");
        const auto end = original.rfind(" and time ");
        if (slave_begin != std::string::npos && master_begin != std::string::npos && end != std::string::npos)
        {
          result.table.slave_set = original.substr(slave_begin + 4, master_begin - slave_begin - 4);
          result.table.master_set = original.substr(master_begin + 13, end - master_begin - 13);
        }
      }
      return result;
    }

    // Contact PRINT uses separate element and face columns for face-to-face
    // penalty contact (printoutelem.f), unlike the FRD reported-node label.
    if (line.find("contact stress (") == 0 || line.find("relative contact displacement (") == 0 || line.find("contact spring energy (") == 0)
    {
      result.kind = Kind::Table;
      const bool face = line.find("slave element+face") != std::string::npos;
      result.table.columns = face ? std::vector<std::string>{"SlaveElementId", "SlaveFace"} : std::vector<std::string>{"ReportedNodeId"};
      if (line.find("contact stress (") == 0)
      {
        result.table.key = "CSTR";
        result.table.columns.insert(result.table.columns.end(), {"Pressure", "Tangential1", "Tangential2"});
      }
      else if (line.find("relative contact displacement (") == 0)
      {
        result.table.key = "CDIS";
        result.table.columns.insert(result.table.columns.end(), {"Normal", "Tangential1", "Tangential2"});
      }
      else
      {
        result.table.key = "CELS";
        result.table.columns.push_back("CELS");
      }
      return result;
    }
    if (line.find("heat flux at the integration points for set ") == 0)
    {
      result.kind = Kind::Table;
      result.table.key = "FLUX";
      result.table.columns = {"ElementId", "Face", "IntegrationPoint", "Flux", "X", "Y", "Z"};
      return result;
    }
    if (line.find("surface stress vector (") == 0)
    {
      result.kind = Kind::Table;
      result.table.key = "DRAG";
      // printoutfacefem.f writes nelem,ig,i: integration point before face.
      result.table.columns = {"ElementId", "IntegrationPoint", "Face", "Tx", "Ty", "Tz", "NormalStress", "ShearStress", "X", "Y", "Z"};
      return result;
    }

    struct FieldFormat
    {
      const char* name;
      const char* key;
      int association;
      std::vector<std::string> components;
    };
    const std::vector<FieldFormat> formats = {
      {"displacements", "U", 1, {"vx", "vy", "vz"}},
      {"temperatures", "NT", 1, {"temperature"}},
      {"static temperatures", "TSF", 1, {"temperature"}},
      {"total temperatures", "TTF", 1, {"temperature"}},
      {"static pressures", "PSF", 1, {"pressure"}},
      {"total pressures", "PTF", 1, {"pressure"}},
      {"network pressures", "PN", 1, {"pressure"}},
      {"pressure coefficients", "CP", 1, {"cp"}},
      {"mach number", "MACH", 1, {"mach"}},
      {"change in depth", "DEPF", 1, {"depth"}},
      {"turbulence conservative variables", "TURB", 1, {"turbulence1", "turbulence2"}},
      {"mass flows", "MF", 1, {"massflow"}},
      {"forces", "RF", 1, {"fx", "fy", "fz"}},
      {"heat generation", "RFL", 1, {"heat"}},
      {"velocities", "VF", 1, {"vx", "vy", "vz"}},
      {"stresses", "S", 2, {"sxx", "syy", "szz", "sxy", "sxz", "syz"}},
      {"viscous stresses", "SVF", 2, {"sxx", "syy", "szz", "sxy", "sxz", "syz"}},
      {"strains", "E", 2, {"exx", "eyy", "ezz", "exy", "exz", "eyz"}},
      {"mechanical strains", "ME", 2, {"exx", "eyy", "ezz", "exy", "exz", "eyz"}},
      {"equivalent plastic strain", "PEEQ", 2, {"pe"}},
      {"internal energy density", "ENER", 2, {"energy"}},
      {"internal state variables", "SDV", 2, {}},
      {"heat flux", "HFL", 2, {"qx", "qy", "qz"}},
      {"global coordinates", "COORD", 2, {"x", "y", "z"}},
      {"ductile damage initiation", "DUCT", 2, {"x", "y", "z"}},
      {"internal energy", "ELSE", 3, {"energy"}},
      {"kinetic energy", "ELKE", 3, {"energy"}},
      {"volume", "EVOL", 3, {"volume"}},
      {"mass", "EMAS", 3, {"mass", "xx", "yy", "zz", "xy", "xz", "yz"}},
      {"body heating", "EBHE", 3, {"heating"}},
      {"centrifugal force", "CENT", 3, {"omega_square"}}
    };
    for (const auto& format : formats)
    {
      if (name == format.name)
      {
        result.kind = Kind::Field;
        result.association = format.association;
        result.table.key = format.key;
        result.components = format.components;
        result.variable = result.components.empty() || line.find("v(i),i=1..ndof") != std::string::npos;
        if (result.variable)
        {
          result.components.clear();
        }
        if (result.association >= 2)
        {
          result.components.insert(result.components.begin(), "element");
        }
        if (result.association == 2)
        {
          result.components.insert(result.components.begin() + 1, "integ.pnt.");
        }
        return result;
      }
    }
    struct SummaryFormat
    {
      const char* prefix;
      const char* key;
      std::vector<std::string> columns;
    };
    const std::vector<SummaryFormat> summaries = {
      {"total force", "RF", {"Fx", "Fy", "Fz"}},
      {"total heat generation", "RFL", {"Heat"}},
      {"total internal energy", "ELSE", {"Energy"}},
      {"total kinetic energy", "ELKE", {"Energy"}},
      {"total volume", "EVOL", {"Volume"}},
      {"total mass", "EMAS", {"Mass"}},
      {"total body heating", "EBHE", {"Heating"}},
      {"center of gravity", "EMAS", {"X", "Y", "Z"}},
      {"total contact spring energy", "CELS", {"Energy"}},
      {"total number of contact elements", "CNUM", {"Count"}},
      {"total surface flux", "FLUX", {"Flux"}},
      {"total surface force", "DRAG", {"Fx", "Fy", "Fz"}}
    };
    if (line.find("total mass moment of inertia") == 0)
    {
      result.kind = Kind::Table;
      result.table.key = "EMAS";
      result.table.columns = {"Ixx", "Iyy", "Izz", "Ixy", "Ixz", "Iyz"};
      return result;
    }
    for (const auto& summary : summaries)
    {
      const std::string prefix(summary.prefix);
      if (line.compare(0, prefix.size(), prefix) == 0 &&
        (line.size() == prefix.size() || line[prefix.size()] == ' ' || line[prefix.size()] == '('))
      {
        result.kind = Kind::Table;
        result.table.key = summary.key;
        result.table.columns = summary.columns;
        return result;
      }
    }
    return result;
  }
}

#endif // CORERESULTSDATFORMAT_HPP
