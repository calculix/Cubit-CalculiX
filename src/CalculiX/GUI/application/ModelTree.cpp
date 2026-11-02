#include <QItemSelectionModel>
#include <QSignalBlocker>
#include <QTreeWidgetItemIterator>
#include <QVariant>
#include "ModelTree.hpp"
#include "GUI/common/TreeContextMenu.hpp"
#include "GUI/mesh/BlocksTree.hpp"
#include "GUI/mesh/NodesetTree.hpp"
#include "GUI/mesh/SidesetTree.hpp"
#include "GUI/materials/MaterialTree.hpp"
#include "GUI/materials/MaterialManagement.hpp"
#include "GUI/sections/SectionsTree.hpp"
#include "GUI/interactions/ConstraintsTree.hpp"
#include "GUI/interactions/SurfaceInteractionsTree.hpp"
#include "GUI/interactions/ContactPairsTree.hpp"
#include "GUI/amplitudes/AmplitudesTree.hpp"
#include "GUI/sections/OrientationsTree.hpp"
#include "GUI/steps/DampingTree.hpp"
#include "GUI/steps/PhysicalConstantsTree.hpp"
#include "GUI/loads/LoadsTree.hpp"
#include "GUI/loads/LoadsForcesTree.hpp"
#include "GUI/loads/LoadsPressuresTree.hpp"
#include "GUI/loads/LoadsHeatfluxesTree.hpp"
#include "GUI/loads/LoadsGravityTree.hpp"
#include "GUI/loads/LoadsCentrifugalTree.hpp"
#include "GUI/loads/LoadsTrajectoryTree.hpp"
#include "GUI/loads/LoadsFilmTree.hpp"
#include "GUI/loads/LoadsRadiationTree.hpp"
#include "GUI/loads/LoadsSurfaceTractionTree.hpp"
#include "GUI/loads/LoadsBodyHeatfluxTree.hpp"
#include "GUI/bcs/BCsTree.hpp"
#include "GUI/bcs/BCsDisplacementsTree.hpp"
#include "GUI/bcs/BCsTemperaturesTree.hpp"
#include "GUI/steps/HistoryOutputsTree.hpp"
#include "GUI/steps/FieldOutputsTree.hpp"
#include "GUI/steps/InitialConditionsTree.hpp"
#include "GUI/bcs/HBCsTree.hpp"
#include "GUI/bcs/HBCsDisplacementsTree.hpp"
#include "GUI/bcs/HBCsTemperaturesTree.hpp"
#include "GUI/bcs/HBCsManagement.hpp"
#include "GUI/steps/StepsTree.hpp"
#include "GUI/topology/TopologyTree.hpp"
#include "GUI/steps/StepsLoadsTree.hpp"
#include "GUI/steps/StepsLoadsForcesTree.hpp"
#include "GUI/steps/StepsLoadsPressuresTree.hpp"
#include "GUI/steps/StepsLoadsHeatfluxesTree.hpp"
#include "GUI/steps/StepsLoadsGravityTree.hpp"
#include "GUI/steps/StepsLoadsCentrifugalTree.hpp"
#include "GUI/steps/StepsLoadsTrajectoryTree.hpp"
#include "GUI/steps/StepsLoadsFilmTree.hpp"
#include "GUI/steps/StepsLoadsRadiationTree.hpp"
#include "GUI/steps/StepsLoadsSurfaceTractionTree.hpp"
#include "GUI/steps/StepsLoadsBodyHeatfluxTree.hpp"
#include "GUI/steps/StepsBCsTree.hpp"
#include "GUI/steps/StepsBCsDisplacementsTree.hpp"
#include "GUI/steps/StepsBCsTemperaturesTree.hpp"
#include "GUI/steps/StepsHistoryOutputsTree.hpp"
#include "GUI/steps/StepsFieldOutputsTree.hpp"
#include "GUI/steps/StepsManagement.hpp"
#include "GUI/jobs/JobsTree.hpp"
#include "GUI/export/CustomLinesTree.hpp"
#include "GUI/jobs/JobsMonitor.hpp"


#include "CubitInterface.hpp"
#include "GuiSendCmd.hpp"

#include "Claro.hpp"
#include "Broker.hpp"
#include "NavigationModel.hpp"
#include "NavigationNode.hpp"
#include <QString>
#include <QStringList>

#include "MyCmdWidgetFactory.hpp"
#include "TaskPanel.hpp"


ModelTree::ModelTree(QDockWidget* parent, NavigationModel* navigation):QTreeWidget(parent)
{

  gui = Claro::instance();
  nav_model = navigation;
  myMaterialManagement = new MaterialManagement();
  myHBCsManagement = new HBCsManagement();
  myStepsManagement = new StepsManagement();
  myJobsMonitor = new JobsMonitor();

  /*
  if (nav_model)
  {
    ccx_application_iface.print_to_log("NavigationModel access");
  }
  */

  cmdpanel = gui->command_buttons();
  /*
  if (nav_model)
  {
    ccx_application_iface.print_to_log("CommandButtonPanel access");
  }
  */

  setSelectionMode(QAbstractItemView::SingleSelection);
  connect(this, &QTreeWidget::itemSelectionChanged, this, &ModelTree::updatePropertySelection);

  this->setColumnCount(2);  
  this->setHeaderLabels(QStringList() << "Name" << "ID");
  this->setColumnWidth(0, 280);
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  bool success;
  success = false;

  success = connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(showContextMenu(const QPoint &)));

  success = connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
          this, SLOT(ModelTreeItemDoubleClicked(QTreeWidgetItem*, int)));
  
  /*if (success)
  {
    ccx_application_iface.print_to_log("connected signals and slots");
  }*/   

}

ModelTree::~ModelTree()
{
}

void ModelTree::showContextMenu(const QPoint &pos)
{
  QTreeWidgetItem *clickedItem = itemAt(pos);
  if (clickedItem == nullptr)
  {
    return;
  }

  for (QTreeWidgetItem *owner = clickedItem; owner != nullptr; owner = owner->parent())
  {
    TreeContextMenu *provider = dynamic_cast<TreeContextMenu *>(owner);
    if (provider != nullptr)
    {
      QMenu menu(this);
      provider->populateContextMenu(menu, clickedItem, *this);
      if (!menu.actions().isEmpty())
      {
        menu.exec(viewport()->mapToGlobal(pos));
      }
      return;
    }
  }
}

void ModelTree::ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column)
{
  Q_UNUSED(column);
  for (auto* owner = item; owner != nullptr; owner = owner->parent())
  {
    if (auto* feature = dynamic_cast<TopologyTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsBCsDisplacementsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsBCsTemperaturesTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsBCsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsFieldOutputsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsHistoryOutputsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsBodyHeatfluxTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsCentrifugalTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsFilmTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsForcesTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsGravityTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsHeatfluxesTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsPressuresTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsRadiationTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsSurfaceTractionTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsTrajectoryTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsLoadsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<HBCsDisplacementsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<HBCsTemperaturesTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<HBCsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<BlocksTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<MaterialTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsForcesTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsPressuresTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsHeatfluxesTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsSurfaceTractionTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsGravityTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsCentrifugalTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsFilmTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsRadiationTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsBodyHeatfluxTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<BCsTemperaturesTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<BCsDisplacementsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<SectionsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<OrientationsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<ConstraintsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<ContactPairsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<AmplitudesTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<CustomLinesTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<StepsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<HistoryOutputsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<FieldOutputsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<InitialConditionsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<JobsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<DampingTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<PhysicalConstantsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<SurfaceInteractionsTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* feature = dynamic_cast<LoadsTrajectoryTree*>(owner))
    {
      feature->activate(item, *this);
      return;
    }
    if (auto* nodesets = dynamic_cast<NodesetTree*>(owner))
    {
      setWidgetInCmdPanelMarker(item == nodesets ? "ExodusCreateNodeset" : "ExodusRemoveContentsNodeset");
      return;
    }
    if (auto* sidesets = dynamic_cast<SidesetTree*>(owner))
    {
      setWidgetInCmdPanelMarker(item == sidesets ? "ExodusCreateSideset" : "ExodusRemoveContentsSideset");
      return;
    }
  }
}
  
void ModelTree::setWidgetInCmdPanelMarker(const QString name) // get Widget from navigation model
{
  QModelIndex widget_index;
  if (nav_model == nullptr || cmdpanel == nullptr)
  {
    return;
  }

  NavigationNode* node = nav_model->getMarkedNode(name);

  if (node == nullptr)
  {
    return;
  }

  widget_index = nav_model->getIndex(node);
  cmdpanel->setCurrent(widget_index);
}

void ModelTree::setWidgetInCmdPanelPath(const QString name) // get Widget from navigation model
{
  QModelIndex widget_index;
  if (nav_model == nullptr || cmdpanel == nullptr)
  {
    return;
  }

  NavigationNode* node = nav_model->getNode(name);

  if (node == nullptr)
  {
    return;
  }

  widget_index = nav_model->getIndex(node);
  cmdpanel->setCurrent(widget_index);
}

void ModelTree::update(){
  myMaterialManagement->update();
  myHBCsManagement->update();
  myStepsManagement->update();
  myJobsMonitor->update();
}

void ModelTree::clear(){
  myMaterialManagement->clear();
  myHBCsManagement->clear();
  myStepsManagement->clear();
  myJobsMonitor->clear();
}


bool ModelTree::selectDrawingEntity(const CalculiXDrawSelection& selection)
{
  if (!selection.active)
  {
    clearSelection();
    return true;
  }
  PropertyFeature feature = PropertyFeature::None;
  switch (selection.entity.feature)
  {
    case DrawFeature::Force: feature = PropertyFeature::Force; break;
    case DrawFeature::Pressure: feature = PropertyFeature::Pressure; break;
    case DrawFeature::Heatflux: feature = PropertyFeature::Heatflux; break;
    case DrawFeature::Gravity: feature = PropertyFeature::Gravity; break;
    case DrawFeature::Centrifugal: feature = PropertyFeature::Centrifugal; break;
    case DrawFeature::Trajectory: feature = PropertyFeature::Trajectory; break;
    case DrawFeature::Film: feature = PropertyFeature::Film; break;
    case DrawFeature::Radiation: feature = PropertyFeature::Radiation; break;
    case DrawFeature::SurfaceTraction: feature = PropertyFeature::SurfaceTraction; break;
    case DrawFeature::BodyHeatflux: feature = PropertyFeature::BodyHeatflux; break;
    case DrawFeature::Displacement: feature = PropertyFeature::Displacement; break;
    case DrawFeature::Temperature: feature = PropertyFeature::Temperature; break;
    case DrawFeature::Orientation: feature = PropertyFeature::Orientation; break;
    case DrawFeature::Equation:
    case DrawFeature::EquationGroup: feature = PropertyFeature::Constraint; break;
    case DrawFeature::Topology: return false;
  }
  QTreeWidgetItem* match = nullptr;
  QTreeWidgetItem* global = nullptr;
  for (QTreeWidgetItemIterator iterator(this); *iterator != nullptr; ++iterator)
  {
    auto* item = *iterator;
    bool valid = false;
    const int id = item->data(0, PropertyIdRole).toInt(&valid);
    const auto value = item->data(0, PropertyFeatureRole);
    if (!valid || !value.isValid() || id != selection.entity.id || value.toInt() != static_cast<int>(feature))
    {
      continue;
    }
    int stepId = -1;
    for (const auto* ancestor = item; ancestor != nullptr; ancestor = ancestor->parent())
    {
      const auto step = ancestor->data(0, PropertyStepIdRole);
      if (step.isValid())
      {
        stepId = step.toInt();
        break;
      }
    }
    if (stepId == selection.stepId)
    {
      match = item;
      break;
    }
    if (stepId < 0 && !item->data(0, PropertyHomogeneousRole).toBool())
    {
      global = item;
    }
  }
  // Inherited conditions may not have an explicit entry beneath the active step.
  if (match == nullptr && selection.stepId >= 0)
  {
    match = global;
  }
  if (match == nullptr)
  {
    return false;
  }
  {
    const QSignalBlocker blocker(this);
    for (auto* ancestor = match->parent(); ancestor != nullptr; ancestor = ancestor->parent())
    {
      ancestor->setExpanded(true);
    }
    setCurrentItem(match, 0, QItemSelectionModel::ClearAndSelect);
    scrollToItem(match, QAbstractItemView::EnsureVisible);
  }
  updatePropertySelection();
  return true;
}

void ModelTree::updatePropertySelection()
{
  PropertySelection selection;
  const auto items = selectedItems();
  if (items.size() == 1)
  {
    const auto* item = items.front();
    const QVariant feature = item->data(0, PropertyFeatureRole);
    bool valid = false;
    const int id = item->data(0, PropertyIdRole).toInt(&valid);
    if (feature.isValid() && valid && id >= 0)
    {
      selection.feature = static_cast<PropertyFeature>(feature.toInt());
      selection.id = id;
      selection.caption = item->text(0);
      selection.icon = item->icon(0);
      selection.homogeneous = item->data(0, PropertyHomogeneousRole).toBool();
      for (const auto* ancestor = item; ancestor != nullptr; ancestor = ancestor->parent())
      {
        const QVariant step = ancestor->data(0, PropertyStepIdRole);
        if (step.isValid())
        {
          selection.stepId = step.toInt();
          break;
        }
      }
      const QVariant topology = item->data(0, PropertyTopologyIdRole);
      if (topology.isValid())
      {
        selection.topologyId = topology.toInt();
      }
      const QVariant iteration = item->data(0, PropertyIterationIdRole);
      if (iteration.isValid())
      {
        selection.iterationId = iteration.toInt();
      }
    }
  }
  emit propertySelectionChanged(selection);
}
