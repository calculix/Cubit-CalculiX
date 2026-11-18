#include "../application/PostprocessorCore.hpp"
#include <exception>
#include <algorithm>
#include <limits>
namespace Postprocessor
{
  bool PostprocessorCore::open_table(const std::string& source_id, const std::string& view_id,
    const std::string& name, bool field, std::string& error,
    const TableQualifiers& tableQualifiers,const FieldQualifiers& fieldQualifiers)
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
      if (field)
      {
        ResultFieldGroup selected;
        if (tableQualifiers.global || tableQualifiers.type || tableQualifiers.entry)
        {
          error = "Scope/type/entry apply to native tables, not spatial field tables.";
          return false;
        }
        if (!resolve_result_field(result_field_groups(*data,std::nullopt),{name,fieldQualifiers},
            std::nullopt,{},false,false,selected,error))
        {
          return false;
        }
        reference.key = selected.key;
      }
      else
      {
        ResultTableDescriptor selected;
        if (fieldQualifiers.origin || fieldQualifiers.association)
        {
          error = "Origin/association apply to spatial field tables.";
          return false;
        }
        if (!resolve_result_table(result_tables(*data,std::nullopt),{name,tableQualifiers},selected,error))
        {
          return false;
        }
        reference.key = selected.key;
        reference.field = selected.field;
      }
      // The reference frame marks active rows; the adapter includes the full
      // history even when this particular frame has no matching records.
      const auto table = result_table_data(data,reference);
      if (table.rows > static_cast<std::size_t>(std::numeric_limits<int>::max()))
      {
        error = "This table exceeds the current Qt table view row limit.";
        return false;
      }
      auto staged = state;
      staged.views.at(view_id).table = reference;
      staged.views.at(view_id).plot_active = false;
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
  bool PostprocessorCore::change_table_filter(const std::string& viewId,const std::string& action,const TableFilter& filter,std::string& error)
  {
    error.clear();
    const auto view = state.views.find(viewId);
    if (view == state.views.end() || !view->second.table || source_operation_active ||
        scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Open a table before changing its column filters.";
      return false;
    }
    auto reference = *view->second.table;
    auto& filters = reference.filters;
    if (action == "clearall")
    {
      filters.clear();
    }
    else if (action == "clear" || action == "set")
    {
      filters.erase(std::remove_if(filters.begin(),filters.end(),[&](const auto& item)
      {
        return item.column == filter.column;
      }),filters.end());
      if (action == "set")
      {
        filters.push_back(filter);
      }
    }
    else
    {
      error = "Unknown table filter operation.";
      return false;
    }
    if (action == "set" && !validate_table_filters(result_table_data(result_data(reference.source_id),reference),filters,error))
    {
      return false;
    }
    view->second.table = std::move(reference);
    ++scene_revision;
    return true;
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
    auto table = result_table_data(result_data(view->second.table->source_id),*view->second.table);
    table.filters = view->second.table->filters;
    // A legacy saved view may now exceed the limit when all frames are combined.
    // Show an unavailable status rather than silently truncating the history.
    if (table.rows > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
      table.parts.clear();
      table.table.reset();
      table.rows = 0;
      table.status = "Combined table history exceeds the current table view row limit";
    }
    return table;
  }
}
