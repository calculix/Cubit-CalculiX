#include "../application/PostprocessorCore.hpp"
#include <utility>
#include <algorithm>
#include <limits>
namespace Postprocessor
{
  bool prune_plot_sources(PostprocessorSession& state,const std::string& source,std::string& error)
  {
    if (state.plot_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Plot revisions are exhausted.";
      return false;
    }
    bool changed = false;
    for (auto& plot : state.plots)
    {
      auto& curves = plot.second.curves;
      const auto size = curves.size();
      curves.erase(std::remove_if(curves.begin(),curves.end(),[&](const auto& curve)
      {
        return source.empty() || plot_curve_source(curve) == source;
      }),curves.end());
      if (curves.size() != size)
      {
        plot.second.data_revision = state.plot_revision+1;
        changed = true;
      }
    }
    if (changed)
    {
      ++state.plot_revision;
    }
    return true;
  }
  std::string PostprocessorCore::plot_id_from_name(const std::string& name) const
  {
    if (name.empty())
    {
      return state.active_plot_id;
    }
    for (const auto& item : state.plots)
    {
      if (item.second.name == name)
      {
        return item.first;
      }
    }
    return {};
  }
  bool PostprocessorCore::create_plot(const std::string& name, std::string& error)
  {
    error.clear();
    if (source_operation_active || !valid_plot_name(name) || !plot_id_from_name(name).empty() ||
        state.plots.size() >= PlotDefaults::maximum_plots ||
        state.plot_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Plot name must be unique and valid; plot creation is unavailable during this operation.";
      return false;
    }
    auto staged = state;
    std::size_t number = 1;
    while (staged.plots.count("plot_"+std::to_string(number)) != 0)
    {
      ++number;
    }
    PlotDefinition plot;
    plot.id = "plot_"+std::to_string(number);
    plot.name = name;
    plot.data_revision = ++staged.plot_revision;
    staged.plots.emplace(plot.id,plot);
    staged.active_plot_id = plot.id;
    staged.views.at(staged.active_view_id).plot_id = plot.id;
    staged.views.at(staged.active_view_id).plot_active = true;
    state.swap(staged);
    return true;
  }
  bool PostprocessorCore::add_plot_curve(const std::string& id, const PlotCurveDefinition& requested, std::string& error)
  {
    error.clear();
    const auto found = state.plots.find(id);
    if (source_operation_active || found == state.plots.end() || found->second.curves.size() >= PlotDefaults::maximum_curves ||
        state.plot_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Create/select a plot first; curve creation is unavailable during this operation.";
      return false;
    }
    auto curve = requested;
    curve.color = PlotDefaults::colors[found->second.curves.size()%PlotDefaults::colors.size()];
    for (const auto& color : PlotDefaults::colors)
    {
      if (std::none_of(found->second.curves.begin(),found->second.curves.end(),[&](const auto& existing)
        {
          return existing.color == color;
        }))
      {
        curve.color = color;
        break;
      }
    }
    const auto exists = [&](const std::string& name)
    {
      return std::any_of(found->second.curves.begin(),found->second.curves.end(),[&](const auto& item)
      {
        return item.name == name;
      });
    };
    if (curve.name.empty())
    {
      std::size_t number = 1;
      while (exists("curve_"+std::to_string(number)))
      {
        ++number;
      }
      curve.name = "curve_"+std::to_string(number);
    }
    if (!valid_plot_name(curve.name) || exists(curve.name) || state.sources.count(plot_curve_source(curve)) == 0)
    {
      error = "Curve name must be unique and its source must exist.";
      return false;
    }
    const auto source = result_data(plot_curve_source(curve));
    PlotCurveData values;
    bool accepted = false;
    if (source)
    {
      if (const auto* entity = std::get_if<PlotEntityReference>(&curve.input))
      {
        accepted = plot_entity_data(*source,*entity,values,error);
      }
      else
      {
        accepted = plot_table_data(source,std::get<PlotTableReference>(curve.input),values,error);
      }
    }
    if (!accepted || !values.status.empty() || std::none_of(values.samples.begin(),values.samples.end(),[](const auto& sample)
      {
        return sample.valid;
      }))
    {
      if (error.empty())
      {
        error = values.status.empty() ? "The curve has no valid samples." : values.status;
      }
      return false;
    }
    auto staged = state;
    auto& plot = staged.plots.at(id);
    plot.curves.push_back(std::move(curve));
    plot.data_revision = ++staged.plot_revision;
    staged.active_plot_id = id;
    staged.views.at(staged.active_view_id).plot_id = id;
    staged.views.at(staged.active_view_id).plot_active = true;
    state.swap(staged);
    return true;
  }
  bool PostprocessorCore::change_plot(const std::string& id, const std::string& action,
    const std::string& value, std::string& error)
  {
    error.clear();
    if (source_operation_active || state.plot_revision == std::numeric_limits<std::uint64_t>::max() ||
        (action != "close" && state.plots.count(id) == 0))
    {
      error = "The plot is unavailable during this operation.";
      return false;
    }
    if (action != "show" && action != "close" && action != "remove" && action != "rename")
    {
      error = "Unknown plot operation.";
      return false;
    }
    if (action == "rename" && (!valid_plot_name(value) ||
        (!plot_id_from_name(value).empty() && plot_id_from_name(value) != id)))
    {
      error = "The replacement plot name is invalid or already used.";
      return false;
    }
    auto staged = state;
    if (action == "close")
    {
      staged.views.at(staged.active_view_id).plot_id.reset();
      staged.views.at(staged.active_view_id).plot_active = false;
    }
    else if (action == "rename")
    {
      staged.plots.at(id).name = value;
    }
    else if (action == "show")
    {
      staged.active_plot_id = id;
      staged.views.at(staged.active_view_id).plot_id = id;
    staged.views.at(staged.active_view_id).plot_active = true;
    }
    else
    {
      staged.plots.erase(id);
      if (staged.active_plot_id == id)
      {
        staged.active_plot_id.clear();
      }
      for (auto& view : staged.views)
      {
        if (view.second.plot_id == id)
        {
          view.second.plot_id.reset();
          view.second.plot_active = false;
        }
      }
    }
    ++staged.plot_revision;
    state.swap(staged);
    return true;
  }
  bool PostprocessorCore::change_plot_curve(const std::string& id, const std::string& name,
    const std::string& action, std::string& error)
  {
    error.clear();
    const auto found = state.plots.find(id);
    if (source_operation_active || found == state.plots.end() ||
        state.plot_revision == std::numeric_limits<std::uint64_t>::max() ||
        (action != "show" && action != "hide" && action != "remove"))
    {
      error = "The plot/curve operation is unavailable.";
      return false;
    }
    auto staged = state;
    auto& plot = staged.plots.at(id);
    const auto curve = std::find_if(plot.curves.begin(),plot.curves.end(),[&](const auto& item)
    {
      return item.name == name;
    });
    if (curve == plot.curves.end())
    {
      error = "No curve has that name in this plot.";
      return false;
    }
    if (action == "remove")
    {
      plot.curves.erase(curve);
      plot.data_revision = state.plot_revision+1;
    }
    else
    {
      curve->visible = action == "show";
    }
    ++staged.plot_revision;
    state.swap(staged);
    return true;
  }
  bool PostprocessorCore::set_plot_axes(const std::string& id, const PlotAxes& axes, std::string& error)
  {
    error.clear();
    const auto found = state.plots.find(id);
    if (source_operation_active || found == state.plots.end() || !valid_plot_axes(axes) ||
        state.plot_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Axis ranges must be finite and increasing, and the plot must exist.";
      return false;
    }
    found->second.axes = axes;
    ++state.plot_revision;
    return true;
  }
  bool PostprocessorCore::plot_data(const std::string& id, PlotData& output, std::string& error) const
  {
    error.clear();
    const auto found = state.plots.find(id);
    if (found == state.plots.end())
    {
      error = "The plot is unavailable.";
      return false;
    }
    PlotData staged;
    for (const auto& curve : found->second.curves)
    {
      PlotCurveData values;
      values.name = curve.name;
      const auto sourceDefinition = state.sources.find(plot_curve_source(curve));
      if (sourceDefinition != state.sources.end())
      {
        values.source_name = sourceDefinition->second.name;
      }
      const auto source = result_data(plot_curve_source(curve));
      if (!source)
      {
        values.status = "Source is unavailable.";
      }
      else if (const auto* entity = std::get_if<PlotEntityReference>(&curve.input))
      {
        if (!plot_entity_data(*source,*entity,values,error))
        {
          return false;
        }
      }
      else if (!plot_table_data(source,std::get<PlotTableReference>(curve.input),values,error))
      {
        return false;
      }
      staged.curves.push_back(std::move(values));
    }
    output = std::move(staged);
    return true;
  }
}
