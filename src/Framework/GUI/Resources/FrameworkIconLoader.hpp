#pragma once
#include <QIcon>
#include <string>

class FrameworkIconLoader {
public:
  static QIcon load(const std::string& componentDirectory, const std::string& filename);
};
