
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
#include "LoadsHeatfluxesTree.hpp"
#include "LoadsGravityTree.hpp"
#include "LoadsCentrifugalTree.hpp"
#include "BCsTree.hpp"
#include "BCsDisplacementsTree.hpp"
#include "BCsTemperaturesTree.hpp"
#include "HistoryOutputsTree.hpp"
#include "FieldOutputsTree.hpp"
#include "InitialConditionsTree.hpp"
#include "HBCsTree.hpp"
#include "HBCsDisplacementsTree.hpp"
#include "HBCsTemperaturesTree.hpp"
#include "StepsTree.hpp"
#include "StepsLoadsTree.hpp"
#include "StepsLoadsForcesTree.hpp"
#include "StepsLoadsPressuresTree.hpp"
#include "StepsLoadsHeatfluxesTree.hpp"
#include "StepsLoadsGravityTree.hpp"
#include "StepsLoadsCentrifugalTree.hpp"
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
  success = connect(parent, SIGNAL(itemSelectionChanged()),
          this, SLOT(ModelTreeItemSelectionChanged()));        
  /*
  if (success)
  {
    std::string log;
    log = "connected signals and slots\n";
    PRINT_INFO("%s", log.c_str()); 
  }
  */
}

Highlight::~Highlight()
{
}

void Highlight::ModelTreeItemSelectionChanged()
{
  selected = dynamic_cast<ModelTree*>(this->parent())->selectedItems();
  for (size_t i = 0; i < selected.size(); i++)
  {
    ModelTreeItemClicked(selected[i],0);
  }  
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
  LoadsHeatfluxesTree* LoadsHeatfluxesTreeItem;
  LoadsGravityTree* LoadsGravityTreeItem;
  LoadsCentrifugalTree* LoadsCentrifugalTreeItem;
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
  StepsBCsTree* StepsBCsTreeItem;
  StepsBCsDisplacementsTree* StepsBCsDisplacementsTreeItem;
  StepsBCsTemperaturesTree* StepsBCsTemperaturesTreeItem;
  StepsHistoryOutputsTree* StepsHistoryOutputsTreeItem;
  StepsFieldOutputsTree* StepsFieldOutputsTreeItem;  
  JobsTree* JobsTreeItem;

  std::vector<std::vector<std::string>> entities;

  CubitInterface::clear_highlight();
    
  //std::string log;
  //log = "item clicked\n";
  //PRINT_INFO("%s", log.c_str()); 

  if (BlocksTreeItem = dynamic_cast<BlocksTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("block",std::stoi(item->text(1).toStdString()));
  } else if (NodesetTreeItem = dynamic_cast<NodesetTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("nodeset",std::stoi(item->text(1).toStdString()));
  } else if (SidesetTreeItem = dynamic_cast<SidesetTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("sideset",std::stoi(item->text(1).toStdString()));
  } else if (MaterialTreeItem = dynamic_cast<MaterialTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("material",std::stoi(item->text(1).toStdString()));
  } else if (SectionsTreeItem = dynamic_cast<SectionsTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("section",std::stoi(item->text(1).toStdString()));
  } else if (ConstraintsTreeItem = dynamic_cast<ConstraintsTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("constraint",std::stoi(item->text(1).toStdString()));
  } else if (SurfaceInteractionsTreeItem = dynamic_cast<SurfaceInteractionsTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("surfaceinteraction",std::stoi(item->text(1).toStdString()));
  } else if (ContactPairsTreeItem = dynamic_cast<ContactPairsTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("contactpair",std::stoi(item->text(1).toStdString()));
  } else if (AmplitudesTreeItem = dynamic_cast<AmplitudesTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("amplitude",std::stoi(item->text(1).toStdString()));
  } else if (LoadsForcesTreeItem = dynamic_cast<LoadsForcesTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadsforce",std::stoi(item->text(1).toStdString()));
  } else if (LoadsPressuresTreeItem = dynamic_cast<LoadsPressuresTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadspressure",std::stoi(item->text(1).toStdString()));
  } else if (LoadsHeatfluxesTreeItem = dynamic_cast<LoadsHeatfluxesTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadsheatflux",std::stoi(item->text(1).toStdString()));
  } else if (LoadsGravityTreeItem = dynamic_cast<LoadsGravityTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadsgravity",std::stoi(item->text(1).toStdString()));
  } else if (LoadsCentrifugalTreeItem = dynamic_cast<LoadsCentrifugalTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadscentrifugal",std::stoi(item->text(1).toStdString()));
  } else if (BCsDisplacementsTreeItem = dynamic_cast<BCsDisplacementsTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("bcsdisplacement",std::stoi(item->text(1).toStdString()));
  } else if (BCsTemperaturesTreeItem = dynamic_cast<BCsTemperaturesTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("bcstemperature",std::stoi(item->text(1).toStdString()));
  } else if (HistoryOutputsTreeItem = dynamic_cast<HistoryOutputsTree*>(item->parent()))
  {

  } else if (FieldOutputsTreeItem = dynamic_cast<FieldOutputsTree*>(item->parent()))
  {

  } else if (InitialConditionsTreeItem = dynamic_cast<InitialConditionsTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("initialcondition",std::stoi(item->text(1).toStdString()));  
  } else if (HBCsTreeItem = dynamic_cast<HBCsTree*>(item->parent()))
  {
    
  } else if (HBCsDisplacementsTreeItem = dynamic_cast<HBCsDisplacementsTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("hbcsdisplacement",std::stoi(item->text(1).toStdString()));
  } else if (HBCsTemperaturesTreeItem = dynamic_cast<HBCsTemperaturesTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("hbcstemperature",std::stoi(item->text(1).toStdString()));
  } else if (StepsTreeItem = dynamic_cast<StepsTree*>(item->parent()))
  {
      
  } else if (StepsLoadsTreeItem = dynamic_cast<StepsLoadsTree*>(item->parent()))
  {
      
  } else if (StepsLoadsForcesTreeItem = dynamic_cast<StepsLoadsForcesTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadsforce",std::stoi(item->text(1).toStdString()));  
  } else if (StepsLoadsPressuresTreeItem = dynamic_cast<StepsLoadsPressuresTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadspressure",std::stoi(item->text(1).toStdString()));  
  } else if (StepsLoadsHeatfluxesTreeItem = dynamic_cast<StepsLoadsHeatfluxesTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadsheatflux",std::stoi(item->text(1).toStdString()));  
  } else if (StepsLoadsGravityTreeItem = dynamic_cast<StepsLoadsGravityTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadsgravity",std::stoi(item->text(1).toStdString()));  
  } else if (StepsLoadsCentrifugalTreeItem = dynamic_cast<StepsLoadsCentrifugalTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("loadscentrifugal",std::stoi(item->text(1).toStdString()));  
  } else if (StepsBCsTreeItem = dynamic_cast<StepsBCsTree*>(item->parent()))
  {
    
  } else if (StepsBCsDisplacementsTreeItem = dynamic_cast<StepsBCsDisplacementsTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("bcsdisplacement",std::stoi(item->text(1).toStdString()));
  } else if (StepsBCsTemperaturesTreeItem = dynamic_cast<StepsBCsTemperaturesTree*>(item->parent()))
  {
    entities = ccx_iface->get_entities("bcstemperature",std::stoi(item->text(1).toStdString()));
  } else if (StepsHistoryOutputsTreeItem = dynamic_cast<StepsHistoryOutputsTree*>(item->parent()))
  {
      
  } else if (StepsFieldOutputsTreeItem = dynamic_cast<StepsFieldOutputsTree*>(item->parent()))
  {
      
  } else if (JobsTreeItem = dynamic_cast<JobsTree*>(item->parent()))
  {
      
  }

  CubitInterface::flush_graphics();

  for (size_t i = 0; i < entities.size(); i++)
  {
    std::string cmd = "highlight " + entities[i][0] + " " + entities[i][1];
    CubitInterface::silent_cmd(cmd.c_str());
  }
}