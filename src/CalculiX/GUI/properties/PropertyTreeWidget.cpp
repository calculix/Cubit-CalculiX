#include "PropertyTreeWidget.hpp"
#include "PropertyEditDelegate.hpp"
#include "PropertyCommandText.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "CubitMessage.hpp"
#include <QPointer>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QFont>
#include <QKeySequence>
#include <QShortcut>
#include <QScrollBar>

namespace
{
constexpr int SectionKeyRole = Qt::UserRole + 1200;
constexpr int SectionOrderRole = SectionKeyRole + 1;

QString statePath(const QTreeWidgetItem* item)
{
  QString path;
  for (const auto* current = item; current != nullptr; current = current->parent())
  {
    const QString key = current->data(0, SectionKeyRole).toString();
    if (key.isEmpty())
    {
      return {};
    }
    // Length-prefix segments avoid collisions when a key contains punctuation.
    path.prepend(QString::number(key.size()) + ":" + key);
  }
  return path;
}

void captureExpansion(const QTreeWidgetItem* item, PropertyViewState& state)
{
  const QString key = statePath(item);
  if (item->childCount() > 0 && !key.isEmpty())
  {
    state.expanded.insert(key, item->isExpanded());
  }
  for (int index = 0; index < item->childCount(); ++index)
  {
    captureExpansion(item->child(index), state);
  }
}

void restoreExpansion(QTreeWidgetItem* item, const PropertyViewState& state)
{
  const QString key = statePath(item);
  const auto expanded = state.expanded.constFind(key);
  if (item->childCount() > 0 && !key.isEmpty() && expanded != state.expanded.cend())
  {
    item->setExpanded(expanded.value());
  }
  for (int index = 0; index < item->childCount(); ++index)
  {
    restoreExpansion(item->child(index), state);
  }
}
}

PropertyTreeWidget::PropertyTreeWidget(QWidget* parent)
  : QTreeWidget(parent)
{
  setColumnCount(2);
  setHeaderLabels({tr("Property"), tr("Value")});
  setAlternatingRowColors(true);
  setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  setItemDelegate(new PropertyEditDelegate(this, [this](const PropertyEditRequest& request) { applyEdit(request); }));
  setSelectionMode(QAbstractItemView::SingleSelection);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setUniformRowHeights(true);
  header()->setSectionResizeMode(0, QHeaderView::Interactive);
  header()->setSectionResizeMode(1, QHeaderView::Stretch);
  setColumnWidth(0, 165);
  auto* copy = new QShortcut(QKeySequence::Copy, this);
  copy->setContext(Qt::WidgetWithChildrenShortcut);
  connect(copy, &QShortcut::activated, this, [this]()
  {
    const auto items = selectedItems();
    if (items.size() == 1)
    {
      QApplication::clipboard()->setText(items.front()->text(0) + "\t" + items.front()->text(1));
    }
  });
}

PropertyViewState PropertyTreeWidget::captureViewState() const
{
  PropertyViewState state;
  state.header = header()->saveState();
  state.verticalScroll = verticalScrollBar()->value();
  state.horizontalScroll = horizontalScrollBar()->value();
  for (int index = 0; index < topLevelItemCount(); ++index)
  {
    captureExpansion(topLevelItem(index), state);
  }
  return state;
}

void PropertyTreeWidget::restoreViewState(const PropertyViewState& state, bool restoreScroll)
{
  if (!state.header.isEmpty())
  {
    header()->restoreState(state.header);
  }
  for (int index = 0; index < topLevelItemCount(); ++index)
  {
    restoreExpansion(topLevelItem(index), state);
  }
  doItemsLayout();
  verticalScrollBar()->setValue(restoreScroll ? state.verticalScroll : 0);
  horizontalScrollBar()->setValue(restoreScroll ? state.horizontalScroll : 0);
}

QTreeWidgetItem* PropertyTreeWidget::addGroup(const QString& key, const QString& title, PropertySection section)
{
  auto* group = new QTreeWidgetItem(QStringList{title});
  group->setData(0, SectionKeyRole, key);
  const int order = static_cast<int>(section);
  group->setData(0, SectionOrderRole, order);
  int position = 0;
  while (position < topLevelItemCount() && topLevelItem(position)->data(0, SectionOrderRole).toInt() <= order)
  {
    ++position;
  }
  insertTopLevelItem(position, group);
  QFont font = group->font(0);
  font.setBold(true);
  group->setFont(0, font);
  group->setExpanded(true);
  return group;
}

void PropertyTreeWidget::clear()
{
  ++editRevision;
  QTreeWidget::clear();
}

quint64 PropertyTreeWidget::propertyEditRevision() const
{
  return editRevision;
}

bool PropertyTreeWidget::prepareNameEdit(const PropertyEditRequest& request, int currentEntityId, QString& quotedName) const
{
  if (request.field != "name" || !isVisible() || request.entityId != currentEntityId || request.revision != propertyEditRevision())
  {
    return false;
  }
  const QString name = request.value.trimmed();
  if (name == request.original)
  {
    return false;
  }
  QString error;
  if (!PropertyCommandText::quoteName(name, quotedName, error))
  {
    PRINT_ERROR("Cannot rename entity %d: %s\n", request.entityId, error.toStdString().c_str());
    return false;
  }
  return true;
}

void PropertyTreeWidget::submitNameCommand(const PropertyEditRequest& request, const QString& currentName, const QString& command, std::function<bool()> reload)
{
  const QPointer<PropertyTreeWidget> self(this);
  if (currentName != request.original)
  {
    PRINT_WARNING("The entity name changed while its property editor was open. The property view has been refreshed.\n");
  }
  else if (!GuiSendCmd::gui_cmd(command))
  {
    PRINT_ERROR("Could not submit the rename command for entity %d.\n", request.entityId);
  }
  // Commands refresh the active property page through the existing observer.
  // A rejected command or a suppressed update still needs an authoritative reload.
  if (self != nullptr && self->isVisible() && self->propertyEditRevision() == request.revision && reload)
  {
    const auto state = self->captureViewState();
    if (reload() && self != nullptr)
    {
      self->restoreViewState(state, true);
    }
  }
}

void PropertyTreeWidget::applyEdit(const PropertyEditRequest&)
{
}

void PropertyTreeWidget::addEditableName(QTreeWidgetItem* group, int id, const QString& name)
{
  addValue(group, tr("Name"), name);
  auto* row = group->child(group->childCount() - 1);
  row->setFlags(row->flags() | Qt::ItemIsEditable);
  row->setData(1, PropertyEditorRole, static_cast<int>(PropertyEditorKind::Name));
  row->setData(1, PropertyEntityRole, id);
  row->setData(1, PropertyFieldRole, "name");
  row->setData(1, PropertyOriginalRole, name);
  row->setData(1, PropertyRevisionRole, static_cast<qulonglong>(editRevision));
  row->setToolTip(1, tr("Double-click or press F2 to rename using a command"));
}

void PropertyTreeWidget::addCommandChoice(QTreeWidgetItem* group, int id, const QString& field, const QString& label, const QString& value, const QStringList& choices)
{
  addValue(group, label, value);
  auto* row = group->child(group->childCount() - 1);
  if (choices.isEmpty())
  {
    row->setToolTip(1, tr("No supported choices are available for this block's current element type"));
    return;
  }
  row->setFlags(row->flags() | Qt::ItemIsEditable);
  row->setData(1, PropertyEditorRole, static_cast<int>(PropertyEditorKind::Choice));
  row->setData(1, PropertyEntityRole, id);
  row->setData(1, PropertyFieldRole, field);
  row->setData(1, PropertyOriginalRole, value);
  row->setData(1, PropertyChoicesRole, choices);
  row->setData(1, PropertyRevisionRole, static_cast<qulonglong>(editRevision));
  row->setToolTip(1, tr("Click the arrow, double-click the value, or press F2 to choose an element type"));
}

void PropertyTreeWidget::addValue(QTreeWidgetItem* group, const QString& property, const QString& value)
{
  const QString display = value.isEmpty() ? tr("Not set") : value;
  auto* row = new QTreeWidgetItem(group, {property, display});
  row->setToolTip(0, property);
  row->setToolTip(1, display);
}

void PropertyTreeWidget::addReference(QTreeWidgetItem* group, PropertyReferenceKind kind, const QString& label, const QString& id)
{
  addValue(group, label + tr(" ID"), id);
  addValue(group, label + tr(" name"), PropertyReferenceResolver::name(kind, id));
}

void PropertyTreeWidget::addNumber(QTreeWidgetItem* group, const QString& property, double value)
{
  addValue(group, property, QString::number(value, 'g', 12));
  group->child(group->childCount() - 1)->setToolTip(1, QString::number(value, 'g', 17));
}

QTreeWidgetItem* PropertyTreeWidget::addIdentity(const QString& type, int id, const std::string& name)
{
  auto* group = addGroup("general", tr("General"), PropertySection::General);
  addValue(group, tr("Type"), type);
  if (id >= 0)
  {
    addValue(group, tr("ID"), QString::number(id));
  }
  if (!name.empty())
  {
    addValue(group, tr("Name"), QString::fromStdString(name));
  }
  return group;
}

void PropertyTreeWidget::addTable(const QString& key, const QString& title, const QStringList& columns, const std::vector<std::vector<double>>& values)
{
  addTableRows(addGroup(key, title), columns, values);
}

void PropertyTreeWidget::addTableRows(QTreeWidgetItem* group, const QStringList& columns, const std::vector<std::vector<double>>& values)
{
  addValue(group, tr("Rows"), QString::number(static_cast<qulonglong>(values.size())));
  for (std::size_t index = 0; index < values.size(); ++index)
  {
    auto* row = new QTreeWidgetItem(group, {tr("Row %1").arg(static_cast<qulonglong>(index + 1))});
    row->setData(0, SectionKeyRole, QString("row_%1").arg(static_cast<qulonglong>(index)));
    for (std::size_t column = 0; column < values[index].size(); ++column)
    {
      const QString label = static_cast<int>(column) < columns.size() ? columns[static_cast<int>(column)] : tr("Value %1").arg(static_cast<qulonglong>(column + 1));
      addNumber(row, label, values[index][column]);
    }
  }
}

void PropertyTreeWidget::addContext(const PropertySelection& selection)
{
  if (selection.stepId < 0 && selection.topologyId < 0 && !selection.homogeneous)
  {
    return;
  }
  auto* group = addGroup("selection_context", tr("Selection context"), PropertySection::Context);
  if (selection.stepId >= 0)
  {
    addReference(group, PropertyReferenceKind::Step, tr("Step"), QString::number(selection.stepId));
  }
  if (selection.topologyId >= 0)
  {
    addReference(group, PropertyReferenceKind::Topology, tr("Topology"), QString::number(selection.topologyId));
  }
  if (selection.iterationId >= 0)
  {
    addValue(group, tr("Iteration ID"), QString::number(selection.iterationId));
  }
  if (selection.homogeneous)
  {
    addValue(group, tr("Assignment"), tr("Homogeneous boundary conditions"));
  }
}
