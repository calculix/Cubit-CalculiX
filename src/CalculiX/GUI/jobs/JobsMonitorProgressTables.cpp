#include "JobsMonitorProgressTables.hpp"
#include <QAbstractTableModel>
#include <QAction>
#include <QApplication>
#include <QBrush>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QFont>
#include <QFontMetrics>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QTableView>
#include <QVBoxLayout>
#include <algorithm>
#include <set>

class JobsMonitorProgressModel : public QAbstractTableModel
{
public:
  JobsMonitorProgressModel(bool status, QObject* parent) : QAbstractTableModel(parent), status(status)
  {
  }

  int rowCount(const QModelIndex& parent = {}) const override
  {
    return parent.isValid() ? 0 : static_cast<int>(status ? sta.size() : cvg.size());
  }

  int columnCount(const QModelIndex& parent = {}) const override
  {
    return parent.isValid() ? 0 : (status ? 8 : 9);
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
  {
    static const QStringList status_headers =
    {
      "Step", "Increment", "Attempt", "Outcome", "Iterations",
      "Total\ntime", "Step\ntime", "Increment\nsize"
    };
    static const QStringList convergence_headers =
    {
      "Step", "Increment", "Attempt", "Iteration", "Contact\nelements",
      "Force\nresidual [%]", "Displacement\ncorrection [%]",
      "Flux\nresidual [%]", "Temperature\ncorrection [%]"
    };
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
      const auto& headers = status ? status_headers : convergence_headers;
      if (section >= 0 && section < headers.size())
      {
        return headers[section];
      }
    }
    if (orientation == Qt::Horizontal && role == Qt::ToolTipRole)
    {
      const auto& headers = status ? status_headers : convergence_headers;
      if (section >= 0 && section < headers.size())
      {
        return headers[section] + (status ? "\nTimes are solver time, not elapsed wall time. U marks an unconverged attempt."
          : "\nPercentages are reported by CalculiX; no generic convergence threshold is applied.");
      }
    }
    return {};
  }

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
  {
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount() || index.column() >= columnCount())
    {
      return {};
    }
    if (role == Qt::TextAlignmentRole)
    {
      return static_cast<int>((status && index.column() == 3 ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter);
    }
    if (status && sta[index.row()].unconverged)
    {
      if (role == Qt::ForegroundRole)
      {
        return QBrush(Qt::darkRed);
      }
      if (role == Qt::FontRole)
      {
        QFont font;
        font.setBold(true);
        return font;
      }
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
      return {};
    }
    const int column = index.column();
    QVariant value;
    if (status)
    {
      const auto& row = sta[index.row()];
      switch (column)
      {
        case 0: value = row.step; break;
        case 1: value = row.increment; break;
        case 2: value = row.attempt; break;
        case 3: value = row.unconverged ? "Unconverged" : "Converged"; break;
        case 4: value = row.iterations; break;
        default: value = row.time[column - 5]; break;
      }
    }
    else
    {
      const auto& row = cvg[index.row()];
      switch (column)
      {
        case 0: value = row.step; break;
        case 1: value = row.increment; break;
        case 2: value = row.attempt; break;
        case 3: value = row.iteration; break;
        case 4: value = row.contact_elements; break;
        default: value = row.measures[column - 5]; break;
      }
    }
    if (role == Qt::DisplayRole && column >= 5)
    {
      return QString::number(value.toDouble(), 'g', 12);
    }
    return value;
  }

  void update(const CalculiXJobProgress::Data& data, bool reset)
  {
    const auto size = status ? data.status.size() : data.convergence.size();
    const int old = rowCount();
    if (reset)
    {
      beginResetModel();
      sta.clear();
      cvg.clear();
      if (status)
      {
        sta = data.status;
      }
      else
      {
        cvg = data.convergence;
      }
      endResetModel();
    }
    else if (size > static_cast<std::size_t>(old))
    {
      if (status)
      {
        sta.reserve(size);
      }
      else
      {
        cvg.reserve(size);
      }
      beginInsertRows({}, old, static_cast<int>(size) - 1);
      if (status)
      {
        sta.insert(sta.end(), data.status.begin() + old, data.status.end());
      }
      else
      {
        cvg.insert(cvg.end(), data.convergence.begin() + old, data.convergence.end());
      }
      endInsertRows();
    }
  }

  const std::vector<CalculiXJobProgress::StatusRow>& status_rows() const
  {
    return sta;
  }

  const std::vector<CalculiXJobProgress::ConvergenceRow>& convergence_rows() const
  {
    return cvg;
  }

private:
  bool status;
  std::vector<CalculiXJobProgress::StatusRow> sta;
  std::vector<CalculiXJobProgress::ConvergenceRow> cvg;
};

class JobsMonitorConvergenceFilter : public QSortFilterProxyModel
{
public:
  explicit JobsMonitorConvergenceFilter(QObject* parent) : QSortFilterProxyModel(parent)
  {
  }

  void set_filters(int step, int increment, int attempt)
  {
    selected = {step, increment, attempt};
    invalidateFilter();
  }

protected:
  bool filterAcceptsRow(int row, const QModelIndex& parent) const override
  {
    for (int column = 0; column < 3; ++column)
    {
      if (selected[column] > 0 && sourceModel()->data(sourceModel()->index(row, column, parent), Qt::EditRole).toInt() != selected[column])
      {
        return false;
      }
    }
    return true;
  }

private:
  std::array<int, 3> selected{};
};

namespace
{
  void configure(QTableView* view, QSortFilterProxyModel* proxy, QCheckBox* follow)
  {
    proxy->setSortRole(Qt::EditRole);
    view->setModel(proxy);
    view->setSortingEnabled(true);
    view->sortByColumn(-1, Qt::AscendingOrder);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setAlternatingRowColors(true);
    view->setWordWrap(false);
    view->setStyleSheet("QTableView::item { padding: 0px 3px; }");
    auto* rows = view->verticalHeader();
    rows->hide();
    const int row_height = view->fontMetrics().height() + 4;
    rows->setMinimumSectionSize(row_height);
    rows->setDefaultSectionSize(row_height);
    rows->setSectionResizeMode(QHeaderView::Fixed);
    auto* headers = view->horizontalHeader();
    headers->setSectionResizeMode(QHeaderView::Interactive);
    headers->setMinimumSectionSize(40);
    headers->setStretchLastSection(false);
    headers->setResizeContentsPrecision(250);
    headers->setFixedHeight(2 * headers->fontMetrics().height() + 8);
    view->resizeColumnsToContents();
    auto* copy = new QAction("Copy rows with headers", view);
    copy->setShortcut(QKeySequence::Copy);
    copy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    view->addAction(copy);
    view->setContextMenuPolicy(Qt::ActionsContextMenu);
    QObject::connect(copy, &QAction::triggered, view, [view]()
    {
      auto rows = view->selectionModel()->selectedRows();
      if (rows.empty())
      {
        return;
      }
      std::sort(rows.begin(), rows.end(), [](const QModelIndex& a, const QModelIndex& b)
      {
        return a.row() < b.row();
      });
      QStringList lines;
      QStringList headers;
      for (int column = 0; column < view->model()->columnCount(); ++column)
      {
        QString title = view->model()->headerData(column, Qt::Horizontal).toString();
        title.replace('\n', ' ');
        headers.append(title);
      }
      lines.append(headers.join('\t'));
      for (const auto& row : rows)
      {
        QStringList values;
        for (int column = 0; column < view->model()->columnCount(); ++column)
        {
          values.append(view->model()->data(view->model()->index(row.row(), column)).toString());
        }
        lines.append(values.join('\t'));
      }
      QApplication::clipboard()->setText(lines.join('\n'));
    });
    QObject::connect(view->verticalScrollBar(), &QScrollBar::sliderMoved, follow, [follow](int)
    {
      follow->setChecked(false);
    });
    QObject::connect(view, &QTableView::pressed, follow, [follow](const QModelIndex&)
    {
      follow->setChecked(false);
    });
    QObject::connect(view->verticalScrollBar(), &QScrollBar::actionTriggered, follow, [view, follow](int)
    {
      if (view->verticalScrollBar()->sliderPosition() < view->verticalScrollBar()->maximum())
      {
        follow->setChecked(false);
      }
    });
  }

  void follow_latest(QTableView* view, QSortFilterProxyModel* proxy, QCheckBox* follow)
  {
    if (!follow->isChecked())
    {
      return;
    }
    for (int row = proxy->sourceModel()->rowCount() - 1; row >= 0; --row)
    {
      const auto mapped = proxy->mapFromSource(proxy->sourceModel()->index(row, 0));
      if (mapped.isValid())
      {
        view->scrollTo(mapped, QAbstractItemView::PositionAtBottom);
        break;
      }
    }
  }

  void choices(QComboBox* combo, const std::set<int>& values)
  {
    const int selected = combo->currentData().toInt();
    const QSignalBlocker blocked(combo);
    combo->clear();
    combo->addItem("All", 0);
    for (int value : values)
    {
      combo->addItem(QString::number(value), value);
    }
    combo->setCurrentIndex((std::max)(0, combo->findData(selected)));
  }

  QGroupBox* unparsed_box(QWidget* page, QPlainTextEdit*& text)
  {
    auto* box = new QGroupBox("Unparsed lines (0)", page);
    box->setCheckable(true);
    box->setChecked(false);
    auto* layout = new QVBoxLayout(box);
    text = new QPlainTextEdit(box);
    text->setReadOnly(true);
    text->hide();
    layout->addWidget(text);
    QObject::connect(box, &QGroupBox::toggled, text, &QWidget::setVisible);
    return box;
  }
}

JobsMonitorProgressTables::JobsMonitorProgressTables(QWidget* owner) : QObject(owner)
{
  status = new QWidget(owner);
  convergence = new QWidget(owner);
  status_model = new JobsMonitorProgressModel(true, this);
  convergence_model = new JobsMonitorProgressModel(false, this);
  status_proxy = new QSortFilterProxyModel(this);
  convergence_proxy = new JobsMonitorConvergenceFilter(this);
  status_proxy->setSourceModel(status_model);
  convergence_proxy->setSourceModel(convergence_model);
  auto* status_layout = new QVBoxLayout(status);
  auto* convergence_layout = new QVBoxLayout(convergence);
  status_layout->setContentsMargins(4, 4, 4, 4);
  status_layout->setSpacing(3);
  convergence_layout->setContentsMargins(4, 4, 4, 4);
  convergence_layout->setSpacing(3);
  follow_status = new QCheckBox("Follow latest", status);
  follow_convergence = new QCheckBox("Follow latest", convergence);
  follow_status->setChecked(true);
  follow_convergence->setChecked(true);
  status_summary = new QLabel(status);
  convergence_summary = new QLabel(convergence);
  status_summary->setWordWrap(true);
  convergence_summary->setWordWrap(true);
  status_view = new QTableView(status);
  convergence_view = new QTableView(convergence);
  configure(status_view, status_proxy, follow_status);
  configure(convergence_view, convergence_proxy, follow_convergence);
  auto* status_controls = new QHBoxLayout();
  status_controls->setContentsMargins(0, 0, 0, 0);
  status_controls->setSpacing(4);
  status_controls->addWidget(status_summary);
  status_controls->addStretch();
  status_controls->addWidget(follow_status);
  status_layout->addLayout(status_controls);
  status_layout->addWidget(status_view, 1);
  auto* filters = new QHBoxLayout();
  filters->setContentsMargins(0, 0, 0, 0);
  filters->setSpacing(4);
  step = new QComboBox(convergence);
  increment = new QComboBox(convergence);
  attempt = new QComboBox(convergence);
  filters->addWidget(new QLabel("Step", convergence));
  filters->addWidget(step);
  filters->addWidget(new QLabel("Increment", convergence));
  filters->addWidget(increment);
  filters->addWidget(new QLabel("Attempt", convergence));
  filters->addWidget(attempt);
  filters->addWidget(convergence_summary);
  filters->addStretch();
  filters->addWidget(follow_convergence);
  convergence_layout->addLayout(filters);
  convergence_layout->addWidget(convergence_view, 1);
  status_unparsed = unparsed_box(status, status_text);
  convergence_unparsed = unparsed_box(convergence, convergence_text);
  status_layout->addWidget(status_unparsed);
  convergence_layout->addWidget(convergence_unparsed);
  for (auto* combo : {step, increment, attempt})
  {
    connect(combo, &QComboBox::currentIndexChanged, this, [this](int)
    {
      update_filters();
      apply_filters();
    });
  }
  connect(status_view->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
    [this](const QModelIndex& current, const QModelIndex&)
    {
      const auto source = status_proxy->mapToSource(current);
      if (!source.isValid())
      {
        return;
      }
      const auto& row = status_model->status_rows()[source.row()];
      const std::array<int, 3> values{row.step, row.increment, row.attempt};
      int i = 0;
      for (auto* combo : {step, increment, attempt})
      {
        const QSignalBlocker blocked(combo);
        int index = combo->findData(values[i]);
        if (index < 0)
        {
          combo->addItem(QString::number(values[i]), values[i]);
          index = combo->count() - 1;
        }
        combo->setCurrentIndex(index);
        ++i;
      }
      apply_filters();
    });
  connect(follow_status, &QCheckBox::toggled, this, [this](bool)
  {
    follow_latest(status_view, status_proxy, follow_status);
  });
  connect(follow_convergence, &QCheckBox::toggled, this, [this](bool)
  {
    follow_latest(convergence_view, convergence_proxy, follow_convergence);
  });
  clear();
}

QWidget* JobsMonitorProgressTables::status_page() const
{
  return status;
}

QWidget* JobsMonitorProgressTables::convergence_page() const
{
  return convergence;
}

void JobsMonitorProgressTables::clear()
{
  generation = 0;
  revision = 0;
  unparsed_count = 0;
  status_unparsed_count = 0;
  convergence_unparsed_count = 0;
  read_error.clear();
  status_model->update({}, true);
  convergence_model->update({}, true);
  status_text->clear();
  convergence_text->clear();
  status_unparsed->setTitle("Unparsed lines (0)");
  convergence_unparsed->setTitle("Unparsed lines (0)");
  status_unparsed->hide();
  convergence_unparsed->hide();
  for (auto* combo : {step, increment, attempt})
  {
    const QSignalBlocker blocked(combo);
    combo->clear();
    combo->addItem("All", 0);
  }
  follow_status->setChecked(true);
  follow_convergence->setChecked(true);
  apply_filters();
}

void JobsMonitorProgressTables::update_filters()
{
  std::set<int> steps;
  std::set<int> increments;
  std::set<int> attempts;
  for (const auto& row : convergence_model->convergence_rows())
  {
    steps.insert(row.step);
  }
  choices(step, steps);
  for (const auto& row : convergence_model->convergence_rows())
  {
    if (step->currentData().toInt() == 0 || step->currentData().toInt() == row.step)
    {
      increments.insert(row.increment);
    }
  }
  choices(increment, increments);
  for (const auto& row : convergence_model->convergence_rows())
  {
    if ((step->currentData().toInt() == 0 || step->currentData().toInt() == row.step) &&
      (increment->currentData().toInt() == 0 || increment->currentData().toInt() == row.increment))
    {
      attempts.insert(row.attempt);
    }
  }
  choices(attempt, attempts);
}

void JobsMonitorProgressTables::apply_filters()
{
  convergence_proxy->set_filters(step->currentData().toInt(), increment->currentData().toInt(), attempt->currentData().toInt());
  update_summary();
  follow_latest(convergence_view, convergence_proxy, follow_convergence);
}

void JobsMonitorProgressTables::update_summary()
{
  const auto suffix = read_error.empty() ? QString() : "\n" + QString::fromStdString(read_error);
  status_summary->setText((status_model->rowCount() == 0 ? QString("No status records yet.") :
    QString("%1 reported increment attempts. Times are solver time.").arg(status_model->rowCount())) + suffix);
  convergence_summary->setText((convergence_model->rowCount() == 0 ? QString("No convergence records yet.") :
    QString("%1 of %2 iterations.").arg(convergence_proxy->rowCount()).arg(convergence_model->rowCount())) + suffix);
}

void JobsMonitorProgressTables::update(const CalculiXJobProgress::Data& data, const std::string& error)
{
  read_error = error;
  if (generation == data.generation && revision == data.revision)
  {
    update_summary();
    return;
  }
  const bool reset = generation != data.generation;
  if (reset)
  {
    clear();
    read_error = error;
  }
  const bool first_status_rows = status_model->rowCount() == 0 && !data.status.empty();
  const bool first_convergence_rows = convergence_model->rowCount() == 0 && !data.convergence.empty();
  status_model->update(data, reset);
  convergence_model->update(data, reset);
  // Fit only the first populated batch of each generation, not every poll.
  // Subsequent user column resizing must remain in effect during live updates.
  if (first_status_rows)
  {
    status_view->resizeColumnsToContents();
    const int outcome_width = status_view->fontMetrics().horizontalAdvance("Unconverged") + 24;
    status_view->setColumnWidth(3, (std::max)(status_view->columnWidth(3), outcome_width));
  }
  if (first_convergence_rows)
  {
    convergence_view->resizeColumnsToContents();
  }
  for (std::size_t i = unparsed_count; i < data.unparsed.size(); ++i)
  {
    const auto& line = data.unparsed[i];
    auto* text = line.source == "STA" ? status_text : convergence_text;
    text->appendPlainText(QString("Line %1: %2").arg(static_cast<qulonglong>(line.line)).arg(QString::fromStdString(line.text)));
    if (line.source == "STA")
    {
      ++status_unparsed_count;
    }
    else
    {
      ++convergence_unparsed_count;
    }
  }
  unparsed_count = data.unparsed.size();
  status_unparsed->setTitle(QString("Unparsed lines (%1)").arg(static_cast<qulonglong>(status_unparsed_count)));
  convergence_unparsed->setTitle(QString("Unparsed lines (%1)").arg(static_cast<qulonglong>(convergence_unparsed_count)));
  status_unparsed->setVisible(status_unparsed_count > 0);
  convergence_unparsed->setVisible(convergence_unparsed_count > 0);
  generation = data.generation;
  revision = data.revision;
  update_filters();
  apply_filters();
  follow_latest(status_view, status_proxy, follow_status);
}
