#include "ForceTargetWidget.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include <QComboBox>
#include <QLineEdit>
#include <QHBoxLayout>

ForceTargetWidget::ForceTargetWidget(QWidget* parent) : QWidget(parent)
{
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  kind = new QComboBox(this);
  kind->addItems({"Nodeset", "Reference point"});
  nodes = new QLineEdit(this);
  nodes->setPlaceholderText("Nodeset ID");
  points = new QComboBox(this);
  points->hide();
  layout->addWidget(kind);
  layout->addWidget(nodes);
  layout->addWidget(points);
  connect(kind, &QComboBox::currentTextChanged, this, [this]()
  {
    const bool rp = kind->currentIndex() == 1;
    if (rp)
    {
      refreshPoints();
    }
    nodes->setVisible(!rp);
    points->setVisible(rp);
  });
}

void ForceTargetWidget::refreshPoints()
{
  points->clear();
  for (const auto& point : CalculiXConnectionsInterface().points())
  {
    points->addItem(QString("%1 / %2").arg(point.id).arg(QString::fromStdString(point.name)), point.id);
  }
}

void ForceTargetWidget::setTarget(int referencePoint, const QString& nodeset)
{
  kind->setCurrentIndex(referencePoint > 0 ? 1 : 0);
  nodes->setText(nodeset);
  if (referencePoint > 0)
  {
    refreshPoints();
    points->setCurrentIndex(points->findData(referencePoint));
  }
}

QString ForceTargetWidget::idText() const
{
  return kind->currentIndex() == 1 ? points->currentData().toString() : nodes->text().trimmed();
}

QString ForceTargetWidget::keyword() const
{
  return kind->currentIndex() == 1 ? "rp" : "nodeset";
}
