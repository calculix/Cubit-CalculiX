#include "PostprocessorResultsTree.hpp"
#include <QSignalBlocker>
#include <QHeaderView>
#include <QTreeWidgetItemIterator>
#include <QSet>
#include <QMenu>
#include <QTimer>
#include <QScopedValueRollback>
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include <map>
#include <algorithm>
#include <optional>

namespace
{
  enum Role
  {
    Kind = Qt::UserRole,
    SourceId,
    SourceName,
    Index,
    Key,
    TableKey,
    Category
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
  setHeaderLabels({tr("Sources / Results"),tr("ID")});
  header()->setStretchLastSection(false);
  header()->setSectionResizeMode(0, QHeaderView::Stretch);
  header()->setSectionResizeMode(1, QHeaderView::Interactive);
  header()->resizeSection(1, 55);
  setSelectionMode(QAbstractItemView::SingleSelection);
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this,&QWidget::customContextMenuRequested,this,[this](const QPoint& position)
  {
    auto* item = itemAt(position);
    if (!item || item->data(0,Kind).toInt() == static_cast<int>(ResultTreeItemKind::Source))
    {
      const auto sourceId = item ? item->data(0,SourceId).toString().toStdString() : std::string{};
      const auto sourceName = item ? item->data(0,SourceName).toString() : QString{};
      const auto viewId = activeView;
      const auto found = snapshots.find(sourceId);
      const auto snapshot = found == snapshots.end() ? nullptr : found->second;
      std::vector<SimResults::ResultJobEntry> jobs;
      std::string error;
      const bool catalogueReady = available_jobs && available_jobs(jobs,error);
      QMenu menu(this);
      auto* add = menu.addMenu(tr("Add job results"));
      std::map<QAction*,std::size_t> jobActions;
      std::map<std::string,QMenu*> groups;
      for (std::size_t index = 0; index < jobs.size(); ++index)
      {
        const auto& job = jobs[index];
        auto*& group = groups[job.reference.application_id];
        if (!group)
        {
          group = add->addMenu(QString::fromUtf8(job.application_name.c_str()));
        }
        auto* action = group->addAction(QString::fromUtf8(job.job_name.c_str())+tr(" — Job %1").arg(job.reference.job_id));
        action->setEnabled(job.available);
        action->setToolTip(QString::fromUtf8(job.unavailable_reason.c_str()));
        jobActions.emplace(action,index);
      }
      add->setEnabled(catalogueReady && !jobs.empty());
      add->setToolTip(QString::fromUtf8(error.c_str()));
      auto* import = menu.addAction(tr("Import result files…"));
      std::map<QAction*,SourceAction> actions;
      if (item)
      {
        menu.addSeparator();
        const bool mesh = snapshot && Postprocessor::first_mesh_increment(*snapshot).has_value();
        auto action = [&](const QString& label,SourceAction operation,bool enabled)
        {
          auto* result = menu.addAction(label);
          result->setEnabled(enabled);
          actions.emplace(result,operation);
        };
        action(tr("Show source"),SourceAction::Show,mesh);
        action(tr("Hide source"),SourceAction::Hide,mesh);
        action(tr("Show only this source"),SourceAction::ShowOnly,mesh);
        action(tr("Refresh source"),SourceAction::Refresh,true);
        menu.addSeparator();
        action(tr("Remove source"),SourceAction::Remove,true);
        action(tr("Remove all sources…"),SourceAction::RemoveAll,true);
      }
      const auto* chosen = menu.exec(viewport()->mapToGlobal(position));
      if (!chosen)
      {
        return;
      }
      if (chosen == import && import_files)
      {
        import_files();
        return;
      }
      const auto job = jobActions.find(const_cast<QAction*>(chosen));
      if (job != jobActions.end() && add_job)
      {
        add_job(jobs[job->second]);
        return;
      }
      const auto action = actions.find(const_cast<QAction*>(chosen));
      const auto current = snapshots.find(sourceId);
      if (action != actions.end() && source_action && activeView == viewId &&
          current != snapshots.end() && current->second == snapshot)
      {
        source_action(sourceName,action->second);
      }
      return;
    }
    const auto kind = static_cast<ResultTreeItemKind>(item->data(0,Kind).toInt());
    if ((kind == ResultTreeItemKind::Field || kind == ResultTreeItemKind::Table) && open_table)
    {
      const auto source = item->data(0,SourceName).toString();
      const auto id = item->data(0,SourceId).toString().toStdString();
      const auto key = item->data(0,TableKey).toString();
      const auto view = activeView;
      const auto snapshot = snapshots.at(id);
      QMenu menu(this);
      auto* open = menu.addAction(tr("Open as table"));
      const auto* chosen = menu.exec(viewport()->mapToGlobal(position));
      const auto found = snapshots.find(id);
      if (chosen == open && found != snapshots.end() && found->second == snapshot && activeView == view)
      {
        open_table(source,key,kind == ResultTreeItemKind::Field);
      }
      return;
    }
    if (!dataset_action || (kind != ResultTreeItemKind::Dataset && kind != ResultTreeItemKind::Source))
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
    if (item && item->data(0,Kind).toInt() == static_cast<int>(ResultTreeItemKind::Table) && open_table)
    {
      const auto source = item->data(0,SourceName).toString();
      const auto key = item->data(0,TableKey).toString();
      const auto id = item->data(0,SourceId).toString().toStdString();
      const auto snapshot = snapshots.at(id);
      const auto view = activeView;
      QTimer::singleShot(0,this,[this,source,key,id,snapshot,view]()
      {
        const auto found = snapshots.find(id);
        if (found != snapshots.end() && found->second == snapshot && activeView == view && open_table)
        {
          open_table(source,key,false);
        }
      });
      return;
    }
    if (item != nullptr && item->data(0,Kind).toInt() == static_cast<int>(ResultTreeItemKind::Field) && activate_field)
    {
      activate_field(item->data(0,SourceName).toString(),static_cast<std::size_t>(item->data(0,Index).toULongLong()));
    }
  });
  connect(this, &QTreeWidget::currentItemChanged, this, [this,select](QTreeWidgetItem* current, QTreeWidgetItem*)
  {
    if (current == nullptr)
    {
      return;
    }
    const auto kind = static_cast<ResultTreeItemKind>(current->data(0, Kind).toInt());
    const auto name = current->data(0, SourceName).toString();
    if (kind == ResultTreeItemKind::Source || kind == ResultTreeItemKind::Dataset || kind == ResultTreeItemKind::Field)
    {
      const auto id = current->data(0,SourceId).toString().toStdString();
      if (id == activeSource)
      {
        return;
      }
      const auto found = snapshots.find(id);
      const auto snapshot = found == snapshots.end() ? nullptr : found->second;
      const auto view = activeView;
      QTimer::singleShot(0,this,[this,select,name,id,snapshot,view]()
      {
        const auto found = snapshots.find(id);
        if (activeView == view && id != activeSource && found != snapshots.end() && found->second == snapshot)
        {
          select(name);
        }
      });
    }
  });
  connect(this, &QTreeWidget::itemChanged, this, [this,visibility](QTreeWidgetItem* item, int column)
  {
    if (item == nullptr || column != 0 || !item->flags().testFlag(Qt::ItemIsUserCheckable))
    {
      return;
    }
    const auto kind = static_cast<ResultTreeItemKind>(item->data(0,Kind).toInt());
    const auto name = item->data(0,SourceName).toString();
    const auto id = item->data(0,SourceId).toString().toStdString();
    const auto dataset = static_cast<std::size_t>(item->data(0,Index).toULongLong());
    const bool shown = item->checkState(0) != Qt::Unchecked;
    const auto category = static_cast<Postprocessor::DatasetCategory>(item->data(0,Category).toInt());
    const auto found = snapshots.find(id);
    const auto snapshot = found == snapshots.end() ? nullptr : found->second;
    const auto view = activeView;
    QTimer::singleShot(0,this,[this,visibility,name,id,dataset,shown,kind,category,snapshot,view]()
    {
      const auto found = snapshots.find(id);
      if (activeView != view || found == snapshots.end() || found->second != snapshot)
      {
        return;
      }
      if (kind == ResultTreeItemKind::Source)
      {
        visibility(name,shown);
      }
      else if (kind == ResultTreeItemKind::Category && category_action)
      {
        category_action(name,category,shown);
      }
      else if (kind == ResultTreeItemKind::Dataset && dataset_action)
      {
        dataset_action(name,shown ? Postprocessor::DatasetVisibilityAction::Show :
          Postprocessor::DatasetVisibilityAction::Hide,dataset);
      }
    });
  });
}

void PostprocessorResultsTree::refresh(const Postprocessor::PostprocessorSession& session,
  const Load& load, const HasMesh& has_mesh, const Groups& catalogue, const Datasets& datasets, const Tables& tables, const Availability& availability)
{
  if (refreshing)
  {
    return;
  }
  const QScopedValueRollback<bool> guard(refreshing,true);
  const QSignalBlocker blocker(this);
  activeView = session.active_view_id;
  activeSource = session.active_source_id;
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
  QTreeWidgetItem* activeField = nullptr;
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
    const auto loadedData = load(source.id);
    snapshots[source.id] = loadedData;
    // A local metadata-only fallback lets unavailable sources retain their
    // saved catalogue in the tree without publishing fabricated result data.
    const auto data = loadedData ? loadedData : std::make_shared<SimResults::VtkResultsData>();
    if (!loadedData)
    {
      root->setText(0,sourceName+tr(" [Unavailable]"));
      const auto reason = availability ? availability(source.id) : std::string("Result source is unavailable.");
      root->setToolTip(0,QString::fromUtf8(reason.c_str()));
    }
    std::optional<std::size_t> current;
    bool visible = false;
    std::set<std::size_t> hidden;
    std::optional<Postprocessor::FieldSelection> coloringField;
    for (const auto& display : session.representations)
    {
      const auto& value = display.second;
      if (value.input.owner == Postprocessor::OutputOwner::Source && value.input.object_id == source.id &&
          value.input.port == 0 && value.view_id == session.active_view_id)
      {
        current = value.increment_index;
        visible = value.visible;
        hidden = value.hidden_datasets;
        coloringField = value.coloring.field;
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
    else if (loadedData)
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
      QTreeWidgetItem* integration = nullptr;
      std::map<std::string,QTreeWidgetItem*> categories;
      for (const auto& dataset : datasetList)
      {
        auto label = QString::fromUtf8(dataset.name.c_str());
        if (label.isEmpty())
        {
          label = tr("Dataset %1").arg(static_cast<qulonglong>(dataset.id));
        }
        QTreeWidgetItem* parent = nullptr;
        if (dataset.kind == "DAT_IntegrationPoints")
        {
          if (!integration)
          {
            integration = make(root,tr("Integration points"),ResultTreeItemKind::MeshGroup,"integrationpoints");
          }
          parent = integration;
          if (label.startsWith("ip_",Qt::CaseInsensitive))
          {
            label = label.mid(3);
          }
        }
        else
        {
          const std::string category = dataset.kind == "Blocks" || dataset.kind == "DAT_Elements" ? "blocks" :
            dataset.kind == "Nodesets" || dataset.kind == "DAT_Nodes" ? "nodesets" :
            dataset.kind == "Surfaces" ? "sidesets" : "other";
          auto& group = categories[category];
          if (!group)
          {
            const QString title = category == "blocks" ? tr("Blocks / Element sets") : category == "nodesets" ? tr("Node sets") :
              category == "sidesets" ? tr("Side sets / Surfaces") : tr("Other datasets");
            group = make(root,title,ResultTreeItemKind::MeshGroup,"mesh:"+QString::fromStdString(category));
          }
          parent = group;
          if (!dataset.element_type.empty())
          {
            label += " ("+QString::fromUtf8(dataset.element_type.c_str())+")";
          }
          if (dataset.kind == "DAT_Nodes" || dataset.kind == "DAT_Elements")
          {
            label += tr(" [DAT output]");
          }
        }
        if (parent && dataset.kind != "" && dataset.kind != "Unknown")
        {
          for (auto category : {Postprocessor::DatasetCategory::Blocks,Postprocessor::DatasetCategory::NodeSets,
              Postprocessor::DatasetCategory::SideSets,Postprocessor::DatasetCategory::IntegrationPoints})
          {
            if (Postprocessor::dataset_in_category(dataset,category))
            {
              parent->setData(0,Kind,static_cast<int>(ResultTreeItemKind::Category));
              parent->setData(0,Category,static_cast<int>(category));
            }
          }
        }
        auto* item = make(parent,label,ResultTreeItemKind::Dataset,"dataset:"+QString::number(static_cast<qulonglong>(dataset.id)),dataset.id);
        if (dataset.model_id >= 0)
        {
          item->setText(1,QString::number(dataset.model_id));
        }
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
    for (int index = 0; index < root->childCount(); ++index)
    {
      auto* category = root->child(index);
      if (category->data(0,Kind).toInt() != static_cast<int>(ResultTreeItemKind::Category) || !displayable)
      {
        continue;
      }
      int shown = 0;
      for (int child = 0; child < category->childCount(); ++child)
      {
        shown += category->child(child)->checkState(0) == Qt::Checked ? 1 : 0;
      }
      category->setFlags((category->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsSelectable);
      category->setCheckState(0,shown == 0 ? Qt::Unchecked : shown == category->childCount() ? Qt::Checked : Qt::PartiallyChecked);
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
          item->setData(0,TableKey,QString::fromStdString(group.key));
          if (source.id == session.active_source_id && coloringField &&
              ((!coloringField->group_key.empty() && coloringField->group_key == group.key) ||
               (coloringField->group_key.empty() && coloringField->array_name == field.array_name &&
                coloringField->association == field.association && (!coloringField->dataset_id ||
                  std::find(group.datasets.begin(),group.datasets.end(),*coloringField->dataset_id) != group.datasets.end()))))
          {
            activeField = item;
          }
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
    const auto tableList = tables ? tables(source.id) : Postprocessor::result_tables(*data,std::nullopt);
    if (!tableList.empty())
    {
      auto* tableRoot = make(root,tr("Tables"),ResultTreeItemKind::MeshGroup,"tables");
      std::map<std::string,QTreeWidgetItem*> categories;
      for (const auto& table : tableList)
      {
        auto& category = categories[table.category];
        if (!category)
        {
          category = make(tableRoot,QString::fromUtf8(table.category.c_str()),ResultTreeItemKind::MeshGroup,"tables:"+QString::fromStdString(table.category));
        }
        auto* item = make(category,QString::fromUtf8(table.name.c_str()),ResultTreeItemKind::Table,"table:"+QString::fromStdString(table.key));
        item->setData(0,TableKey,QString::fromStdString(table.key));
        item->setToolTip(0,QString::fromStdString(table.key));
      }
    }
  }
  // Restore expansion after children exist; initial empty branches may not yet
  // be expandable when their QTreeWidgetItem is first constructed.
  for (QTreeWidgetItemIterator item(this); *item; ++item)
  {
    if ((*item)->childCount() == 0)
    {
      continue;
    }
    const auto key = (*item)->data(0,Key).toString();
    const auto kind = static_cast<ResultTreeItemKind>((*item)->data(0,Kind).toInt());
    const bool initiallyExpanded = !known.contains(key) && (kind == ResultTreeItemKind::Source ||
      kind == ResultTreeItemKind::FieldsGroup || kind == ResultTreeItemKind::ResultGroup);
    (*item)->setExpanded(expanded.contains(key) || initiallyExpanded);
  }
  if (activeField && (!selected || selected->data(0,Kind).toInt() == static_cast<int>(ResultTreeItemKind::Field)))
  {
    selected = activeField;
  }
  setCurrentItem(selected != nullptr ? selected : activeRoot);
}
