#include "CameraCommandText.hpp"
#include "Postprocessor/Core/views/CameraOperations.hpp"
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

bool format_camera_command(const Postprocessor::CameraDefinition& camera, std::string& command, std::string& error)
{
  error.clear();
  if (!Postprocessor::CameraOperations::valid(camera))
  {
    error = "Cannot format an invalid camera.";
    return false;
  }
  std::ostringstream text;
  text.imbue(std::locale::classic());
  text << std::setprecision(std::numeric_limits<double>::max_digits10) << "post view camera position";
  for (const auto value : camera.position)
  {
    text << ' ' << value;
  }
  text << " focal";
  for (const auto value : camera.focal_point)
  {
    text << ' ' << value;
  }
  text << " up";
  for (const auto value : camera.view_up)
  {
    text << ' ' << value;
  }
  text << " projection " << (camera.parallel_projection ? "parallel" : "perspective")
    << " angle " << camera.vertical_view_angle_degrees << " scale " << camera.parallel_scale;
  command = text.str();
  return true;
}
