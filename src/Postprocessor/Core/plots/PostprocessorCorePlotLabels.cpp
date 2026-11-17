#include "../application/PostprocessorCore.hpp"
#include <algorithm>
#include <limits>
namespace Postprocessor
{
  bool PostprocessorCore::set_plot_label(const std::string& id,const std::string& target,const std::string& value,
    const std::string& curveName,std::string& error)
  {
    error.clear();
    const auto found = state.plots.find(id);
    if (source_operation_active || found == state.plots.end() || state.plot_revision == std::numeric_limits<std::uint64_t>::max() ||
        value.size() > 256 || (!value.empty() && !valid_plot_name(value)))
    {
      error = "Plot labels must be journalable text of at most 256 bytes and the plot must exist.";
      return false;
    }
    auto staged = state;
    auto& plot = staged.plots.at(id);
    std::optional<std::string> label;
    if (!value.empty())
    {
      label = value;
    }
    if (target == "title")
    {
      plot.title = label;
    }
    else if (target == "xlabel")
    {
      plot.x_label = label;
    }
    else if (target == "ylabel")
    {
      plot.y_label = label;
    }
    else if (target == "curve")
    {
      const auto curve = std::find_if(plot.curves.begin(),plot.curves.end(),[&](const auto& item)
      {
        return item.name == curveName;
      });
      if (curve == plot.curves.end())
      {
        error = "No curve has that name in this plot.";
        return false;
      }
      curve->label = label;
    }
    else if (target == "auto")
    {
      plot.title.reset();
      plot.x_label.reset();
      plot.y_label.reset();
      for (auto& curve : plot.curves)
      {
        curve.label.reset();
      }
    }
    else
    {
      error = "Unknown plot label target.";
      return false;
    }
    ++staged.plot_revision;
    state.swap(staged);
    return true;
  }
}
