#include "ConnectionEndpointWidget.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <limits>

ConnectionEndpointWidget::ConnectionEndpointWidget(QWidget* parent) : QWidget(parent)
{
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  kind = new QComboBox(this);
  kind->addItems({"Vertex","Reference point"});
  vertex = new QSpinBox(this);
  vertex->setRange(1,(std::numeric_limits<int>::max)());
  pick = new QPushButton("Use selected vertex",this);
  points = new QComboBox(this);
  layout->addWidget(kind);
  layout->addWidget(vertex);
  layout->addWidget(pick);
  layout->addWidget(points);
  points->hide();
  connect(kind,&QComboBox::currentTextChanged,this,[this]()
  {
    const bool rp = kind->currentIndex() == 1;
    if (rp)
    {
      refresh();
    }
    points->setVisible(rp);
    vertex->setVisible(!rp);
    pick->setVisible(!rp);
  });
  connect(pick,&QPushButton::clicked,this,[this]()
  {
    const auto selected = CubitInterface::get_selected_entities();
    if (selected.size() != 1 || selected.front().first != "vertex")
    {
      PRINT_ERROR("Select one Cubit vertex.\n");
      return;
    }
    vertex->setValue(selected.front().second);
  });
}

void ConnectionEndpointWidget::refresh()
{
  const auto previous = points->currentData();
  points->clear();
  for (const auto& point : CalculiXConnectionsInterface().points())
  {
    points->addItem(QString("%1 / %2").arg(point.id).arg(QString::fromStdString(point.name)),point.id);
  }
  if (previous.isValid())
  {
    points->setCurrentIndex(points->findData(previous));
  }
}

QString ConnectionEndpointWidget::command() const
{
  if (kind->currentIndex() == 0)
  {
    return "vertex " + QString::number(vertex->value());
  }
  return "rp " + QString::number(points->currentData().toInt());
}

void ConnectionEndpointWidget::setEndpoint(const ConnectionEndpoint& endpoint)
{
  if (endpoint.kind == ConnectionEndpointKind::Vertex)
  {
    kind->setCurrentIndex(0);
    vertex->setValue(endpoint.id);
  }
  else
  {
    kind->setCurrentIndex(1);
    refresh();
    points->setCurrentIndex(points->findData(endpoint.id));
  }
}
