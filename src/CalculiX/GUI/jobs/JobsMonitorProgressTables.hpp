#ifndef JOBSMONITORPROGRESSTABLES_HPP
#define JOBSMONITORPROGRESSTABLES_HPP

#include "Core/jobs/JobProgress.hpp"
#include <QObject>

class QWidget;
class QTableView;
class QCheckBox;
class QComboBox;
class QLabel;
class QGroupBox;
class QPlainTextEdit;
class QSortFilterProxyModel;
class JobsMonitorProgressModel;
class JobsMonitorConvergenceFilter;

// STA/CVG presentation for one job monitor, not a reusable panel framework.
class JobsMonitorProgressTables : public QObject
{
public:
  explicit JobsMonitorProgressTables(QWidget* owner);
  QWidget* status_page() const;
  QWidget* convergence_page() const;
  void clear();
  void update(const CalculiXJobProgress::Data& data, const std::string& error);

private:
  void update_filters();
  void apply_filters();
  void update_summary();
  QWidget* status;
  QWidget* convergence;
  QTableView* status_view;
  QTableView* convergence_view;
  JobsMonitorProgressModel* status_model;
  JobsMonitorProgressModel* convergence_model;
  QSortFilterProxyModel* status_proxy;
  JobsMonitorConvergenceFilter* convergence_proxy;
  QCheckBox* follow_status;
  QCheckBox* follow_convergence;
  QComboBox* step;
  QComboBox* increment;
  QComboBox* attempt;
  QLabel* status_summary;
  QLabel* convergence_summary;
  QGroupBox* status_unparsed;
  QGroupBox* convergence_unparsed;
  QPlainTextEdit* status_text;
  QPlainTextEdit* convergence_text;
  std::uint64_t generation = 0;
  std::uint64_t revision = 0;
  std::size_t unparsed_count = 0;
  std::size_t status_unparsed_count = 0;
  std::size_t convergence_unparsed_count = 0;
  std::string read_error;
};

#endif
