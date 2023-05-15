#include "cmdPanelManager.hpp"
#include "Claro.hpp"
#include "NavigationModel.hpp"
#include "NavigationNode.hpp"
#include "MyCmdWidgetFactory.hpp"

#include <QAction>
#include <QIcon>
#include <QStringList>

cmdPanelManager::cmdPanelManager() :
  isInitialized(false),
  myFactory(NULL)
{}

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
    my_markers.push_back("CCXBlocksElementType");
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
  //##############################
  // add BlocksTree Nodes
  // add new Node between Exodus/Block and ExodusElementTypeBlock
  root_node = model->getNode("Exodus/Block");
  node = model->addNode("Element Types", root_node);
  model->setNodeMarker(node, "ElementTypeBlockNavigation");
  // set new Parent to ExodusElementTypeBlock and remove from old
  node = model->getMarkedNode("ExodusElementTypeBlock");
  root_node->removeChild(root_node->getChildIndex(node));
  root_node = model->getMarkedNode("ElementTypeBlockNavigation");
  node->setParent(root_node);
  root_node->insertChild(root_node->childCount()+1,node);
  NodeIconPointer = node->getIcon();
  root_node->setIcon(NodeIconPointer);
  // add Element Type Node
  root_node = model->getMarkedNode("ElementTypeBlockNavigation");
  node = model->addNode("CCX Element Types", root_node);
  model->setNodeMarker(node, "CCXBlocksElementType");

  //##############################
  // add Materials and Section Nodes
  root_node = model->getNode("Exodus");
  node = model->addNode("CCX Sections", root_node);
  model->setNodeMarker(node, "CCXSections");
  root_node = model->getMarkedNode("CCXSections");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXSectionsCreate");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXSectionsModify");
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXSectionsDelete");
  //##
  root_node = model->getMarkedNode("CCXSectionsCreate");
  node = model->addNode("Solid Section", root_node);
  model->setNodeMarker(node, "CCXSectionsCreateSolid");
  node = model->addNode("Shell Section", root_node);
  model->setNodeMarker(node, "CCXSectionsCreateShell");
  node = model->addNode("Beam Section", root_node);
  model->setNodeMarker(node, "CCXSectionsCreateBeam");
  node = model->addNode("CMembrane Section", root_node);
  model->setNodeMarker(node, "CCXSectionsCreateMembrane");
  //##
  root_node = model->getMarkedNode("CCXSectionsModify");
  node = model->addNode("Solid Section", root_node);
  model->setNodeMarker(node, "CCXSectionsModifySolid");
  node = model->addNode("Shell Section", root_node);
  model->setNodeMarker(node, "CCXSectionsModifyShell");
  node = model->addNode("Beam Section", root_node);
  model->setNodeMarker(node, "CCXSectionsModifyBeam");
  node = model->addNode("Membrane Section", root_node);
  model->setNodeMarker(node, "CCXSectionsModifyMembrane");

  //##############################
  // add Constraints Nodes
  // add new Node between FEA/Create and FEAConstraintCreate
  root_node = model->getNode("FEA/Create");
  node = model->addNode("Constraints", root_node);
  model->setNodeMarker(node, "FEAConstraintCreateNavigation");
  // set new Parent to FEAConstraintCreate and remove from old
  node = model->getMarkedNode("FEAConstraintCreate");
  root_node->removeChild(root_node->getChildIndex(node));
  root_node = model->getMarkedNode("FEAConstraintCreateNavigation");
  node->setParent(root_node);
  root_node->insertChild(root_node->childCount()+1,node);
  NodeIconPointer = node->getIcon();
  root_node->setIcon(NodeIconPointer);
  // add Constraint Nodes
  root_node = model->getMarkedNode("FEAConstraintCreateNavigation");
  node = model->addNode("CCX Rigid Body", root_node);
  model->setNodeMarker(node, "CCXConstraintsCreateRigidBody");
  node = model->addNode("CCX Tie", root_node);
  model->setNodeMarker(node, "CCXConstraintsCreateTie");
  // add new Node between FEA/Modify and FEAConstraintModify
  root_node = model->getNode("FEA/Modify");
  node = model->addNode("Constraints", root_node);
  model->setNodeMarker(node, "FEAConstraintModifyNavigation");
  // set new Parent to FEAConstraintModify and remove from old
  node = model->getMarkedNode("FEAConstraintModify");
  root_node->removeChild(root_node->getChildIndex(node));
  root_node = model->getMarkedNode("FEAConstraintModifyNavigation");
  node->setParent(root_node);
  root_node->insertChild(root_node->childCount()+1,node);
  NodeIconPointer = node->getIcon();
  root_node->setIcon(NodeIconPointer);
  // add Constraint Nodes
  root_node = model->getMarkedNode("FEAConstraintModifyNavigation");
  node = model->addNode("CCX Rigid Body", root_node);
  model->setNodeMarker(node, "CCXConstraintsModifyRigidBody");
  node = model->addNode("CCX Tie", root_node);
  model->setNodeMarker(node, "CCXConstraintsModifyTie");
  root_node = model->getNode("FEA/Delete");
  node = model->addNode("CCX Delete", root_node);
  model->setNodeMarker(node, "FEADeleteNavigation");
  root_node = model->getMarkedNode("FEADeleteNavigation");
  node = model->addNode("Delete Constraint", root_node);
  model->setNodeMarker(node, "CCXConstraintsDelete");

  //##############################
  // add SurfaceInteractions Nodes
  root_node = model->getNode("FEA/Create");
  node = model->addNode("CCX Surface Interactions", root_node);
  model->setNodeMarker(node, "CCXSurfaceInteractionsCreate");
  root_node = model->getNode("FEA/Modify");
  node = model->addNode("CCX Surface Interactions", root_node);
  model->setNodeMarker(node, "CCXSurfaceInteractionsModify");
  root_node = model->getMarkedNode("FEADeleteNavigation");
  node = model->addNode("Delete Surface Interaction", root_node);
  model->setNodeMarker(node, "CCXSurfaceInteractionsDelete");
  
  //##############################
  // add ContactPairs Nodes
  // add new Node between FEA/Create and FEAContactPairCreate
  root_node = model->getNode("FEA/Create");
  node = model->addNode("Contact Pairs", root_node);
  model->setNodeMarker(node, "FEAContactPairCreateNavigation");
  // set new Parent to FEAContactPairCreate and remove from old
  node = model->getMarkedNode("FEAContactPairCreate");
  root_node->removeChild(root_node->getChildIndex(node));
  root_node = model->getMarkedNode("FEAContactPairCreateNavigation");
  node->setParent(root_node);
  root_node->insertChild(root_node->childCount()+1,node);
  NodeIconPointer = node->getIcon();
  root_node->setIcon(NodeIconPointer);
  // add Constraint Nodes
  root_node = model->getMarkedNode("FEAContactPairCreateNavigation");
  node = model->addNode("CCX Contact Pairs", root_node);
  model->setNodeMarker(node, "CCXContactPairsCreate");
  // add new Node between FEA/Modify and FEAContactPairModify
  root_node = model->getNode("FEA/Modify");
  node = model->addNode("Contact Pairs", root_node);
  model->setNodeMarker(node, "FEAContactPairModifyNavigation");
  // set new Parent to FEAContactPairModify and remove from old
  node = model->getMarkedNode("FEAContactPairModify");
  root_node->removeChild(root_node->getChildIndex(node));
  root_node = model->getMarkedNode("FEAContactPairModifyNavigation");
  node->setParent(root_node);
  root_node->insertChild(root_node->childCount()+1,node);
  NodeIconPointer = node->getIcon();
  root_node->setIcon(NodeIconPointer);
  // add Constraint Nodes
  root_node = model->getMarkedNode("FEAContactPairModifyNavigation");
  node = model->addNode("CCX Contact Pairs", root_node);
  model->setNodeMarker(node, "CCXContactPairsModify");
  root_node = model->getMarkedNode("FEADeleteNavigation");
  node = model->addNode("Delete Contact Pair", root_node);
  model->setNodeMarker(node, "CCXContactPairsDelete");

  //##############################
  // add Amplitudes Nodes
  root_node = model->getNode("FEA/Create");
  node = model->addNode("CCX Amplitudes", root_node);
  model->setNodeMarker(node, "CCXAmplitudesCreate");
  root_node = model->getNode("FEA/Modify");
  node = model->addNode("CCX Amplitudes", root_node);
  model->setNodeMarker(node, "CCXAmplitudesModify");
  root_node = model->getMarkedNode("FEADeleteNavigation");
  node = model->addNode("Delete Amplitude", root_node);
  model->setNodeMarker(node, "CCXAmplitudesDelete");

  //##############################
  // add LoadsForces Nodes
  // add new Node between FEA/Modify and FEAForceModify
  root_node = model->getNode("FEA/Modify");
  node = model->addNode("Force", root_node);
  model->setNodeMarker(node, "FEAForceModifyNavigation");
  node = model->getMarkedNode("FEAForceModify");
  root_node->removeChild(root_node->getChildIndex(node));
  root_node = model->getMarkedNode("FEAForceModifyNavigation");
  node->setParent(root_node);
  root_node->insertChild(root_node->childCount()+1,node);
  NodeIconPointer = node->getIcon();
  root_node->setIcon(NodeIconPointer);
  root_node = model->getMarkedNode("FEAForceModifyNavigation");
  node = model->addNode("CCX Force", root_node);
  model->setNodeMarker(node, "CCXLoadsForcesModify");

  //##############################
  // add LoadsPressures Nodes
  // add new Node between FEA/Modify and FEAPressureModify
  root_node = model->getNode("FEA/Modify");
  node = model->addNode("Pressure", root_node);
  model->setNodeMarker(node, "FEAPressureModifyNavigation");
  node = model->getMarkedNode("FEAPressureModify");
  root_node->removeChild(root_node->getChildIndex(node));
  root_node = model->getMarkedNode("FEAPressureModifyNavigation");
  node->setParent(root_node);
  root_node->insertChild(root_node->childCount()+1,node);
  NodeIconPointer = node->getIcon();
  root_node->setIcon(NodeIconPointer);
  root_node = model->getMarkedNode("FEAPressureModifyNavigation");
  node = model->addNode("CCX Pressure", root_node);
  model->setNodeMarker(node, "CCXLoadsPressuresModify");

  //##############################
  // add BCsDisplacements Nodes
  // add new Node between FEA/Modify and FEADisplacementModify
  root_node = model->getNode("FEA/Modify");
  node = model->addNode("Displacement", root_node);
  model->setNodeMarker(node, "FEADisplacementModifyNavigation");
  node = model->getMarkedNode("FEADisplacementModify");
  root_node->removeChild(root_node->getChildIndex(node));
  root_node = model->getMarkedNode("FEADisplacementModifyNavigation");
  node->setParent(root_node);
  root_node->insertChild(root_node->childCount()+1,node);
  NodeIconPointer = node->getIcon();
  root_node->setIcon(NodeIconPointer);
  root_node = model->getMarkedNode("FEADisplacementModifyNavigation");
  node = model->addNode("CCX Displacement", root_node);
  model->setNodeMarker(node, "CCXBCsDisplacementsModify");

  //##############################
  // add BCsTemperatures Nodes
  // add new Node between FEA/Modify and FEADTemperatureModify
  root_node = model->getNode("FEA/Modify");
  node = model->addNode("Temperature", root_node);
  model->setNodeMarker(node, "FEATemperatureModifyNavigation");
  node = model->getMarkedNode("FEATemperatureModify");
  root_node->removeChild(root_node->getChildIndex(node));
  root_node = model->getMarkedNode("FEATemperatureModifyNavigation");
  node->setParent(root_node);
  root_node->insertChild(root_node->childCount()+1,node);
  NodeIconPointer = node->getIcon();
  root_node->setIcon(NodeIconPointer);
  root_node = model->getMarkedNode("FEATemperatureModifyNavigation");
  node = model->addNode("CCX Temperature", root_node);
  model->setNodeMarker(node, "CCXBCsTemperaturesModify");

  //##############################
  // add History Outputs Nodes
  root_node = model->addNode("CCX", model->getRoot());
  root_node->setTitle("CCX");
  model->setNodeMarker(root_node, "CCX");  
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("History Outputs", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputs");

  root_node = model->getMarkedNode("CCXHistoryOutputs");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsCreate");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsModify");
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsDelete");

  root_node = model->getMarkedNode("CCXHistoryOutputsModify");
  node = model->addNode("Node", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsModifyNode");
  node = model->addNode("Element", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsModifyElement");
  node = model->addNode("Contact", root_node);
  model->setNodeMarker(node, "CCXHistoryOutputsModifyContact");
  
  //##############################
  // add Field Outputs Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Field Outputs", root_node);
  model->setNodeMarker(node, "CCXFieldOutputs");

  root_node = model->getMarkedNode("CCXFieldOutputs");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsCreate");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsModify");
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsDelete");

  root_node = model->getMarkedNode("CCXFieldOutputsModify");
  node = model->addNode("Node", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsModifyNode");
  node = model->addNode("Element", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsModifyElement");
  node = model->addNode("Contact", root_node);
  model->setNodeMarker(node, "CCXFieldOutputsModifyContact");
  
  //##############################
  // add Initial Conditions Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Initial Conditions", root_node);
  node->setTitle("Initial Conditions");
  model->setNodeMarker(node, "CCXInitialConditions");  

  root_node = model->getMarkedNode("CCXInitialConditions");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXInitialConditionsCreate");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXInitialConditionsModify");
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXInitialConditionsDelete");

  //##############################
  // add Steps Nodes
  root_node = model->getMarkedNode("CCX");
  node = model->addNode("Steps", root_node);
  node->setTitle("Steps");
  model->setNodeMarker(node, "CCXSteps");  

  root_node = model->getMarkedNode("CCXSteps");
  node = model->addNode("Create", root_node);
  model->setNodeMarker(node, "CCXStepsCreate");
  node = model->addNode("Modify", root_node);
  model->setNodeMarker(node, "CCXStepsModify");
  node = model->addNode("Delete", root_node);
  model->setNodeMarker(node, "CCXStepsDelete");
  
  root_node = model->getMarkedNode("CCXStepsModify");
  node = model->addNode("Parameter", root_node);
  model->setNodeMarker(node, "CCXStepsModifyParameter");
  node = model->addNode("Static", root_node);
  model->setNodeMarker(node, "CCXStepsModifyStatic");
  node = model->addNode("Frequency", root_node);
  model->setNodeMarker(node, "CCXStepsModifyFrequency");
  node = model->addNode("Buckle", root_node);
  model->setNodeMarker(node, "CCXStepsModifyBuckle");
  node = model->addNode("Heat Transfer", root_node);
  model->setNodeMarker(node, "CCXStepsModifyHeatTransfer");
  node = model->addNode("Coupled Temperature Displacement", root_node);
  model->setNodeMarker(node, "CCXStepsModifyCoupledTemperatureDisplacement");
  node = model->addNode("Uncoupled Temperature Displacement", root_node);
  model->setNodeMarker(node, "CCXStepsModifyUncoupledTemperatureDisplacement");
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
  my_markers.push_back("CCXBlocksElementType");
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

