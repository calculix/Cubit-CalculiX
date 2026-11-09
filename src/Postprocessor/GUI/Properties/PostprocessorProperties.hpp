#ifndef POSTPROCESSORPROPERTIES_HPP
#define POSTPROCESSORPROPERTIES_HPP

#include "Postprocessor/Core/sources/DataSource.hpp"
#include <QWidget>
#include <memory>
#include <functional>

class QLabel;
class PostprocessorApplicationInterface;
class PostprocessorDisplayProperties;
class PostprocessorProbeProperties;

class PostprocessorProperties : public QWidget
{
public:
  PostprocessorProperties(std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<bool(const std::string&,std::string&)> submit, QWidget* parent = nullptr);
  void show_source(const Postprocessor::DataSource* source);

private:
  QLabel* name;
  QLabel* origin;
  PostprocessorDisplayProperties* display;
  PostprocessorProbeProperties* probe;
};

#endif // POSTPROCESSORPROPERTIES_HPP
