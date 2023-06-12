
#include "Highlight.hpp"

#include "ModelTree.hpp"
#include "BlocksTree.hpp"
#include "NodesetTree.hpp"
#include "SidesetTree.hpp"
#include "MaterialTree.hpp"
#include "MaterialManagement.hpp"
#include "SectionsTree.hpp"
#include "ConstraintsTree.hpp"
#include "SurfaceInteractionsTree.hpp"
#include "ContactPairsTree.hpp"
#include "AmplitudesTree.hpp"
#include "LoadsTree.hpp"
#include "LoadsForcesTree.hpp"
#include "LoadsPressuresTree.hpp"
#include "BCsTree.hpp"
#include "BCsDisplacementsTree.hpp"
#include "BCsTemperaturesTree.hpp"
#include "HistoryOutputsTree.hpp"
#include "FieldOutputsTree.hpp"
#include "InitialConditionsTree.hpp"
#include "StepsTree.hpp"
#include "StepsLoadsTree.hpp"
#include "StepsLoadsForcesTree.hpp"
#include "StepsLoadsPressuresTree.hpp"
#include "StepsBCsTree.hpp"
#include "StepsBCsDisplacementsTree.hpp"
#include "StepsBCsTemperaturesTree.hpp"
#include "StepsHistoryOutputsTree.hpp"
#include "StepsFieldOutputsTree.hpp"
#include "StepsManagement.hpp"
#include "JobsTree.hpp"
#include "JobsMonitor.hpp"

#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

Highlight::Highlight(ModelTree* parent):QWidget(parent)
{
  ccx_iface = new CalculiXCoreInterface();

  bool success;
  success = false;

  success = connect(parent, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
          this, SLOT(ModelTreeItemClicked(QTreeWidgetItem*, int)));
  
  if (success)
  {
    std::string log;
    log = "connected signals and slots\n";
    PRINT_INFO("%s", log.c_str()); 
  }
}

Highlight::~Highlight()
{
}

void Highlight::ModelTreeItemClicked(QTreeWidgetItem* item, int column)
{
  BlocksTree* BlocksTreeItem;
  NodesetTree* NodesetTreeItem;
  SidesetTree* SidesetTreeItem;
  MaterialTree* MaterialTreeItem;
  SectionsTree* SectionsTreeItem;
  ConstraintsTree* ConstraintsTreeItem;
  SurfaceInteractionsTree* SurfaceInteractionsTreeItem;
  ContactPairsTree* ContactPairsTreeItem;
  AmplitudesTree* AmplitudesTreeItem;
  LoadsForcesTree* LoadsForcesTreeItem;
  LoadsPressuresTree* LoadsPressuresTreeItem;
  BCsDisplacementsTree* BCsDisplacementsTreeItem;
  BCsTemperaturesTree* BCsTemperaturesTreeItem;
  HistoryOutputsTree* HistoryOutputsTreeItem;
  FieldOutputsTree* FieldOutputsTreeItem;
  InitialConditionsTree* InitialConditionsTreeItem;
  StepsTree* StepsTreeItem;
  StepsLoadsTree* StepsLoadsTreeItem;
  StepsLoadsForcesTree* StepsLoadsForcesTreeItem;
  StepsLoadsPressuresTree* StepsLoadsPressuresTreeItem;
  StepsBCsTree* StepsBCsTreeItem;
  StepsBCsDisplacementsTree* StepsBCsDisplacementsTreeItem;
  StepsBCsTemperaturesTree* StepsBCsTemperaturesTreeItem;
  StepsHistoryOutputsTree* StepsHistoryOutputsTreeItem;
  StepsFieldOutputsTree* StepsFieldOutputsTreeItem;  
  JobsTree* JobsTreeItem;

  CubitInterface::clear_highlight();
  CubitInterface::highlight("block",1);
  std::string log;
  log = "item clicked\n";
  PRINT_INFO("%s", log.c_str()); 

/*
  if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item))
  {
    if (BlocksTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("BlocksCCXElementType");
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
  }else if (LoadsForcesTreeItem = dynamic_cast<LoadsForcesTree*>(item))
  {
    if (LoadsForcesTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("FEAForceCreate");
    }
  }else if (LoadsPressuresTreeItem = dynamic_cast<LoadsPressuresTree*>(item))
  {
    if (LoadsPressuresTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("FEAPressureCreate");
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
      myStepsManagement->show();
    }
  }else if (StepsLoadsForcesTreeItem = dynamic_cast<StepsLoadsForcesTree*>(item))
  {
    if (StepsLoadsForcesTreeItem->text(1).toStdString()=="")
    {
      myStepsManagement->show();
    }
  }else if (StepsLoadsPressuresTreeItem = dynamic_cast<StepsLoadsPressuresTree*>(item))
  {
    if (StepsLoadsPressuresTreeItem->text(1).toStdString()=="")
    {
      myStepsManagement->show();
    }
  }else if (StepsBCsTreeItem = dynamic_cast<StepsBCsTree*>(item))
  {
    if (StepsBCsTreeItem->text(1).toStdString()=="")
    {
      myStepsManagement->show();
    }
  }else if (StepsBCsDisplacementsTreeItem = dynamic_cast<StepsBCsDisplacementsTree*>(item))
  {
    if (StepsBCsDisplacementsTreeItem->text(1).toStdString()=="")
    {
      myStepsManagement->show();
    }
  }else if (StepsBCsTemperaturesTreeItem = dynamic_cast<StepsBCsTemperaturesTree*>(item))
  {
    if (StepsBCsTemperaturesTreeItem->text(1).toStdString()=="")
    {
      myStepsManagement->show();
    }
  }else if (StepsHistoryOutputsTreeItem = dynamic_cast<StepsHistoryOutputsTree*>(item))
  {
    if (StepsHistoryOutputsTreeItem->text(1).toStdString()=="")
    {
      myStepsManagement->show();
    }
  }else if (StepsFieldOutputsTreeItem = dynamic_cast<StepsFieldOutputsTree*>(item))
  {
    if (StepsFieldOutputsTreeItem->text(1).toStdString()=="")
    {
      myStepsManagement->show();
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
      this->setWidgetInCmdPanelMarker("BlocksCCXElementType");
    } else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("ExodusRemoveContentsNodeset");
    } else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("ExodusRemoveContentsSideset");
    } else if (MaterialTreeItem = dynamic_cast<MaterialTree*>(item->parent()))
    {
      myMaterialManagement->show();
    } else if (SectionsTreeItem = dynamic_cast<SectionsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXSectionsModfiy");
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
    } else if (LoadsForcesTreeItem = dynamic_cast<LoadsForcesTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("FEAForceModify");
    } else if (LoadsPressuresTreeItem = dynamic_cast<LoadsPressuresTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("FEAPressureModify");
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
    } else if (StepsTreeItem = dynamic_cast<StepsTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXStepsModify");
    } else if (StepsLoadsTreeItem = dynamic_cast<StepsLoadsTree*>(item->parent()))
    {
      myStepsManagement->show();
    } else if (StepsLoadsForcesTreeItem = dynamic_cast<StepsLoadsForcesTree*>(item->parent()))
    {
      myStepsManagement->show();
    } else if (StepsLoadsPressuresTreeItem = dynamic_cast<StepsLoadsPressuresTree*>(item->parent()))
    {
      myStepsManagement->show();
    } else if (StepsBCsTreeItem = dynamic_cast<StepsBCsTree*>(item->parent()))
    {
      myStepsManagement->show();
    } else if (StepsBCsDisplacementsTreeItem = dynamic_cast<StepsBCsDisplacementsTree*>(item->parent()))
    {
      myStepsManagement->show();
    } else if (StepsBCsTemperaturesTreeItem = dynamic_cast<StepsBCsTemperaturesTree*>(item->parent()))
    {
      myStepsManagement->show();
    } else if (StepsHistoryOutputsTreeItem = dynamic_cast<StepsHistoryOutputsTree*>(item->parent()))
    {
      myStepsManagement->show();
    } else if (StepsFieldOutputsTreeItem = dynamic_cast<StepsFieldOutputsTree*>(item->parent()))
    {
      myStepsManagement->show();
    } else if (JobsTreeItem = dynamic_cast<JobsTree*>(item->parent()))
    {
      myJobsMonitor->close();
      myJobsMonitor->show();
      myJobsMonitor->setJob(std::stoi(item->text(1).toStdString()));
      myJobsMonitor->update();
    }
  }
  */
}