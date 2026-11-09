#include "PostprocessorResultsTree.hpp"
#include <QSignalBlocker>
#include <QTreeWidgetItemIterator>
#include <QSet>
#include <QMenu>
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include <map>
#include <optional>

namespace
{
  enum Role
  {
    Kind = Qt::UserRole,
    SourceId,
    SourceName,
    Index,
    Key
  };

  QString association(SimResults::Association value)
  {
    switch (value)
    {
      case SimResults::Association::Node: return QObject::tr("Node");
      case SimResults::Association::Element: return QObject::tr("Element");
      case SimResults::Association::IntegrationPoint: return QObject::tr("Integration point");
      case SimResults::Association::Table: return QObject::tr("Table");
    }
    return {};
  }
}

PostprocessorResultsTree::PostprocessorResultsTree(std::function<void(const QString&)> select,
  std::function<void(const QString&, bool)> visibility, QWidget* parent)
  : QTreeWidget(parent)
{
  setObjectName("Postprocessor.ResultsTree");
  setHeaderLabel(tr("Sources / Results"));
  setSelectionMode(QAbstractItemView::SingleSelection);
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this,&QWidget::customContextMenuRequested,this,[this](const QPoint& position)
  {
    auto* item = itemAt(position);
    if (!item || !dataset_action)
    {
      return;
    }
    const auto kind = static_cast<ResultTreeItemKind>(item->data(0,Kind).toInt());
    if (kind != ResultTreeItemKind::Dataset && kind != ResultTreeItemKind::Source)
    {
      return;
    }
    const auto source = item->data(0,SourceName).toString();
    const auto id = item->data(0,SourceId).toString().toStdString();
    const auto dataset = static_cast<std::size_t>(item->data(0,Index).toULongLong());
    const auto snapshot = snapshots.find(id);
    const auto data = snapshot == snapshots.end() ? nullptr : snapshot->second;
    const auto view = activeView;
    QMenu menu(this);
    QAction* show = nullptr;
    QAction* hide = nullptr;
    QAction* only = nullptr;
    if (kind == ResultTreeItemKind::Dataset)
    {
      show = menu.addAction(tr("Show"));
      hide = menu.addAction(tr("Hide"));
      only = menu.addAction(tr("Show only"));
    }
    auto* all = menu.addAction(tr("Show all datasets"));
    const auto* chosen = menu.exec(viewport()->mapToGlobal(position));
    // A modal menu can outlive a tree rebuild. Never reuse item pointers or act
    // on a different snapshot/view after returning from its event loop.
    const auto current = snapshots.find(id);
    if (!chosen || !data || current == snapshots.end() || current->second != data || activeView != view)
    {
      return;
    }
    using Action = Postprocessor::DatasetVisibilityAction;
    if (chosen == all)
    {
      dataset_action(source,Action::ShowAll,std::nullopt);
    }
    else if (chosen == show || chosen == hide || chosen == only)
    {
      dataset_action(source,chosen == show ? Action::Show : chosen == hide ? Action::Hide : Action::ShowOnly,dataset);
    }
  });
  connect(this,&QTreeWidget::itemDoubleClicked,this,[this](QTreeWidgetItem* item,int)
  {
    if (item != nullptr && item->data(0,Kind).toInt() == static_cast<int>(ResultTreeItemKind::Field) && activate_field)
    {
      activate_field(item->data(0,SourceName).toString(),static_cast<std::size_t>(item->data(0,Index).toULongLong()));
    }
  });
  connect(this, &QTreeWidget::currentItemChanged, this, [select](QTreeWidgetItem* current, QTreeWidgetItem*)
  {
    if (current == nullptr)
    {
      return;
    }
    const auto kind = static_cast<ResultTreeItemKind>(current->data(0, Kind).toInt());
    const auto name = current->data(0, SourceName).toString();
    if (kind == ResultTreeItemKind::Source || kind == ResultTreeItemKind::Dataset || kind == ResultTreeItemKind::Field)
    {
      select(name);
    }
  });
  connect(this, &QTreeWidget::itemChanged, this, [this,visibility](QTreeWidgetItem* item, int column)
  {
    if (item == nullptr || column != 0 || !item->flags().testFlag(Qt::ItemIsUserCheckable))
    {
      return;
    }
    const auto kind = static_cast<ResultTreeItemKind>(item->data(0,Kind).toInt());
    if (kind == ResultTreeItemKind::Source)
    {
      visibility(item->data(0, SourceName).toString(), item->checkState(0) == Qt::Checked);
    }
    else if (kind == ResultTreeItemKind::Dataset && dataset_action)
    {
      dataset_action(item->data(0,SourceName).toString(),item->checkState(0) == Qt::Checked ?
        Postprocessor::DatasetVisibilityAction::Show : Postprocessor::DatasetVisibilityAction::Hide,
        static_cast<std::size_t>(item->data(0,Index).toULongLong()));
    }
  });
}

void PostprocessorResultsTree::refresh(const Postprocessor::PostprocessorSession& session,
  const Load& load, const HasMesh& has_mesh, const Groups& catalogue, const Datasets& datasets)
{
  const QSignalBlocker blocker(this);
  activeView = session.active_view_id;
  snapshots.clear();
  QSet<QString> expanded;
  QSet<QString> known;
  QString selectedKey;
  if (currentItem() != nullptr)
  {
    selectedKey = currentItem()->data(0, Key).toString();
  }
  for (QTreeWidgetItemIterator it(this); *it != nullptr; ++it)
  {
    const auto key = (*it)->data(0, Key).toString();
    known.insert(key);
    if ((*it)->isExpanded())
    {
      expanded.insert(key);
    }
  }
  clear();
  QTreeWidgetItem* selected = nullptr;
  QTreeWidgetItem* activeRoot = nullptr;
  for (const auto& entry : session.sources)
  {
    const auto& source = entry.second;
    const auto sourceId = QString::fromStdString(source.id);
    const auto sourceName = QString::fromUtf8(source.name.c_str());
    auto make = [&](QTreeWidgetItem* parent, const QString& text, ResultTreeItemKind kind, const QString& suffix, std::size_t index = 0)
    {
      auto* item = parent ? new QTreeWidgetItem(parent, {text}) : new QTreeWidgetItem(this, {text});
      const auto key = sourceId + ":" + suffix;
      item->setData(0, Kind, static_cast<int>(kind));
      item->setData(0, SourceId, sourceId);
      item->setData(0, SourceName, sourceName);
      item->setData(0, Index, QVariant::fromValue<qulonglong>(index));
      item->setData(0, Key, key);
      if (kind == ResultTreeItemKind::MeshGroup || kind == ResultTreeItemKind::FieldsGroup || kind == ResultTreeItemKind::ResultGroup)
      {
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
      }
      item->setExpanded(expanded.contains(key) || (kind == ResultTreeItemKind::Source && !known.contains(key)));
      if (key == selectedKey && source.id == session.active_source_id)
      {
        selected = item;
      }
      return item;
    };
    auto* root = make(nullptr, sourceName, ResultTreeItemKind::Source, "source");
    if (source.id == session.active_source_id)
    {
      activeRoot = root;
    }
    const auto data = load(source.id);
    snapshots[source.id] = data;
    std::optional<std::size_t> current;
    bool visible = false;
    std::set<std::size_t> hidden;
    for (const auto& display : session.representations)
    {
      const auto& value = display.second;
      if (value.input.owner == Postprocessor::OutputOwner::Source && value.input.object_id == source.id &&
          value.input.port == 0 && value.view_id == session.active_view_id)
      {
        current = value.increment_index;
        visible = value.visible;
        hidden = value.hidden_datasets;
        break;
      }
    }
    if (!data)
    {
      continue;
    }
    bool displayable = false;
    for (std::size_t i = 0; i < data->increments.size() && !displayable; ++i)
    {
      displayable = has_mesh(source.id,i);
    }
    if (displayable)
    {
      root->setFlags(root->flags() | Qt::ItemIsUserCheckable);
      root->setCheckState(0, visible ? Qt::Checked : Qt::Unchecked);
    }
    else
    {
      root->setToolTip(0, tr("This source has no displayable mesh increment."));
    }
    const auto datasetList = datasets ? datasets(source.id) :
      (!data->datasets.empty() ? data->datasets : Postprocessor::result_dataset_catalogue(*data));
    std::set<std::size_t> available;
    if (current && *current < data->increments.size())
    {
      for (const auto& part : Postprocessor::result_mesh_parts(data->increments[*current].dataset.get()))
      {
        available.insert(part.first);
      }
    }
    if (!datasetList.empty())
    {
      auto* mesh = make(root, tr("Mesh"), ResultTreeItemKind::MeshGroup, "mesh");
      for (const auto& dataset : datasetList)
      {
        auto label = QString::fromUtf8(dataset.name.c_str());
        if (label.isEmpty())
        {
          label = tr("Dataset %1").arg(static_cast<qulonglong>(dataset.id));
        }
        auto* item = make(mesh,label,ResultTreeItemKind::Dataset,"dataset:"+QString::number(static_cast<qulonglong>(dataset.id)),dataset.id);
        if (displayable)
        {
          item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
          item->setCheckState(0,hidden.count(dataset.id) == 0 ? Qt::Checked : Qt::Unchecked);
        }
        auto tooltip = tr("Dataset %1: %2 points, %3 cells").arg(static_cast<qulonglong>(dataset.id))
          .arg(static_cast<qulonglong>(dataset.point_count)).arg(static_cast<qulonglong>(dataset.cell_count));
        if (available.count(dataset.id) == 0)
        {
          tooltip += tr(" — not present in this frame");
        }
        if (!visible)
        {
          tooltip += tr(" — parent source is hidden");
        }
        item->setToolTip(0,tooltip);
      }
    }
    const auto groups = catalogue ? catalogue(source.id,current) : Postprocessor::result_field_groups(*data,current);
    if (!groups.empty())
    {
      auto* results = make(root,tr("Results"),ResultTreeItemKind::FieldsGroup,"results");
      std::map<std::string,std::vector<std::size_t>> names;
      for (std::size_t i = 0; i < groups.size(); ++i)
      {
        names[groups[i].field.name].push_back(i);
      }
      for (const auto& name : names)
      {
        auto* parent = results;
        if (name.second.size() > 1)
        {
          parent = make(results,QString::fromUtf8(name.first.c_str()),ResultTreeItemKind::ResultGroup,"result:"+QString::fromUtf8(name.first.c_str()));
        }
        for (auto index : name.second)
        {
          const auto& group = groups[index];
          const auto& field = group.field;
          QString label = QString::fromUtf8(field.name.c_str());
          if (name.second.size() > 1)
          {
            label = association(field.association) + " / " + QString::fromUtf8(field.source.c_str());
            if (!field.set_name.empty())
            {
              label += " / " + QString::fromUtf8(field.set_name.c_str());
            }
          }
          auto* item = make(parent,label,ResultTreeItemKind::Field,"field:"+QString::fromStdString(group.key),index);
          QStringList components;
          for (const auto& component : field.components)
          {
            components.append(QString::fromUtf8(component.c_str()));
          }
          item->setToolTip(0,components.join(", ") + (group.available ? QString() : tr(" (unavailable in this frame)")));
          if (!group.available || field.association == SimResults::Association::Table)
          {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
          }
        }
      }
    }
  }
  setCurrentItem(selected != nullptr ? selected : activeRoot);
}
