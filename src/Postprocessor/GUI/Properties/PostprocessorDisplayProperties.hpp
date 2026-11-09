#ifndef POSTPROCESSOR_DISPLAYPROPERTIES_HPP
#define POSTPROCESSOR_DISPLAYPROPERTIES_HPP

#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include <QWidget>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class PostprocessorApplicationInterface;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;

// A page-local draft. Only Apply submits commands; animation refreshes preserve
// edits while the representation, result snapshot and committed coloring agree.
class PostprocessorDisplayProperties : public QWidget
{
public:
  using Submit = std::function<bool(const std::string&, std::string&)>;
  PostprocessorDisplayProperties(std::weak_ptr<PostprocessorApplicationInterface> application,
    Submit submit, QWidget* parent = nullptr);
  void refresh();
  bool apply(std::string& error);

private:
  struct Context
  {
    std::string source_id;
    std::string source_name;
    std::string view_id;
    std::string view_name;
    std::string representation_id;
    std::size_t increment = 0;
    std::shared_ptr<const SimResults::VtkResultsData> data;
    Postprocessor::ResultColoring coloring;
  };
  struct Command
  {
    std::string text;
    Postprocessor::ResultColoring expected;
  };
  bool current_context(Context& context) const;
  bool same_context(const Context& context) const;
  void load(const Postprocessor::ResultColoring& coloring);
  void changed();
  void update_enabled();
  bool draft(Postprocessor::ResultColoring& result, std::string& error) const;
  bool commands(const Postprocessor::ResultColoring& target, std::vector<Command>& plan, std::string& error) const;
  void choose_color(QPushButton* button);
  void recover(const std::string& error);

  std::weak_ptr<PostprocessorApplicationInterface> application;
  Submit submit;
  Context baseline;
  bool available = false;
  bool dirty = false;
  bool loading = false;
  bool applying = false;
  QLabel* view;
  QLabel* field;
  QLabel* component;
  QComboBox* palette;
  QCheckBox* reversed;
  QComboBox* contours;
  QSpinBox* bands;
  QComboBox* range;
  QLineEdit* rangeMinimum;
  QLineEdit* rangeMaximum;
  QCheckBox* minimumEnabled;
  QCheckBox* maximumEnabled;
  QLineEdit* minimumValue;
  QLineEdit* maximumValue;
  QPushButton* minimumColor;
  QPushButton* maximumColor;
  QCheckBox* legend;
  QPushButton* applyButton;
  QLabel* errorMessage;
};
#endif
