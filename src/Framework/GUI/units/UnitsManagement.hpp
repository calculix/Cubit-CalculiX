#pragma once
#include "Framework/FrameworkExport.hpp"
#include <QDialog>
#include <functional>
#include <string>

class QComboBox;
class QTableWidget;
class QLineEdit;
class QCloseEvent;

class CUBITSIM_FRAMEWORK_EXPORT UnitsManagement : public QDialog {
public:
  using Submit = std::function<bool(const std::string&, std::string&)>;
  UnitsManagement(std::string workspace, std::string commandPrefix, Submit submit, QWidget* parent = nullptr);
  void refresh();
  bool close_from_command();
  void reject() override;
protected:
  void closeEvent(QCloseEvent* event) override;
private:
  void submit_command(const std::string& command);
  void request_close();
  std::string workspace;
  std::string prefix;
  Submit submit;
  QComboBox* systems = nullptr;
  QComboBox* quantities = nullptr;
  QLineEdit* expression = nullptr;
  QTableWidget* table = nullptr;
  bool commandClose = false;
  bool closePending = false;
};
