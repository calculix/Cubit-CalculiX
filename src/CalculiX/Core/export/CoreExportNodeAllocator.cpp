#include "CoreExportNodeAllocator.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <tuple>

bool CoreExportNodeAllocator::prepare(const std::vector<int>& mesh_nodes, const std::vector<std::vector<std::string>>& custom_lines,
  const std::string& filename, bool discrete_elements, std::string& error)
{
  highest_ = mesh_nodes.empty() ? 0 : *std::max_element(mesh_nodes.begin(), mesh_nodes.end());
  sets_.clear();
  highest_element_ = 0;
  custom_element_nodes_.clear();
  if (discrete_elements)
  {
    CalculiXMeshInterface mesh;
    for (int block : mesh.get_blocks())
    {
      sets_.insert(QString::fromStdString(mesh.get_block_name(block)).toUpper().toStdString());
      for (int id : mesh.get_block_element_ids(block))
      {
        highest_element_ = std::max(highest_element_, id);
      }
    }
  }
  for (int id : CubitInterface::get_nodeset_id_list())
  {
    sets_.insert(QString::fromStdString(CalculiXMeshInterface().get_nodeset_name(id)).toUpper().toStdString());
  }
  for (int id : CubitInterface::get_sideset_id_list())
  {
    sets_.insert(QString::fromStdString(CalculiXMeshInterface().get_sideset_name(id)).toUpper().toStdString());
  }
  std::map<std::tuple<std::string,std::string,std::string>,QStringList> insertions;
  for (const auto& row : custom_lines)
  {
    if (row.size() != 6)
    {
      error = "Invalid custom export line.";
      return false;
    }
    insertions[{row[2],row[3],row[4]}].append(QString::fromStdString(row[5]).split('\n'));
  }
  std::set<QString> active;
  bool node_records = false;
  bool element_records = false;
  bool continuing_element = false;
  bool custom_mass = false;
  std::function<bool(const QString&,const QDir&)> scan;
  scan = [&](const QString& raw,const QDir& directory)
  {
    const auto line = raw.trimmed();
    if (line.isEmpty() || line.startsWith("**"))
    {
      return true;
    }
    if (!line.startsWith('*'))
    {
      if (element_records)
      {
        if (!continuing_element)
        {
          bool ok = false;
          const int id = line.section(',',0,0).trimmed().toInt(&ok);
          if (!ok || id <= 0)
          {
            error = "Cannot reserve the element ID from a custom *ELEMENT record.";
            return false;
          }
          highest_element_ = std::max(highest_element_, id);
        }
        if (!custom_mass)
        {
          const auto columns = line.split(',');
          for (int i = continuing_element ? 0 : 1; i < columns.size(); ++i)
          {
            bool ok = false;
            const int node = columns[i].trimmed().toInt(&ok);
            if (ok && node > 0)
            {
              custom_element_nodes_.insert(node);
            }
          }
        }
        continuing_element = line.endsWith(',');
      }
      if (node_records)
      {
        bool ok = false;
        const int id = line.section(',',0,0).trimmed().toInt(&ok);
        if (!ok || id <= 0)
        {
          error = "Cannot reserve the node IDs from a custom *NODE record.";
          return false;
        }
        highest_ = std::max(highest_,id);
      }
      return true;
    }
    const auto fields = line.split(',');
    const auto key = fields.front().trimmed().toUpper();
    if (key == "*NGEN" || key == "*NFILL" || key == "*NCOPY" || key == "*IMPORT")
    {
      error = "Cannot determine generated node IDs from custom " + key.toStdString() + ".";
      return false;
    }
    if (discrete_elements && (key == "*ELGEN" || key == "*ELCOPY"))
    {
      error = "Cannot allocate discrete element IDs alongside custom " + key.toStdString() + ".";
      return false;
    }
    QString include;
    bool mass = false;
    for (const auto& field : fields)
    {
      const auto parameter = field.section('=',0,0).trimmed().toUpper();
      auto value = field.section('=',1).trimmed();
      if (value.size() > 1 && (value.front() == '"' || value.front() == QChar(39)) && value.back() == value.front())
      {
        value = value.mid(1,value.size()-2);
      }
      if ((parameter == "NSET" && (key == "*NODE" || key == "*NSET")) ||
          (discrete_elements && parameter == "ELSET" && (key == "*ELEMENT" || key == "*ELSET")) ||
          (parameter == "NAME" && key == "*SURFACE") || (parameter == "CONSTRAINT NAME" && key == "*COUPLING"))
      {
        sets_.insert(value.toUpper().toStdString());
      }
      if (parameter == "TYPE" && value.compare("MASS",Qt::CaseInsensitive) == 0)
      {
        mass = true;
      }
      if (parameter == "INPUT")
      {
        include = value;
      }
    }
    if (key != "*INCLUDE")
    {
      node_records = key == "*NODE";
      element_records = discrete_elements && key == "*ELEMENT";
      custom_mass = mass;
      continuing_element = false;
    }
    if (key == "*INCLUDE" || (!include.isEmpty() && (node_records || element_records)))
    {
      const auto path = QFileInfo(directory.filePath(include)).canonicalFilePath();
      QFile file(path);
      if (path.isEmpty() || active.size() >= 32 || !active.insert(path).second || !file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        error = "Cannot read custom input while reserving solver IDs: " + include.toStdString();
        return false;
      }
      QTextStream stream(&file);
      while (!stream.atEnd())
      {
        if (!scan(stream.readLine(),QFileInfo(file).absoluteDir()))
        {
          return false;
        }
      }
      active.erase(path);
      return file.error() == QFileDevice::NoError;
    }
    return true;
  };
  const auto directory = QFileInfo(QString::fromStdString(filename)).absoluteDir();
  for (const auto& insertion : insertions)
  {
    node_records = false;
    element_records = false;
    continuing_element = false;
    for (const auto& line : insertion.second)
    {
      if (!scan(line,directory))
      {
        return false;
      }
    }
  }
  return true;
}

bool CoreExportNodeAllocator::reserve_set(const std::string& name)
{
  return sets_.insert(name).second;
}

bool CoreExportNodeAllocator::next(int& node)
{
  if (highest_ == (std::numeric_limits<int>::max)())
  {
    return false;
  }
  node = ++highest_;
  return true;
}

bool CoreExportNodeAllocator::custom_element_uses(int node) const
{
  return custom_element_nodes_.count(node) != 0;
}

bool CoreExportNodeAllocator::next_element(int& element)
{
  if (highest_element_ == (std::numeric_limits<int>::max)())
  {
    return false;
  }
  element = ++highest_element_;
  return true;
}
