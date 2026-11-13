#ifndef POSTPROCESSORANNOTATIONCONTROLS_HPP
#define POSTPROCESSORANNOTATIONCONTROLS_HPP
#include <QWidget>
#include <QString>
#include <functional>
#include <memory>
class QToolButton;
class PostprocessorApplicationInterface;
class PostprocessorAnnotationControls : public QWidget
{
public:
  PostprocessorAnnotationControls(std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<void(const QString&)> submit, QWidget* parent);
  void refresh();
private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<void(const QString&)> submit;
  QToolButton* extrema = nullptr;
  QToolButton* add = nullptr;
  QToolButton* visible = nullptr;
};
#endif
