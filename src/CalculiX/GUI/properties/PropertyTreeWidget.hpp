#ifndef CALCULIX_PROPERTY_TREE_WIDGET_HPP
#define CALCULIX_PROPERTY_TREE_WIDGET_HPP

#include "PropertySelection.hpp"
#include "PropertyEditRequest.hpp"
#include "PropertyViewState.hpp"
#include "PropertyReferenceResolver.hpp"
#include <QTreeWidget>
#include <string>
#include <functional>
#include <vector>

class PropertyTreeWidget : public QTreeWidget
{
public:
  explicit PropertyTreeWidget(QWidget* parent = nullptr);
  void clear();
  void addContext(const PropertySelection& selection);
  PropertyViewState captureViewState() const;
  void restoreViewState(const PropertyViewState& state, bool restoreScroll);

protected:
  bool prepareNameEdit(const PropertyEditRequest& request, int currentEntityId, QString& quotedName) const;
  void submitNameCommand(const PropertyEditRequest& request, const QString& currentName, const QString& command, std::function<bool()> reload);
  virtual void applyEdit(const PropertyEditRequest& request);
  quint64 propertyEditRevision() const;
  void addEditableName(QTreeWidgetItem* group, int id, const QString& name);
  void addCommandChoice(QTreeWidgetItem* group, int id, const QString& field, const QString& label, const QString& value, const QStringList& choices);
  QTreeWidgetItem* addGroup(const QString& key, const QString& title, PropertySection section = PropertySection::Data);
  void addValue(QTreeWidgetItem* group, const QString& property, const QString& value);
  void addReference(QTreeWidgetItem* group, PropertyReferenceKind kind, const QString& label, const QString& id);
  void addNumber(QTreeWidgetItem* group, const QString& property, double value);
  void addTableRows(QTreeWidgetItem* group, const QStringList& columns, const std::vector<std::vector<double>>& values);
  QTreeWidgetItem* addIdentity(const QString& type, int id, const std::string& name = {});
  void addTable(const QString& key, const QString& title, const QStringList& columns, const std::vector<std::vector<double>>& values);
private:
  quint64 editRevision = 0;
};

#endif // CALCULIX_PROPERTY_TREE_WIDGET_HPP
