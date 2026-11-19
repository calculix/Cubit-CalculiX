#include "ReportMeshQualitySettings.hpp"
#include "Framework/MeshQuality/MeshQualityDefaults.hpp"
#include "Framework/MeshQuality/MeshQualitySettings.hpp"
#include <QComboBox>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QLocale>
#include <utility>

ReportMeshQualitySettings::ReportMeshQualitySettings(const ReportConfiguration& configuration, QWidget* parent)
  : QWidget(parent)
{
  auto* layout = new QVBoxLayout(this);
  familySelector = new QComboBox(this);
  familySelector->setObjectName("ReportQualityFamily");
  pages = new QStackedWidget(this);
  layout->addWidget(familySelector);
  layout->addWidget(pages);
  const auto definitions = MeshQualityDefaults::metrics();
  for (const auto& family : std::vector<std::pair<std::string, QString>>{
    {"hex", tr("Hexahedra")}, {"tet", tr("Tetrahedra")},
    {"quad", tr("Quadrilaterals")}, {"tri", tr("Triangles")}})
  {
    familySelector->addItem(family.second, QString::fromStdString(family.first));
    auto* page = new QWidget(pages);
    auto* pageLayout = new QVBoxLayout(page);
    auto* table = new QTableWidget(0, 4, page);
    table->setObjectName(QString::fromStdString("ReportQuality_" + family.first));
    table->setHorizontalHeaderLabels({tr("Include / metric"), tr("Minimum"), tr("Maximum"), tr("Units")});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    familyTables.emplace(family.first, table);
    for (const auto& metric : definitions)
    {
      if (metric.family != family.first)
      {
        continue;
      }
      const auto text = [&](const std::string& suffix, const std::string& initial)
      {
        const auto found = configuration.values.find(metric.key() + suffix);
        if (found != configuration.values.end())
        {
          return QString::fromStdString(found->second);
        }
        return QString::fromStdString(initial);
      };
      const int row = table->rowCount();
      table->insertRow(row);
      auto* item = new QTableWidgetItem(QString::fromStdString(metric.label));
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
      item->setData(Qt::UserRole, QString::fromStdString(metric.key()));
      std::string selected = "false";
      if (metric.enabled_by_default)
      {
        selected = "true";
      }
      item->setCheckState(Qt::Unchecked);
      if (text(".enabled", selected) == "true")
      {
        item->setCheckState(Qt::Checked);
      }
      if (metric.requires_group_reference)
      {
        item->setToolTip(tr("Uses one reference context for the complete block/family group."));
      }
      table->setItem(row, 0, item);
      for (int column : {1, 2})
      {
        std::string suffix = ".minimum";
        auto initial = metric.minimum;
        if (column == 2)
        {
          suffix = ".maximum";
          initial = metric.maximum;
        }
        std::string number;
        if (initial)
        {
          number = QString::number(*initial, 'g', 15).toStdString();
        }
        auto* edit = new QLineEdit(text(suffix, number), table);
        auto* validator = new QDoubleValidator(edit);
        validator->setLocale(QLocale::c());
        validator->setNotation(QDoubleValidator::ScientificNotation);
        edit->setValidator(validator);
        edit->setPlaceholderText(tr("Unbounded"));
        edit->setObjectName(QString::fromStdString(metric.key() + suffix));
        table->setCellWidget(row, column, edit);
      }
      auto* unit = new QLineEdit(text(".unit", metric.unit), table);
      unit->setObjectName(QString::fromStdString(metric.key() + ".unit"));
      unit->setMaxLength(128);
      table->setCellWidget(row, 3, unit);
    }
    pageLayout->addWidget(table);
    pages->addWidget(page);
  }
  auto* hint = new QLabel(tr("Limits are reporting defaults independent of Cubit preferences. Empty limits mean unbounded. Limit units are explicit; report values use Report display units. Apply saves changes; profiles retain them."), this);
  hint->setWordWrap(true);
  layout->addWidget(hint);
  auto* restore = new QPushButton(tr("Restore family defaults"), this);
  restore->setObjectName("ReportQualityRestore");
  layout->addWidget(restore);
  connect(familySelector, qOverload<int>(&QComboBox::currentIndexChanged), pages, &QStackedWidget::setCurrentIndex);
  connect(restore, &QPushButton::clicked, this, [this]()
  {
    restore_family_defaults();
  });
}
void ReportMeshQualitySettings::restore_family_defaults()
{
  const auto family = familySelector->currentData().toString().toStdString();
  const auto found = familyTables.find(family);
  if (found == familyTables.end())
  {
    return;
  }
  auto* table = found->second;
  for (const auto& metric : MeshQualityDefaults::metrics())
  {
    if (metric.family != family)
    {
      continue;
    }
    for (int row = 0; row < table->rowCount(); ++row)
    {
      if (table->item(row, 0)->data(Qt::UserRole).toString().toStdString() != metric.key())
      {
        continue;
      }
      table->item(row, 0)->setCheckState(Qt::Unchecked);
      if (metric.enabled_by_default)
      {
        table->item(row, 0)->setCheckState(Qt::Checked);
      }
      qobject_cast<QLineEdit*>(table->cellWidget(row, 3))->setText(QString::fromStdString(metric.unit));
      for (int column : {1, 2})
      {
        auto value = metric.minimum;
        if (column == 2)
        {
          value = metric.maximum;
        }
        QString number;
        if (value)
        {
          number = QString::number(*value, 'g', 15);
        }
        qobject_cast<QLineEdit*>(table->cellWidget(row, column))->setText(number);
      }
    }
  }
}
bool ReportMeshQualitySettings::configuration_patch(ReportConfiguration& output, std::string& error) const
{
  ReportConfiguration candidate;
  for (const auto& family : familyTables)
  {
    auto* table = family.second;
    for (int row = 0; row < table->rowCount(); ++row)
    {
      const auto key = table->item(row, 0)->data(Qt::UserRole).toString().toStdString();
      candidate.values[key + ".enabled"] = "false";
      if (table->item(row, 0)->checkState() == Qt::Checked)
      {
        candidate.values[key + ".enabled"] = "true";
      }
      candidate.values[key + ".unit"] = qobject_cast<QLineEdit*>(table->cellWidget(row, 3))->text().trimmed().toStdString();
      candidate.values[key + ".minimum"] = qobject_cast<QLineEdit*>(table->cellWidget(row, 1))->text().trimmed().toStdString();
      candidate.values[key + ".maximum"] = qobject_cast<QLineEdit*>(table->cellWidget(row, 2))->text().trimmed().toStdString();
    }
  }
  MeshQualitySettings decoded;
  if (!MeshQualitySettings::decode(candidate, decoded, error))
  {
    return false;
  }
  output = std::move(candidate);
  return true;
}
