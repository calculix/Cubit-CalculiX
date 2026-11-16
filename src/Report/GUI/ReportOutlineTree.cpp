#include "ReportOutlineTree.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include <QSignalBlocker>
#include <QTreeWidgetItemIterator>
#include <QScrollBar>
#include <array>
#include <map>
#include <utility>

namespace
{
constexpr int pageRole = Qt::UserRole + 1;
using Counts = std::array<qulonglong, 3>;

Counts counts(const ReportSection& section)
{
  Counts result = {static_cast<qulonglong>(section.tables.size()), 0, static_cast<qulonglong>(section.images.size())};
  for (const auto& table : section.tables)
  {
    result[1] += static_cast<qulonglong>(table.rows.size());
  }
  for (const auto& child : section.children)
  {
    const auto nested = counts(child);
    for (std::size_t i = 0; i < result.size(); ++i)
    {
      result[i] += nested[i];
    }
  }
  return result;
}

QString description(const Counts& value)
{
  return QString("%1 tables / %2 rows / %3 images").arg(value[0]).arg(value[1]).arg(value[2]);
}
}

ReportOutlineTree::ReportOutlineTree(QWidget* parent, std::function<void(int)> navigate)
  : QTreeWidget(parent)
{
  setObjectName("ReportOutlineTree");
  setColumnCount(3);
  setHeaderLabels({tr("Generated report"), tr("Content"), tr("Page")});
  connect(this, &QTreeWidget::itemActivated, this, [navigate](QTreeWidgetItem* item, int)
  {
    const int page = item->data(0, pageRole).toInt();
    if (page > 0)
    {
      navigate(page);
    }
  });
}

ReportOutlineTree::~ReportOutlineTree() = default;

void ReportOutlineTree::refresh(std::shared_ptr<const ReportDocument> document,
  const std::vector<std::pair<std::string, int>>* pages, int pageCount)
{
  // A pre-layout refresh of the same immutable document must retain its page map.
  if (!pages && snapshot == document)
  {
    return;
  }
  std::vector<std::pair<std::string, int>> nextPages;
  if (pages)
  {
    nextPages = *pages;
  }
  if (snapshot == document && pageMap == nextPages && preparedPages == pageCount)
  {
    return;
  }
  QSignalBlocker guard(this);
  std::map<QString, bool> expanded;
  for (QTreeWidgetItemIterator it(this); *it; ++it)
  {
    expanded[(*it)->data(0, Qt::UserRole).toString()] = (*it)->isExpanded();
  }
  QString selected;
  if (currentItem())
  {
    selected = currentItem()->data(0, Qt::UserRole).toString();
  }
  const int scroll = verticalScrollBar()->value();
  snapshot = std::move(document);
  pageMap = std::move(nextPages);
  preparedPages = pageCount;
  clear();
  if (!snapshot)
  {
    return;
  }
  std::map<std::string, int> locations;
  for (const auto& entry : pageMap)
  {
    locations[entry.first] = entry.second + 1;
  }
  auto* root = new QTreeWidgetItem(this);
  root->setText(0, QString::fromStdString(snapshot->title));
  root->setData(0, Qt::UserRole, "report");
  Counts totals = {0, 0, 0};
  std::function<void(const ReportSection&, QTreeWidgetItem*)> append = [&](const ReportSection& section, QTreeWidgetItem* parent)
  {
    auto* item = new QTreeWidgetItem(parent);
    item->setText(0, QString::fromStdString(section.title));
    item->setText(1, description(counts(section)));
    item->setData(0, Qt::UserRole, QString::fromStdString(section.id));
    const auto location = locations.find(section.id);
    if (location != locations.end())
    {
      item->setText(2, QString::number(location->second));
      item->setData(0, pageRole, location->second);
    }
    for (std::size_t index = 0; index < section.tables.size(); ++index)
    {
      const auto& table = section.tables[index];
      auto* child = new QTreeWidgetItem(item);
      child->setText(0, QString::fromStdString(table.title));
      child->setText(1, tr("%1 rows / %2 columns").arg(static_cast<qulonglong>(table.rows.size())).arg(static_cast<qulonglong>(table.columns.size())));
      child->setData(0, Qt::UserRole, QString::fromStdString(section.id + "/table/" + std::to_string(index)));
      child->setData(0, pageRole, item->data(0, pageRole));
      child->setToolTip(0, tr("Opens the containing section; individual table page positions are not recorded."));
    }
    for (std::size_t index = 0; index < section.images.size(); ++index)
    {
      const auto& image = section.images[index];
      auto* child = new QTreeWidgetItem(item);
      child->setText(0, QString::fromStdString(image.caption));
      child->setText(1, tr("Image"));
      child->setData(0, Qt::UserRole, QString::fromStdString(section.id + "/image/" + std::to_string(index)));
      child->setData(0, pageRole, item->data(0, pageRole));
      child->setToolTip(0, tr("Opens the containing section."));
    }
    for (const auto& child : section.children)
    {
      append(child, item);
    }
  };
  for (const auto& section : snapshot->sections)
  {
    const auto value = counts(section);
    for (std::size_t i = 0; i < totals.size(); ++i)
    {
      totals[i] += value[i];
    }
    append(section, root);
  }
  root->setText(1, description(totals));
  if (preparedPages > 0)
  {
    root->setText(2, tr("%1 pages").arg(preparedPages));
  }
  if (!pages)
  {
    root->setToolTip(0, tr("Content counts are available; pagination has not completed."));
  }
  auto* diagnostics = new QTreeWidgetItem(root);
  diagnostics->setText(0, tr("Sources and diagnostics"));
  diagnostics->setText(1, tr("%1 diagnostics").arg(static_cast<qulonglong>(snapshot->diagnostics.size())));
  diagnostics->setData(0, Qt::UserRole, "report.diagnostics");
  const auto location = locations.find("report.diagnostics");
  if (location != locations.end())
  {
    diagnostics->setText(2, QString::number(location->second));
    diagnostics->setData(0, pageRole, location->second);
  }
  for (QTreeWidgetItemIterator it(this); *it; ++it)
  {
    const auto id = (*it)->data(0, Qt::UserRole).toString();
    const auto previous = expanded.find(id);
    bool open = (*it)->parent() == nullptr;
    if (previous != expanded.end())
    {
      open = previous->second;
    }
    (*it)->setExpanded(open);
    if (id == selected)
    {
      setCurrentItem(*it);
    }
  }
  verticalScrollBar()->setValue(scroll);
}
