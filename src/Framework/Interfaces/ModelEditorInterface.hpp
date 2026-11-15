#pragma once
#include "Framework/ModelExport.hpp"
#include <functional>
#include <string>

class CUBITSIM_MODEL_EXPORT ModelEditorInterface {
public:
  using Open = std::function<bool(const std::string&, const std::string&, int, std::string&)>;
  using Close = std::function<bool(const std::string&, const std::string&, std::string&)>;
  static void bind(Open open, Close close);
  static void unbind() noexcept;
  static bool open(const std::string& workspace, const std::string& feature, int material, std::string& error);
  static bool close(const std::string& workspace, const std::string& feature, std::string& error);
};
