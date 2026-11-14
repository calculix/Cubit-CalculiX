#pragma once
#include "Postprocessor/Core/tables/ResultTables.hpp"
#include <QString>
class PostprocessorApplicationInterface;
namespace Postprocessor
{
  // Return quoted name plus readable qualifiers, without an open/field/table verb.
  // All resolution uses the application interface and immutable metadata catalogues.
  QString table_selector_text(const PostprocessorApplicationInterface& application,
    const TableReference& reference,std::string& error);
  QString table_open_command_text(const PostprocessorApplicationInterface& application,
    const TableReference& reference,std::string& error);
}
