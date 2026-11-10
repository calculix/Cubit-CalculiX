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
  ResultTableData result_table_data(std::shared_ptr<const SimResults::VtkResultsData> data, const TableReference& reference)
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
  std::string result_table_cell(const ResultTableData& data, std::size_t row, std::size_t column)
  {
    if (row >= data.rows || column >= data.columns.size())
    {
      return {};
    }
    if (data.table)
    {
      auto* array = data.table->GetColumn(static_cast<vtkIdType>(column));
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
    for (const auto& part : data.parts)
    {
      if (row < part.first_row || row-part.first_row >= part.count)
      {
        continue;
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
    return {};
  }
}
