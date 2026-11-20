#include "ConnectionEditorPanel.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "GUI/properties/PropertyCommandText.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitMessage.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include "CubitInterface.hpp"
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <limits>

ConnectionEditorPanel::ConnectionEditorPanel(const QString& feature, QWidget* parent) : QWidget(parent)
{
  auto* form = new QFormLayout(this);
  auto* id = new QSpinBox(this);
  id->setRange(1, (std::numeric_limits<int>::max)());
  CalculiXConnectionsInterface iface;
  int last = 0;
  if (feature == "rp")
  {
    const auto points = iface.points();
    if (!points.empty())
    {
      last = points.back().id;
    }
  }
  else if (feature == "rigid body")
  {
    const auto bodies = iface.bodies();
    if (!bodies.empty())
    {
      last = bodies.back().id;
    }
  }
  else
  {
    const auto joints = iface.joints();
    if (!joints.empty())
    {
      last = joints.back().id;
    }
  }
  id->setValue(last < id->maximum() ? last + 1 : last);
  form->addRow("ID", id);
  auto* name = new QLineEdit(this);
  form->addRow("Name", name);
  auto* mode = new QComboBox(this);
  auto* type = new QComboBox(this);
  type->addItems({"revolute", "cylindrical", "prismatic", "spherical", "fixed"});
  if (feature == "rp")
  {
    mode->addItems({"vertex", "center curve", "center surface", "center volume", "xyz"});
    form->addRow("Placement", mode);
  }
  else if (feature == "rigid body")
  {
    mode->addItems({"surface", "curve", "volume", "block", "nodeset"});
    form->addRow("Attach to", mode);
  }
  else
  {
    mode->addItems({"Coaxial surfaces", "Reference points", "Ground"});
    form->addRow("Type", type);
    form->addRow("Endpoints", mode);
  }
  if (feature == "joint")
  {
    auto* summary = new QLabel(this);
    summary->setWordWrap(true);
    const QStringList descriptions{
      "The endpoints stay together. Relative rotation about the axis is free.",
      "The endpoints can slide along the axis and rotate about it.",
      "The endpoints can slide along the axis. Relative rotation is locked.",
      "The endpoints stay together. Relative rotation is free in all directions.",
      "The endpoints stay together and maintain their relative orientation."
    };
    auto describe = [=]() { summary->setText(descriptions.at(type->currentIndex())); };
    connect(type, &QComboBox::currentTextChanged, this, describe);
    describe();
    form->addRow(summary);
  }
  auto* first = new QSpinBox(this);
  auto* second = new QSpinBox(this);
  for (auto* control : {first, second})
  {
    control->setRange(1, (std::numeric_limits<int>::max)());
  }
  form->addRow(feature == "joint" ? "Side A ID" : "Entity ID", first);
  if (feature != "rp")
  {
    form->addRow(feature == "joint" ? "Side B ID" : "Reference point ID", second);
  }
  auto* automatic = new QCheckBox("Create reference point at center", this);
  automatic->setChecked(feature == "rigid body");
  automatic->setVisible(feature == "rigid body");
  if (feature == "rigid body")
  {
    form->addRow(automatic);
  }
  auto* pick = new QPushButton("Use selected geometry", this);
  form->addRow(pick);
  connect(pick, &QPushButton::clicked, this, [=]()
  {
    const auto selected = CubitInterface::get_selected_entities();
    if (selected.size() == 2 && feature == "joint" && selected[0].first == "surface" && selected[1].first == "surface")
    {
      mode->setCurrentIndex(0);
      first->setValue(selected[0].second);
      second->setValue(selected[1].second);
      return;
    }
    if (selected.size() == 1 && feature != "joint")
    {
      QString kind = QString::fromStdString(selected.front().first).toLower();
      if (feature == "rp" && kind != "vertex")
      {
        kind.prepend("center ");
      }
      const int index = mode->findText(kind);
      if (index >= 0)
      {
        mode->setCurrentIndex(index);
        first->setValue(selected.front().second);
        return;
      }
    }
    PRINT_ERROR("Select compatible geometry in Cubit. A coaxial joint requires two surfaces.\n");
  });
  auto* vector = new QLineEdit("0 0 1", this);
  if (feature != "rigid body")
  {
    form->addRow(feature == "rp" ? "Coordinates" : "Axis", vector);
  }
  type->setVisible(feature == "joint");
  second->setVisible(feature != "rp");
  vector->setVisible(feature != "rigid body");
  if (feature == "rp")
  {
    vector->setText("0 0 0");
  }
  auto sync = [=]()
  {
    first->setEnabled(mode->currentText() != "xyz");
    const bool center = feature == "rigid body" && mode->currentText() != "block" && mode->currentText() != "nodeset";
    automatic->setEnabled(center);
    second->setEnabled(mode->currentText() != "Ground" && !(center && automatic->isChecked()));
    vector->setEnabled(mode->currentText() == "xyz" || (feature == "joint" && mode->currentIndex() != 0));
  };
  connect(mode, &QComboBox::currentTextChanged, this, sync);
  connect(automatic, &QCheckBox::toggled, this, sync);
  sync();
  auto* history = new QComboBox(this);
  history->setVisible(feature != "rigid body");
  if (feature != "rigid body")
  {
    history->addItem("All mechanical steps", "all");
    for (const auto& step : CalculiXStepsInterface().get_steps_tree_data())
    {
      if (!step.empty())
      {
        const auto id_text = QString::fromStdString(step.front());
        history->addItem("Step " + id_text, "step " + id_text);
      }
    }
    history->addItem("Off", "off");
    form->addRow("History output (U + RF)", history);
  }
  auto* apply = new QPushButton("Create", this);
  form->addRow(apply);
  connect(apply, &QPushButton::clicked, this, [=]()
  {
    QString label, error;
    if (!PropertyCommandText::quoteName(name->text(), label, error))
    {
      PRINT_ERROR("%s\n", error.toUtf8().constData());
      return;
    }
    QString command = QString("ccx create %1 %2 name %3 ").arg(feature).arg(id->value()).arg(label);
    if (feature == "joint")
    {
      command += "type " + type->currentText();
      if (mode->currentIndex() == 0)
      {
        command += QString(" surface %1 to surface %2 coaxial").arg(first->value()).arg(second->value());
      }
      else
      {
        command += QString(" rp %1 to ").arg(first->value());
        command += mode->currentIndex() == 2 ? "ground" : QString("rp %1").arg(second->value());
      }
    }
    else
    {
      command += mode->currentText() + " ";
      if (mode->currentText() != "xyz")
      {
        command += QString::number(first->value());
      }
      if (feature == "rigid body")
      {
        command += automatic->isEnabled() && automatic->isChecked() ? " center" : QString(" rp %1").arg(second->value());
      }
    }
    if (vector->isEnabled() && feature != "rigid body")
    {
      const auto values = vector->text().simplified().split(' ');
      if (values.size() != 3)
      {
        PRINT_ERROR("Enter three numeric components.\n");
        return;
      }
      for (const auto& value : values)
      {
        bool ok = false;
        value.toDouble(&ok);
        if (!ok)
        {
          PRINT_ERROR("Invalid numeric component.\n");
          return;
        }
      }
      command += (feature == "joint" ? " axis " : "") + values.join(" ");
    }
    if (feature == "joint")
    {
      command += " history " + history->currentData().toString();
    }
    if (!GuiSendCmd::gui_cmd(command))
    {
      return;
    }
    if (feature != "rigid body" && feature != "joint" && history->currentIndex() != 0)
    {
      GuiSendCmd::gui_cmd(QString("ccx %1 %2 history %3").arg(feature).arg(id->value()).arg(history->currentData().toString()));
    }
    if (id->value() < id->maximum())
    {
      id->stepUp();
    }
  });
}
