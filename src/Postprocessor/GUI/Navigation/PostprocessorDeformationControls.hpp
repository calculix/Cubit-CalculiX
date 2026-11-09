#ifndef POSTPROCESSORDEFORMATIONCONTROLS_HPP
#define POSTPROCESSORDEFORMATIONCONTROLS_HPP
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QWidget>
#include <functional>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
class QTimer;
class QHideEvent;

class PostprocessorDeformationControls : public QWidget
{
public:
  using Submit = std::function<bool(const std::string&,std::string&)>;
  PostprocessorDeformationControls(std::weak_ptr<PostprocessorApplicationInterface> application,
    Submit submit, Submit silent, QWidget* parent);
  void refresh();

protected:
  bool eventFilter(QObject* watched, QEvent* event) override;
  void hideEvent(QHideEvent* event) override;

private:
  struct Context
  {
    std::string source, name, view, representation;
    std::size_t frame = 0;
    std::shared_ptr<const SimResults::VtkResultsData> data;
    Postprocessor::ResultDeformation settings;
  };
  std::optional<Context> current() const;
  bool same_context(const Context& a, const Context& b) const;
  void populate_fields(const Context& context, const Postprocessor::ResultDeformation& settings);
  void change_flag(bool deformation, bool value);
  void select_field(int index);
  void edit_scale();
  void preview_scale();
  void finish_scale();
  void cancel_scale();
  bool read_scale(double& value) const;
  bool owned() const;
  bool dispatch(const std::string& operation, const Postprocessor::ResultDeformation& expected,
    bool preview, std::string& error);
  void show_error(const std::string& error);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Submit submit;
  Submit silent;
  std::optional<Context> baseline;
  std::optional<Context> scaleOrigin;
  std::vector<Postprocessor::ResultFieldGroup> catalogue;
  QCheckBox* enabled = nullptr;
  QComboBox* fields = nullptr;
  QLineEdit* scale = nullptr;
  QCheckBox* overlay = nullptr;
  QTimer* scaleTimer = nullptr;
  QLabel* errorLabel = nullptr;
  bool loading = false;
  bool dispatching = false;
};
#endif
