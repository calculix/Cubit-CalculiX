#include "VectorCommandText.hpp"
#include "Postprocessor/Commands/FieldCommandText.hpp"
namespace Postprocessor
{
  QString vector_command_text(const QString& operation,const std::string& source)
  {
    const auto quoted = field_command_text({source,{}});
    if (operation.isEmpty() || quoted.isEmpty() || operation.contains('\n') || operation.contains('\r'))
    {
      return {};
    }
    return "post vector "+operation+" source "+quoted.mid(6);
  }
  QString vector_field_command_text(const ResultFieldSelector& field,const std::string& source)
  {
    return vector_command_text(field_command_text(field),source);
  }
}
