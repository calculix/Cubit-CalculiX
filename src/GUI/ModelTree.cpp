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
#include "OrientationsTree.hpp"
#include "DampingTree.hpp"
#include "PhysicalConstantsTree.hpp"
#include "LoadsTree.hpp"
#include "LoadsForcesTree.hpp"
#include "LoadsPressuresTree.hpp"
#include "LoadsHeatfluxesTree.hpp"
#include "LoadsGravityTree.hpp"
#include "LoadsCentrifugalTree.hpp"
#include "LoadsTrajectoryTree.hpp"
#include "LoadsFilmTree.hpp"
#include "LoadsRadiationTree.hpp"
#include "BCsTree.hpp"
#include "BCsDisplacementsTree.hpp"
#include "BCsTemperaturesTree.hpp"
#include "HistoryOutputsTree.hpp"
#include "FieldOutputsTree.hpp"
#include "InitialConditionsTree.hpp"
#include "HBCsTree.hpp"
#include "HBCsDisplacementsTree.hpp"
#include "HBCsTemperaturesTree.hpp"
#include "HBCsManagement.hpp"
#include "StepsTree.hpp"
#include "StepsLoadsTree.hpp"
#include "StepsLoadsForcesTree.hpp"
#include "StepsLoadsPressuresTree.hpp"
#include "StepsLoadsHeatfluxesTree.hpp"
#include "StepsLoadsGravityTree.hpp"
#include "StepsLoadsCentrifugalTree.hpp"
#include "StepsLoadsTrajectoryTree.hpp"
#include "StepsLoadsFilmTree.hpp"
#include "StepsLoadsRadiationTree.hpp"
#include "StepsBCsTree.hpp"
#include "StepsBCsDisplacementsTree.hpp"
#include "StepsBCsTemperaturesTree.hpp"
#include "StepsHistoryOutputsTree.hpp"
#include "StepsFieldOutputsTree.hpp"
#include "StepsManagement.hpp"
#include "JobsTree.hpp"
#include "CustomLinesTree.hpp"
#include "JobsMonitor.hpp"


#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

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
  ccx_iface = new CalculiXCoreInterface();

  gui = Claro::instance();
  nav_model = gui->navigation_model();
  myMaterialManagement = new MaterialManagement();
  myHBCsManagement = new HBCsManagement();
  myStepsManagement = new StepsManagement();
  myJobsMonitor = new JobsMonitor();

  /*
  if (nav_model)
  {
    ccx_iface->log_str("NavigationModel access");
  }
  */

  cmdpanel = dynamic_cast<CommandButtonPanel*>(gui->command_buttons());
  /*
  if (nav_model)
  {
    ccx_iface->log_str("CommandButtonPanel access");
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
    ccx_iface->log_str("connected signals and slots");
  }*/   

  // to get our current right clicked action
  // contextMenuAction[0][0] => Tree
  // contextMenuAction[0][1] => action
  // contextMenuAction[0][2] => ID
  std::vector<int> v = {-1,-1,-1};
  contextMenuAction.push_back(v);
}

ModelTree::~ModelTree()
{
}

void ModelTree::showContextMenu(const QPoint &pos)
{
  //current highest contextMenuAction[0][0] = 46;

  QTreeWidgetItem* item = this->itemAt(pos);
  if (item)
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
    OrientationsTree* OrientationsTreeItem;
    DampingTree* DampingTreeItem;
    PhysicalConstantsTree* PhysicalConstantsTreeItem;
    LoadsTree* LoadsTreeItem;
    LoadsForcesTree* LoadsForcesTreeItem;
    LoadsPressuresTree* LoadsPressuresTreeItem;
    LoadsHeatfluxesTree* LoadsHeatfluxesTreeItem;
    LoadsGravityTree* LoadsGravityTreeItem;
    LoadsCentrifugalTree* LoadsCentrifugalTreeItem;
    LoadsTrajectoryTree* LoadsTrajectoryTreeItem;
    LoadsFilmTree* LoadsFilmTreeItem;
    LoadsRadiationTree* LoadsRadiationTreeItem;
    BCsTree* BCsTreeItem;
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
    StepsBCsTree* StepsBCsTreeItem;
    StepsBCsDisplacementsTree* StepsBCsDisplacementsTreeItem;
    StepsBCsTemperaturesTree* StepsBCsTemperaturesTreeItem;
    StepsHistoryOutputsTree* StepsHistoryOutputsTreeItem;
    StepsFieldOutputsTree* StepsFieldOutputsTreeItem;
    CustomLinesTree* CustomLinesTreeItem;
    JobsTree* JobsTreeItem;
    //QTreeWidgetItem* TreeItem;

    if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item))
    {
      if (BlocksTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Assign CCX Element Type",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);      
        
        contextMenuAction[0][0] = 0;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item))
    {
      if (NodesetTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Nodeset",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 1;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item))
    {
      if (SidesetTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Sideset",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 2;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (MaterialTreeItem = dynamic_cast<MaterialTree*>(item))
    {
      if (MaterialTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Open Material Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 3;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (SectionsTreeItem = dynamic_cast<SectionsTree*>(item))
    {
      if (SectionsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Section",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 4;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (ConstraintsTreeItem = dynamic_cast<ConstraintsTree*>(item))
    {
      if (ConstraintsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Constraint",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 5;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (SurfaceInteractionsTreeItem = dynamic_cast<SurfaceInteractionsTree*>(item))
    {
      if (SurfaceInteractionsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Surface Interaction",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 6;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (ContactPairsTreeItem = dynamic_cast<ContactPairsTree*>(item))
    {
      if (ContactPairsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Contact Pair",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 7;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (AmplitudesTreeItem = dynamic_cast<AmplitudesTree*>(item))
    {
      if (AmplitudesTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Amplitude",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 8;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (OrientationsTreeItem = dynamic_cast<OrientationsTree*>(item))
    {
      if (OrientationsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Orientation",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Orientations", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 36;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (DampingTreeItem = dynamic_cast<DampingTree*>(item))
    {
      if (DampingTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Modify Damping",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 41;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (PhysicalConstantsTreeItem = dynamic_cast<PhysicalConstantsTree*>(item))
    {
      if (PhysicalConstantsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Modify PhysicalConstants",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 42;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (LoadsTreeItem = dynamic_cast<LoadsTree*>(item)) 
    {
      if (LoadsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Draw Loads",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 37;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (LoadsForcesTreeItem = dynamic_cast<LoadsForcesTree*>(item))
    {
      if (LoadsForcesTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Force",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Forces", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 9;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (LoadsPressuresTreeItem = dynamic_cast<LoadsPressuresTree*>(item))
    {
      if (LoadsPressuresTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Pressure",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Pressures", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 10;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (LoadsHeatfluxesTreeItem = dynamic_cast<LoadsHeatfluxesTree*>(item))
    {
      if (LoadsHeatfluxesTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Heatflux",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Heatfluxes", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 11;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (LoadsGravityTreeItem = dynamic_cast<LoadsGravityTree*>(item))
    {
      if (LoadsGravityTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Gravity",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Gravities", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 12;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (LoadsCentrifugalTreeItem = dynamic_cast<LoadsCentrifugalTree*>(item))
    {
      if (LoadsCentrifugalTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Centrifugal",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Centrifugals", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 13;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (LoadsTrajectoryTreeItem = dynamic_cast<LoadsTrajectoryTree*>(item))
    {
      if (LoadsTrajectoryTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Trajectory",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Trajectories", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 39;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (LoadsFilmTreeItem = dynamic_cast<LoadsFilmTree*>(item))
    {
      if (LoadsFilmTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Film",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Films", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 43;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (LoadsRadiationTreeItem = dynamic_cast<LoadsRadiationTree*>(item))
    {
      if (LoadsRadiationTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Radiation",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Radiations", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 44;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (BCsTreeItem = dynamic_cast<BCsTree*>(item)) 
    {
      if (BCsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Draw Bcs",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 38;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (BCsDisplacementsTreeItem = dynamic_cast<BCsDisplacementsTree*>(item))
    {
      if (BCsDisplacementsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Displacement",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Displacements", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 14;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (BCsTemperaturesTreeItem = dynamic_cast<BCsTemperaturesTree*>(item))
    {
      if (BCsTemperaturesTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Temperature",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Temperatures", this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 15;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (HistoryOutputsTreeItem = dynamic_cast<HistoryOutputsTree*>(item))
    {
      if (HistoryOutputsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create History Output",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 16;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (FieldOutputsTreeItem = dynamic_cast<FieldOutputsTree*>(item))
    {
      if (FieldOutputsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Field Output",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 17;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (InitialConditionsTreeItem = dynamic_cast<InitialConditionsTree*>(item))
    {
      if (InitialConditionsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Initial Condition",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 18;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (HBCsTreeItem = dynamic_cast<HBCsTree*>(item))
    {
      if (HBCsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("homogeneous BCs Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 19;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (HBCsDisplacementsTreeItem = dynamic_cast<HBCsDisplacementsTree*>(item))
    {
      if (HBCsDisplacementsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("homogeneous BCs Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 20;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (HBCsTemperaturesTreeItem = dynamic_cast<HBCsTemperaturesTree*>(item))
    {
      if (HBCsTemperaturesTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("homogeneous BCs Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 21;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsTreeItem = dynamic_cast<StepsTree*>(item))
    {
      if (StepsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Step",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Steps",this); //needs function Clemens
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 22;
        contextMenu.exec(mapToGlobal(pos));
      }
    }
    else if (StepsLoadsTreeItem = dynamic_cast<StepsLoadsTree*>(item))
    {
      if (StepsLoadsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Loads",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 23;
        contextMenuAction[0][2] = std::stoi(item->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsLoadsForcesTreeItem = dynamic_cast<StepsLoadsForcesTree*>(item))
    {
      if (StepsLoadsForcesTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Forces",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 24;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsLoadsPressuresTreeItem = dynamic_cast<StepsLoadsPressuresTree*>(item))
    {
      if (StepsLoadsPressuresTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Pressures",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 25;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsLoadsHeatfluxesTreeItem = dynamic_cast<StepsLoadsHeatfluxesTree*>(item))
    {
      if (StepsLoadsHeatfluxesTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Heatfluxes",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 26;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsLoadsGravityTreeItem = dynamic_cast<StepsLoadsGravityTree*>(item))
    {
      if (StepsLoadsGravityTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Gravities",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 27;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsLoadsCentrifugalTreeItem = dynamic_cast<StepsLoadsCentrifugalTree*>(item))
    {
      if (StepsLoadsCentrifugalTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Centrifugals",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 28;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsLoadsTrajectoryTreeItem = dynamic_cast<StepsLoadsTrajectoryTree*>(item))
    {
      if (StepsLoadsTrajectoryTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Trajectories",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 40;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsLoadsFilmTreeItem = dynamic_cast<StepsLoadsFilmTree*>(item))
    {
      if (StepsLoadsFilmTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Films",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 45;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsLoadsRadiationTreeItem = dynamic_cast<StepsLoadsRadiationTree*>(item))
    {
      if (StepsLoadsRadiationTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Radiations",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 46;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsBCsTreeItem = dynamic_cast<StepsBCsTree*>(item))
    {
      if (StepsBCsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Draw BCs",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 29;
        contextMenuAction[0][2] = std::stoi(item->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsBCsDisplacementsTreeItem = dynamic_cast<StepsBCsDisplacementsTree*>(item))
    {
      if (StepsBCsDisplacementsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Displacements",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 30;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsBCsTemperaturesTreeItem = dynamic_cast<StepsBCsTemperaturesTree*>(item))
    {
      if (StepsBCsTemperaturesTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action1);
        QAction action2("Draw Temperatures",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 31;
        contextMenuAction[0][2] = std::stoi(item->parent()->parent()->text(1).toStdString()); //Step id
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsHistoryOutputsTreeItem = dynamic_cast<StepsHistoryOutputsTree*>(item))
    {
      if (StepsHistoryOutputsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 32;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (StepsFieldOutputsTreeItem = dynamic_cast<StepsFieldOutputsTree*>(item))
    {
      if (StepsFieldOutputsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 33;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (CustomLinesTreeItem = dynamic_cast<CustomLinesTree*>(item))
    {
      if (CustomLinesTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Custom Line",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 34;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else if (JobsTreeItem = dynamic_cast<JobsTree*>(item))
    {
      if (JobsTreeItem->text(1).toStdString()=="")
      { 
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Job",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 35;
        contextMenu.exec(mapToGlobal(pos));
      }
    }else 
    {
      if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
      {
        //ccx_iface->log_str("right Click - Block ID " + item->text(1).toStdString());
        QMenu contextMenu("Context Menu",this);
        QAction action1("Assign CCX Element Type",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 0;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Nodeset",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Nodeset",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Nodeset",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 1;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Sideset",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Sideset",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Sideset",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 2;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (MaterialTreeItem = dynamic_cast<MaterialTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Open Material Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 3;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (SectionsTreeItem = dynamic_cast<SectionsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Section",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Section",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Section",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 4;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (ConstraintsTreeItem = dynamic_cast<ConstraintsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Constraint",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Constraint",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Constraint",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 5;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (SurfaceInteractionsTreeItem = dynamic_cast<SurfaceInteractionsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Surface Interaction",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Surface Interaction",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Surface Interaction",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 6;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (ContactPairsTreeItem = dynamic_cast<ContactPairsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Contact Pair",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Contact Pair",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Contact Pair",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 7;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (AmplitudesTreeItem = dynamic_cast<AmplitudesTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Amplitude",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Amplitude",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Amplitude",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 8;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (OrientationsTreeItem = dynamic_cast<OrientationsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Orientation",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Orientation",this); //action 2 is "Draw Orientation All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Orientation",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);

        contextMenuAction[0][0] = 36;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (DampingTreeItem = dynamic_cast<DampingTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Modify Damping",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Delete Damping",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 41;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (PhysicalConstantsTreeItem = dynamic_cast<PhysicalConstantsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Modify PhysicalConstants",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Delete PhysicalConstants",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);

        contextMenuAction[0][0] = 42;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (LoadsForcesTreeItem = dynamic_cast<LoadsForcesTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Force",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Force",this); //action 2 is "Draw Force All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("CCX Modify Force",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Delete Force",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action6("Draw Force",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 9;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (LoadsPressuresTreeItem = dynamic_cast<LoadsPressuresTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Pressure",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Pressure",this); //action 2 is "Draw Pressure All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("CCX Modify Pressure",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Delete Pressure",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action6("Draw Pressure",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 10;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (LoadsHeatfluxesTreeItem = dynamic_cast<LoadsHeatfluxesTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Heatflux",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Heatflux",this); //action 2 is "Draw Heatflux All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("CCX Modify Heatflux",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Delete Heatflux",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action6("Draw Heatflux",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 11;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (LoadsGravityTreeItem = dynamic_cast<LoadsGravityTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Gravity",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Gravity",this); //action 2 is "Draw Gravity All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Gravity",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Draw Gravity",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);

        contextMenuAction[0][0] = 12;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (LoadsCentrifugalTreeItem = dynamic_cast<LoadsCentrifugalTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Centrifugal",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Centrifugal",this); //action 2 is "Draw Centrifugal All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Centrifugal",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Draw Centrifugal",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);

        contextMenuAction[0][0] = 13;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (LoadsTrajectoryTreeItem = dynamic_cast<LoadsTrajectoryTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Trajectory",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Trajectory",this); //action 2 is "Draw Trajectory All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Trajectory",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Draw Trajectory",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);

        contextMenuAction[0][0] = 39;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (LoadsFilmTreeItem = dynamic_cast<LoadsFilmTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Film",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Film",this); //action 2 is "Draw Film All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Film",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Draw Film",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);

        contextMenuAction[0][0] = 43;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (LoadsRadiationTreeItem = dynamic_cast<LoadsRadiationTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Radiation",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Radiation",this); //action 2 is "Draw Radiation All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Radiation",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Draw Radiation",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);

        contextMenuAction[0][0] = 44;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (BCsDisplacementsTreeItem = dynamic_cast<BCsDisplacementsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Displacement",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Displacement",this); //action 2 is "Draw Displacement All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("CCX Modify Displacement",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Delete Displacement",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action6("Draw Displacement",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 14;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (BCsTemperaturesTreeItem = dynamic_cast<BCsTemperaturesTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Temperature",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action3("Modify Temperature",this); //action 2 is "Draw Temperature All"
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("CCX Modify Temperature",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Delete Temperature",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action6("Draw Temperature",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 15;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (HistoryOutputsTreeItem = dynamic_cast<HistoryOutputsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create History Output",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify History Output",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete History Output",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 16;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (FieldOutputsTreeItem = dynamic_cast<FieldOutputsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Field Output",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Field Output",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Field Output",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 17;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (InitialConditionsTreeItem = dynamic_cast<InitialConditionsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Initial Condition",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Initial Condition",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Initial Condition",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 18;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (HBCsTreeItem = dynamic_cast<HBCsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("homogeneous BCs Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 19;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (HBCsDisplacementsTreeItem = dynamic_cast<HBCsDisplacementsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Displacement",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Displacement",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("CCX Modify Displacement",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Displacement",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("homogeneous BCs Management",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);

        contextMenuAction[0][0] = 20;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (HBCsTemperaturesTreeItem = dynamic_cast<HBCsTemperaturesTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Temperature",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Temperature",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("CCX Modify Temperature",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Temperature",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("homogeneous BCs Management",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);

        contextMenuAction[0][0] = 21;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsTreeItem = dynamic_cast<StepsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Step",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Step",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Step",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Draw Step",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action4);

        contextMenuAction[0][0] = 22;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsLoadsTreeItem = dynamic_cast<StepsLoadsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 23;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsLoadsForcesTreeItem = dynamic_cast<StepsLoadsForcesTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Force",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Force",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("CCX Modify Force",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Force",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Steps Management",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action7("Draw Force",this);
        connect(&action7, SIGNAL(triggered()),this,SLOT(ContextMenuAction7()));
        contextMenu.addAction(&action7);

        contextMenuAction[0][0] = 24;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsLoadsPressuresTreeItem = dynamic_cast<StepsLoadsPressuresTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Pressure",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Pressure",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("CCX Modify Pressure",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Pressure",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Steps Management",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action7("Draw Pressure",this);
        connect(&action7, SIGNAL(triggered()),this,SLOT(ContextMenuAction7()));
        contextMenu.addAction(&action7);

        contextMenuAction[0][0] = 25;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsLoadsHeatfluxesTreeItem = dynamic_cast<StepsLoadsHeatfluxesTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Heatflux",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Heatflux",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("CCX Modify Heatflux",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Heatflux",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Steps Management",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action7("Draw Heatflux",this);
        connect(&action7, SIGNAL(triggered()),this,SLOT(ContextMenuAction7()));
        contextMenu.addAction(&action7);

        contextMenuAction[0][0] = 26;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (StepsLoadsGravityTreeItem = dynamic_cast<StepsLoadsGravityTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Gravity",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Gravity",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Gravity",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Steps Management",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action6("Draw Gravity",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 27;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (StepsLoadsCentrifugalTreeItem = dynamic_cast<StepsLoadsCentrifugalTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Centrifugal",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Centrifugal",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Centrifugal",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Steps Management",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action6("Draw Centrifugal",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 28;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (StepsLoadsTrajectoryTreeItem = dynamic_cast<StepsLoadsTrajectoryTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Trajectory",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Trajectory",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Trajectory",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Steps Management",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action6("Draw Trajectory",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 40;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (StepsLoadsFilmTreeItem = dynamic_cast<StepsLoadsFilmTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Film",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Film",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Film",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Steps Management",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action6("Draw Film",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 45;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      } else if (StepsLoadsRadiationTreeItem = dynamic_cast<StepsLoadsRadiationTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Radiation",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Radiation",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Radiation",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Steps Management",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action6("Draw Radiation",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 46;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsBCsTreeItem = dynamic_cast<StepsBCsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Steps Management",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);

        contextMenuAction[0][0] = 29;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsBCsDisplacementsTreeItem = dynamic_cast<StepsBCsDisplacementsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Displacement",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Displacement",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("CCX Modify Displacement",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Displacement",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Steps Management",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action7("Draw Displacement",this);
        connect(&action7, SIGNAL(triggered()),this,SLOT(ContextMenuAction7()));
        contextMenu.addAction(&action7);

        contextMenuAction[0][0] = 30;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsBCsTemperaturesTreeItem = dynamic_cast<StepsBCsTemperaturesTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Temperature",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Temperature",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("CCX Modify Temperature",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Delete Temperature",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Steps Management",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action7("Draw Temperature",this);
        connect(&action7, SIGNAL(triggered()),this,SLOT(ContextMenuAction7()));
        contextMenu.addAction(&action7);

        contextMenuAction[0][0] = 31;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsHistoryOutputsTreeItem = dynamic_cast<StepsHistoryOutputsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create History Output",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify History Output",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete History Output",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Steps Management",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);

        contextMenuAction[0][0] = 32;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }else if (StepsFieldOutputsTreeItem = dynamic_cast<StepsFieldOutputsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Field Output",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Field Output",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Field Output",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Steps Management",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);

        contextMenuAction[0][0] = 33;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }
      else if (CustomLinesTreeItem = dynamic_cast<CustomLinesTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Custom Line",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Custom Line",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Custom Line",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);

        contextMenuAction[0][0] = 34;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }
      else if (JobsTreeItem = dynamic_cast<JobsTree*>(item->parent()))
      {
        QMenu contextMenu("Context Menu",this);
        QAction action1("Create Job",this);
        connect(&action1, SIGNAL(triggered()),this,SLOT(ContextMenuAction1()));
        contextMenu.addAction(&action1);
        QAction action2("Modify Job",this);
        connect(&action2, SIGNAL(triggered()),this,SLOT(ContextMenuAction2()));
        contextMenu.addAction(&action2);
        QAction action3("Delete Job",this);
        connect(&action3, SIGNAL(triggered()),this,SLOT(ContextMenuAction3()));
        contextMenu.addAction(&action3);
        QAction action4("Job Monitor",this);
        connect(&action4, SIGNAL(triggered()),this,SLOT(ContextMenuAction4()));
        contextMenu.addAction(&action4);
        QAction action5("Result CGX",this);
        connect(&action5, SIGNAL(triggered()),this,SLOT(ContextMenuAction5()));
        contextMenu.addAction(&action5);
        QAction action6("Result ParaView",this);
        connect(&action6, SIGNAL(triggered()),this,SLOT(ContextMenuAction6()));
        contextMenu.addAction(&action6);

        contextMenuAction[0][0] = 35;
        contextMenuAction[0][2] = std::stoi(item->text(1).toStdString());
        contextMenu.exec(mapToGlobal(pos));
      }
    }
  }
}

void ModelTree::ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column)
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
      this->setWidgetInCmdPanelMarker("FEAForceCreate");
    }
  }else if (LoadsPressuresTreeItem = dynamic_cast<LoadsPressuresTree*>(item))
  {
    if (LoadsPressuresTreeItem->text(1).toStdString()=="")
    {
      this->setWidgetInCmdPanelMarker("FEAPressureCreate");
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
      this->setWidgetInCmdPanelMarker("CCXLoadsTrajectoryCreate");
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
      this->setWidgetInCmdPanelMarker("FEAForceModify");
    } else if (LoadsPressuresTreeItem = dynamic_cast<LoadsPressuresTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("FEAPressureModify");
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
      this->setWidgetInCmdPanelMarker("CCXLoadsTrajectoryModify");
    } else if (LoadsFilmTreeItem = dynamic_cast<LoadsFilmTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsFilmModify");
    } else if (LoadsRadiationTreeItem = dynamic_cast<LoadsRadiationTree*>(item->parent()))
    {
      this->setWidgetInCmdPanelMarker("CCXLoadsRadiationModify");
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

void ModelTree::execContextMenuAction(){
  if (contextMenuAction[0][0]!=-1 && contextMenuAction[0][1]!=-1)
  {
    if (contextMenuAction[0][0]==0) //Blocktree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXBlocksModify");
      }
    }else if (contextMenuAction[0][0]==1) //NodesetTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("ExodusCreateNodeset");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("ExodusRemoveContentsNodeset");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("ExodusDeleteNodeset");
      }  
    }else if (contextMenuAction[0][0]==2) //SidesetTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("ExodusCreateSideset");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("ExodusRemoveContentsSideset");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("ExodusDeleteSideset");
      }  
    }else if (contextMenuAction[0][0]==3) //MaterialTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        myMaterialManagement->show();
      }  
    }else if (contextMenuAction[0][0]==4) //SectionsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXSectionsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXSectionsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXSectionsDelete");
      }  
    }else if (contextMenuAction[0][0]==5) //ConstraintsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXConstraintsCreateRigidBody");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXConstraintsModifyRigidBody");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXConstraintsDelete");
      }  
    }else if (contextMenuAction[0][0]==6) //SurfaceInteractionsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXSurfaceInteractionsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXSurfaceInteractionsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXSurfaceInteractionsDelete");
      }  
    }else if (contextMenuAction[0][0]==7) //ContactPairsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXContactPairsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXContactPairsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXContactPairsDelete");
      }  
    }else if (contextMenuAction[0][0]==8) //AmplitudesTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXAmplitudesCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXAmplitudesModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXAmplitudesDelete");
      }
    }else if (contextMenuAction[0][0]==36) //OrientationsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXOrientationsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXOrientationsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXOrientationsDelete");
      }
    }else if (contextMenuAction[0][0]==41) //DampingTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXDampingModify");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXDampingDelete");
      }
    }else if (contextMenuAction[0][0]==42) //PhysicalConstantsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXPhysicalConstantsModify");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXPhysicalConstantsDelete");
      }
    }else if (contextMenuAction[0][0]==37) //LoadsTree
    {
      if(contextMenuAction[0][1]==0) //Action1
      {
        std::string command = "ccx draw load_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==9) //LoadsForcesTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEAForceCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw load_force_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
      else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("FEAForceModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsForcesModify");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        this->setWidgetInCmdPanelMarker("FEAForceDelete");
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw load force " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==10) //LoadsPressuresTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEAPressureCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw load_pressure_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("FEAPressureModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsPressuresModify");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        this->setWidgetInCmdPanelMarker("FEAPressureDelete");
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw load pressure " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==11) //LoadsHeatfluxesTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEAHeatfluxCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw load_heatflux_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("FEAHeatfluxModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsHeatfluxesModify");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        this->setWidgetInCmdPanelMarker("FEAHeatfluxDelete");
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw load heatflux " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==12) //LoadsGravityTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsGravityCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw load_gravity_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsGravityModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsGravityDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw load gravity " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==13) //LoadsCentrifugalTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsCentrifugalCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw load_centrifugal_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsCentrifugalModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsCentrifugalDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw load centrifugal " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==39) //LoadsTrajectoryTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsTrajectoryCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw load_trajectory_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsTrajectoryModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsTrajectoryDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw load trajectory " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==43) //LoadsFilmTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsFilmCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw load_film_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsFilmModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsFilmDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw load film " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==44) //LoadsRadiationTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsRadiationCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw load_radiation_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsRadiationModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsRadiationDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw load radiation " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==38) //BCsTree
    {
      if(contextMenuAction[0][1]==0)
      {
        std::string command = "ccx draw bc_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==14) //BCsDisplacementsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEADisplacementCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw bc_displacement_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("FEADisplacementModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXBCsDisplacementsModify");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        this->setWidgetInCmdPanelMarker("FEADisplacementDelete");
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw bc displacement " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==15) //BCsTemperaturesTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEATemperatureCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw bc_temperature_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("FEATemperatureModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("CCXBCsTemperaturesModify");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        this->setWidgetInCmdPanelMarker("FEATemperatureDelete");
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw bc temperature " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==16) //HistoryOutputsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXHistoryOutputsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXHistoryOutputsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXHistoryOutputsDelete");
      }  
    }else if (contextMenuAction[0][0]==17) //FieldOutputsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXFieldOutputsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXFieldOutputsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXFieldOutputsDelete");
      }  
    }else if (contextMenuAction[0][0]==18) //InitialConditionsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXInitialConditionsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXInitialConditionsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXInitialConditionsDelete");
      }  
    }else if (contextMenuAction[0][0]==19) //HBCsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        myHBCsManagement->show();
      }  
    }else if (contextMenuAction[0][0]==20) //HBCsDisplacementsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEADisplacementCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("FEADisplacementModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXBCsDisplacementsModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("FEADisplacementDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        myHBCsManagement->show();
      }  
    }else if (contextMenuAction[0][0]==21) //HBCsTemperaturesTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEATemperatureCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("FEATemperatureModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXBCsTemperaturesModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("FEATemperatureDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        myHBCsManagement->show();
      }
    }else if (contextMenuAction[0][0]==22) //StepsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXStepsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXStepsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXStepsDelete");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        std::vector<int> step_ids;
        step_ids = ccx_iface->get_steps_ids();
        for (size_t i=0; i < step_ids.size(); i++)
        {
          std::string command = "ccx draw step " + std::to_string(step_ids[i]);
          //CubitInterface::cmd(command.c_str());
          ccx_iface->cmd(command);
        }
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==23) //StepsLoadsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        myStepsManagement->show();
      }else if(contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " load_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==24) //StepsLoadsForcesTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEAForceCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("FEAForceModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsForcesModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("FEAForceDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " load_force_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==6) //Action7
      {
        std::string command = "ccx draw load force " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==25) //StepsLoadsPressuresTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEAPressureCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("FEAPressureModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsPressuresModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("FEAPressureDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " load_pressure_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==6) //Action7
      {
        std::string command = "ccx draw load pressure " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==26) //StepsLoadsHeatfluxesTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEAHeatfluxCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("FEAHeatfluxModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsHeatfluxesModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("FEAHeatfluxDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " load_heatflux_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==6) //Action7
      {
        std::string command = "ccx draw load heatflux " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==27) //StepsLoadsGravityTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsGravityCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsGravityModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsGravityDelete");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " load_gravity_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw load gravity " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==28) //StepsLoadsCentrifugalTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsCentrifugalCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsCentrifugalModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsCentrifugalDelete");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " load_centrifugal_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw load centrifugal " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }   
    }else if (contextMenuAction[0][0]==40) //StepsLoadsTrajectoryTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsTrajectoryCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsTrajectoryModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsTrajectoryDelete");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " load_trajectory_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw load trajectory " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }   
    }else if (contextMenuAction[0][0]==45) //StepsLoadsFilmTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsFilmCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsFilmModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsFilmDelete");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " load_film_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw load film " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }   
    }else if (contextMenuAction[0][0]==46) //StepsLoadsRadiationTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsRadiationCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsRadiationModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXLoadsRadiationDelete");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " load_radiation_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw load radiation " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }   
    }else if (contextMenuAction[0][0]==29) //StepsBCsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        myStepsManagement->show();
      }else if(contextMenuAction[0][1]==1) //Action2
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " bc_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==30) //StepsBCsDisplacementsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEADisplacementCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("FEADisplacementModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXBCsDisplacementsModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("FEADisplacementDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " bc_displacement_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==6) //Action7
      {
        std::string command = "ccx draw bc displacement " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==31) //StepsBCsTemperaturesTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("FEATemperatureCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("FEATemperatureModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXBCsTemperaturesModify");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        this->setWidgetInCmdPanelMarker("FEATemperatureDelete");
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        myStepsManagement->show();
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx draw step " + std::to_string(contextMenuAction[0][2]) + " bc_temperature_all";
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==6) //Action7
      {
        std::string command = "ccx draw bc temperature " + std::to_string(contextMenuAction[0][2]);
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }else if (contextMenuAction[0][0]==32) //StepsHistoryOutputsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXHistoryOutputsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXHistoryOutputsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXHistoryOutputsDelete");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        myStepsManagement->show();
      }
    }else if (contextMenuAction[0][0]==33) //StepsFieldOutputsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXFieldOutputsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXFieldOutputsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXFieldOutputsDelete");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        myStepsManagement->show();
      }  
    }else if (contextMenuAction[0][0]==34) //CustomLinesTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXCustomLinesCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXCustomLinesModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXCustomLinesDelete");
      }  
    }else if (contextMenuAction[0][0]==35) //JobsTree
    {
      if (contextMenuAction[0][1]==0) //Action1
      {
        this->setWidgetInCmdPanelMarker("CCXJobsCreate");
      }else if (contextMenuAction[0][1]==1) //Action2
      {
        this->setWidgetInCmdPanelMarker("CCXJobsModify");
      }else if (contextMenuAction[0][1]==2) //Action3
      {
        this->setWidgetInCmdPanelMarker("CCXJobsDelete");
      }else if (contextMenuAction[0][1]==3) //Action4
      {
        myJobsMonitor->close();
        myJobsMonitor->show();
        myJobsMonitor->setJob(contextMenuAction[0][2]);
        myJobsMonitor->update();
      }else if (contextMenuAction[0][1]==4) //Action5
      {
        std::string command = "ccx result cgx job " + std::to_string(contextMenuAction[0][2]);   
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }else if (contextMenuAction[0][1]==5) //Action6
      {
        std::string command = "ccx result paraview job " + std::to_string(contextMenuAction[0][2]);   
        //CubitInterface::cmd(command.c_str());
        ccx_iface->cmd(command);
      }
    }
  }
  contextMenuAction[0][0]=-1;
  contextMenuAction[0][1]=-1;
  contextMenuAction[0][2]=-1;
}

void ModelTree::ContextMenuAction1(){
  contextMenuAction[0][1]=0;
  this->execContextMenuAction();
}

void ModelTree::ContextMenuAction2(){
  contextMenuAction[0][1]=1;
  this->execContextMenuAction();
}

void ModelTree::ContextMenuAction3(){
  contextMenuAction[0][1]=2;
  this->execContextMenuAction();
}

void ModelTree::ContextMenuAction4(){
  contextMenuAction[0][1]=3;
  this->execContextMenuAction();
}

void ModelTree::ContextMenuAction5(){
  contextMenuAction[0][1]=4;
  this->execContextMenuAction();
}

void ModelTree::ContextMenuAction6(){
  contextMenuAction[0][1]=5;
  this->execContextMenuAction();
}

void ModelTree::ContextMenuAction7(){
  contextMenuAction[0][1]=6;
  this->execContextMenuAction();
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