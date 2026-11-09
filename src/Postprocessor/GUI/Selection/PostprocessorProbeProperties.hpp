#ifndef POSTPROCESSOR_PROBEPROPERTIES_HPP
#define POSTPROCESSOR_PROBEPROPERTIES_HPP
#include <QGroupBox>
#include <memory>
class QLabel;
class PostprocessorApplicationInterface;
class PostprocessorProbeProperties : public QGroupBox
{
public:
  PostprocessorProbeProperties(std::weak_ptr<PostprocessorApplicationInterface> application, QWidget* parent);
  void refresh();
private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
  QLabel* result = nullptr;
};
#endif
