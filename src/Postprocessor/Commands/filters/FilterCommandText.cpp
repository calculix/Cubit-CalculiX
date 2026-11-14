#include "FilterCommandText.hpp"
#include <iomanip>
#include <locale>
#include <sstream>
namespace Postprocessor
{
  QString plane_command_text(const PlaneDefinition& plane)
  {
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text << std::setprecision(17) << "origin " << plane.origin[0] << ' ' << plane.origin[1] << ' ' << plane.origin[2]
      << " normal " << plane.normal[0] << ' ' << plane.normal[1] << ' ' << plane.normal[2];
    return QString::fromStdString(text.str());
  }
}
