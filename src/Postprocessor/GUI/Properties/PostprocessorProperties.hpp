#ifndef POSTPROCESSORPROPERTIES_HPP
#define POSTPROCESSORPROPERTIES_HPP

#include "Postprocessor/Core/sources/DataSource.hpp"
#include <QWidget>
#include <memory>
#include <functional>

class QLabel;
class PostprocessorApplicationInterface;
class PostprocessorProbeProperties;

class PostprocessorProperties : public QWidget
{
public:
  PostprocessorProperties(std::weak_ptr<PostprocessorApplicationInterface> application,
    QWidget* parent = nullptr);
  void show_source(const Postprocessor::DataSource* source);

private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
  QLabel* name;
  QLabel* origin;

  PostprocessorProbeProperties* probe;
};

#endif // POSTPROCESSORPROPERTIES_HPP
