#include "ResultTables.hpp"
#include "../representations/ResultMesh.hpp"
#include "../representations/ResultColoring.hpp"
#include <vtkAbstractArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkFieldData.h>
#include <vtkPointData.h>
#include <vtkTable.h>
#include <vtkVariant.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <map>
#include <sstream>
#include <limits>
#include <iterator>
namespace
{
  std::string metadata(vtkTable* table, const char* key)
  {
    auto* array = table->GetFieldData()->GetAbstractArray(key);
    return array && array->GetNumberOfValues() ? std::string(array->GetVariantValue(0).ToString()) : "";
  }
  std::vector<Postprocessor::ResultTableDescriptor> descriptors(const std::vector<std::shared_ptr<vtkTable>>& tables, bool global)
  {
    std::vector<Postprocessor::ResultTableDescriptor> result;
    std::map<std::string,unsigned int> occurrences;
    for (const auto& table : tables)
    {
      if (!table)
      {
        continue;
      }
      auto name = metadata(table.get(),"TableName");
      auto type = metadata(table.get(),"TableType");
      if (type.empty())
      {
        type = metadata(table.get(),"ResultKind");
      }
      if (name.empty())
      {
        name = type.empty() ? "Table" : type;
      }
      const auto set = metadata(table.get(),"SetName");
      const auto scope = metadata(table.get(),"TableScope");
      if (!set.empty())
      {
        name += " / "+set;
      }
      const auto step = metadata(table.get(),"Step");
      if (scope == "StepSummary" && !step.empty())
      {
        name += " / Step "+step;
      }
      const auto identity = std::string(global ? "global:" : "frame:")+std::to_string(name.size())+":"+name+
        std::to_string(type.size())+":"+type;
      const auto key = identity+":"+std::to_string(++occurrences[identity]);
      result.push_back({key,name,scope == "StepSummary" ? "Summaries" : global ? "Global tables" : "Frame tables",global});
    }
    return result;
  }
}
namespace Postprocessor
{
  std::vector<ResultTableDescriptor> result_tables(const SimResults::VtkResultsData& data, std::optional<std::size_t> frame)
  {
    auto result = descriptors(data.tables,true);
    const auto append = [&](std::size_t index)
    {
      for (auto item : descriptors(data.increments[index].tables,false))
      {
        if (std::none_of(result.begin(),result.end(),[&](const ResultTableDescriptor& existing)
        {
          return existing.key == item.key;
        }))
        {
          result.push_back(std::move(item));
        }
      }
    };
    if (frame && *frame < data.increments.size())
    {
      append(*frame);
    }
    else if (!frame)
    {
      for (std::size_t index = 0; index < data.increments.size(); ++index)
      {
        append(index);
      }
    }
    return result;
  }
  static ResultTableData frame_table_data(std::shared_ptr<const SimResults::VtkResultsData> data, const TableReference& reference)
  {
    ResultTableData result;
    result.owner = data;
    result.status = "Unavailable in this frame";
    if (!data)
    {
      result.status = "Result source unavailable";
      return result;
    }
    if (!reference.field)
    {
      const auto find = [&](const std::vector<std::shared_ptr<vtkTable>>& tables, bool global)
      {
        const auto list = descriptors(tables,global);
        std::size_t index = 0;
        for (const auto& table : tables)
        {
          if (!table)
          {
            continue;
          }
          if (list[index].key == reference.key)
          {
            result.table = table;
            result.title = list[index].name;
          }
          ++index;
        }
      };
      find(data->tables,true);
      if (reference.frame < data->increments.size())
      {
        find(data->increments[reference.frame].tables,false);
      }
      if (result.table)
      {
        result.rows = static_cast<std::size_t>(result.table->GetNumberOfRows());
        for (vtkIdType column = 0; column < result.table->GetNumberOfColumns(); ++column)
        {
          auto* array = result.table->GetColumn(column);
          result.columns.push_back(array->GetName() ? array->GetName() : "Column");
          result.numeric.push_back(array->IsNumeric() != 0 && array->GetNumberOfComponents() == 1);
        }
        result.status.clear();
      }
      return result;
    }
    if (reference.frame >= data->increments.size())
    {
      return result;
    }
    const auto& frame = data->increments[reference.frame];
    for (const auto& field : frame.fields)
    {
      if (field_group_key(field) != reference.key || field.association == SimResults::Association::Table)
      {
        continue;
      }
      for (const auto& mesh : result_mesh_parts(frame.dataset.get()))
      {
        if (mesh.first != field.dataset_id)
        {
          continue;
        }
        if (result.columns.empty())
        {
          result.title = field.name;
          result.columns = {"Dataset",field.association == SimResults::Association::Node ? "Node ID" : "Element ID"};
          if (field.association == SimResults::Association::IntegrationPoint)
          {
            result.columns.push_back("IP");
          }
          result.columns.insert(result.columns.end(),field.components.begin(),field.components.end());
          result.numeric.assign(result.columns.size(),true);
        }
        const auto count = static_cast<std::size_t>(field.association == SimResults::Association::Element ?
          mesh.second->GetNumberOfCells() : mesh.second->GetNumberOfPoints());
        if (count > std::numeric_limits<std::size_t>::max()-result.rows)
        {
          result.rows = std::numeric_limits<std::size_t>::max();
          result.status = "Table row count exceeds the supported limit";
          return result;
        }
        result.parts.push_back({mesh.second,field,mesh.first,result.rows,count});
        result.rows += count;
      }
    }
    if (!result.columns.empty())
    {
      result.status.clear();
    }
    return result;
  }
  static std::string native_cell(vtkTable* table,std::size_t row,std::size_t column)
  {
    auto* array = table->GetColumn(static_cast<vtkIdType>(column));
    if (!array || row >= static_cast<std::size_t>(array->GetNumberOfTuples()))
    {
      return "Unavailable";
    }
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text << std::setprecision(17);
    const int components = array->GetNumberOfComponents();
    if (components > 1)
    {
      text << '[';
    }
    for (int component = 0; component < components; ++component)
    {
      if (component)
      {
        text << ", ";
      }
      const auto value = array->GetVariantValue(static_cast<vtkIdType>(row)*components+component);
      const bool floating = array->GetDataType() == VTK_FLOAT || array->GetDataType() == VTK_DOUBLE;
      if (!value.IsValid() || (floating && !std::isfinite(value.ToDouble())))
      {
        text << "Unavailable";
      }
      else if (floating)
      {
        text << value.ToDouble();
      }
      else
      {
        text << value.ToString();
      }
    }
    if (components > 1)
    {
      text << ']';
    }
    return text.str();
  }

  ResultTableData result_table_data(std::shared_ptr<const SimResults::VtkResultsData> data,const TableReference& reference)
  {
    // Globals/step summaries retain their own columns and occur exactly once.
    if (!reference.field && reference.key.rfind("global:",0) == 0)
    {
      return frame_table_data(std::move(data),reference);
    }
    ResultTableData result;
    result.owner = data;
    result.frame_columns = true;
    result.status = data ? "Unavailable across frames" : "Result source unavailable";
    if (!data)
    {
      return result;
    }
    if (reference.frame < data->increments.size())
    {
      result.active_frame = reference.frame;
    }
    std::vector<std::pair<std::string,std::size_t>> schema;
    for (std::size_t frame = 0; frame < data->increments.size(); ++frame)
    {
      auto requested = reference;
      requested.frame = frame;
      auto current = frame_table_data(data,requested);
      if (!current.table && current.columns.empty())
      {
        continue;
      }
      if (result.columns.empty())
      {
        result.columns = {"Frame","Step","Increment","Time"};
        result.numeric.assign(4,true);
        result.title = current.title;
        result.status.clear();
      }
      std::map<std::string,std::size_t> occurrences;
      std::vector<int> columns(schema.size(),-1);
      for (std::size_t column = 0; column < current.columns.size(); ++column)
      {
        const auto& name = current.columns[column];
        const auto occurrence = occurrences[name]++;
        const auto identity = std::make_pair(name,occurrence);
        const auto found = std::find(schema.begin(),schema.end(),identity);
        const auto index = static_cast<std::size_t>(found-schema.begin());
        if (found == schema.end())
        {
          schema.push_back(identity);
          columns.push_back(-1);
          auto label = name;
          if (name == "Frame" || name == "Step" || name == "Increment" || name == "Time")
          {
            label = "Result "+label;
          }
          if (occurrence)
          {
            label += " ("+std::to_string(occurrence+1)+")";
          }
          result.columns.push_back(label);
          result.numeric.push_back(current.numeric[column]);
        }
        else
        {
          result.numeric[index+4] = result.numeric[index+4] && current.numeric[column];
        }
        columns[index] = static_cast<int>(column);
      }
      const auto append = [&](ResultTablePart part)
      {
        if (part.count > std::numeric_limits<std::size_t>::max()-result.rows)
        {
          result.rows = std::numeric_limits<std::size_t>::max();
          result.status = "Combined table row count exceeds the supported limit";
          return false;
        }
        part.frame = frame;
        part.first_row = result.rows;
        part.column_map = columns;
        result.rows += part.count;
        result.parts.push_back(std::move(part));
        return true;
      };
      if (current.table)
      {
        if (!result.table)
        {
          result.table = current.table;
        }
        ResultTablePart part;
        part.table = current.table;
        part.count = current.rows;
        if (!append(std::move(part)))
        {
          return result;
        }
      }
      else
      {
        for (auto& part : current.parts)
        {
          if (!append(std::move(part)))
          {
            return result;
          }
        }
      }
    }
    for (auto& part : result.parts)
    {
      part.column_map.resize(schema.size(),-1);
    }
    if (!result.columns.empty() && result.active_frame && std::none_of(result.parts.begin(),result.parts.end(),[&](const auto& part)
      {
        return part.frame == result.active_frame && part.count != 0;
      }))
    {
      result.status = "No records in the active frame";
    }
    return result;
  }
  static const ResultTablePart* row_part(const ResultTableData& data,std::size_t row)
  {
    if (row >= data.rows)
    {
      return nullptr;
    }
    const auto end = std::upper_bound(data.parts.begin(),data.parts.end(),row,[](std::size_t value,const ResultTablePart& part)
    {
      return value < part.first_row;
    });
    if (end == data.parts.begin())
    {
      return nullptr;
    }
    const auto& part = *std::prev(end);
    return row-part.first_row < part.count ? &part : nullptr;
  }
  std::optional<std::size_t> result_table_row_frame(const ResultTableData& data,std::size_t row)
  {
    const auto* part = row_part(data,row);
    return part ? part->frame : std::nullopt;
  }
  std::string result_table_cell(const ResultTableData& data,std::size_t row,std::size_t column)
  {
    if (row >= data.rows || column >= data.columns.size())
    {
      return {};
    }
    if (!data.frame_columns && data.table)
    {
      return native_cell(data.table.get(),row,column);
    }
    const auto* matched = row_part(data,row);
    if (!matched)
    {
      return "Unavailable";
    }
    const auto& part = *matched;
    if (data.frame_columns && column < 4)
    {
      if (!part.frame || !data.owner || *part.frame >= data.owner->increments.size())
      {
        return "Unavailable";
      }
      const auto& frame = data.owner->increments[*part.frame];
      if (column == 0)
      {
        return std::to_string(*part.frame+1);
      }
      if (column == 1 || column == 2)
      {
        const int value = column == 1 ? frame.step : frame.increment;
        return value > 0 ? std::to_string(value) : "Unavailable";
      }
      std::ostringstream value;
      value.imbue(std::locale::classic());
      value << std::setprecision(17) << frame.time;
      return std::isfinite(frame.time) ? value.str() : "Unavailable";
    }
    const auto payload = data.frame_columns ? column-4 : column;
    if (payload >= part.column_map.size() || part.column_map[payload] < 0)
    {
      return "Unavailable";
    }
    column = static_cast<std::size_t>(part.column_map[payload]);
    if (part.table)
    {
      return native_cell(part.table.get(),row-part.first_row,column);
    }
    const auto index = row-part.first_row;
    if (column == 0)
    {
      return std::to_string(part.dataset);
    }
    const bool ip = part.field.association == SimResults::Association::IntegrationPoint;
    auto* attributes = part.field.association == SimResults::Association::Element ?
      static_cast<vtkDataSetAttributes*>(part.mesh->GetCellData()) : static_cast<vtkDataSetAttributes*>(part.mesh->GetPointData());
    if (column == 1 || (ip && column == 2))
    {
      const char* key = column == 2 ? "IntegrationPointId" : part.field.association == SimResults::Association::Node ? "OriginalNodeId" : "OriginalElementId";
      auto* array = attributes->GetArray(key);
      return array && index < static_cast<std::size_t>(array->GetNumberOfTuples()) ?
        std::string(array->GetVariantValue(static_cast<vtkIdType>(index)).ToString()) : "Unavailable";
    }
    const auto component = column-(ip ? 3 : 2);
    if (component >= part.field.components.size())
    {
      return "Unavailable";
    }
    FieldSelection selection;
    selection.component = part.field.components[component];
    double value = 0;
    std::string error;
    if (!color_sample(part.mesh,part.field,selection,index,value,error) || !std::isfinite(value))
    {
      return "Unavailable";
    }
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text << std::setprecision(17) << value;
    return text.str();
  }
}
