#include <type_traits>
#include "Postprocessor/Commands/tables/TableCommandOptions.hpp"
#include "Postprocessor/Commands/FieldCommandOptions.hpp"
#include "PostPlotCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <charconv>
#include <cmath>
#include <exception>
#include <utility>
PostPlotCommand::PostPlotCommand(std::weak_ptr<PostprocessorApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> PostPlotCommand::get_syntax()
{
  const std::string plot = " [plot <string:type='quoted',number='1',label='plot_name'>]";
  const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
  const std::string name = " [name <string:type='quoted',number='1',label='curve_name'>]";
  const std::string id = "{<value:label='entity_id'>|<string:type='quoted',number='1',label='entity_text'>}";
  const std::string target = "<string:type='quoted',number='1',label='target'>";
  const std::string filters = " [{where <string:type='quoted',number='1',label='filter_columns'> equals <string:type='quoted',number='1',label='filter_values'> [unit <string:type='quoted',number='1',label='exact_units'>]|where <string:type='quoted',number='1',label='range_columns'> range <value:label='range_minima'> <value:label='range_maxima'> [unit <string:type='quoted',number='1',label='range_units'>]}]...";
  const std::string tail = " [dataset {<value:label='dataset_id'>|<string:type='quoted',number='1',label='dataset_name'>}]"+source+
    " [field <string:type='quoted',number='1',label='field_name'>]"+Postprocessor::field_qualifier_syntax()+
    " [component {magnitude|<string:type='quoted',number='1',label='component_name'>}]"
    " [reduction {minimum|maximum}]"+name+plot;
  return {"post plot create "+target,"post plot {show|remove} "+target,"post plot close",
    "post plot rename "+target+" <string:type='quoted',number='1',label='replacement'>",
    "post plot add node "+id+tail,"post plot add element "+id+tail,
    "post plot add integrationpoint <value:label='ip'> element "+id+tail,
    "post plot add table <string:type='quoted',number='1',label='table_name'>"+Postprocessor::table_qualifier_syntax()+
    " x <string:type='quoted',number='1',label='x_column'> y <string:type='quoted',number='1',label='y_column'>"
    +filters+source+name+plot,
    "post plot add field <string:type='quoted',number='1',label='table_name'>"+Postprocessor::field_qualifier_syntax()+
    " x <string:type='quoted',number='1',label='x_column'> y <string:type='quoted',number='1',label='y_column'>"
    +filters+source+name+plot,
    "post plot curve {hide|show|remove} <string:type='quoted',number='1',label='curve_name'>"+plot,
    "post plot {title|xlabel|ylabel} <string:type='quoted',number='1',label='label_text'>"+plot,
    "post plot labels auto"+plot,
    "post plot curve label <string:type='quoted',number='1',label='label_text'> curve <string:type='quoted',number='1',label='curve_name'>"+plot,
    "post plot axes auto"+plot,
    "post plot axes range x <value:label='xmin'> <value:label='xmax'> y <value:label='ymin'> <value:label='ymax'>"+plot,
    "post plot export csv <string:type='quoted',number='1',label='path'>"+plot};
}
std::vector<std::string> PostPlotCommand::get_syntax_help()
{
  return {"post plot create \"name\"","post plot {show|remove} \"name\"","post plot close",
    "post plot rename \"name\" \"replacement\"",
    "post plot add {node ID|element ID|integrationpoint N element ID} [dataset ID|\"name\"] [source \"name\"] [field \"name\"] [origin \"FRD\"] [association node|element|integrationpoint] [set \"name\"] [component magnitude|\"name\"] [reduction minimum|maximum] [name \"curve\"] [plot \"name\"]",
    "post plot add table \"name\" [scope frame|global] [type \"type\"] [entry N] x \"column\" y \"column\" [where \"column\" {equals \"value\"|range MIN MAX} [unit \"unit\"]]... [source \"name\"] [name \"curve\"] [plot \"name\"]",
    "post plot add field \"name\" [origin \"FRD\"] [association node|element|integrationpoint] [set \"name\"] x \"column\" y \"column\" [where \"column\" {equals \"value\"|range MIN MAX} [unit \"unit\"]]... [source \"name\"] [name \"curve\"] [plot \"name\"]",
    "post plot curve {show|hide|remove} \"curve\" [plot \"name\"]",
    "post plot {title|xlabel|ylabel} \"text\" [plot \"name\"]",
    "post plot labels auto [plot \"name\"]",
    "post plot curve label \"caption\" curve \"curve\" [plot \"name\"]",
    "post plot axes auto [plot \"name\"]","post plot axes range x MIN MAX y MIN MAX [plot \"name\"]",
    "post plot export csv \"new-file.csv\" [plot \"name\"]"};
}
std::vector<std::string> PostPlotCommand::get_help()
{
  return {"Plots store references and settings, never full result arrays. Entity histories preserve frame order and invalid-sample gaps.",
    "Elements with nodal values require minimum/maximum reduction. Quote IDs above 9007199254740991.",
    "Table filters combine with AND; exact values retain identifier text and ranges are inclusive. Labels alter presentation only; an empty label restores automatic text. CSV files are never overwritten. Closing a plot hides its pane without deleting it."};
}
bool PostPlotCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    if (!active)
    {
      PRINT_ERROR("Postprocessor plotting is unavailable.\n");
      return false;
    }
    std::string plot, target, name, source, error, replacement;
    data.get_string("plot_name",plot);
    data.get_string("target",target);
    data.get_string("curve_name",name);
    data.get_string("source_name",source);
    std::string tableTarget;
    const bool tableInput = data.get_string("table_name",tableTarget);
    bool accepted = false;
    if (data.find_keyword("CREATE"))
    {
      accepted = active->create_plot(target,error);
    }
    else if (data.find_keyword("EXPORT"))
    {
      std::string path;
      data.get_string("path",path);
      accepted = active->export_plot(plot,path,error);
    }
    else if (data.find_keyword("AXES"))
    {
      Postprocessor::PlotAxes axes;
      if (data.find_keyword("RANGE"))
      {
        std::array<double,2> x{}, y{};
        if (!data.get_value("xmin",x[0]) || !data.get_value("xmax",x[1]) ||
            !data.get_value("ymin",y[0]) || !data.get_value("ymax",y[1]))
        {
          PRINT_ERROR("Four numeric axis bounds are required.\n");
          return false;
        }
        axes.x = x;
        axes.y = y;
      }
      accepted = active->set_plot_axes(plot,axes,error);
    }
    else if (data.find_keyword("TITLE") || data.find_keyword("XLABEL") || data.find_keyword("YLABEL") || data.find_keyword("LABELS") || data.find_keyword("LABEL"))
    {
      std::string text;
      data.get_string("label_text",text);
      std::string labelTarget = "auto";
      if (data.find_keyword("TITLE"))
      {
        labelTarget = "title";
      }
      else if (data.find_keyword("XLABEL"))
      {
        labelTarget = "xlabel";
      }
      else if (data.find_keyword("YLABEL"))
      {
        labelTarget = "ylabel";
      }
      else if (data.find_keyword("LABEL"))
      {
        labelTarget = "curve";
      }
      accepted = active->set_plot_label(plot,labelTarget,text,name,error);
    }
    else if (data.find_keyword("CURVE"))
    {
      accepted = active->change_plot_curve(plot,name,[&]() -> std::common_type_t<decltype("remove"), const char*> {
        if (data.find_keyword("REMOVE")) {
          return "remove";
        } else {
          return [&]() -> const char* {
            if (data.find_keyword("HIDE")) {
              return "hide";
            } else {
              return "show";
            }
          }();
        }
      }(),error);
    }
    else if (data.find_keyword("ADD") && tableInput)
    {
      std::string table, x, y, column, value;
      data.get_string("table_name",table);
      data.get_string("x_column",x);
      data.get_string("y_column",y);
      std::vector<std::string> columns, values, rangeColumns, exactUnits, rangeUnits;
      std::vector<double> minima, maxima;
      data.get_strings("filter_columns",columns);
      data.get_strings("filter_values",values);
      data.get_strings("range_columns",rangeColumns);
      data.get_strings("exact_units",exactUnits);
      data.get_strings("range_units",rangeUnits);
      data.get_values("range_minima",minima);
      data.get_values("range_maxima",maxima);
      if (columns.size() != values.size() || rangeColumns.size() != minima.size() || minima.size() != maxima.size() ||
          (!exactUnits.empty() && exactUnits.size() != columns.size()) || (!rangeUnits.empty() && rangeUnits.size() != rangeColumns.size()))
      {
        PRINT_ERROR("Each filter needs a column and matching value or range bounds.\n");
        return false;
      }
      std::vector<Postprocessor::TableFilter> filters;
      for (std::size_t i = 0; i < columns.size(); ++i)
      {
        Postprocessor::TableFilter filter;
        filter.column = columns[i];
        filter.value = values[i];
        if (!exactUnits.empty())
        {
          filter.unit = exactUnits[i];
        }
        filters.push_back(filter);
      }
      for (std::size_t i = 0; i < rangeColumns.size(); ++i)
      {
        Postprocessor::TableFilter filter;
        filter.column = rangeColumns[i];
        filter.mode = Postprocessor::TableFilterMode::Range;
        filter.minimum = minima[i];
        filter.maximum = maxima[i];
        if (!rangeUnits.empty())
        {
          filter.unit = rangeUnits[i];
        }
        filters.push_back(filter);
      }
      const bool field = data.find_keyword("FIELD");
      Postprocessor::TableQualifiers tableQualifiers;
      Postprocessor::FieldQualifiers fieldQualifiers;
      if (!([&]() -> std::common_type_t<decltype(Postprocessor::read_field_qualifiers(data,fieldQualifiers,error)), decltype(Postprocessor::read_table_qualifiers(data,tableQualifiers,error))> {
        if (field) {
          return Postprocessor::read_field_qualifiers(data,fieldQualifiers,error);
        } else {
          return Postprocessor::read_table_qualifiers(data,tableQualifiers,error);
        }
      }()))
      {
        PRINT_ERROR("Cannot select table: %s\n",error.c_str());
        return false;
      }
      accepted = active->add_plot_table(plot,name,source,table,x,y,column,value,error,tableQualifiers,field,fieldQualifiers,filters);
    }
    else if (data.find_keyword("ADD"))
    {
      double number = 0;
      std::int64_t id = -1;
      std::string exact;
      if (data.get_string("entity_text",exact))
      {
        const auto parsed = std::from_chars(exact.data(),exact.data()+exact.size(),id);
        if (parsed.ec != std::errc{} || parsed.ptr != exact.data()+exact.size() || id < 0)
        {
          PRINT_ERROR("Invalid exact entity ID.\n");
          return false;
        }
      }
      else if (data.get_value("entity_id",number) && std::isfinite(number) && number >= 0 &&
          number <= 9007199254740991.0 && number == std::floor(number))
      {
        id = static_cast<std::int64_t>(number);
      }
      else
      {
        PRINT_ERROR("Invalid entity ID; quote large decimal IDs.\n");
        return false;
      }
      const auto kind = [&]() -> std::common_type_t<decltype(Postprocessor::EntityKind::IntegrationPoint), std::common_type_t<decltype(Postprocessor::EntityKind::Node), decltype(Postprocessor::EntityKind::Element)>> {
        if (data.find_keyword("INTEGRATIONPOINT")) {
          return Postprocessor::EntityKind::IntegrationPoint;
        } else {
          return [&]() -> std::common_type_t<decltype(Postprocessor::EntityKind::Node), decltype(Postprocessor::EntityKind::Element)> {
            if (data.find_keyword("NODE")) {
              return Postprocessor::EntityKind::Node;
            } else {
              return Postprocessor::EntityKind::Element;
            }
          }();
        }
      }();
      std::optional<std::int64_t> ip;
      if (kind == Postprocessor::EntityKind::IntegrationPoint)
      {
        if (!data.get_value("ip",number) || !std::isfinite(number) || number < 1 || number > 2147483647 || number != std::floor(number))
        {
          PRINT_ERROR("Integration point must be a positive integer.\n");
          return false;
        }
        ip = static_cast<std::int64_t>(number);
      }
      std::optional<PostprocessorApplicationInterface::DatasetSelector> dataset;
      std::string datasetName, field, component;
      if (data.get_string("dataset_name",datasetName))
      {
        dataset = datasetName;
      }
      else if (data.find_keyword("DATASET"))
      {
        if (!data.get_value("dataset_id",number) || !std::isfinite(number) || number < 0 ||
            number > 2147483647 || number != std::floor(number))
        {
          PRINT_ERROR("Invalid dataset ID.\n");
          return false;
        }
        dataset = static_cast<std::size_t>(number);
      }
      data.get_string("field_name",field);
      data.get_string("component_name",component);
      Postprocessor::FieldQualifiers qualifiers;
      if (!Postprocessor::read_field_qualifiers(data,qualifiers,error))
      {
        PRINT_ERROR("Cannot select field: %s\n",error.c_str());
        return false;
      }
      const bool magnitude = data.find_keyword("MAGNITUDE");
      if (magnitude)
      {
        component = "magnitude";
      }
      const auto reduction = [&]() -> std::common_type_t<decltype(Postprocessor::PlotReduction::Minimum), std::common_type_t<decltype(Postprocessor::PlotReduction::Maximum), decltype(Postprocessor::PlotReduction::None)>> {
        if (data.find_keyword("MINIMUM")) {
          return Postprocessor::PlotReduction::Minimum;
        } else {
          return [&]() -> std::common_type_t<decltype(Postprocessor::PlotReduction::Maximum), decltype(Postprocessor::PlotReduction::None)> {
            if (data.find_keyword("MAXIMUM")) {
              return Postprocessor::PlotReduction::Maximum;
            } else {
              return Postprocessor::PlotReduction::None;
            }
          }();
        }
      }();
      accepted = active->add_plot_entity(plot,name,source,dataset,kind,id,ip,field,component,magnitude,reduction,error,qualifiers);
    }
    else
    {
      data.get_string("replacement",replacement);
      const std::string action = [&]() -> std::string {
        if (data.find_keyword("RENAME")) {
          return "rename";
        } else {
          if (data.find_keyword("CLOSE")) {
            return "close";
          } else {
            if (data.find_keyword("REMOVE")) {
              return "remove";
            } else {
              return "show";
            }
          }
        }
      }();
      accepted = active->change_plot(target,action,replacement,error);
    }
    if (!accepted)
    {
      PRINT_ERROR("Cannot change/export plot: %s\n",error.c_str());
    }
    return accepted;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot change/export plot: %s\n",exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot change/export plot: unknown failure.\n");
  }
  return false;
}
