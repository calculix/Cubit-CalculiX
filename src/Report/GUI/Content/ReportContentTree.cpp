#include "ReportContentTree.hpp"
#include "Report/GUI/Settings/ReportSettingsCommandText.hpp"
#include "CubitMessage.hpp"
#include <QSignalBlocker>
#include <QTreeWidgetItemIterator>
#include <QScrollBar>
#include <QTimer>
#include <QComboBox>
#include <map>
#include <set>
#include <utility>
#include <algorithm>

namespace
{
constexpr int groupRole = Qt::UserRole + 1;
constexpr int gateRole = Qt::UserRole + 2;
constexpr int choiceRole = Qt::UserRole + 3;

QString identity(QTreeWidgetItem* item)
{
  const auto group = item->data(0, groupRole).toString();
  if (!group.isEmpty())
  {
    return "group:" + group;
  }
  return "option:" + item->data(0, Qt::UserRole).toString();
}
}

ReportContentTree::ReportContentTree(QWidget* parent, std::function<bool(const std::string&, std::string&)> submit)
  : QTreeWidget(parent), submit(std::move(submit))
{
  setObjectName("ReportContentTree");
  setColumnCount(2);
  setHeaderLabels({tr("Report content"), tr("Selection")});
  connect(this, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int)
  {
    change(item);
  });
}

ReportContentTree::~ReportContentTree() = default;

void ReportContentTree::schedule_refresh()
{
  if (refreshPending)
  {
    return;
  }
  refreshPending = true;
  QTimer::singleShot(0, this, [this]()
  {
    refreshPending = false;
    refresh(definition, configuration);
  });
}

void ReportContentTree::collect(QTreeWidgetItem* item, bool checked, ReportConfiguration& patch) const
{
  if (item->data(0, choiceRole).toBool())
  {
    return;
  }
  std::string value = "false";
  if (checked)
  {
    value = "true";
  }
  const auto key = item->data(0, Qt::UserRole).toString().toStdString();
  const auto gate = item->data(0, gateRole).toString().toStdString();
  if (!key.empty())
  {
    patch.values[key] = value;
  }
  if (!gate.empty())
  {
    patch.values[gate] = value;
  }
  for (int i = 0; i < item->childCount(); ++i)
  {
    collect(item->child(i), checked, patch);
  }
}

void ReportContentTree::change(QTreeWidgetItem* item)
{
  if (item->data(0, choiceRole).toBool())
  {
    return;
  }
  ReportConfiguration patch;
  const bool checked = item->checkState(0) != Qt::Unchecked;
  collect(item, checked, patch);
  if (checked)
  {
    for (auto* parent = item->parent(); parent; parent = parent->parent())
    {
      const auto gate = parent->data(0, gateRole).toString().toStdString();
      if (!gate.empty())
      {
        patch.values[gate] = "true";
      }
    }
  }
  submit_patch(patch);
}

void ReportContentTree::submit_patch(const ReportConfiguration& patch)
{
  if (patch.values.empty())
  {
    return;
  }
  std::string command, error;
  if (!ReportSettingsCommandText::build(patch, QString(), command, error, &definition))
  {
    PRINT_ERROR("Cannot change report content: %s\n", error.c_str());
    schedule_refresh();
    return;
  }
  const auto previous = configuration;
  for (const auto& entry : patch.values)
  {
    configuration.values[entry.first] = entry.second;
  }
  // Dispatch may rebuild the widget through an observer: don't reuse item afterwards.
  if (!submit(command, error))
  {
    configuration = previous;
    PRINT_ERROR("Cannot change report content: %s\n", error.c_str());
  }
  // Don't delete the edited item inside Qt's itemChanged signal delivery.
  schedule_refresh();
}

void ReportContentTree::refresh(const ReportTemplate& nextDefinition, const ReportConfiguration& nextConfiguration)
{
  const auto newDefinition = nextDefinition;
  const auto newConfiguration = nextConfiguration;
  QSignalBlocker guard(this);
  std::map<QString, bool> expansion;
  for (QTreeWidgetItemIterator it(this); *it; ++it)
  {
    expansion[identity(*it)] = (*it)->isExpanded();
  }
  QString selected;
  if (currentItem())
  {
    selected = identity(currentItem());
  }
  const int scroll = verticalScrollBar()->value();
  definition = newDefinition;
  configuration = newConfiguration;
  clear();
  std::map<std::string, QTreeWidgetItem*> groups;
  std::set<std::string> gates;
  for (const auto& group : definition.content_groups)
  {
    auto* item = new QTreeWidgetItem();
    item->setText(0, QString::fromStdString(group.label));
    item->setData(0, groupRole, QString::fromStdString(group.id));
    item->setData(0, gateRole, QString::fromStdString(group.gate_option));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    groups[group.id] = item;
    if (!group.gate_option.empty())
    {
      gates.insert(group.gate_option);
    }
  }
  for (const auto& group : definition.content_groups)
  {
    auto* item = groups.at(group.id);
    const auto parent = groups.find(group.parent_id);
    if (parent == groups.end())
    {
      addTopLevelItem(item);
    }
    else
    {
      parent->second->addChild(item);
    }
  }
  for (const auto& option : definition.options)
  {
    if ((option.kind != "boolean" && option.kind != "choice") || gates.count(option.key) != 0)
    {
      continue;
    }
    auto* item = new QTreeWidgetItem();
    const auto parent = groups.find(option.group_id);
    if (parent == groups.end())
    {
      addTopLevelItem(item);
    }
    else
    {
      parent->second->addChild(item);
    }
    item->setText(0, QString::fromStdString(option.label));
    item->setData(0, Qt::UserRole, QString::fromStdString(option.key));
    if (option.kind == "choice")
    {
      item->setData(0, choiceRole, true);
      item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
      auto* editor = new QComboBox(this);
      for (const auto& value : option.choices)
      {
        QString label = QString::fromStdString(value);
        if (value == "last_per_step")
        {
          label = tr("Last frame of each step");
        }
        else if (value == "envelope")
        {
          label = tr("Envelope across selected frames");
        }
        else if (value == "all_frames")
        {
          label = tr("Every selected frame");
        }
        editor->addItem(label, QString::fromStdString(value));
      }
      std::string value = option.default_value;
      const auto found = configuration.values.find(option.key);
      if (found != configuration.values.end())
      {
        value = found->second;
      }
      editor->setCurrentIndex(editor->findData(QString::fromStdString(value)));
      setItemWidget(item, 1, editor);
      connect(editor, QOverload<int>::of(&QComboBox::activated), this,
        [this, editor, key = option.key](int index)
        {
          ReportConfiguration patch;
          patch.values[key] = editor->itemData(index).toString().toStdString();
          submit_patch(patch);
        });
      continue;
    }
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    std::string value = option.default_value;
    const auto found = configuration.values.find(option.key);
    if (found != configuration.values.end())
    {
      value = found->second;
    }
    Qt::CheckState state = Qt::Unchecked;
    if (value == "true")
    {
      state = Qt::Checked;
    }
    item->setCheckState(0, state);
  }
  std::function<void(QTreeWidgetItem*, bool)> update = [&](QTreeWidgetItem* item, bool enabled)
  {
    const auto gate = item->data(0, gateRole).toString().toStdString();
    bool open = true;
    if (!gate.empty())
    {
      const auto found = configuration.values.find(gate);
      if (found != configuration.values.end())
      {
        open = found->second == "true";
      }
      else
      {
        const auto option = std::find_if(definition.options.begin(), definition.options.end(), [&](const ReportOption& value)
        {
          return value.key == gate;
        });
        open = option != definition.options.end() && option->default_value == "true";
      }
    }
    item->setDisabled(!enabled);
    for (int i = 0; i < item->childCount(); ++i)
    {
      update(item->child(i), enabled && open);
    }
    if (item->data(0, groupRole).toString().isEmpty())
    {
      return;
    }
    bool any = false;
    bool all = true;
    bool hasChecks = false;
    for (int i = 0; i < item->childCount(); ++i)
    {
      if (item->child(i)->data(0, choiceRole).toBool())
      {
        continue;
      }
      hasChecks = true;
      const auto state = item->child(i)->checkState(0);
      any = any || state != Qt::Unchecked;
      all = all && state == Qt::Checked;
    }
    Qt::CheckState state = Qt::Unchecked;
    if (open && all && hasChecks)
    {
      state = Qt::Checked;
    }
    else if (open && any)
    {
      state = Qt::PartiallyChecked;
    }
    item->setCheckState(0, state);
  };
  for (int i = 0; i < topLevelItemCount(); ++i)
  {
    update(topLevelItem(i), true);
  }
  for (QTreeWidgetItemIterator it(this); *it; ++it)
  {
    const auto id = identity(*it);
    const auto previous = expansion.find(id);
    bool expanded = (*it)->parent() == nullptr;
    if (previous != expansion.end())
    {
      expanded = previous->second;
    }
    (*it)->setExpanded(expanded);
    if (id == selected)
    {
      setCurrentItem(*it);
    }
  }
  verticalScrollBar()->setValue(scroll);
}
