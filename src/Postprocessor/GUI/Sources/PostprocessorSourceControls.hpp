#ifndef POSTPROCESSORSOURCECONTROLS_HPP
#define POSTPROCESSORSOURCECONTROLS_HPP
#include "Framework/Results/ResultJobEntry.hpp"
#include <QWidget>
#include <functional>
#include <vector>
class QComboBox;
class QIcon;
class QToolButton;
class PostprocessorSourceControls : public QWidget
{
public:
  using Catalogue = std::function<bool(std::vector<SimResults::ResultJobEntry>&,std::string&)>;
  PostprocessorSourceControls(Catalogue catalogue,
    std::function<void(const SimResults::ResultJobEntry&)> addJob,
    std::function<void()> importFiles, QWidget* parent);
  void refresh();
  void set_import_icon(const QIcon& icon);
private:
  void showEvent(QShowEvent* event) override;
  Catalogue catalogue;
  std::vector<SimResults::ResultJobEntry> entries;
  QComboBox* sources = nullptr;
  QToolButton* add = nullptr;
  QToolButton* importButton = nullptr;
};
#endif
