#include "cmdPanelManager.hpp"
#include "Claro.hpp"
#include "NavigationModel.hpp"
#include "NavigationNode.hpp"
#include "MyCmdWidgetFactory.hpp"
#include <iostream>
#include "CalculiXCoreInterface.hpp"

cmdPanelManager::cmdPanelManager() :
  isInitialized(false),
  myFactory(NULL)
{
  ccx_iface = new CalculiXCoreInterface();
}

cmdPanelManager::~cmdPanelManager()
{
  if(myFactory)
    delete myFactory;
}

void cmdPanelManager::initialize()
{
  if(isInitialized)
    return;

  // Set this variable to choose how to initialize your navigation nodes.
  bool use_xml = false;

  if(use_xml)
    initialize_from_xml();
  else
    initialize_from_code();

  // Once nodes are created and initialized, we want to associate them
  // with command panels.
  associate_panels_with_nodes();
  isInitialized = true;
}

void cmdPanelManager::clear()
{
  if(myFactory)
  {
    // Clean up my widgets
    myFactory->clear();

    // Remove the factory reference from my navigation nodes
    QStringList my_markers;
    my_markers.push_back("CCXBlocksModify");
    my_markers.push_back("CCXSectionsCreateSolid");
    my_markers.push_back("CCXSectionsCreateShell");
    my_markers.push_back("CCXSectionsCreateBeam");
    my_markers.push_back("CCXSectionsCreateMembrane");
    my_markers.push_back("CCXSectionsModifySolid");
    my_markers.push_back("CCXSectionsModifyShell");
    my_markers.push_back("CCXSectionsModifyBeam");
    my_markers.push_back("CCXSectionsModifyMembrane");
    my_markers.push_back("CCXSectionsDelete");
    my_markers.push_back("CCXConstraintsCreateRigidBody");
    my_markers.push_back("CCXConstraintsCreateTie");
    my_markers.push_back("CCXConstraintsModifyRigidBody");
    my_markers.push_back("CCXConstraintsModifyTie");
    my_markers.push_back("CCXConstraintsDelete");
    my_markers.push_back("CCXSurfaceInteractionsCreate");
    my_markers.push_back("CCXSurfaceInteractionsModify");
    my_markers.push_back("CCXSurfaceInteractionsDelete");
    my_markers.push_back("CCXContactPairsCreate");
    my_markers.push_back("CCXContactPairsModify");
    my_markers.push_back("CCXContactPairsDelete");
    my_markers.push_back("CCXAmplitudesCreate");
    my_markers.push_back("CCXAmplitudesModify");
    my_markers.push_back("CCXAmplitudesDelete");
    my_markers.push_back("CCXLoadsForcesModify");
    my_markers.push_back("CCXLoadsPressuresModify");
    my_markers.push_back("CCXBCsDisplacementsModify");
    my_markers.push_back("CCXBCsTemperaturesModify");
    my_markers.push_back("CCXHistoryOutputsCreate");
    my_markers.push_back("CCXHistoryOutputsModifyNode");
    my_markers.push_back("CCXHistoryOutputsModifyElement");
    my_markers.push_back("CCXHistoryOutputsModifyContact");
    my_markers.push_back("CCXHistoryOutputsDelete");
    my_markers.push_back("CCXFieldOutputsCreate");
    my_markers.push_back("CCXFieldOutputsModifyNode");
    my_markers.push_back("CCXFieldOutputsModifyElement");
    my_markers.push_back("CCXFieldOutputsModifyContact");
    my_markers.push_back("CCXFieldOutputsDelete");
    my_markers.push_back("CCXInitialConditionsCreate");
    my_markers.push_back("CCXInitialConditionsModify");
    my_markers.push_back("CCXInitialConditionsDelete");
    my_markers.push_back("CCXStepsCreate");
    my_markers.push_back("CCXStepsModifyParameter");
    my_markers.push_back("CCXStepsModifyStatic");
    my_markers.push_back("CCXStepsModifyFrequency");
    my_markers.push_back("CCXStepsModifyBuckle");
    my_markers.push_back("CCXStepsModifyHeatTransfer");
    my_markers.push_back("CCXStepsModifyCoupledTemperatureDisplacement");
    my_markers.push_back("CCXStepsModifyUncoupledTemperatureDisplacement");
    my_markers.push_back("CCXStepsDelete");
    my_markers.push_back("CCXJobsCreate");
    my_markers.push_back("CCXJobsModify");
    my_markers.push_back("CCXJobsDelete");
    my_markers.push_back("CCXJobsActions");

    // For each marker, we want to get the navigation node and assign the node
    // to use this factory to get widgets as needed.
    NavigationModel* model = Claro::instance()->navigation_model();
    for(int i = 0; i < my_markers.size(); i++)
    {
      NavigationNode* node = model->getMarkedNode(my_markers[i]);
      if(node)
        node->setFactory(0);
    }
  }

  isInitialized = false;
}

void cmdPanelManager::initialize_from_xml()
{
  // If you are creating your command panel navigation buttons using xml files,
  // there is nothing you need to do to create navigation nodes (it is done
  // automatically). For an example, see MyNavigation.xml and copy it to the
  // Cubit/bin/xml folder.

  return;
}

void cmdPanelManager::initialize_from_code()
{
  QIcon *NodeIconPointer;
  // This example shows how to create the command panel navigation buttons (nodes)
  // using the NavigationModel directly.
  NavigationModel* model = Claro::instance()->navigation_model();
  if(!model)
    return;
  
  NavigationNode* root_node;
  NavigationNode* node;

  // CCX Root Node
  root_node = model->addNode("CCX", model->getRoot());
  root_node->setTitle("CCX");
  root_node->setIcon(ccx_iface->getIcon("CCX"));
  model->setNodeMarker(root_node, "CCX"); 
  model->updateNode(root_node); 

  //##############################
  // add BlocksTree Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Blocks", root_node);
  node->setTitle("Blocks");
  model->setNodeMarker(node, "CCXBlocks");
  node->setIcon(ccx_iface->getIcon("CCXBlocks"));

  root_node = model->getMarkedNode("CCXBlocks");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXBlocksModify");
  node->setIcon(ccx_iface->getIcon("CCXBlocksModify"));

  //##############################
  // add Materials and Section Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Sections", root_node);
  node->setTitle("Sections");
  model->setNodeMarker(node, "CCXSections");
  node->setIcon(ccx_iface->getIcon("CCXSections"));
  root_node = model->getMarkedNode("CCXSections");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXSectionsCreate");
  node->setIcon(ccx_iface->getIcon("CCXSectionsCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXSectionsModify");
  node->setIcon(ccx_iface->getIcon("CCXSectionsModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXSectionsDelete");
  node->setIcon(ccx_iface->getIcon("CCXSectionsDelete"));
  //##
  root_node = model->getMarkedNode("CCXSectionsCreate");
  node = model->addNode("Solid Section", root_node);
  model->setNodeMarker(node, "CCXSectionsCreateSolid");
  node->setIcon(ccx_iface->getIcon("CCXSectionsCreateSolid"));
  node = model->addNode("Shell Section", root_node);
  model->setNodeMarker(node, "CCXSectionsCreateShell");
  node->setIcon(ccx_iface->getIcon("CCXSectionsCreateShell"));
  node = model->addNode("Beam Section", root_node);
  model->setNodeMarker(node, "CCXSectionsCreateBeam");
  node->setIcon(ccx_iface->getIcon("CCXSectionsCreateBeam"));
  node = model->addNode("Membrane Section", root_node);
  model->setNodeMarker(node, "CCXSectionsCreateMembrane");
  node->setIcon(ccx_iface->getIcon("CCXSectionsCreateMembrane"));
  //##
  root_node = model->getMarkedNode("CCXSectionsModify");
  node = model->addNode("Solid Section", root_node);
  model->setNodeMarker(node, "CCXSectionsModifySolid");
  node->setIcon(ccx_iface->getIcon("CCXSectionsModifySolid"));
  node = model->addNode("Shell Section", root_node);
  model->setNodeMarker(node, "CCXSectionsModifyShell");
  node->setIcon(ccx_iface->getIcon("CCXSectionsModifyShell"));
  node = model->addNode("Beam Section", root_node);
  model->setNodeMarker(node, "CCXSectionsModifyBeam");
  node->setIcon(ccx_iface->getIcon("CCXSectionsModifyBeam"));
  node = model->addNode("Membrane Section", root_node);
  model->setNodeMarker(node, "CCXSectionsModifyMembrane");
  node->setIcon(ccx_iface->getIcon("CCXSectionsModifyMembrane"));

  //##############################
  // add Constraints Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Constraints", root_node);
  node->setTitle("Constraints");
  model->setNodeMarker(node, "CCXConstraints");
  root_node = model->getMarkedNode("CCXConstraints");
  node->setIcon(ccx_iface->getIcon("CCXConstraints"));
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXConstraintsCreate");
  node->setIcon(ccx_iface->getIcon("CCXConstraintsCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXConstraintsModify");
  node->setIcon(ccx_iface->getIcon("CCXConstraintsModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXConstraintsDelete");
  node->setIcon(ccx_iface->getIcon("CCXConstraintsDelete"));

  root_node = model->getMarkedNode("CCXConstraintsCreate");
  node = model->addNode("Rigid Body", root_node);
  model->setNodeMarker(node, "CCXConstraintsCreateRigidBody");
  node->setIcon(ccx_iface->getIcon("CCXConstraintsCreateRigidBody"));
  node = model->addNode("Tie", root_node);
  model->setNodeMarker(node, "CCXConstraintsCreateTie");
  node->setIcon(ccx_iface->getIcon("CCXConstraintsCreateTie"));
  root_node = model->getMarkedNode("CCXConstraintsModify");
  node = model->addNode("Rigid Body", root_node);
  model->setNodeMarker(node, "CCXConstraintsModifyRigidBody");
  node->setIcon(ccx_iface->getIcon("CCXConstraintsModifyRigidBody"));
  node = model->addNode("Tie", root_node);
  model->setNodeMarker(node, "CCXConstraintsModifyTie");
  node->setIcon(ccx_iface->getIcon("CCXConstraintsModifyTie"));
  /*root_node = model->getMarkedNode("CCXConstraintsDelete");
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXConstraintsDelete");
  node->setIcon(ccx_iface->getIcon("CCXConstraintsDelete"));*/

  //##############################
  // add SurfaceInteractions Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Surface Interactions", root_node);
  node->setTitle("Surface Interactions");
  model->setNodeMarker(node, "CCXSurfaceInteractions");
  node->setIcon(ccx_iface->getIcon("CCXSurfaceInteractions"));
  root_node = model->getMarkedNode("CCXSurfaceInteractions");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXSurfaceInteractionsCreate");
  node->setIcon(ccx_iface->getIcon("CCXSurfaceInteractionsCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXSurfaceInteractionsModify");
  node->setIcon(ccx_iface->getIcon("CCXSurfaceInteractionsModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXSurfaceInteractionsDelete");
  node->setIcon(ccx_iface->getIcon("CCXSurfaceInteractionsDelete"));

  //##############################
  // add ContactPairs Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Contact Pairs", root_node);
  node->setTitle("Contact Pairs");
  model->setNodeMarker(node, "CCXContactPairs");
  node->setIcon(ccx_iface->getIcon("CCXContactPairs"));
  root_node = model->getMarkedNode("CCXContactPairs");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXContactPairsCreate");
  node->setIcon(ccx_iface->getIcon("CCXContactPairsCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXContactPairsModify");
  node->setIcon(ccx_iface->getIcon("CCXContactPairsModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXContactPairsDelete");
  node->setIcon(ccx_iface->getIcon("CCXContactPairsDelete"));

  //##############################
  // add Amplitudes Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Amplitudes", root_node);
  node->setTitle("Amplitudes");
  model->setNodeMarker(node, "CCXAmplitudes");
  node->setIcon(ccx_iface->getIcon("CCXAmplitudes"));
  root_node = model->getMarkedNode("CCXAmplitudes");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXAmplitudesCreate");
  node->setIcon(ccx_iface->getIcon("CCXAmplitudesCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXAmplitudesModify");
  node->setIcon(ccx_iface->getIcon("CCXAmplitudesModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXAmplitudesDelete");
  node->setIcon(ccx_iface->getIcon("CCXAmplitudesDelete"));

  //##############################
  // add Loads Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Loads", root_node);
  node->setTitle("Loads");
  model->setNodeMarker(node, "CCXLoads");
  node->setIcon(ccx_iface->getIcon("CCXLoads"));

  //##############################
  // add LoadsForces Nodes
  root_node = model->getMarkedNode("CCXLoads");
  node = model->addNode("Force", root_node);
  model->setNodeMarker(node, "CCXLoadsForces");
  node->setIcon(ccx_iface->getIcon("CCXLoadsForces"));
  root_node = model->getMarkedNode("CCXLoadsForces");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXLoadsForcesModify");
  node->setIcon(ccx_iface->getIcon("CCXLoadsForcesModify"));

  //##############################
  // add LoadsPressures Nodes
  root_node = model->getMarkedNode("CCXLoads");
  node = model->addNode("Pressure", root_node);
  model->setNodeMarker(node, "CCXLoadsPressures");
  node->setIcon(ccx_iface->getIcon("CCXLoadsPressures"));
  root_node = model->getMarkedNode("CCXLoadsPressures");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXLoadsPressuresModify");
  node->setIcon(ccx_iface->getIcon("CCXLoadsPressuresModify"));

  //##############################
  // add BCs Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("BCs", root_node);
  node->setTitle("BCs");
  model->setNodeMarker(node, "CCXBCs");
  node->setIcon(ccx_iface->getIcon("CCXBCs"));

  //##############################
  // add BCsDisplacements Nodes
  root_node = model->getMarkedNode("CCXBCs");
  node = model->addNode("Displacement", root_node);
  model->setNodeMarker(node, "CCXBCsDisplacements");
  node->setIcon(ccx_iface->getIcon("CCXBCsDisplacements"));
  root_node = model->getMarkedNode("CCXBCsDisplacements");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXBCsDisplacementsModify");
  node->setIcon(ccx_iface->getIcon("CCXBCsDisplacementsModify"));

  //##############################
  // add BCsTemperatures Nodes
  root_node = model->getMarkedNode("CCXBCs");
  node = model->addNode("Temperature", root_node);
  model->setNodeMarker(node, "CCXBCsTemperatures");
  node->setIcon(ccx_iface->getIcon("CCXBCsTemperatures"));
  root_node = model->getMarkedNode("CCXBCsTemperatures");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXBCsTemperaturesModify");
  node->setIcon(ccx_iface->getIcon("CCXBCsTemperaturesModify"));

  //##############################
  // add History Outputs Nodes 
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("History Outputs", root_node);
  node->setTitle("History Outputs");
  model->setNodeMarker(node, "CCXHistoryOutputs");
  node->setIcon(ccx_iface->getIcon("CCXHistoryOutputs"));

  root_node = model->getMarkedNode("CCXHistoryOutputs");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsCreate");
  node->setIcon(ccx_iface->getIcon("CCXHistoryOutputsCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsModify");
  node->setIcon(ccx_iface->getIcon("CCXHistoryOutputsModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsDelete");
  node->setIcon(ccx_iface->getIcon("CCXHistoryOutputsDelete"));

  root_node = model->getMarkedNode("CCXHistoryOutputsModify");
  node = model->addNode("Node", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsModifyNode");
  node->setIcon(ccx_iface->getIcon("CCXHistoryOutputsModifyNode"));
  node = model->addNode("Element", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsModifyElement");
  node->setIcon(ccx_iface->getIcon("CCXHistoryOutputsModifyElement"));
  node = model->addNode("Contact", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsModifyContact");
  node->setIcon(ccx_iface->getIcon("CCXHistoryOutputsModifyContact"));
  
  //##############################
  // add Field Outputs Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Field Outputs", root_node);
  node->setTitle("Field Outputs");
  model->setNodeMarker(node, "CCXFieldOutputs");
  node->setIcon(ccx_iface->getIcon("CCXFieldOutputs"));

  root_node = model->getMarkedNode("CCXFieldOutputs");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsCreate");
  node->setIcon(ccx_iface->getIcon("CCXFieldOutputsCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsModify");
  node->setIcon(ccx_iface->getIcon("CCXFieldOutputsModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsDelete");
  node->setIcon(ccx_iface->getIcon("CCXFieldOutputsDelete"));

  root_node = model->getMarkedNode("CCXFieldOutputsModify");
  node = model->addNode("Node", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsModifyNode");
  node->setIcon(ccx_iface->getIcon("CCXFieldOutputsModifyNode"));
  node = model->addNode("Element", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsModifyElement");
  node->setIcon(ccx_iface->getIcon("CCXFieldOutputsModifyElement"));
  node = model->addNode("Contact", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsModifyContact");
  node->setIcon(ccx_iface->getIcon("CCXFieldOutputsModifyContact"));
  
  //##############################
  // add Initial Conditions Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Initial Conditions", root_node);
  node->setTitle("Initial Conditions");
  model->setNodeMarker(node, "CCXInitialConditions");  
  node->setIcon(ccx_iface->getIcon("CCXInitialConditions"));

  root_node = model->getMarkedNode("CCXInitialConditions");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXInitialConditionsCreate");
  node->setIcon(ccx_iface->getIcon("CCXInitialConditionsCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXInitialConditionsModify");
  node->setIcon(ccx_iface->getIcon("CCXInitialConditionsModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXInitialConditionsDelete");
  node->setIcon(ccx_iface->getIcon("CCXInitialConditionsDelete"));

  //##############################
  // add Steps Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Steps", root_node);
  node->setTitle("Steps");
  model->setNodeMarker(node, "CCXSteps");
  node->setIcon(ccx_iface->getIcon("CCXSteps"));

  root_node = model->getMarkedNode("CCXSteps");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXStepsCreate");
  node->setIcon(ccx_iface->getIcon("CCXStepsCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXStepsModify");
  node->setIcon(ccx_iface->getIcon("CCXStepsModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXStepsDelete");
  node->setIcon(ccx_iface->getIcon("CCXStepsDelete"));
  
  root_node = model->getMarkedNode("CCXStepsModify");
  node = model->addNode("Parameter", root_node);
  model->setNodeMarker(node, "CCXStepsModifyParameter");
  node->setIcon(ccx_iface->getIcon("CCXStepsModifyParameter"));
  node = model->addNode("Static", root_node);
  model->setNodeMarker(node, "CCXStepsModifyStatic");
  node->setIcon(ccx_iface->getIcon("CCXStepsModifyStatic"));
  node = model->addNode("Frequency", root_node);
  model->setNodeMarker(node, "CCXStepsModifyFrequency");
  node->setIcon(ccx_iface->getIcon("CCXStepsModifyFrequency"));
  node = model->addNode("Buckle", root_node);
  model->setNodeMarker(node, "CCXStepsModifyBuckle");
  node->setIcon(ccx_iface->getIcon("CCXStepsModifyBuckle"));
  node = model->addNode("Heat Transfer", root_node);
  model->setNodeMarker(node, "CCXStepsModifyHeatTransfer");
  node->setIcon(ccx_iface->getIcon("CCXStepsModifyHeatTransfer"));
  node = model->addNode("Coupled Temperature Displacement", root_node);
  model->setNodeMarker(node, "CCXStepsModifyCoupledTemperatureDisplacement");
  node->setIcon(ccx_iface->getIcon("CCXStepsModifyCoupledTemperatureDisplacement"));
  node = model->addNode("Uncoupled Temperature Displacement", root_node);
  model->setNodeMarker(node, "CCXStepsModifyUncoupledTemperatureDisplacement");
  node->setIcon(ccx_iface->getIcon("CCXStepsModifyUncoupledTemperatureDisplacement"));

  //##############################
  // add Jobs Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Jobs", root_node);
  node->setTitle("Jobs");
  model->setNodeMarker(node, "CCXJobs");  
  node->setIcon(ccx_iface->getIcon("CCXJobs"));

  root_node = model->getMarkedNode("CCXJobs");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXJobsCreate");
  node->setIcon(ccx_iface->getIcon("CCXJobsCreate"));
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXJobsModify");
  node->setIcon(ccx_iface->getIcon("CCXJobsModify"));
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXJobsDelete");
  node->setIcon(ccx_iface->getIcon("CCXJobsDelete"));
  node = model->addNode("Actions", root_node);
  model->setNodeMarker(node, "CCXJobsActions");
  node->setIcon(ccx_iface->getIcon("CCXJobsActions"));
}

void cmdPanelManager::associate_panels_with_nodes()
{
  // The widget factory is in charge of giving out our panels when the framework
  // needs them.
  if(!myFactory)
    myFactory = new MyCmdWidgetFactory();

  // Setup a list of the markers that identify my navigation nodes. Note that
  // we did not include our root marker because we do not need to create a
  // command panel for it.
  QStringList my_markers;
  my_markers.push_back("CCXBlocksModify");
  my_markers.push_back("CCXSectionsCreateSolid");
  my_markers.push_back("CCXSectionsCreateShell");
  my_markers.push_back("CCXSectionsCreateBeam");
  my_markers.push_back("CCXSectionsCreateMembrane");
  my_markers.push_back("CCXSectionsModifySolid");
  my_markers.push_back("CCXSectionsModifyShell");
  my_markers.push_back("CCXSectionsModifyBeam");
  my_markers.push_back("CCXSectionsModifyMembrane");
  my_markers.push_back("CCXSectionsDelete");
  my_markers.push_back("CCXConstraintsCreateRigidBody");
  my_markers.push_back("CCXConstraintsCreateTie");
  my_markers.push_back("CCXConstraintsModifyRigidBody");
  my_markers.push_back("CCXConstraintsModifyTie");
  my_markers.push_back("CCXConstraintsDelete");
  my_markers.push_back("CCXSurfaceInteractionsCreate");
  my_markers.push_back("CCXSurfaceInteractionsModify");
  my_markers.push_back("CCXSurfaceInteractionsDelete");
  my_markers.push_back("CCXContactPairsCreate");
  my_markers.push_back("CCXContactPairsModify");
  my_markers.push_back("CCXContactPairsDelete");
  my_markers.push_back("CCXAmplitudesCreate");
  my_markers.push_back("CCXAmplitudesModify");
  my_markers.push_back("CCXAmplitudesDelete");
  my_markers.push_back("CCXLoadsForcesModify");
  my_markers.push_back("CCXLoadsPressuresModify");
  my_markers.push_back("CCXBCsDisplacementsModify");
  my_markers.push_back("CCXBCsTemperaturesModify");
  my_markers.push_back("CCXHistoryOutputsCreate");
  my_markers.push_back("CCXHistoryOutputsModifyNode");
  my_markers.push_back("CCXHistoryOutputsModifyElement");
  my_markers.push_back("CCXHistoryOutputsModifyContact");
  my_markers.push_back("CCXHistoryOutputsDelete");
  my_markers.push_back("CCXFieldOutputsCreate");
  my_markers.push_back("CCXFieldOutputsModifyNode");
  my_markers.push_back("CCXFieldOutputsModifyElement");
  my_markers.push_back("CCXFieldOutputsModifyContact");
  my_markers.push_back("CCXFieldOutputsDelete");
  my_markers.push_back("CCXInitialConditionsCreate");
  my_markers.push_back("CCXInitialConditionsModify");
  my_markers.push_back("CCXInitialConditionsDelete");
  my_markers.push_back("CCXStepsCreate");
  my_markers.push_back("CCXStepsModifyParameter");
  my_markers.push_back("CCXStepsModifyStatic");
  my_markers.push_back("CCXStepsModifyFrequency");
  my_markers.push_back("CCXStepsModifyBuckle");
  my_markers.push_back("CCXStepsModifyHeatTransfer");
  my_markers.push_back("CCXStepsModifyCoupledTemperatureDisplacement");
  my_markers.push_back("CCXStepsModifyUncoupledTemperatureDisplacement");
  my_markers.push_back("CCXStepsDelete");
  my_markers.push_back("CCXJobsCreate");
  my_markers.push_back("CCXJobsModify");
  my_markers.push_back("CCXJobsDelete");
  my_markers.push_back("CCXJobsActions");
  // For each marker, we want to get the navigation node and assign the node
  // to use this factory to get widgets as needed.
  NavigationModel* model = Claro::instance()->navigation_model();
  for(int i = 0; i < my_markers.size(); i++)
  {
    NavigationNode* node = model->getMarkedNode(my_markers[i]);
    if(node)
      node->setFactory(myFactory);
  }
}