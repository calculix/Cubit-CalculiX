#include "DiscreteConnectionPanel.hpp"
#include "ConnectionEndpointWidget.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "GUI/common/CalculiXUnitPresentation.hpp"
#include "GUI/properties/PropertyCommandText.hpp"
#include "CubitMessage.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSpinBox>
#include <limits>

DiscreteConnectionPanel::DiscreteConnectionPanel(Feature type,QWidget* parent) : QWidget(parent), feature(type)
{
  auto* form = new QFormLayout(this);
  id = new QSpinBox(this);
  id->setRange(1,(std::numeric_limits<int>::max)());
  name = new QLineEdit(this);
  first = new ConnectionEndpointWidget(this);
  form->addRow("ID",id);
  form->addRow("Name",name);
  form->addRow(feature == Feature::PointMass ? "Point" : "Endpoint A",first);
  if (feature == Feature::Spring)
  {
    springType = new QComboBox(this);
    springType->addItems({"Between endpoints","To ground"});
    form->addRow("Spring type",springType);
    second = new ConnectionEndpointWidget(this);
    auto* endLabel = new QLabel("Endpoint B",this);
    form->addRow(endLabel,second);
    direction = new QComboBox(this);
    direction->addItems({"X","Y","Z"});
    auto* axisLabel = new QLabel("Global direction",this);
    form->addRow(axisLabel,direction);
    axisLabel->hide();
    direction->hide();
    connect(springType,&QComboBox::currentTextChanged,this,[this,endLabel,axisLabel]()
    {
      const bool ground = springType->currentIndex() == 1;
      endLabel->setVisible(!ground);
      second->setVisible(!ground);
      axisLabel->setVisible(ground);
      direction->setVisible(ground);
    });
  }
  value = new QLineEdit(this);
  auto* quantity = new QLabel(feature == Feature::Spring ? "Stiffness" : "Mass",this);
  form->addRow(quantity,value);
  auto* units = new CalculiXUnitPresentation(this);
  units->bind(*quantity,*value,quantity->text(),feature == Feature::Spring ? "force_per_length" : "mass");
  history = new QComboBox(this);
  history->addItems({"All supported steps","Off","Selected step"});
  step = new QSpinBox(this);
  step->setRange(1,id->maximum());
  auto* stepLabel = new QLabel("Step ID",this);
  form->addRow("History output",history);
  form->addRow(stepLabel,step);
  stepLabel->hide();
  step->hide();
  connect(history,&QComboBox::currentTextChanged,this,[this,stepLabel]()
  {
    step->setVisible(history->currentIndex() == 2);
    stepLabel->setVisible(history->currentIndex() == 2);
  });
  apply = new QPushButton(this);
  auto* newButton = new QPushButton("New",this);
  form->addRow(newButton,apply);
  connect(newButton,&QPushButton::clicked,this,[this]() { newRecord(); });
  connect(apply,&QPushButton::clicked,this,[this]() { submit(); });
  newRecord();
}

DiscreteConnectionPanel::Feature DiscreteConnectionPanel::type() const
{
  return feature;
}

void DiscreteConnectionPanel::newRecord()
{
  const CalculiXConnectionsInterface iface;
  int last = 0;
  if (feature == Feature::PointMass)
  {
    const auto records = iface.point_masses();
    if (!records.empty())
    {
      last = records.back().id;
    }
  }
  else
  {
    const auto records = iface.springs();
    if (!records.empty())
    {
      last = records.back().id;
    }
    springType->setCurrentIndex(0);
    second->setEndpoint({ConnectionEndpointKind::Vertex,2});
  }
  editing = false;
  id->setEnabled(true);
  id->setValue(last < id->maximum() ? last+1 : last);
  name->clear();
  value->clear();
  history->setCurrentIndex(0);
  first->setEndpoint({ConnectionEndpointKind::Vertex,1});
  apply->setText("Create");
}

void DiscreteConnectionPanel::setReferencePoint(int point)
{
  newRecord();
  first->setEndpoint({ConnectionEndpointKind::ReferencePoint,point});
}

void DiscreteConnectionPanel::editRecord(int selected)
{
  newRecord();
  CalculiXConnectionsInterface iface;
  int assignment = -1;
  double si = 0.0;
  if (feature == Feature::PointMass)
  {
    ConnectionPointMass item;
    if (!iface.point_mass(selected,item))
    {
      PRINT_ERROR("Unknown point mass %d.\n",selected);
      return;
    }
    name->setText(QString::fromStdString(item.name));
    first->setEndpoint(item.endpoint);
    assignment = item.history_step;
    si = item.mass_si;
  }
  else
  {
    ConnectionSpring item;
    if (!iface.spring(selected,item))
    {
      PRINT_ERROR("Unknown spring %d.\n",selected);
      return;
    }
    name->setText(QString::fromStdString(item.name));
    first->setEndpoint(item.first);
    springType->setCurrentIndex(item.second ? 0 : 1);
    if (item.second)
    {
      second->setEndpoint(*item.second);
    }
    else
    {
      direction->setCurrentIndex(item.direction-1);
    }
    assignment = item.history_step;
    si = item.stiffness_si;
  }
  const std::string key = feature == Feature::PointMass ? "mass" : "force_per_length";
  SimUnits::QuantityUnit quantity;
  std::string unit,error;
  double displayed;
  if (!framework_units().quantity(key,quantity,error) || !framework_units().display_unit("calculix",key,unit,error) ||
      !framework_units().from_si(si,unit,quantity.dimension,quantity.role,displayed,error))
  {
    PRINT_ERROR("%s\n",error.c_str());
    return;
  }
  value->setText(QString::number(displayed,'g',17));
  CalculiXUnitPresentation::accept_loaded_values(*this);
  history->setCurrentIndex(assignment < 0 ? 0 : (assignment == 0 ? 1 : 2));
  if (assignment > 0)
  {
    step->setValue(assignment);
  }
  editing = true;
  id->setValue(selected);
  id->setEnabled(false);
  apply->setText("Apply");
}

void DiscreteConnectionPanel::submit()
{
  QString quoted,error;
  if (!PropertyCommandText::quoteName(name->text(),quoted,error))
  {
    PRINT_ERROR("%s\n",error.toUtf8().constData());
    return;
  }
  std::string unitError;
  if (!CalculiXUnitPresentation::refresh_panel(*this,unitError))
  {
    PRINT_ERROR("%s\n",unitError.c_str());
    return;
  }
  bool ok = false;
  const double number = value->text().trimmed().toDouble(&ok);
  if (!ok)
  {
    PRINT_ERROR("Enter a numeric mass or stiffness.\n");
    return;
  }
  const bool spring = feature == Feature::Spring;
  QString command = editing ? "ccx modify " : "ccx create ";
  command += spring ? "spring " : "point mass ";
  command += QString::number(id->value()) + " name " + quoted + " ";
  if (spring)
  {
    command += "from " + first->command() + " to ";
    command += springType->currentIndex() == 0 ? second->command() : "ground direction " + direction->currentText().toLower();
    command += " stiffness ";
  }
  else
  {
    command += first->command() + " mass ";
  }
  std::string unit;
  if (!framework_units().display_unit("calculix",spring ? "force_per_length" : "mass",unit,unitError))
  {
    PRINT_ERROR("%s\n",unitError.c_str());
    return;
  }
  command += QString::number(number,'g',17) + " unit \"" + QString::fromStdString(unit) + "\" history ";
  if (history->currentIndex() == 2)
  {
    command += "step " + QString::number(step->value());
  }
  else
  {
    command += history->currentIndex() == 0 ? "all" : "off";
  }
  if (editing)
  {
    GuiSendCmd::gui_cmd(command);
    return;
  }

  // GUI submission reports dispatch, including when the command rejects the draft.
  CalculiXConnectionsInterface iface;
  const int requestedId = id->value();
  const auto exists = [&]()
  {
    if (spring)
    {
      ConnectionSpring record;
      return iface.spring(requestedId,record);
    }
    ConnectionPointMass record;
    return iface.point_mass(requestedId,record);
  };
  const bool existed = exists();
  QPointer<DiscreteConnectionPanel> guard(this);
  if (GuiSendCmd::gui_cmd(command) && guard && !existed && exists())
  {
    newRecord();
  }
}
