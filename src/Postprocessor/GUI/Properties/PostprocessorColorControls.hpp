#ifndef POSTPROCESSOR_COLORCONTROLS_HPP
#define POSTPROCESSOR_COLORCONTROLS_HPP
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QWidget>
#include <functional>
class QComboBox;
class PostprocessorColorControls : public QWidget
{
public:
  PostprocessorColorControls(std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<void(const QString&)> submit, QWidget* parent = nullptr);
  void refresh();
  void apply_field(const QString& source, std::size_t field_index);
private:
  void choose_field(int index);
  void publish();
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<void(const QString&)> submit;
  QComboBox* fields;
  QComboBox* components;
  std::string source;
  std::vector<Postprocessor::ResultFieldGroup> catalogue;
};
#endif
