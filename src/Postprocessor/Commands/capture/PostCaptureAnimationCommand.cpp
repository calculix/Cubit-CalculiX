#include "PostCaptureAnimationCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <algorithm>
#include <cctype>
#include <exception>
#include <utility>
PostCaptureAnimationCommand::PostCaptureAnimationCommand(std::weak_ptr<PostprocessorApplicationInterface> value)
  : application(std::move(value))
{
}
std::vector<std::string> PostCaptureAnimationCommand::get_syntax()
{
  return {"post capture animation {status|cancel}",
    "post capture animation view <string:type='quoted',number='1',label='animation_view'> format {gif|mp4|png}"
    " file <string:type='quoted',number='1',label='animation_file'>"
    " [source <string:type='quoted',number='1',label='source_name'>]"
    " [width <value:label='animation_width'> height <value:label='animation_height'>]"
    " [first <value:label='first_frame'> last <value:label='last_frame'>]"
    " [resultfps <value:label='result_fps'>] [fps <value:label='output_fps'>] [passes <value:label='passes'>]"
    " [loop <string:type='unquoted',number='1',label='animation_loop'>]"
    " [rotatex <string:type='unquoted',number='1',label='rotate_x'> anglex <value:label='angle_x'>"
    " rotatey <string:type='unquoted',number='1',label='rotate_y'> angley <value:label='angle_y'>"
    " rotatez <string:type='unquoted',number='1',label='rotate_z'> anglez <value:label='angle_z'>]"
    " [background <string:type='unquoted',number='1',label='animation_background'>]"
    " [legend <string:type='unquoted',number='1',label='animation_legend'>]"
    " [labels <string:type='unquoted',number='1',label='animation_labels'>]"
    " [triad <string:type='unquoted',number='1',label='animation_triad'>]"};
}
std::vector<std::string> PostCaptureAnimationCommand::get_syntax_help()
{
  return {"post capture animation view \"view1\" format gif|mp4|png file \"new path\" [source \"name\"] [width W height H] [first N last N] [resultfps N] [fps N] [passes N] [loop on|off] [background current|white|transparent] [legend on|off] [labels on|off] [triad on|off]",
    "post capture animation status; post capture animation cancel",
    "PNG output is a new directory of frames plus animation.json. MP4 requires open-source FFmpeg/libx264."};
}
std::vector<std::string> PostCaptureAnimationCommand::get_help()
{
  return {"Start asynchronous animation capture from a frozen view. GUI/OpenGL rendering stays on its owner thread; reusable Framework utilities encode/write on workers.",
    "Result FPS controls mesh timing; output FPS controls smooth camera samples. Existing files/directories are not overwritten. Reset requests cancellation.",
    "GIF uses centisecond timing (1..100 FPS). MP4 uses libx264 H.264/yuv420p and requires even dimensions. PNG retains transparency."};
}
bool PostCaptureAnimationCommand::execute(CubitCommandData& data)
{
  std::string error;
  try
  {
    const auto app = application.lock();
    if (!app)
    {
      PRINT_ERROR("Animation capture is unavailable.\n");
      return false;
    }
    if (data.find_keyword("STATUS"))
    {
      PRINT_INFO("%s\n",app->animation_capture_status().c_str());
      return true;
    }
    if (data.find_keyword("CANCEL"))
    {
      app->cancel_animation_capture();
      PRINT_INFO("%s\n",app->animation_capture_status().c_str());
      return true;
    }
    Postprocessor::AnimationCaptureDefinition request;
    std::string path;
    bool valid = data.get_string("animation_view",request.image.name) && data.get_string("animation_file",path);
    request.output.path = QString::fromStdString(path);
    data.get_string("source_name",request.source);
    if (data.find_keyword("MP4"))
    {
      request.output.format = Framework::Media::AnimationFormat::Mp4;
    }
    else if (data.find_keyword("PNG"))
    {
      request.output.format = Framework::Media::AnimationFormat::Png;
    }
    const auto integer = [&](const char* keyword,const char* label,int minimum,int maximum,int& output)
    {
      if (!data.find_keyword(keyword))
      {
        return;
      }
      double number = 0;
      if (!data.get_value(label,number) || !std::isfinite(number) || number != std::floor(number) || number < minimum || number > maximum)
      {
        valid = false;
        error = std::string("Invalid animation integer: ")+keyword;
        return;
      }
      output = static_cast<int>(number);
    };
    integer("WIDTH","animation_width",64,8192,request.image.width);
    integer("HEIGHT","animation_height",64,8192,request.image.height);
    integer("FPS","output_fps",1,120,request.output.fps);
    integer("RESULTFPS","result_fps",1,120,request.result_fps);
    integer("PASSES","passes",1,1000,request.passes);
    int first = 1, last = 0;
    integer("FIRST","first_frame",1,2147483647,first);
    integer("LAST","last_frame",1,2147483647,last);
    request.first = static_cast<std::size_t>(first);
    request.last = static_cast<std::size_t>(last);
    const auto read_text = [&](const char* label,std::string& value)
    {
      if (!data.get_string(label,value))
      {
        return false;
      }
      std::transform(value.begin(),value.end(),value.begin(),[](unsigned char character)
      {
        return static_cast<char>(std::tolower(character));
      });
      return true;
    };
    const auto flag = [&](const char* keyword,const char* label,std::optional<bool>& output)
    {
      if (data.find_keyword(keyword))
      {
        std::string value;
        if (!read_text(label,value) || (value != "on" && value != "off"))
        {
          valid = false;
          error = std::string(keyword)+" must be on or off.";
        }
        else
        {
          output = value == "on";
        }
      }
    };
    flag("LEGEND","animation_legend",request.image.legend);
    flag("LABELS","animation_labels",request.image.labels);
    flag("TRIAD","animation_triad",request.image.triad);
    std::optional<bool> loop;
    flag("LOOP","animation_loop",loop);
    request.output.loop = loop.value_or(true);
    if (data.find_keyword("ROTATEX"))
    {
      Postprocessor::AnimationRotation rotation;
      const char* flags[] = {"rotate_x","rotate_y","rotate_z"};
      const char* angles[] = {"angle_x","angle_y","angle_z"};
      for (std::size_t axis = 0; axis < rotation.size(); ++axis)
      {
        std::string value;
        if (!read_text(flags[axis],value) || (value != "on" && value != "off") ||
            !data.get_value(angles[axis],rotation[axis].degrees) || !std::isfinite(rotation[axis].degrees))
        {
          valid = false;
          error = "Export rotation requires on/off and finite degree values for all three axes.";
        }
        rotation[axis].enabled = value == "on";
      }
      request.rotation = rotation;
    }
    if (data.find_keyword("BACKGROUND"))
    {
      std::string value;
      read_text("animation_background",value);
      if (value == "white")
      {
        request.image.background = Postprocessor::CaptureBackground::White;
      }
      else if (value == "transparent")
      {
        request.image.background = Postprocessor::CaptureBackground::Transparent;
      }
      else if (value != "current")
      {
        valid = false;
        error = "Animation background must be current, white or transparent.";
      }
    }
    if (valid && app->start_animation_capture(request,error))
    {
      PRINT_INFO("Animation export started. Use post capture animation status or cancel.\n");
      return true;
    }
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  catch (...)
  {
    error = "Unexpected animation capture failure.";
  }
  PRINT_ERROR("Cannot capture animation: %s\n",error.c_str());
  return false;
}
