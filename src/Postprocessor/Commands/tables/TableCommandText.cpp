#include <type_traits>
#include "TableCommandText.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/FieldCommandText.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
namespace Postprocessor
{
  QString table_selector_text(const PostprocessorApplicationInterface& application,
    const TableReference& reference,std::string& error)
  {
    error.clear();
    if (reference.field)
    {
      ResultFieldSelector selector;
      const auto groups = application.field_groups(reference.source_id,std::nullopt);
      bool found = false;
      for (const auto& group : groups)
      {
        if (group.key == reference.key)
        {
          FieldSelection field{std::nullopt,group.field.array_name,group.field.association,{},false,group.key};
          found = readable_result_field(groups,field,selector,error);
          break;
        }
      }
      if (!found)
      {
        if (error.empty())
        {
          error = "The field table is unavailable in this source.";
        }
        return {};
      }
      // field_command_text owns common quoting/qualifier formatting.
      const auto text = field_command_text(selector);
      if (text.isEmpty())
      {
        error = "The field name or qualifiers cannot be quoted in a journal.";
        return {};
      }
      return text.mid(6);
    }
    ResultTableSelector selector;
    if (!readable_result_table(application.tables(reference.source_id),reference.key,selector,error))
    {
      return {};
    }
    QString result = plot_quote(selector.name);
    if (result.isEmpty())
    {
      error = "The table name cannot be quoted in a journal.";
      return {};
    }
    if (selector.qualifiers.global)
    {
      if (*selector.qualifiers.global) {
        result += " scope global";
      } else {
        result += " scope frame";
      }
    }
    if (selector.qualifiers.type)
    {
      const auto type = [&]() -> std::common_type_t<decltype(QStringLiteral("\"\"")), decltype(plot_quote(*selector.qualifiers.type))> {
        if (selector.qualifiers.type->empty()) {
          return QStringLiteral("\"\"");
        } else {
          return plot_quote(*selector.qualifiers.type);
        }
      }();
      if (type.isEmpty())
      {
        error = "The table type cannot be quoted in a journal.";
        return {};
      }
      result += " type " + type;
    }
    if (selector.qualifiers.entry)
    {
      result += " entry " + QString::number(static_cast<qulonglong>(*selector.qualifiers.entry));
    }
    return result;
  }
  QString table_open_command_text(const PostprocessorApplicationInterface& application,
    const TableReference& reference,std::string& error)
  {
    const auto selector = table_selector_text(application,reference,error);
    const auto source = application.session().sources.find(reference.source_id);
    if (selector.isEmpty() || source == application.session().sources.end())
    {
      if (error.empty())
      {
        error = "The table source is unavailable.";
      }
      return {};
    }
    const auto name = plot_quote(source->second.name);
    if (name.isEmpty())
    {
      error = "The source name cannot be quoted in a journal.";
      return {};
    }
    return QString([&]() -> const char* {
      if (reference.field) {
        return "post table field ";
      } else {
        return "post table open ";
      }
    }())+selector+" source "+name;
  }
}
