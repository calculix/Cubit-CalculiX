#include "ConnectionsTree.hpp"
#include "DistributingAttachmentPanel.hpp"
#include "DiscreteConnectionPanel.hpp"
#include "GUI/application/ModelTree.hpp"
#include "GUI/loads/LoadsForcesCreatePanel.hpp"
#include "TaskPanel.hpp"
#include <QAction>
#include "Core/steps/CalculiXStepsInterface.hpp"
#include <QMenu>
#include "GUI/properties/PropertySelection.hpp"
#include <utility>
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"

namespace
{
void openDiscretePanel(ModelTree& host, const QString& marker, int id, int point)
{
  if (!host.cmdpanel)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker(marker);
  const auto type = marker == "PointMass" ? DiscreteConnectionPanel::Feature::PointMass : DiscreteConnectionPanel::Feature::Spring;
  for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
  {
    auto* panel = dynamic_cast<DiscreteConnectionPanel*>(widget);
    if (panel && panel->type() == type)
    {
      if (id > 0)
      {
        panel->editRecord(id);
      }
      else if (point > 0)
      {
        panel->setReferencePoint(point);
      }
      else
      {
        panel->newRecord();
      }
      return;
    }
  }
}
}

ConnectionsTree::ConnectionsTree(QTreeWidget* parent) : QTreeWidgetItem(parent)
{
  setText(0, "Connections");
  setIcon(0, CalculiXGuiResourcesInterface().getIcon2("Connections"));
  for (const auto& category : {std::make_pair("Reference points", "ReferencePoint"),
    std::make_pair("Rigid bodies", "RigidBody"), std::make_pair("Joints", "Joint"), std::make_pair("Distributing attachments","Connections"),
    std::make_pair("Point masses","PointMass"), std::make_pair("Springs","Spring")})
  {
    auto* group = new QTreeWidgetItem(this);
    group->setText(0, category.first);
    group->setIcon(0, CalculiXGuiResourcesInterface().getIcon2(category.second));
  }
}

void ConnectionsTree::clear()
{
  for (int i = 0; i < childCount(); ++i)
  {
    qDeleteAll(child(i)->takeChildren());
  }
}

void ConnectionsTree::populateContextMenu(QMenu& menu, QTreeWidgetItem* item, ModelTree& host)
{
  const QStringList kinds{"rp", "rigid body", "joint", "distributing attachment", "point mass", "spring"};
  const QStringList panels{"ReferencePoint", "RigidBody", "Joint", "DistributingAttachment", "PointMass", "Spring"};
  for (int i = 0; i < childCount(); ++i)
  {
    if (item == this || child(i) == item || item->parent() == child(i))
    {
      if (i >= 4)
      {
        const auto marker = panels[i];
        QObject::connect(menu.addAction("Create " + kinds[i]), &QAction::triggered, &menu, [&host,marker]()
        {
          openDiscretePanel(host,marker,0,0);
        });
      }
      else
      {
        addPanelAction(menu, "Create " + kinds[i], host, panels[i]);
      }
    }
    if (item->parent() != child(i))
    {
      continue;
    }
    const QString target = kinds[i] + " " + item->text(1);
    if (i >= 4)
    {
      const auto marker = panels[i];
      const int id = item->text(1).toInt();
      QObject::connect(menu.addAction("Modify " + kinds[i]), &QAction::triggered, &menu, [&host,marker,id]()
      {
        openDiscretePanel(host,marker,id,0);
      });
    }
    if (i == 0)
    {
      const int rp = item->text(1).toInt();
      for (const int index : {4,5})
      {
        const auto marker = panels[index];
        QObject::connect(menu.addAction("Add " + kinds[index] + "..."), &QAction::triggered, &menu, [&host,marker,rp]()
        {
          openDiscretePanel(host,marker,0,rp);
        });
      }
      QObject::connect(menu.addAction("Attach to deformable surface..."), &QAction::triggered, &menu, [&host, rp]()
      {
        if (host.cmdpanel)
        {
          host.setWidgetInCmdPanelMarker("DistributingAttachment");
          for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
          {
            if (auto* panel = dynamic_cast<DistributingAttachmentPanel*>(widget))
            {
              panel->selectPoint(rp);
              return;
            }
          }
        }
      });
      QObject::connect(menu.addAction("Apply force or moment..."), &QAction::triggered, &menu, [&host, rp]()
      {
        if (host.cmdpanel)
        {
          host.setWidgetInCmdPanelMarker("CCXLoadsForcesCreate");
          for (auto* widget : host.cmdpanel->findChildren<QWidget*>())
          {
            if (auto* panel = dynamic_cast<LoadsForcesCreatePanel*>(widget))
            {
              panel->setReferencePoint(rp);
              return;
            }
          }
        }
      });
    }
    if (i == 0 || i == 2 || i >= 4)
    {
      QString label = i >= 4 ? "History output" : "History output (U + RF)";
      if (i == 0)
      {
        for (const auto& attachment : CalculiXConnectionsInterface().attachments())
        {
          if (attachment.point == item->text(1).toInt())
          {
            label = "Surface history output";
            break;
          }
        }
      }
      auto* history = menu.addMenu(label);
      const QString command = "ccx " + target + " history ";
      addCommandAction(*history, "All mechanical steps", command + "all");
      for (const auto& step : CalculiXStepsInterface().get_steps_tree_data())
      {
        if (!step.empty())
        {
          const auto id = QString::fromStdString(step.front());
          addCommandAction(*history, "Step " + id, command + "step " + id);
        }
      }
      addCommandAction(*history, "Off", command + "off");
    }
    addCommandAction(menu, "Delete", "ccx delete " + target);
  }
}

void ConnectionsTree::update()
{
  CalculiXConnectionsInterface iface;
  auto populate = [&](int index, const auto& records)
  {
    auto* group = child(index);
    int row = 0;
    for (const auto& record : records)
    {
      auto* item = row < group->childCount() ? group->child(row) : new QTreeWidgetItem(group);
      const PropertyFeature features[] = {PropertyFeature::ReferencePoint, PropertyFeature::RigidBody, PropertyFeature::Joint, PropertyFeature::None,
        PropertyFeature::PointMass, PropertyFeature::Spring};
      item->setData(0, PropertyFeatureRole, static_cast<int>(features[index]));
      item->setData(0, PropertyIdRole, record.id);
      item->setText(0, QString::fromStdString(record.name));
      item->setText(1, QString::number(record.id));
      item->setIcon(0, group->icon(0));
      ++row;
    }
    while (group->childCount() > row)
    {
      delete group->takeChild(row);
    }
  };
  const auto points = iface.points();
  const auto bodies = iface.bodies();
  const auto joints = iface.joints();
  populate(0, points);
  populate(1, bodies);
  populate(2, joints);
  const auto attachments = iface.attachments();
  populate(3, attachments);
  const auto masses = iface.point_masses();
  const auto springs = iface.springs();
  populate(4, masses);
  populate(5, springs);
  for (int row = 0; row < child(0)->childCount(); ++row)
  {
    auto* item = child(0)->child(row);
    const int id = item->text(1).toInt();
    QStringList uses;
    for (const auto& body : bodies)
    {
      if (body.point == id)
      {
        uses.append("Rigid body " + QString::number(body.id) + " / " + QString::fromStdString(body.name));
      }
    }
    for (const auto& joint : joints)
    {
      if (joint.point_a == id || joint.point_b == id)
      {
        uses.append("Joint " + QString::number(joint.id) + " / " + QString::fromStdString(joint.name));
      }
    }
    for (const auto& a : attachments)
    {
      if (a.point == id)
      {
        uses.append("Distributing attachment " + QString::number(a.id));
      }
    }
    for (const auto& mass : masses)
    {
      if (mass.endpoint.uses_point(id))
      {
        uses.append("Point mass " + QString::number(mass.id) + " / " + QString::fromStdString(mass.name));
      }
    }
    for (const auto& spring : springs)
    {
      if (spring.uses_point(id))
      {
        uses.append("Spring " + QString::number(spring.id) + " / " + QString::fromStdString(spring.name));
      }
    }
    for (int force : iface.point_forces(id))
    {
      uses.append("Force " + QString::number(force));
    }
    item->setToolTip(0, uses.empty() ? QString("Standalone reference point") : uses.join("\n"));
  }
}
