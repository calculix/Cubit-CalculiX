#include "CoreResultsInpReader.hpp"
#include "CoreResultsVtkElements.hpp"
#include "Core/results/ResultReadNumbers.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <system_error>
#include <exception>

namespace
{
  std::string trim(const std::string& value)
  {
    return std::string(ResultReadNumbers::trim(value));
  }

  std::string upper(std::string value)
  {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
    {
      return static_cast<char>(std::toupper(c));
    });
    return value;
  }

  bool csv(const std::string& line, std::vector<std::string>& fields)
  {
    fields.clear();
    std::string field;
    char quote = 0;
    for (char c : line)
    {
      if (quote != 0)
      {
        if (c == quote)
        {
          quote = 0;
        }
        else
        {
          field += c;
        }
      }
      else if (c == '\'' || c == '"')
      {
        quote = c;
      }
      else if (c == ',')
      {
        fields.push_back(trim(field));
        field.clear();
      }
      else
      {
        field += c;
      }
    }
    fields.push_back(trim(field));
    return quote == 0;
  }

  struct SetDefinition
  {
    std::string name;
    std::vector<int> ids;
    std::vector<std::string> references;
  };

  struct SurfaceDefinition
  {
    std::string name;
    std::vector<std::pair<std::string, std::string>> entries;
  };

  class Parser
  {
  public:
    std::map<int, CoreResultsVtkNode> nodes;
    std::map<int, CoreResultsVtkElement> elements;
    std::map<std::string, SetDefinition> node_sets;
    std::map<std::string, SetDefinition> element_sets;
    std::map<std::string, SurfaceDefinition> surfaces;
    std::vector<std::string> files;
    std::string error;

    bool file(const std::filesystem::path& filename)
    {
      std::error_code ec;
      const auto path = std::filesystem::canonical(filename, ec);
      if (ec || active.size() >= 32 || active.count(path) != 0)
      {
        error = "Cannot read INP/include (missing file, recursion or depth limit): " + filename.string();
        return false;
      }
      std::ifstream stream(path);
      if (!stream)
      {
        error = "Cannot open INP file: " + path.string();
        return false;
      }
      active.insert(path);
      files.push_back(path.string());
      std::string line;
      std::string header;
      std::size_t line_number = 0;
      bool success = true;
      while (std::getline(stream, line))
      {
        ++line_number;
        line = trim(line);
        if (line.empty() || line.rfind("**", 0) == 0)
        {
          continue;
        }
        if (!header.empty())
        {
          header += line;
        }
        else if (line.front() == '*')
        {
          header = line;
        }
        if (!header.empty())
        {
          if (header.back() == ',')
          {
            continue;
          }
          success = keyword(header, path.parent_path());
          header.clear();
        }
        else
        {
          success = record(line);
        }
        if (!success)
        {
          error += " [" + path.string() + ":" + std::to_string(line_number) + "]";
          break;
        }
      }
      if (success && (!stream.eof() || !header.empty()))
      {
        error = "Incomplete or unreadable INP file: " + path.string();
        success = false;
      }
      active.erase(path);
      return success;
    }

    bool finish(CoreResultsVtkMesh& output)
    {
      if (pending.id > 0 || nodes.empty() || elements.empty())
      {
        error = "Incomplete INP element or empty mesh.";
        return false;
      }
      CoreResultsVtkMesh result;
      for (auto& entry : nodes)
      {
        result.nodes.push_back(std::move(entry.second));
      }
      for (auto& entry : elements)
      {
        for (int id : entry.second.connectivity)
        {
          if (nodes.count(id) == 0)
          {
            error = "INP element references missing node " + std::to_string(id) + ".";
            return false;
          }
        }
      }
      if (!sets(node_sets, true, result) || !sets(element_sets, false, result))
      {
        return false;
      }
      for (const auto& entry : surfaces)
      {
        CoreResultsVtkGroup group;
        group.kind = CoreResultsVtkGroupKind::Surface;
        group.name = entry.second.name;
        for (const auto& face : entry.second.entries)
        {
          std::vector<int> ids;
          int id = 0;
          if (ResultReadNumbers::integer(face.first, id))
          {
            ids.push_back(id);
          }
          else
          {
            const auto set = element_sets.find(upper(face.first));
            if (set == element_sets.end())
            {
              error = "Unknown surface ELSET '" + face.first + "'.";
              return false;
            }
            ids = set->second.ids;
          }
          for (int element : ids)
          {
            if (elements.count(element) == 0)
            {
              error = "Surface references missing element " + std::to_string(element) + ".";
              return false;
            }
            CoreResultsVtkElements::Description description;
            if (!CoreResultsVtkElements::describe(elements.at(element).calculix_type, description, error) || !CoreResultsVtkElements::valid_side(description, face.second))
            {
              error = "Invalid surface side for INP element " + std::to_string(element) + ".";
              return false;
            }
            group.sides.push_back({element, face.second});
          }
        }
        result.groups.push_back(std::move(group));
      }
      for (auto& entry : elements)
      {
        result.elements.push_back(std::move(entry.second));
      }
      output = std::move(result);
      return true;
    }

  private:
    std::set<std::filesystem::path> active;
    std::string section;
    std::string set_name;
    std::string element_type;
    bool generated = false;
    CoreResultsVtkElement pending;
    std::size_t expected_nodes = 0;

    bool keyword(const std::string& line, const std::filesystem::path& directory)
    {
      std::vector<std::string> fields;
      if (!csv(line, fields))
      {
        error = "Unterminated INP keyword quote.";
        return false;
      }
      const std::string key = upper(fields[0]);
      std::map<std::string, std::string> parameters;
      for (std::size_t i = 1; i < fields.size(); ++i)
      {
        const auto equal = fields[i].find('=');
        parameters[upper(trim(fields[i].substr(0, equal)))] = equal == std::string::npos ? "" : trim(fields[i].substr(equal + 1));
      }
      if (key == "*INCLUDE")
      {
        const auto input = parameters.find("INPUT");
        if (input == parameters.end() || input->second.empty())
        {
          error = "INCLUDE requires INPUT.";
          return false;
        }
        return file(directory / input->second);
      }
      if (pending.id > 0)
      {
        error = "INP keyword interrupts incomplete element connectivity.";
        return false;
      }
      if (key == "*NGEN" || key == "*NFILL" || key == "*NCOPY" || key == "*ELGEN" || key == "*ELCOPY" || key == "*SYSTEM" || key == "*PART" || key == "*INSTANCE" || key == "*IMPORT")
      {
        error = "Unsupported INP mesh-changing keyword " + key + ".";
        return false;
      }
      section = key;
      generated = parameters.count("GENERATE") != 0;
      set_name.clear();
      if (key == "*NODE" || key == "*NSET")
      {
        set_name = parameters["NSET"];
      }
      else if (key == "*ELEMENT" || key == "*ELSET")
      {
        set_name = parameters["ELSET"];
      }
      if (key == "*NSET" || key == "*ELSET")
      {
        if (set_name.empty() || parameters.count("INSTANCE") != 0)
        {
          error = "Unnamed or instance-qualified INP set is unsupported.";
          return false;
        }
        auto& definitions = key == "*NSET" ? node_sets : element_sets;
        definitions[upper(set_name)].name = set_name;
      }
      if (key == "*ELEMENT")
      {
        element_type = upper(parameters["TYPE"]);
        CoreResultsVtkElements::Description description;
        if (!CoreResultsVtkElements::describe(element_type, description, error))
        {
          return false;
        }
        expected_nodes = description.node_count;
      }
      if (key == "*SURFACE")
      {
        set_name = parameters["NAME"];
        if (set_name.empty() || (parameters.count("TYPE") != 0 && upper(parameters["TYPE"]) != "ELEMENT"))
        {
          error = "Only named element-based INP surfaces are supported.";
          return false;
        }
        surfaces[upper(set_name)].name = set_name;
      }
      return true;
    }

    bool record(const std::string& line)
    {
      if (section != "*NODE" && section != "*ELEMENT" && section != "*NSET" && section != "*ELSET" && section != "*SURFACE")
      {
        return true;
      }
      std::vector<std::string> fields;
      if (!csv(line, fields))
      {
        error = "Unterminated INP record quote.";
        return false;
      }
      if (section == "*NODE")
      {
        CoreResultsVtkNode node;
        if (fields.size() < 2 || fields.size() > 4 || !ResultReadNumbers::integer(fields[0], node.id) || node.id <= 0)
        {
          error = "Invalid INP node.";
          return false;
        }
        for (std::size_t i = 1; i < fields.size(); ++i)
        {
          if (!fields[i].empty() && !ResultReadNumbers::real(fields[i], node.coordinates[i - 1]))
          {
            error = "Invalid INP node coordinate.";
            return false;
          }
        }
        const auto found = nodes.find(node.id);
        if (found != nodes.end() && found->second.coordinates != node.coordinates)
        {
          error = "Conflicting INP node definition.";
          return false;
        }
        nodes[node.id] = node;
        if (!set_name.empty())
        {
          auto& set = node_sets[upper(set_name)];
          set.name = set_name;
          set.ids.push_back(node.id);
        }
        return true;
      }
      if (section == "*ELEMENT")
      {
        std::size_t start = 0;
        if (pending.id <= 0)
        {
          if (!ResultReadNumbers::integer(fields[0], pending.id) || pending.id <= 0)
          {
            error = "Invalid INP element ID.";
            return false;
          }
          pending.calculix_type = element_type;
          CoreResultsVtkElements::Description description;
          if (!CoreResultsVtkElements::describe(element_type, description, error))
          {
            return false;
          }
          pending.frd_type = description.frd_type;
          start = 1;
        }
        for (std::size_t i = start; i < fields.size(); ++i)
        {
          if (fields[i].empty() && i + 1 == fields.size())
          {
            continue;
          }
          int node = 0;
          if (!ResultReadNumbers::integer(fields[i], node) || node <= 0)
          {
            error = "Invalid INP element node.";
            return false;
          }
          pending.connectivity.push_back(node);
        }
        if (pending.connectivity.size() > expected_nodes)
        {
          error = "Too many INP connectivity nodes.";
          return false;
        }
        if (pending.connectivity.size() == expected_nodes)
        {
          if (elements.count(pending.id) != 0)
          {
            error = "Duplicate INP element ID.";
            return false;
          }
          if (!set_name.empty())
          {
            auto& set = element_sets[upper(set_name)];
            set.name = set_name;
            set.ids.push_back(pending.id);
          }
          elements.emplace(pending.id, std::move(pending));
          pending = CoreResultsVtkElement{};
        }
        return true;
      }
      if (section == "*SURFACE")
      {
        while (!fields.empty() && fields.back().empty())
        {
          fields.pop_back();
        }
        if (fields.size() != 2)
        {
          error = "Invalid INP surface row.";
          return false;
        }
        const std::string side = upper(fields[1]);
        if (side != "SPOS" && side != "SNEG" && (side.size() != 2 || side[0] != 'S' || side[1] < '1' || side[1] > '6'))
        {
          error = "Unsupported INP surface side '" + side + "'.";
          return false;
        }
        surfaces[upper(set_name)].entries.emplace_back(fields[0], side);
        return true;
      }
      auto& definitions = section == "*NSET" ? node_sets : element_sets;
      auto& set = definitions[upper(set_name)];
      if (generated)
      {
        while (!fields.empty() && fields.back().empty())
        {
          fields.pop_back();
        }
        int first = 0;
        int last = 0;
        int stride = 1;
        if (fields.size() < 2 || fields.size() > 3 || !ResultReadNumbers::integer(fields[0], first) || !ResultReadNumbers::integer(fields[1], last) || (fields.size() == 3 && !ResultReadNumbers::integer(fields[2], stride)) || first <= 0 || last < first || stride <= 0 || (last - first) % stride != 0)
        {
          error = "Invalid INP GENERATE range.";
          return false;
        }
        for (int id = first; ; )
        {
          set.ids.push_back(id);
          if (last - id < stride)
          {
            break;
          }
          id += stride;
        }
      }
      else
      {
        for (const auto& field : fields)
        {
          if (field.empty())
          {
            continue;
          }
          int id = 0;
          if (ResultReadNumbers::integer(field, id))
          {
            if (id <= 0)
            {
              error = "Non-positive INP set ID.";
              return false;
            }
            set.ids.push_back(id);
          }
          else
          {
            set.references.push_back(upper(field));
          }
        }
      }
      return true;
    }

    bool resolve(const std::string& name, std::map<std::string, SetDefinition>& definitions, std::set<std::string>& visiting, std::set<std::string>& done)
    {
      if (done.count(name) != 0)
      {
        return true;
      }
      auto found = definitions.find(name);
      if (found == definitions.end() || visiting.count(name) != 0 || visiting.size() >= 128)
      {
        error = "Missing or cyclic INP set reference '" + name + "'.";
        return false;
      }
      visiting.insert(name);
      for (const auto& reference : found->second.references)
      {
        if (!resolve(reference, definitions, visiting, done))
        {
          return false;
        }
        const auto& ids = definitions.at(reference).ids;
        found->second.ids.insert(found->second.ids.end(), ids.begin(), ids.end());
      }
      auto& ids = found->second.ids;
      std::sort(ids.begin(), ids.end());
      ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
      visiting.erase(name);
      done.insert(name);
      return true;
    }

    bool sets(std::map<std::string, SetDefinition>& definitions, bool nodal, CoreResultsVtkMesh& result)
    {
      std::set<std::string> visiting;
      std::set<std::string> done;
      for (const auto& entry : definitions)
      {
        if (!resolve(entry.first, definitions, visiting, done))
        {
          return false;
        }
      }
      for (const auto& entry : definitions)
      {
        for (int id : entry.second.ids)
        {
          if ((nodal && nodes.count(id) == 0) || (!nodal && elements.count(id) == 0))
          {
            error = "INP set '" + entry.second.name + "' references a missing entity.";
            return false;
          }
        }
        CoreResultsVtkGroup group;
        group.kind = nodal ? CoreResultsVtkGroupKind::Nodeset : CoreResultsVtkGroupKind::Block;
        group.name = entry.second.name;
        group.ids = entry.second.ids;
        result.groups.push_back(std::move(group));
      }
      return true;
    }
  };
}

bool CoreResultsInpReader::read(const std::string& filename, CoreResultsVtkMesh& mesh, std::vector<std::string>& input_files, std::string& error) const
{
  try
  {
    Parser parser;
    CoreResultsVtkMesh result;
    if (!parser.file(std::filesystem::path(filename)) || !parser.finish(result))
    {
      error = parser.error;
      return false;
    }
    mesh = std::move(result);
    input_files = std::move(parser.files);
    error.clear();
    return true;
  }
  catch (const std::exception& failure)
  {
    error = "INP mesh read failed: " + std::string(failure.what());
    return false;
  }
}
