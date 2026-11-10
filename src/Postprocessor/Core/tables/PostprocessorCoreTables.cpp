#include "../application/PostprocessorCore.hpp"
#include <exception>
#include <limits>
namespace Postprocessor
{
  bool PostprocessorCore::open_table(const std::string& source_id, const std::string& view_id,
    const std::string& name, bool field, std::string& error)
  {
    error.clear();
    const auto data = result_data(source_id);
    if (!data || state.views.find(view_id) == state.views.end() || source_operation_active ||
        scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Table source/view is unavailable during this operation.";
      return false;
    }
    try
    {
      TableReference reference;
      reference.source_id = source_id;
      reference.field = field;
      for (const auto& item : state.representations)
      {
        if (item.second.view_id == view_id && item.second.input.object_id == source_id)
        {
          reference.frame = item.second.increment_index;
          break;
        }
      }
      std::vector<std::pair<std::string,std::string>> matches;
      if (field)
      {
        for (const auto& group : result_field_groups(*data,std::nullopt))
        {
          if (group.field.association != SimResults::Association::Table && (group.key == name || group.field.name == name))
          {
            matches.emplace_back(group.key,group.field.name);
          }
        }
      }
      else
      {
        for (const auto& table : result_tables(*data,std::nullopt))
        {
          if (table.key == name || table.name == name)
          {
            matches.emplace_back(table.key,table.name);
          }
        }
      }
      if (matches.size() != 1)
      {
        error = matches.empty() ? "No matching table/result field is available." : "Ambiguous table/result field. Use one of these exact keys:";
        for (const auto& match : matches)
        {
          error += "\n"+match.first;
        }
        return false;
      }
      reference.key = matches.front().first;
      // A table-only source may first publish its table after an empty frame.
      if (!field && !result_table_data(data,reference).table)
      {
        for (std::size_t index = 0; index < data->increments.size(); ++index)
        {
          reference.frame = index;
          if (result_table_data(data,reference).table)
          {
            break;
          }
        }
      }
      const auto table = result_table_data(data,reference);
      if (table.rows > static_cast<std::size_t>(std::numeric_limits<int>::max()))
      {
        error = "This table exceeds the current Qt table view row limit.";
        return false;
      }
      auto staged = state;
      staged.views.at(view_id).table = reference;
      staged.active_source_id = source_id;
      staged.active_representation_id.clear();
      for (auto& item : staged.representations)
      {
        if (item.second.view_id == view_id)
        {
          item.second.animation.playing = false;
        }
      }
      state.swap(staged);
      ++scene_revision;
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot open result table: ")+exception.what();
      return false;
    }
  }
  bool PostprocessorCore::show_3d(const std::string& view_id, std::string& error)
  {
    error.clear();
    const auto view = state.views.find(view_id);
    if (view == state.views.end() || source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Cannot change this view during the current operation.";
      return false;
    }
    if (view->second.table)
    {
      view->second.table.reset();
      ++scene_revision;
    }
    return true;
  }
  ResultTableData PostprocessorCore::table_data(const std::string& view_id) const
  {
    const auto view = state.views.find(view_id);
    if (view == state.views.end() || !view->second.table)
    {
      return {};
    }
    return result_table_data(result_data(view->second.table->source_id),*view->second.table);
  }
}
