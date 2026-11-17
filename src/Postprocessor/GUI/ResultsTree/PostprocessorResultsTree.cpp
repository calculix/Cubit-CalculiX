#include <type_traits>
#include "PostprocessorResultsTree.hpp"
#include "PostprocessorStyleDelegate.hpp"
#include <QColorDialog>
#include <QItemSelectionModel>
#include "Postprocessor/GUI/Resources/PostprocessorIconLoader.hpp"
#include <QSignalBlocker>
#include <QMouseEvent>
#include <QHeaderView>
#include <QTreeWidgetItemIterator>
#include <QSet>
#include <QMenu>
#include <QInputDialog>
#include <QLineEdit>
#include <QTimer>
#include <QScopedValueRollback>
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include <map>
#include <algorithm>
#include <optional>
#include <tuple>

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
    Category,
    FilterName
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
  setIconSize(QSize(16,16));
  treeIcons["blocks"] = QIcon(":/cubitgui/pblockset.svg");
  treeIcons["nodesets"] = QIcon(":/cubitgui/nodeset.svg");
  treeIcons["sidesets"] = QIcon(":/cubitgui/sideset.svg");
  setHeaderLabels({tr("Sources / Results"),tr("ID"),tr("Style")});
  header()->setStretchLastSection(false);
  header()->setSectionResizeMode(0, QHeaderView::Stretch);
  header()->setSectionResizeMode(1, QHeaderView::Interactive);
  header()->resizeSection(1, 55);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setColumnHidden(1,true);
  header()->setSectionResizeMode(2,QHeaderView::Fixed);
  header()->resizeSection(2,46);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setItemDelegateForColumn(2,new PostprocessorStyleDelegate([this](const QModelIndex& index,Postprocessor::DisplayStyle style)
  {
    auto* item = itemFromIndex(index);
    auto targets = appearance_targets(item);
    if (item && item->isSelected())
    {
      targets = appearance_targets();
    }
    if (appearance_action && !targets.empty())
    {
      Postprocessor::AppearancePatch patch;
      patch.style = style;
      const auto view = activeView;
      const auto expected = snapshots;
      QTimer::singleShot(0,this,[this,targets,patch,view,expected]()
      {
        if (activeView != view || !appearance_action)
        {
          return;
        }
        for (const auto& target : targets)
        {
          const auto before = expected.find(target.source_id);
          const auto now = snapshots.find(target.source_id);
          if (before == expected.end() || now == snapshots.end() || before->second != now->second)
          {
            return;
          }
        }
        appearance_action(targets,patch);
      });
    }
  },this));
  connect(this,&QTreeWidget::itemClicked,this,[this](QTreeWidgetItem* item,int column)
  {
    if (column == 2 && item->data(2,PostprocessorTreeRoles::Style).isValid())
    {
      editItem(item,2);
    }
  });
  connect(this,&QTreeWidget::itemSelectionChanged,this,[this]()
  {
    if (!refreshing)
    {
      QTimer::singleShot(0,this,[this]()
      {
        if (appearance_selection)
        {
          appearance_selection(appearance_targets());
        }
      });
    }
  });
  header()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(header(),&QWidget::customContextMenuRequested,this,[this](const QPoint& point)
  {
    QMenu menu(this);
    auto* ids = menu.addAction(tr("Show IDs"));
    ids->setCheckable(true);
    ids->setChecked(!isColumnHidden(1));
    if (menu.exec(header()->mapToGlobal(point)) == ids && ids_action)
    {
      ids_action(ids->isChecked());
    }
  });
  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this,&QWidget::customContextMenuRequested,this,[this](const QPoint& position)
  {
    auto* item = itemAt(position);
    if (item && item->data(0,Kind).toInt() == static_cast<int>(ResultTreeItemKind::Filter))
    {
      const auto name = item->data(0,FilterName).toString();
      QMenu menu(this);
      auto* show = menu.addAction(tr("Show output"));
      auto* hide = menu.addAction(tr("Hide output"));
      auto* rename = menu.addAction(tr("Rename…"));
      auto* remove = menu.addAction(tr("Remove"));
      const auto* chosen = menu.exec(viewport()->mapToGlobal(position));
      if (!chosen || !filter_action)
      {
        return;
      }
      QString action,replacement;
      if (chosen == show)
      {
        action = "show";
      }
      else if (chosen == hide)
      {
        action = "hide";
      }
      else if (chosen == remove)
      {
        action = "remove";
      }
      else if (chosen == rename)
      {
        bool accepted = false;
        replacement = QInputDialog::getText(this,tr("Rename filter"),tr("Name"),QLineEdit::Normal,name,&accepted);
        if (!accepted || replacement.isEmpty())
        {
          return;
        }
        action = "rename";
      }
      filter_action(name,action,replacement);
      return;
    }
    if (!item || item->data(0,Kind).toInt() == static_cast<int>(ResultTreeItemKind::Source))
    {
      const auto sourceId = [&]() -> std::common_type_t<decltype(item->data(0,SourceId).toString().toStdString()), decltype(std::string{})> {
        if (item) {
          return item->data(0,SourceId).toString().toStdString();
        } else {
          return std::string{};
        }
      }();
      const auto sourceName = [&]() -> std::common_type_t<decltype(item->data(0,SourceName).toString()), decltype(QString{})> {
        if (item) {
          return item->data(0,SourceName).toString();
        } else {
          return QString{};
        }
      }();
      const auto viewId = activeView;
      const auto found = snapshots.find(sourceId);
      const auto snapshot = [&]() -> std::common_type_t<decltype(nullptr), decltype(found->second)> {
        if (found == snapshots.end()) {
          return nullptr;
        } else {
          return found->second;
        }
      }();
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
    const auto data = [&]() -> std::common_type_t<decltype(nullptr), decltype(snapshot->second)> {
      if (snapshot == snapshots.end()) {
        return nullptr;
      } else {
        return snapshot->second;
      }
    }();
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
    auto targets = appearance_targets(item);
    if (item->isSelected())
    {
      targets = appearance_targets();
    }
    std::map<QAction*,Postprocessor::AppearancePatch> appearanceActions;
    QAction* chooseColor = nullptr;
    if (!targets.empty() && appearance_action)
    {
      auto* styles = menu.addMenu(tr("Display style"));
      for (auto style : {Postprocessor::DisplayStyle::Wireframe,Postprocessor::DisplayStyle::HiddenLine,
          Postprocessor::DisplayStyle::Shaded,Postprocessor::DisplayStyle::Transparent,
          Postprocessor::DisplayStyle::Points,Postprocessor::DisplayStyle::RoundedPoints})
      {
        auto* action = styles->addAction(appearance_style_icon(style),appearance_style_label(style));
        Postprocessor::AppearancePatch patch;
        patch.style = style;
        appearanceActions[action] = patch;
        bool compatible = true;
        for (const auto& target : targets)
        {
          const auto loaded = snapshots.find(target.source_id);
          if (loaded != snapshots.end() && loaded->second)
          {
            for (const auto& metadata : loaded->second->datasets)
            {
              if (metadata.id == target.dataset_id)
              {
                compatible = compatible && Postprocessor::supports_style(metadata,style);
              }
            }
          }
        }
        action->setEnabled(compatible);
      }
      chooseColor = menu.addAction(tr("Solid color…"));
      Postprocessor::AppearancePatch nativeColor;
      nativeColor.color_change = Postprocessor::ColorChange::Native;
      appearanceActions[menu.addAction(tr("Use native color"))] = nativeColor;
      auto* edges = menu.addMenu(tr("Element edges"));
      Postprocessor::AppearancePatch on,off,reset;
      on.edges = true;
      off.edges = false;
      reset.reset = true;
      appearanceActions[edges->addAction(tr("On"))] = on;
      appearanceActions[edges->addAction(tr("Off"))] = off;
      appearanceActions[menu.addAction(tr("Reset appearance"))] = reset;
    }
    const auto expectedSnapshots = snapshots;
    const auto targetsCurrent = [&]()
    {
      if (activeView != view)
      {
        return false;
      }
      for (const auto& target : targets)
      {
        const auto before = expectedSnapshots.find(target.source_id);
        const auto now = snapshots.find(target.source_id);
        if (before == expectedSnapshots.end() || now == snapshots.end() || before->second != now->second)
        {
          return false;
        }
      }
      return true;
    };
    const auto* chosen = menu.exec(viewport()->mapToGlobal(position));
    // A modal menu can outlive a tree rebuild. Never reuse item pointers or act
    // on a different snapshot/view after returning from its event loop.
    const auto current = snapshots.find(id);
    if (!chosen || !data || current == snapshots.end() || current->second != data || activeView != view)
    {
      return;
    }
    const auto appearance = appearanceActions.find(const_cast<QAction*>(chosen));
    if (appearance != appearanceActions.end() && targetsCurrent())
    {
      appearance_action(targets,appearance->second);
      return;
    }
    if (chosen == chooseColor && appearance_action)
    {
      const auto color = QColorDialog::getColor(Qt::white,this,tr("Solid color"));
      const auto currentData = snapshots.find(id);
      if (color.isValid() && currentData != snapshots.end() && currentData->second == data && targetsCurrent())
      {
        Postprocessor::AppearancePatch patch;
        patch.color_change = Postprocessor::ColorChange::Override;
        patch.color = {color.redF(),color.greenF(),color.blueF()};
        appearance_action(targets,patch);
      }
      return;
    }
    using Action = Postprocessor::DatasetVisibilityAction;
    if (chosen == all)
    {
      dataset_action(source,Action::ShowAll,std::nullopt);
    }
    else if (chosen == show || chosen == hide || chosen == only)
    {
      dataset_action(source,[&]() -> std::common_type_t<decltype(Action::Show), std::common_type_t<decltype(Action::Hide), decltype(Action::ShowOnly)>> {
        if (chosen == show) {
          return Action::Show;
        } else {
          return [&]() -> std::common_type_t<decltype(Action::Hide), decltype(Action::ShowOnly)> {
            if (chosen == hide) {
              return Action::Hide;
            } else {
              return Action::ShowOnly;
            }
          }();
        }
      }(),dataset);
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
    if (refreshing)
    {
      return;
    }
    if (current == nullptr)
    {
      QTimer::singleShot(0,this,[this]()
      {
        if (!currentItem() && selection_cleared)
        {
          selection_cleared();
        }
      });
      return;
    }
    const auto kind = static_cast<ResultTreeItemKind>(current->data(0, Kind).toInt());
    const auto name = current->data(0, SourceName).toString();
    if (kind == ResultTreeItemKind::Filter)
    {
      const auto filter = current->data(0,FilterName).toString();
      QTimer::singleShot(0,this,[this,filter]()
      {
        if (filter_action)
        {
          filter_action(filter,"select",{});
        }
      });
      return;
    }
    if (inspect_source && !name.isEmpty())
    {
      // Dispatch after this signal returns: commands can rebuild the tree.
      // Never retain an item pointer across a command boundary.
      QTimer::singleShot(0,this,[this,name]()
      {
        const auto* selected = currentItem();
        if (selected && selected->data(0,SourceName).toString() == name && inspect_source)
        {
          inspect_source(name);
        }
      });
    }

    if (kind == ResultTreeItemKind::Source || kind == ResultTreeItemKind::Dataset || kind == ResultTreeItemKind::Field)
    {
      const auto id = current->data(0,SourceId).toString().toStdString();
      if (id == activeSource)
      {
        return;
      }
      const auto found = snapshots.find(id);
      const auto snapshot = [&]() -> std::common_type_t<decltype(nullptr), decltype(found->second)> {
        if (found == snapshots.end()) {
          return nullptr;
        } else {
          return found->second;
        }
      }();
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
    if (kind == ResultTreeItemKind::Filter)
    {
      const auto filter = item->data(0,FilterName).toString();
      QString action = "hide";
      if (item->checkState(0) == Qt::Checked)
      {
        action = "show";
      }
      QTimer::singleShot(0,this,[this,filter,action]()
      {
        if (filter_action)
        {
          filter_action(filter,action,{});
        }
      });
      return;
    }
    const auto name = item->data(0,SourceName).toString();
    const auto id = item->data(0,SourceId).toString().toStdString();
    const auto dataset = static_cast<std::size_t>(item->data(0,Index).toULongLong());
    const bool shown = item->checkState(0) != Qt::Unchecked;
    const auto category = static_cast<Postprocessor::DatasetCategory>(item->data(0,Category).toInt());
    const auto found = snapshots.find(id);
    const auto snapshot = [&]() -> std::common_type_t<decltype(nullptr), decltype(found->second)> {
      if (found == snapshots.end()) {
        return nullptr;
      } else {
        return found->second;
      }
    }();
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
        dataset_action(name,[&]() -> std::common_type_t<decltype(Postprocessor::DatasetVisibilityAction::Show), decltype(Postprocessor::DatasetVisibilityAction::Hide)> {
          if (shown) {
            return Postprocessor::DatasetVisibilityAction::Show;
          } else {
            return Postprocessor::DatasetVisibilityAction::Hide;
          }
        }(),dataset);
      }
    });
  });
}

void PostprocessorResultsTree::set_icons_directory(const std::string& directory)
{
  for (const auto& item : std::map<std::string,std::string>{
      {"source","PostprocessorTreeSource"},{"integrationpoints","PostprocessorTreeIntegrationPoints"},
      {"results","PostprocessorTreeResults"},{"field","PostprocessorTreeField"},
      {"tables","PostprocessorTreeTables"},{"table","PostprocessorTreeTable"},
      {"clip","PostprocessorClip"},{"section","PostprocessorSection"},{"threshold","PostprocessorThreshold"}})
  {
    treeIcons[item.first] = PostprocessorIconLoader::load(directory,item.second);
  }
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
  QSet<QString> selectedKeys;
  for (const auto* item : selectedItems())
  {
    selectedKeys.insert(item->data(0,Key).toString());
  }
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
      auto* item = [&]() -> std::common_type_t<decltype(new QTreeWidgetItem(parent, {text})), decltype(new QTreeWidgetItem(this, {text}))> {
        if (parent) {
          return new QTreeWidgetItem(parent, {text});
        } else {
          return new QTreeWidgetItem(this, {text});
        }
      }();
      const auto key = sourceId + ":" + suffix;
      item->setData(0, Kind, static_cast<int>(kind));
      item->setData(0, SourceId, sourceId);
      item->setData(0, SourceName, sourceName);
      item->setData(0, Index, QVariant::fromValue<qulonglong>(index));
      item->setData(0, Key, key);
      std::string icon = "field";
      switch (kind)
      {
        case ResultTreeItemKind::Source:
          icon = "source";
          break;
        case ResultTreeItemKind::FieldsGroup:
        case ResultTreeItemKind::ResultGroup:
          icon = "results";
          break;
        case ResultTreeItemKind::FiltersGroup:
          icon = "clip";
          break;
        case ResultTreeItemKind::Table:
          icon = "table";
          break;
        case ResultTreeItemKind::MeshGroup:
          if (suffix == "tables")
          {
            icon = "tables";
          }
          else if (suffix == "integrationpoints")
          {
            icon = "integrationpoints";
          }
          break;
        default:
          break;
      }
      item->setIcon(0,treeIcons[icon]);
      if (kind == ResultTreeItemKind::MeshGroup || kind == ResultTreeItemKind::FieldsGroup || kind == ResultTreeItemKind::ResultGroup || kind == ResultTreeItemKind::FiltersGroup)
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
    const auto data = [&]() -> std::common_type_t<decltype(loadedData), decltype(std::make_shared<SimResults::VtkResultsData>())> {
      if (loadedData) {
        return loadedData;
      } else {
        return std::make_shared<SimResults::VtkResultsData>();
      }
    }();
    if (!loadedData)
    {
      root->setText(0,sourceName+tr(" [Unavailable]"));
      const auto reason = [&]() -> std::common_type_t<decltype(availability(source.id)), decltype(std::string("Result source is unavailable."))> {
        if (availability) {
          return availability(source.id);
        } else {
          return std::string("Result source is unavailable.");
        }
      }();
      root->setToolTip(0,QString::fromUtf8(reason.c_str()));
    }
    std::optional<std::size_t> current;
    bool visible = false;
    std::set<std::size_t> hidden;
    std::optional<Postprocessor::FieldSelection> coloringField;
    // Same resolution priority as Core visibility commands: active visible,
    // another visible output, active hidden, then the original source.
    const auto belongs = [&](const Postprocessor::Representation& display)
    {
      if (display.view_id != session.active_view_id || display.input.port != 0)
      {
        return false;
      }
      if (display.input.owner == Postprocessor::OutputOwner::Source)
      {
        return display.input.object_id == source.id;
      }
      const auto filter = session.filters.find(display.input.object_id);
      return filter != session.filters.end() && filter->second.input.object_id == source.id;
    };
    const Postprocessor::Representation* sourceDisplay = nullptr;
    const Postprocessor::Representation* visibleDisplay = nullptr;
    bool sourceVisible = false;
    for (const auto& item : session.representations)
    {
      const auto& value = item.second;
      if (!belongs(value))
      {
        continue;
      }
      if (value.input.owner == Postprocessor::OutputOwner::Source)
      {
        sourceDisplay = &value;
      }
      if (value.visible)
      {
        sourceVisible = true;
        if (!visibleDisplay)
        {
          visibleDisplay = &value;
        }
      }
    }
    const auto activeDisplay = session.representations.find(session.active_representation_id);
    if (activeDisplay != session.representations.end() && belongs(activeDisplay->second))
    {
      sourceDisplay = &activeDisplay->second;
      if (sourceDisplay->visible)
      {
        visibleDisplay = sourceDisplay;
      }
    }
    if (visibleDisplay)
    {
      sourceDisplay = visibleDisplay;
    }
    if (sourceDisplay)
    {
      current = sourceDisplay->increment_index;
      visible = sourceDisplay->visible;
      hidden = sourceDisplay->hidden_datasets;
      coloringField = sourceDisplay->coloring.field;
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
      root->setCheckState(0, [&]() -> std::common_type_t<decltype(Qt::Checked), decltype(Qt::Unchecked)> {
        if (sourceVisible) {
          return Qt::Checked;
        } else {
          return Qt::Unchecked;
        }
      }());
    }
    else if (loadedData)
    {
      root->setToolTip(0, tr("This source has no displayable mesh increment."));
    }
    const auto datasetList = [&]() -> std::common_type_t<decltype(datasets(source.id)), std::common_type_t<decltype(data->datasets), decltype(Postprocessor::result_dataset_catalogue(*data))>> {
      if (datasets) {
        return datasets(source.id);
      } else {
        return ([&]() -> std::common_type_t<decltype(data->datasets), decltype(Postprocessor::result_dataset_catalogue(*data))> {
          if (!data->datasets.empty()) {
            return data->datasets;
          } else {
            return Postprocessor::result_dataset_catalogue(*data);
          }
        }());
      }
    }();
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
          const std::string category = [&]() -> std::string {
            if (dataset.kind == "Blocks" || dataset.kind == "DAT_Elements") {
              return "blocks";
            } else {
              if (dataset.kind == "Nodesets" || dataset.kind == "DAT_Nodes") {
                return "nodesets";
              } else {
                if (dataset.kind == "Surfaces") {
                  return "sidesets";
                } else {
                  return "other";
                }
              }
            }
          }();
          auto& group = categories[category];
          if (!group)
          {
            const QString title = [&]() -> QString {
              if (category == "blocks") {
                return tr("Blocks / Element sets");
              } else {
                if (category == "nodesets") {
                  return tr("Node sets");
                } else {
                  if (category == "sidesets") {
                    return tr("Side sets / Surfaces");
                  } else {
                    return tr("Other datasets");
                  }
                }
              }
            }();
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
        QString identity = QString::fromStdString(dataset.kind)+":name:"+QString::fromStdString(dataset.name);
        if (dataset.model_id >= 0)
        {
          identity = QString::fromStdString(dataset.kind)+":model:"+QString::number(dataset.model_id)+":name:"+QString::fromStdString(dataset.name);
        }
        auto* item = make(parent,label,ResultTreeItemKind::Dataset,"dataset:"+identity,dataset.id);
        Postprocessor::AppearanceTarget target{source.id,activeView,dataset.id};
        auto appearance = Postprocessor::default_appearance(dataset);
        if (appearance_value)
        {
          appearance = appearance_value(target);
        }
        item->setIcon(2,appearance_style_icon(appearance.style));
        const bool editableAppearance = Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::Blocks) ||
          Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::NodeSets) ||
          Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::SideSets) ||
          Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::IntegrationPoints);
        if (editableAppearance)
        {
          item->setData(2,PostprocessorTreeRoles::Style,static_cast<int>(appearance.style));
          item->setData(2,PostprocessorTreeRoles::DatasetKind,QString::fromStdString(dataset.kind));
          item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
        item->setToolTip(2,appearance_style_label(appearance.style));
        std::string datasetIcon = "field";
        if (Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::Blocks))
        {
          datasetIcon = "blocks";
        }
        else if (Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::NodeSets))
        {
          datasetIcon = "nodesets";
        }
        else if (Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::SideSets))
        {
          datasetIcon = "sidesets";
        }
        else if (Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::IntegrationPoints))
        {
          datasetIcon = "integrationpoints";
        }
        item->setIcon(0,treeIcons[datasetIcon]);
        if (parent)
        {
          parent->setIcon(0,treeIcons[datasetIcon]);
        }
        if (dataset.model_id >= 0)
        {
          item->setText(1,QString::number(dataset.model_id));
        }
        if (displayable)
        {
          item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
          item->setCheckState(0,[&]() -> std::common_type_t<decltype(Qt::Checked), decltype(Qt::Unchecked)> {
            if (hidden.count(dataset.id) == 0) {
              return Qt::Checked;
            } else {
              return Qt::Unchecked;
            }
          }());
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
        if (dataset.model_id >= 0)
        {
          tooltip += tr(" — model ID %1").arg(dataset.model_id);
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
        if (category->child(child)->checkState(0) == Qt::Checked) {
          shown += 1;
        } else {
          shown += 0;
        }
      }
      category->setFlags((category->flags() | Qt::ItemIsUserCheckable) & ~Qt::ItemIsSelectable);
      category->setCheckState(0,[&]() -> std::common_type_t<decltype(Qt::Unchecked), std::common_type_t<decltype(Qt::Checked), decltype(Qt::PartiallyChecked)>> {
        if (shown == 0) {
          return Qt::Unchecked;
        } else {
          return [&]() -> std::common_type_t<decltype(Qt::Checked), decltype(Qt::PartiallyChecked)> {
            if (shown == category->childCount()) {
              return Qt::Checked;
            } else {
              return Qt::PartiallyChecked;
            }
          }();
        }
      }());
    }
    const auto groups = [&]() -> std::common_type_t<decltype(catalogue(source.id,current)), decltype(Postprocessor::result_field_groups(*data,current))> {
      if (catalogue) {
        return catalogue(source.id,current);
      } else {
        return Postprocessor::result_field_groups(*data,current);
      }
    }();
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
          item->setToolTip(0,components.join(", ") + ([&]() -> std::common_type_t<decltype(QString()), decltype(tr(" (unavailable in this frame)"))> {
            if (group.available) {
              return QString();
            } else {
              return tr(" (unavailable in this frame)");
            }
          }()));
          if (!group.available || field.association == SimResults::Association::Table)
          {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
          }
        }
      }
    }
    const auto tableList = [&]() -> std::common_type_t<decltype(tables(source.id)), decltype(Postprocessor::result_tables(*data,std::nullopt))> {
      if (tables) {
        return tables(source.id);
      } else {
        return Postprocessor::result_tables(*data,std::nullopt);
      }
    }();
    if (!tableList.empty())
    {
      auto* tableRoot = make(root,tr("Tables"),ResultTreeItemKind::MeshGroup,"tables");
      for (const auto& table : tableList)
      {
        auto* item = make(tableRoot,QString::fromUtf8(table.name.c_str()),ResultTreeItemKind::Table,"table:"+QString::fromStdString(table.key));
        item->setData(0,TableKey,QString::fromStdString(table.key));
        item->setToolTip(0,QString::fromStdString(table.category)+"\n"+QString::fromStdString(table.key));
      }
    }
    QTreeWidgetItem* filterRoot = nullptr;
    for (const auto& entry : session.filters)
    {
      const auto& filter = entry.second;
      if (filter.input.object_id != source.id)
      {
        continue;
      }
      if (!filterRoot)
      {
        filterRoot = make(root,tr("Filters"),ResultTreeItemKind::FiltersGroup,"filters");
      }
      auto* item = make(filterRoot,QString::fromUtf8(filter.name.c_str()),ResultTreeItemKind::Filter,
        "filter:"+QString::fromStdString(filter.id));
      item->setData(0,FilterName,QString::fromUtf8(filter.name.c_str()));
      item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
      item->setCheckState(0,Qt::Unchecked);
      const bool outputVisible = std::any_of(session.representations.begin(),session.representations.end(),[&](const auto& display)
      {
        return display.second.view_id == session.active_view_id && display.second.visible &&
          display.second.input.owner == Postprocessor::OutputOwner::Feature && display.second.input.object_id == filter.id;
      });
      if (outputVisible)
      {
        item->setCheckState(0,Qt::Checked);
      }
      std::string icon = "clip";
      if (filter.kind == Postprocessor::FilterKind::Section)
      {
        icon = "section";
      }
      if (filter.kind == Postprocessor::FilterKind::Threshold)
      {
        icon = "threshold";
      }
      item->setIcon(0,treeIcons[icon]);
      item->setToolTip(0,tr("Checked: show this output. Unchecked: hide it. Enable or bypass the filter in its properties."));
      const auto active = session.representations.find(session.active_representation_id);
      if (active != session.representations.end() && active->second.input.owner == Postprocessor::OutputOwner::Feature &&
          active->second.input.object_id == filter.id && source.id == session.active_source_id)
      {
        selected = item;
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
      kind == ResultTreeItemKind::FieldsGroup || kind == ResultTreeItemKind::ResultGroup || kind == ResultTreeItemKind::FiltersGroup);
    (*item)->setExpanded(expanded.contains(key) || initiallyExpanded);
  }
  if (activeField && (!selected || selected->data(0,Kind).toInt() == static_cast<int>(ResultTreeItemKind::Field)))
  {
    selected = activeField;
  }
  setCurrentItem([&]() -> std::common_type_t<decltype(selected), decltype(activeRoot)> {
    if (selected != nullptr) {
      return selected;
    } else {
      return activeRoot;
    }
  }(),0,QItemSelectionModel::NoUpdate);
  bool restored = false;
  for (QTreeWidgetItemIterator item(this); *item; ++item)
  {
    if (selectedKeys.contains((*item)->data(0,Key).toString()))
    {
      (*item)->setSelected(true);
      restored = true;
    }
  }
  if (!restored && currentItem())
  {
    currentItem()->setSelected(true);
  }
}

void PostprocessorResultsTree::mousePressEvent(QMouseEvent* event)
{
  const bool empty = itemAt(event->position().toPoint()) == nullptr;
  QTreeWidget::mousePressEvent(event);
  if (empty && event->button() == Qt::LeftButton && !refreshing)
  {
    clearSelection();
    setCurrentItem(nullptr);
  }
}

std::vector<Postprocessor::AppearanceTarget> PostprocessorResultsTree::appearance_targets(QTreeWidgetItem* only) const
{
  auto items = selectedItems();
  if (only)
  {
    items = {only};
  }
  std::vector<Postprocessor::AppearanceTarget> targets;
  for (const auto* item : items)
  {
    if (item->data(0,Kind).toInt() == static_cast<int>(ResultTreeItemKind::Dataset) &&
        item->data(2,PostprocessorTreeRoles::Style).isValid())
    {
      targets.push_back({item->data(0,SourceId).toString().toStdString(),activeView,
        static_cast<std::size_t>(item->data(0,Index).toULongLong())});
    }
  }
  std::sort(targets.begin(),targets.end(),[](const auto& a,const auto& b)
  {
    return std::tie(a.source_id,a.dataset_id) < std::tie(b.source_id,b.dataset_id);
  });
  return targets;
}
