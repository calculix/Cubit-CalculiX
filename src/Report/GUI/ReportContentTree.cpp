#include "ReportContentTree.hpp"
#include "ReportCommandText.hpp"
#include "Framework/Reporting/ReportTemplate.hpp"
#include "CubitMessage.hpp"
#include <QSignalBlocker>

ReportContentTree::ReportContentTree(QWidget* parent, std::function<bool(const std::string&, std::string&)> submit) : QTreeWidget(parent)
{
  setHeaderLabel(tr("Report content"));
  connect(this, &QTreeWidget::itemChanged, this, [submit](QTreeWidgetItem* item, int)
  {
    std::string key, error;
    if (!ReportCommandText::quote(item->data(0, Qt::UserRole).toString(), key, error) ||
        !submit("report configure " + key + (item->checkState(0) == Qt::Checked ? " \"true\"" : " \"false\""), error))
    {
      PRINT_ERROR("Cannot change report content: %s\n", error.c_str());
    }
  });
}

ReportContentTree::~ReportContentTree() = default;

void ReportContentTree::refresh(const ReportTemplate& definition, const ReportConfiguration& configuration)
{
  QSignalBlocker guard(this);
  const QString selected = currentItem() ? currentItem()->data(0, Qt::UserRole).toString() : QString();
  clear();
  for (const auto& option : definition.options)
  {
    if (option.kind != "boolean")
    {
      continue;
    }
    auto* item = new QTreeWidgetItem(this);
    item->setText(0, QString::fromStdString(option.label));
    item->setData(0, Qt::UserRole, QString::fromStdString(option.key));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    const auto entry = configuration.values.find(option.key);
    item->setCheckState(0, entry != configuration.values.end() && entry->second == "true" ? Qt::Checked : Qt::Unchecked);
    if (item->data(0, Qt::UserRole).toString() == selected)
    {
      setCurrentItem(item);
    }
  }
}
