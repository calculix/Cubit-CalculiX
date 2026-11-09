#ifndef POSTPROCESSOR_PICKCONTROLS_HPP
#define POSTPROCESSOR_PICKCONTROLS_HPP
#include <QWidget>
#include <functional>
#include <memory>
class PostprocessorApplicationInterface;
class QButtonGroup;
class PostprocessorPickControls : public QWidget
{
public:
  PostprocessorPickControls(std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<void(const QString&)> submit, QWidget* parent);
  void refresh();
private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<void(const QString&)> submit;
  QButtonGroup* modes = nullptr;
};
#endif
