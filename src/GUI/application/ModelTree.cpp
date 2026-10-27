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
#include "CommandButtonPanel.hpp"


ModelTree::ModelTree(QDockWidget* parent):QTreeWidget(parent)
{

  gui = Claro::instance();
  nav_model = gui->navigation_model();
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

  cmdpanel = dynamic_cast<CommandButtonPanel*>(gui->command_buttons());
  /*
  if (nav_model)
  {
    ccx_application_iface.print_to_log("CommandButtonPanel access");
  }
  */

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
  for (QTreeWidgetItem *owner = item; owner != nullptr; owner = owner->parent())
  {
    TopologyTree *topology_tree = dynamic_cast<TopologyTree *>(owner);
    if (topology_tree != nullptr)
    {
      topology_tree->activate(item, *this);
      return;
    }
  }

  BlocksTree* BlocksTreeItem;
  NodesetTree* NodesetTreeItem;
  SidesetTree* SidesetTreeItem;
  MaterialTree* MaterialTreeItem;
  SectionsTree* SectionsTreeItem;
  ConstraintsTree* ConstraintsTreeItem;
  SurfaceInteractionsTree* SurfaceInteractionsTreeItem;
  ContactPairsTree* ContactPairsTreeItem;
  AmplitudesTree* AmplitudesTreeItem;
  OrientationsTree* OrientationsTreeItem;
  DampingTree* DampingTreeItem;
  PhysicalConstantsTree* PhysicalConstantsTreeItem;
  LoadsForcesTree* LoadsForcesTreeItem;
  LoadsPressuresTree* LoadsPressuresTreeItem;
  LoadsHeatfluxesTree* LoadsHeatfluxesTreeItem;
  LoadsGravityTree* LoadsGravityTreeItem;
  LoadsCentrifugalTree* LoadsCentrifugalTreeItem;
  LoadsTrajectoryTree* LoadsTrajectoryTreeItem;
  LoadsFilmTree* LoadsFilmTreeItem;
  LoadsRadiationTree* LoadsRadiationTreeItem;
  LoadsSurfaceTractionTree* LoadsSurfaceTractionTreeItem;
  LoadsBodyHeatfluxTree* LoadsBodyHeatfluxTreeItem;
  BCsDisplacementsTree* BCsDisplacementsTreeItem;
  BCsTemperaturesTree* BCsTemperaturesTreeItem;
  HistoryOutputsTree* HistoryOutputsTreeItem;
  FieldOutputsTree* FieldOutputsTreeItem;
  InitialConditionsTree* InitialConditionsTreeItem;
  HBCsTree* HBCsTreeItem;
  HBCsDisplacementsTree* HBCsDisplacementsTreeItem;
  HBCsTemperaturesTree* HBCsTemperaturesTreeItem;
  StepsTree* StepsTreeItem;
  StepsLoadsTree* StepsLoadsTreeItem;
  StepsLoadsForcesTree* StepsLoadsForcesTreeItem;
  StepsLoadsPressuresTree* StepsLoadsPressuresTreeItem;
  StepsLoadsHeatfluxesTree* StepsLoadsHeatfluxesTreeItem;
  StepsLoadsGravityTree* StepsLoadsGravityTreeItem;
  StepsLoadsCentrifugalTree* StepsLoadsCentrifugalTreeItem;
  StepsLoadsTrajectoryTree* StepsLoadsTrajectoryTreeItem;
  StepsLoadsFilmTree* StepsLoadsFilmTreeItem;
  StepsLoadsRadiationTree* StepsLoadsRadiationTreeItem;
  StepsLoadsSurfaceTractionTree* StepsLoadsSurfaceTractionTreeItem;
  StepsLoadsBodyHeatfluxTree* StepsLoadsBodyHeatfluxTreeItem;
  StepsBCsTree* StepsBCsTreeItem;
  StepsBCsDisplacementsTree* StepsBCsDisplacementsTreeItem;
  StepsBCsTemperaturesTree* StepsBCsTemperaturesTreeItem;
  StepsHistoryOutputsTree* StepsHistoryOutputsTreeItem;
  StepsFieldOutputsTree* StepsFieldOutputsTreeItem;  
  CustomLinesTree* CustomLinesTreeItem;
  JobsTree* JobsTreeItem;
  QTreeWidgetItem* TreeItem;

  if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item))
  {
    if (BlocksTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXBlocksModify");
    }
  }else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item))
  {
    if (NodesetTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("ExodusCreateNodeset");
    }
  }else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item))
  {
    if (SidesetTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("ExodusCreateSideset");
    }
  }else if (MaterialTreeItem = dynamic_cast<MaterialTree*>(item))
  {
    if (MaterialTreeItem->text(1).toStdString()=="")
    {
      myMaterialManagement->show();
    }
  }else if (SectionsTreeItem = dynamic_cast<SectionsTree*>(item))
  {
    if (SectionsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXSectionsCreate");
    }
  }else if (ConstraintsTreeItem = dynamic_cast<ConstraintsTree*>(item))
  {
    if (ConstraintsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXConstraintsCreateRigidBody");
    }
  }else if (SurfaceInteractionsTreeItem = dynamic_cast<SurfaceInteractionsTree*>(item))
  {
    if (SurfaceInteractionsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXSurfaceInteractionsCreate");
    }
  }else if (ContactPairsTreeItem = dynamic_cast<ContactPairsTree*>(item))
  {
    if (ContactPairsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXContactPairsCreate");
    }
  }else if (AmplitudesTreeItem = dynamic_cast<AmplitudesTree*>(item))
  {
    if (AmplitudesTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXAmplitudesCreate");
    }
  }else if (OrientationsTreeItem = dynamic_cast<OrientationsTree*>(item))
  {
    if (OrientationsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXOrientationsCreate");
    }
  }else if (DampingTreeItem = dynamic_cast<DampingTree*>(item))
  {
    if (DampingTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXDampingModify");
    }
  }else if (PhysicalConstantsTreeItem = dynamic_cast<PhysicalConstantsTree*>(item))
  {
    if (PhysicalConstantsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXPhysicalConstantsModify");
    }
  }else if (LoadsForcesTreeItem = dynamic_cast<LoadsForcesTree*>(item))
  {
    if (LoadsForcesTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsForcesCreate");
    }
  }else if (LoadsPressuresTreeItem = dynamic_cast<LoadsPressuresTree*>(item))
  {
    if (LoadsPressuresTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsPressuresCreate");
    }
  }else if (LoadsHeatfluxesTreeItem = dynamic_cast<LoadsHeatfluxesTree*>(item))
  {
    if (LoadsHeatfluxesTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("FEAHeatfluxCreate");
    }
  }else if (LoadsGravityTreeItem = dynamic_cast<LoadsGravityTree*>(item))
  {
    if (LoadsGravityTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsGravityCreate");
    }
  }else if (LoadsCentrifugalTreeItem = dynamic_cast<LoadsCentrifugalTree*>(item))
  {
    if (LoadsCentrifugalTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsCentrifugalCreate");
    }
  }else if (LoadsTrajectoryTreeItem = dynamic_cast<LoadsTrajectoryTree*>(item))
  {
    if (LoadsTrajectoryTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsTrajectory");
    }
  }else if (LoadsFilmTreeItem = dynamic_cast<LoadsFilmTree*>(item))
  {
    if (LoadsFilmTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsFilmCreate");
    }
  }else if (LoadsRadiationTreeItem = dynamic_cast<LoadsRadiationTree*>(item))
  {
    if (LoadsRadiationTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsRadiationCreate");
    }
  }else if (LoadsSurfaceTractionTreeItem = dynamic_cast<LoadsSurfaceTractionTree*>(item))
  {
    if (LoadsSurfaceTractionTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsSurfaceTractionCreate");
    }
  }else if (LoadsBodyHeatfluxTreeItem = dynamic_cast<LoadsBodyHeatfluxTree*>(item))
  {
    if (LoadsBodyHeatfluxTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsBodyHeatfluxCreate");
    }
  }else if (BCsDisplacementsTreeItem = dynamic_cast<BCsDisplacementsTree*>(item))
  {
    if (BCsDisplacementsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("FEADisplacementCreate");
    }
  }else if (BCsTemperaturesTreeItem = dynamic_cast<BCsTemperaturesTree*>(item))
  {
    if (BCsTemperaturesTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("FEATemperatureCreate");
    }
  }else if (HistoryOutputsTreeItem = dynamic_cast<HistoryOutputsTree*>(item))
  {
    if (HistoryOutputsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXHistoryOutputsCreate");
    }
  }else if (FieldOutputsTreeItem = dynamic_cast<FieldOutputsTree*>(item))
  {
    if (FieldOutputsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXFieldOutputsCreate");
    }
  }else if (InitialConditionsTreeItem = dynamic_cast<InitialConditionsTree*>(item))
  {
    if (InitialConditionsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXInitialConditionsCreate");
    }
  }else if (HBCsTreeItem = dynamic_cast<HBCsTree*>(item))
  {
    if (HBCsTreeItem->text(1).toStdString()=="")
    {
      myHBCsManagement->close();
      myHBCsManagement->show();
    }
  }else if (HBCsDisplacementsTreeItem = dynamic_cast<HBCsDisplacementsTree*>(item))
  {
    if (HBCsDisplacementsTreeItem->text(1).toStdString()=="")
    {
      myHBCsManagement->close();
      myHBCsManagement->show();
    }
  }else if (HBCsTemperaturesTreeItem = dynamic_cast<HBCsTemperaturesTree*>(item))
  {
    if (HBCsTemperaturesTreeItem->text(1).toStdString()=="")
    {
      myHBCsManagement->close();
      myHBCsManagement->show();
    }
  }else if (StepsTreeItem = dynamic_cast<StepsTree*>(item))
  {
    if (StepsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXStepsCreate");
    }
  }else if (StepsLoadsTreeItem = dynamic_cast<StepsLoadsTree*>(item))
  {
    if (StepsLoadsTreeItem->text(1).toStdString()=="")
    { 
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsForcesTreeItem = dynamic_cast<StepsLoadsForcesTree*>(item))
  {
    if (StepsLoadsForcesTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsPressuresTreeItem = dynamic_cast<StepsLoadsPressuresTree*>(item))
  {
    if (StepsLoadsPressuresTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsHeatfluxesTreeItem = dynamic_cast<StepsLoadsHeatfluxesTree*>(item))
  {
    if (StepsLoadsHeatfluxesTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsGravityTreeItem = dynamic_cast<StepsLoadsGravityTree*>(item))
  {
    if (StepsLoadsGravityTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsCentrifugalTreeItem = dynamic_cast<StepsLoadsCentrifugalTree*>(item))
  {
    if (StepsLoadsCentrifugalTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsTrajectoryTreeItem = dynamic_cast<StepsLoadsTrajectoryTree*>(item))
  {
    if (StepsLoadsTrajectoryTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsFilmTreeItem = dynamic_cast<StepsLoadsFilmTree*>(item))
  {
    if (StepsLoadsFilmTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsRadiationTreeItem = dynamic_cast<StepsLoadsRadiationTree*>(item))
  {
    if (StepsLoadsRadiationTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsSurfaceTractionTreeItem = dynamic_cast<StepsLoadsSurfaceTractionTree*>(item))
  {
    if (StepsLoadsSurfaceTractionTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsLoadsBodyHeatfluxTreeItem = dynamic_cast<StepsLoadsBodyHeatfluxTree*>(item))
  {
    if (StepsLoadsBodyHeatfluxTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsBCsTreeItem = dynamic_cast<StepsBCsTree*>(item))
  {
    if (StepsBCsTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsBCsDisplacementsTreeItem = dynamic_cast<StepsBCsDisplacementsTree*>(item))
  {
    if (StepsBCsDisplacementsTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsBCsTemperaturesTreeItem = dynamic_cast<StepsBCsTemperaturesTree*>(item))
  {
    if (StepsBCsTemperaturesTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsHistoryOutputsTreeItem = dynamic_cast<StepsHistoryOutputsTree*>(item))
  {
    if (StepsHistoryOutputsTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (StepsFieldOutputsTreeItem = dynamic_cast<StepsFieldOutputsTree*>(item))
  {
    if (StepsFieldOutputsTreeItem->text(1).toStdString()=="")
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    }
  }else if (CustomLinesTreeItem = dynamic_cast<CustomLinesTree*>(item))
  {
    if (CustomLinesTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXCustomLinesCreate");
    }
  }else if (JobsTreeItem = dynamic_cast<JobsTree*>(item))
  {
    if (JobsTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("CCXJobsCreate");
    }
  } else {
    if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXBlocksModify");
    } else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("ExodusRemoveContentsNodeset");
    } else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("ExodusRemoveContentsSideset");
    } else if (MaterialTreeItem = dynamic_cast<MaterialTree*>(item->parent()))
    {
      myMaterialManagement->setMaterial(item->text(1));
      myMaterialManagement->close();
      myMaterialManagement->show();
    } else if (SectionsTreeItem = dynamic_cast<SectionsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXSectionsModify");
    } else if (ConstraintsTreeItem = dynamic_cast<ConstraintsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXConstraintsModifyRigidBody");
    } else if (SurfaceInteractionsTreeItem = dynamic_cast<SurfaceInteractionsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXSurfaceInteractionsModify");
    } else if (ContactPairsTreeItem = dynamic_cast<ContactPairsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXContactPairsModify");
    } else if (AmplitudesTreeItem = dynamic_cast<AmplitudesTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXAmplitudesModify");
    } else if (OrientationsTreeItem = dynamic_cast<OrientationsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXOrientationsModify");
    } else if (DampingTreeItem = dynamic_cast<DampingTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXDampingModify");
    } else if (PhysicalConstantsTreeItem = dynamic_cast<PhysicalConstantsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXPhysicalConstantsModify");
    } else if (LoadsForcesTreeItem = dynamic_cast<LoadsForcesTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsForcesModify");
    } else if (LoadsPressuresTreeItem = dynamic_cast<LoadsPressuresTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsPressuresModify");
    } else if (LoadsHeatfluxesTreeItem = dynamic_cast<LoadsHeatfluxesTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("FEAHeatfluxModify");
    } else if (LoadsGravityTreeItem = dynamic_cast<LoadsGravityTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsGravityModify");
    } else if (LoadsCentrifugalTreeItem = dynamic_cast<LoadsCentrifugalTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsCentrifugalModify");
    } else if (LoadsTrajectoryTreeItem = dynamic_cast<LoadsTrajectoryTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsTrajectory");
    } else if (LoadsFilmTreeItem = dynamic_cast<LoadsFilmTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsFilmModify");
    } else if (LoadsRadiationTreeItem = dynamic_cast<LoadsRadiationTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsRadiationModify");
    } else if (LoadsSurfaceTractionTreeItem = dynamic_cast<LoadsSurfaceTractionTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsSurfaceTractionModify");
    } else if (LoadsBodyHeatfluxTreeItem = dynamic_cast<LoadsBodyHeatfluxTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsBodyHeatfluxModify");
    } else if (BCsDisplacementsTreeItem = dynamic_cast<BCsDisplacementsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("FEADisplacementModify");
    } else if (BCsTemperaturesTreeItem = dynamic_cast<BCsTemperaturesTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("FEATemperatureModify");
    } else if (HistoryOutputsTreeItem = dynamic_cast<HistoryOutputsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXHistoryOutputsModify");
    } else if (FieldOutputsTreeItem = dynamic_cast<FieldOutputsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXFieldOutputsModify");
    } else if (InitialConditionsTreeItem = dynamic_cast<InitialConditionsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXInitialConditionsModify");
    } else if (HBCsTreeItem = dynamic_cast<HBCsTree*>(item->parent()))
    {
      myHBCsManagement->close();
      myHBCsManagement->show();
    } else if (HBCsDisplacementsTreeItem = dynamic_cast<HBCsDisplacementsTree*>(item->parent()))
    {
      myHBCsManagement->close();
      myHBCsManagement->show();
    } else if (HBCsTemperaturesTreeItem = dynamic_cast<HBCsTemperaturesTree*>(item->parent()))
    {
      myHBCsManagement->close();
      myHBCsManagement->show();
    } else if (StepsTreeItem = dynamic_cast<StepsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXStepsModify");
    } else if (StepsLoadsTreeItem = dynamic_cast<StepsLoadsTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsForcesTreeItem = dynamic_cast<StepsLoadsForcesTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsPressuresTreeItem = dynamic_cast<StepsLoadsPressuresTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsHeatfluxesTreeItem = dynamic_cast<StepsLoadsHeatfluxesTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsGravityTreeItem = dynamic_cast<StepsLoadsGravityTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsCentrifugalTreeItem = dynamic_cast<StepsLoadsCentrifugalTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsTrajectoryTreeItem = dynamic_cast<StepsLoadsTrajectoryTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsFilmTreeItem = dynamic_cast<StepsLoadsFilmTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsRadiationTreeItem = dynamic_cast<StepsLoadsRadiationTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsSurfaceTractionTreeItem = dynamic_cast<StepsLoadsSurfaceTractionTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsLoadsBodyHeatfluxTreeItem = dynamic_cast<StepsLoadsBodyHeatfluxTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsBCsTreeItem = dynamic_cast<StepsBCsTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsBCsDisplacementsTreeItem = dynamic_cast<StepsBCsDisplacementsTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsBCsTemperaturesTreeItem = dynamic_cast<StepsBCsTemperaturesTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsHistoryOutputsTreeItem = dynamic_cast<StepsHistoryOutputsTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (StepsFieldOutputsTreeItem = dynamic_cast<StepsFieldOutputsTree*>(item->parent()))
    {
      if (TreeItem = dynamic_cast<QTreeWidgetItem*>(item->parent()->parent()))
      {
        myStepsManagement->setStep(TreeItem->text(1));
      }
      myStepsManagement->close();
      myStepsManagement->show();
    } else if (CustomLinesTreeItem = dynamic_cast<CustomLinesTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXCustomLinesModify");
    } else if (JobsTreeItem = dynamic_cast<JobsTree*>(item->parent()))
    {
      myJobsMonitor->close();
      myJobsMonitor->show();
      myJobsMonitor->setJob(std::stoi(item->text(1).toStdString()));
      myJobsMonitor->update();
    }
  }
}
  
void ModelTree::setWidgetInCmdPanelMarker(const QString name) // get Widget from navigation model
{
  QModelIndex widget_index;
  NavigationNode* node = nav_model->getMarkedNode(name);
  widget_index = nav_model->getIndex(node);
  cmdpanel->setCurrent(widget_index);
}

void ModelTree::setWidgetInCmdPanelPath(const QString name) // get Widget from navigation model
{
  QModelIndex widget_index;
  NavigationNode* node = nav_model->getNode(name);
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
