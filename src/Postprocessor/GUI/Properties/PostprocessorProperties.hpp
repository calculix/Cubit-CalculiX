#ifndef POSTPROCESSORPROPERTIES_HPP
#define POSTPROCESSORPROPERTIES_HPP

#include "Postprocessor/Core/sources/DataSource.hpp"
#include "PostprocessorAppearanceControls.hpp"
#include <QWidget>
#include <memory>
#include <functional>

class QLabel;
class PostprocessorApplicationInterface;
class PostprocessorProbeProperties;
class PostprocessorTopologyCutoffControls;

class PostprocessorProperties : public QWidget
{
public:
  PostprocessorProperties(std::weak_ptr<PostprocessorApplicationInterface> application,
    QWidget* parent = nullptr,PostprocessorAppearanceControls::Submit submit = {});
  void show_source(const Postprocessor::DataSource* source);
  void set_appearance_targets(const std::vector<Postprocessor::AppearanceTarget>& targets);

private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
  QLabel* name;
  QLabel* origin;

  PostprocessorTopologyCutoffControls* topologyCutoff;
  PostprocessorProbeProperties* probe;
  PostprocessorAppearanceControls* appearance;
};

#endif // POSTPROCESSORPROPERTIES_HPP
