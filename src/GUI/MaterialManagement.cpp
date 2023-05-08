#include "MaterialManagement.hpp"
#include "CalculiXCoreInterface.hpp"
#include "MaterialManagementItem.hpp"
#include "MaterialManagementElasticCard.hpp"
#include "MaterialManagementPlasticCard.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"

MaterialManagement::MaterialManagement()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();


  // main window
  //this->setGeometry(0,0,700,570);
  this->setWindowTitle("Material Management");
  gridLayout = new QGridLayout(this);
  boxLayout_window = new QHBoxLayout();
  boxLayout_material1 = new QVBoxLayout();
  boxLayout_material2 = new QVBoxLayout();
  boxLayout_material_cards1 = new QVBoxLayout();
  boxLayout_material_cards2 = new QVBoxLayout();
  boxLayout_material_cards3 = new QVBoxLayout();
  boxLayout_widget = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_window,2,4, Qt::AlignRight);
  gridLayout->addLayout(boxLayout_material1,0,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_material2,0,1, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_material_cards1,0,2, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_material_cards2,0,3, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_material_cards3,0,4, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_widget,1,0,1,5, Qt::AlignTop);
  boxLayout_material2->addSpacing(30);
  boxLayout_material_cards2->addSpacing(30);

  // buttons
  pushButton_ok = new QPushButton();
  //pushButton_ok->setGeometry(310,530,75,23);
  pushButton_ok->setText("OK");
  boxLayout_window->addWidget(pushButton_ok);

  pushButton_apply = new QPushButton();
  //pushButton_apply->setGeometry(400,530,75,23);
  pushButton_apply->setText("Apply");
  boxLayout_window->addWidget(pushButton_apply);

  pushButton_close = new QPushButton();
  //pushButton_close->setGeometry(490,530,75,23);
  pushButton_close->setText("Close");
  boxLayout_window->addWidget(pushButton_close);
  
  pushButton_new = new QPushButton();
  //pushButton_new->setGeometry(200,30,75,23);
  pushButton_new->setText("New");
  boxLayout_material2->addWidget(pushButton_new);

  pushButton_delete = new QPushButton();
  //pushButton_delete->setGeometry(200,60,75,23);
  pushButton_delete->setText("Delete");
  boxLayout_material2->addWidget(pushButton_delete);
  
  pushButton_add = new QPushButton();
  //pushButton_add->setGeometry(420,70,21,21);
  pushButton_add->setText(">");
  boxLayout_material_cards2->addWidget(pushButton_add);

  pushButton_remove = new QPushButton();
  //pushButton_remove->setGeometry(420,100,21,21);
  pushButton_remove->setText("<");
  boxLayout_material_cards2->addWidget(pushButton_remove);

  // labels
  label_material = new QLabel();
  //label_material->setGeometry(10,10,80,16);
  label_material->setText("Material");
  boxLayout_material1->addWidget(label_material);

  label_available = new QLabel();
  //label_available->setGeometry(300,10,100,16);
  label_available->setText("Available Cards");
  boxLayout_material_cards1->addWidget(label_available);

  label_used = new QLabel();
  //label_used->setGeometry(450,10,100,16);
  label_used->setText("Used Cards");
  boxLayout_material_cards3->addWidget(label_used);

  // tree/lists
  tree_material = new QTreeWidget();
  //tree_material->setGeometry(10,30,181,191);
  tree_material->setColumnCount(2);
  tree_material->setHeaderLabels(QStringList() << "Name" << "ID");
  boxLayout_material1->addWidget(tree_material);

  list_available = new QListWidget();
  //list_available->setGeometry(300,30,111,191);
  boxLayout_material_cards1->addWidget(list_available);

  list_used = new QListWidget();
  //list_used->setGeometry(450,30,111,191);
  boxLayout_material_cards3->addWidget(list_used);

  // card widgets
  card_frame = new QFrame();
  //card_widget->setGeometry(10,10,500,250);
  card_frame->setMinimumSize(700,300);
  card_frame->setLineWidth(1);
  card_frame->setMidLineWidth(0);
  card_frame->setFrameStyle(QFrame::Box | QFrame::Raised);
  //card_widget->setStyleSheet("border: 1px solid black");
  
  elastic_widget = new MaterialManagementElasticCard(card_frame,current_material_item);
  plastic_widget = new MaterialManagementPlasticCard(card_frame,current_material_item);

  density_widget = new QWidget(card_frame);
  density_widget->setGeometry(10,10,150,23);
  density_label_title = new QLabel(density_widget);
  density_label_title->setText("Density Card");

  expansion_widget = new QWidget(card_frame);
  expansion_widget->setGeometry(10,10,150,23);
  expansion_label_title = new QLabel(expansion_widget);
  expansion_label_title->setText("Expansion Card");

  boxLayout_widget->addWidget(card_frame);

  //card_frame->show();
  elastic_widget->hide();
  plastic_widget->hide();
  density_widget->hide();
  expansion_widget->hide();

  // Signals
  QObject::connect(pushButton_ok, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_ok_clicked(bool)));
  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_close_clicked(bool)));
  QObject::connect(pushButton_new, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_new_clicked(bool)));
  QObject::connect(pushButton_delete, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_delete_clicked(bool)));
  QObject::connect(pushButton_add, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_add_clicked(bool)));
  QObject::connect(pushButton_remove, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_remove_clicked(bool)));
  QObject::connect(tree_material, SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,  SLOT(material_clicked(QTreeWidgetItem*, int)));
  QObject::connect(tree_material, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),this,  SLOT(material_changed(QTreeWidgetItem*,QTreeWidgetItem*)));
  QObject::connect(list_used, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(material_card_clicked(QListWidgetItem*)));
  QObject::connect(list_available, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(material_card_clicked(QListWidgetItem*)));
  QObject::connect(list_used, SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,  SLOT(material_card_doubleclicked(QListWidgetItem*)));
  QObject::connect(list_available, SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,  SLOT(material_card_doubleclicked(QListWidgetItem*)));
  QObject::connect(list_used, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(material_card_changed(QListWidgetItem*,QListWidgetItem*)));  
  QObject::connect(list_available, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(material_card_changed(QListWidgetItem*,QListWidgetItem*)));

  // Update list items and data
  this->update();
}

MaterialManagement::~MaterialManagement()
{}

void MaterialManagement::clear()
{
  // Remove all of our tree items from material tree.
  while (tree_material->topLevelItemCount()>0)
  {
    tree_material->removeItemWidget(tree_material->topLevelItem(0),0);
  }
}

void MaterialManagement::update()
{
  std::vector<std::vector<std::string>> material_tree_data;
  material_tree_data = ccx_iface->get_material_tree_data(); 

  MaterialManagementItem *temp_child;
  int ChildId;
  std::string str_check;
  bool erase_item;

  for (size_t i = 0; i < material_tree_data.size(); i++)
  {
    // check if material already exists as item
    
    ChildId = this->get_child_id(material_tree_data[i][0]);

    if (ChildId == -1)
    {
      this->addMaterial(QString::fromStdString(material_tree_data[i][0]), QString::fromStdString(material_tree_data[i][1]));  
    }else{
      // modify name if necessary
      temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(ChildId));
      
      str_check = material_tree_data[i][1];    

      if (temp_child->text(0).toStdString() != str_check)
      {
        temp_child->setText(0, QString::fromStdString(str_check));
      }
    }
  }

  // check if item has been removed

  for (size_t i = tree_material->topLevelItemCount(); i > 0; i--)
  { 
    erase_item = true;
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(i-1));

    for (size_t ii = 0; ii < material_tree_data.size(); ii++)
    {    
      if (temp_child->text(1).toStdString()==material_tree_data[ii][0])
      {
        erase_item = false;
        break;
      }
    }

    if (erase_item)
    {
      this->removeMaterial(temp_child);
    }
  }

  // update childs
  for (size_t i = tree_material->topLevelItemCount(); i > 0; i--)
  { 
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(i-1));
    temp_child->update();
  }
}

void MaterialManagement::addMaterial(QString material_id, QString material_name)
{
  MaterialManagementItem *MaterialTreeChild = new MaterialManagementItem(tree_material);
  MaterialTreeChild->initialize(material_id, material_name);
}

void MaterialManagement::removeMaterial(MaterialManagementItem *material)
{
  tree_material->removeItemWidget(material,0);
  delete material;
}

void MaterialManagement::printproperties()
{
  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < tree_material->topLevelItemCount(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(i));
    
    log = "########### \n";
    log.append(temp_child->text(1).toStdString() + " child id \n");
    for (size_t ii = 0; ii < temp_child->properties.size(); ii++)
    {
      log.append("----------\n");
      log.append(temp_child->group_properties[temp_child->properties[ii][0]][0] + " \n");
      log.append("----------\n");
      log.append(std::to_string(temp_child->properties[ii][0]) + " " + std::to_string(temp_child->properties[ii][1]) + " " + std::to_string(temp_child->properties[ii][2]) + " \n");
      if (temp_child->properties[ii][1]==1)
      {
        log.append("SCALAR\n");
        log.append(std::to_string(temp_child->property_scalar[temp_child->properties[ii][2]]) + " \n");
        log.append("SCALAR GUI\n");
        log.append(std::to_string(temp_child->property_scalar_gui[temp_child->properties[ii][2]]) + " \n");
      } else if (temp_child->properties[ii][1]==4)
      {        
        log.append("MATRIX\n");
        for (size_t iii = 0; iii < temp_child->property_matrix[temp_child->properties[ii][2]].size(); iii++)
        {
          for (size_t iv = 0; iv < temp_child->property_matrix[temp_child->properties[ii][2]][iii].size(); iv++)
          {
            log.append(std::to_string(temp_child->property_matrix[temp_child->properties[ii][2]][iii][iv]) + " ");
          }
          log.append("\n");
        }
        log.append("MATRIX GUI\n");
        for (size_t iii = 0; iii < temp_child->property_matrix_gui[temp_child->properties[ii][2]].size(); iii++)
        {
          for (size_t iv = 0; iv < temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii].size(); iv++)
          {
            log.append(std::to_string(temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii][iv]) + " ");
          }
          log.append("\n");
        }
      }
    }
    PRINT_INFO("%s", log.c_str());
  }
}

int MaterialManagement::get_child_id(std::string material_id)
{
  int int_return;
  int_return = -1;

  //log = std::to_string(tree_material->topLevelItemCount()) + " child count \n";
  //PRINT_INFO("%s", log.c_str());

  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < tree_material->topLevelItemCount(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(i));
    
    //log = temp_child->text(1).toStdString() + " == " + material_id + " child ids \n";
    //PRINT_INFO("%s", log.c_str());
    
    if (temp_child->text(1).toStdString()==material_id)
    {
      int_return = i;
      break;
    }
  }
  return int_return;
}

void MaterialManagement::createListItems(MaterialManagementItem *material)
{
  bool use_elastic = false;
  bool use_plastic = false;
  bool use_density = false;
  bool use_expansion = false;

  for (size_t i = 0; i < material->properties.size(); i++)
  {
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_ELASTIC_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_elastic = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_PLASTIC_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_plastic = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_DENSITY_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_density = true;
      }
    }
    if ((material->group_properties[material->properties[i][0]][0]=="CCX_EXPANSION_USE_CARD"))
    {
      if (material->property_scalar_gui[material->properties[i][2]]==1)
      {
        use_expansion = true;
      }
    }
  }
  
  if (use_elastic)
  {
    list_elastic = new QListWidgetItem("Elastic",list_used);
  } else {
    list_elastic = new QListWidgetItem("Elastic",list_available);
  }
  
  if (use_plastic)
  {
    list_plastic = new QListWidgetItem("Plastic",list_used);
  } else {
    list_plastic = new QListWidgetItem("Plastic",list_available);
  }

  if (use_density)
  {
    list_density = new QListWidgetItem("Density",list_used);
  } else {
    list_density = new QListWidgetItem("Density",list_available);
  }

  if (use_expansion)
  {
    list_expansion = new QListWidgetItem("Expansion",list_used);
  } else {
    list_expansion = new QListWidgetItem("Expansion",list_available);
  }

  list_available->sortItems();
  list_used->sortItems();
}

void MaterialManagement::removeListItems()
{
  if (list_available->count()>0)
  {
    list_available->setCurrentItem(list_available->item(0));
    while (list_available->currentItem())
    {
      delete list_available->currentItem();
    }
  } 

  if (list_used->count()>0)
  {
    list_used->setCurrentItem(list_used->item(0));
    while (list_used->currentItem())
    {
      delete list_used->currentItem();
    }
  } 
}

void MaterialManagement::switchListItem(QListWidget* source, QListWidget* target)
{
  if(source->currentItem()){
    QListWidgetItem* newItem = source->currentItem()->clone();
    target->addItem(newItem);
    target->setCurrentItem(newItem);
    delete source->currentItem();
    this->selectListItem(newItem);
    this->loadWidget(newItem);
    source->sortItems();
    target->sortItems();

    if (newItem->text()=="Elastic")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_ELASTIC_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_ELASTIC_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_ELASTIC_USE_CARD", 0);
      }
    }else if (newItem->text()=="Plastic")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_PLASTIC_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_PLASTIC_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_PLASTIC_USE_CARD", 0);
      }
    }else if (newItem->text()=="Density")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_DENSITY_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_DENSITY_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_DENSITY_USE_CARD", 0);
      }
    }else if (newItem->text()=="Expansion")
    {
      if (current_material_item->getScalarPropertyGUI("CCX_EXPANSION_USE_CARD")==0)
      {
        current_material_item->setScalarPropertyGUI("CCX_EXPANSION_USE_CARD", 1);
      }else{
        current_material_item->setScalarPropertyGUI("CCX_EXPANSION_USE_CARD", 0);
      }
    }
  }
}

void MaterialManagement::selectListItem(QListWidgetItem* item)
{
  QListWidgetItem* temp_item;

  for (size_t i = 0; i < list_available->count(); i++)
  {
    temp_item = list_available->item(i);
    if (temp_item != item)
    {
      temp_item->setSelected(false);
    }
  }
  for (size_t i = 0; i < list_used->count(); i++)
  {
    temp_item = list_used->item(i);
    if (temp_item != item)
    {
      temp_item->setSelected(false);
    }
  }  
}

void MaterialManagement::loadWidget(QListWidgetItem* item)
{
  if (item->text().toStdString()=="Elastic")
  {
    elastic_widget->show();
    plastic_widget->hide();
    density_widget->hide();
    expansion_widget->hide();
  }else if (item->text().toStdString()=="Plastic")
  {
    elastic_widget->hide();
    plastic_widget->show();
    density_widget->hide();
    expansion_widget->hide();
  }else if (item->text().toStdString()=="Density")
  { 
    elastic_widget->hide();
    plastic_widget->hide();
    density_widget->show();
    expansion_widget->hide();
  }else if (item->text().toStdString()=="Expansion")
  {
    elastic_widget->hide();
    plastic_widget->hide();
    density_widget->hide();
    expansion_widget->show();
  } 
}

void MaterialManagement::on_pushButton_ok_clicked(bool)
{
  //log = " clicked ok \n";
  //PRINT_INFO("%s", log.c_str());
  this->on_pushButton_apply_clicked(true);
  this->on_pushButton_close_clicked(true);
}

void MaterialManagement::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  this->printproperties();


  //modify material "Test3" matrix_property "CCX_ELASTIC_ANISO_CONSTANTS_VS_TEMPERATURE" 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22
  //modify material "Test3" scalar_properties "CCX_ELASTIC_ISO_USE_CARD" 1

  MaterialManagementItem *temp_child;

  for (size_t i = 0; i < tree_material->topLevelItemCount(); i++)
  {
    temp_child = dynamic_cast<MaterialManagementItem*>(tree_material->topLevelItem(i));
    
    log = "########### \n";
    log.append(temp_child->text(1).toStdString() + " child id \n");
    for (size_t ii = 0; ii < temp_child->properties.size(); ii++)
    {
      log.append("----------\n");
      log.append(temp_child->group_properties[temp_child->properties[ii][0]][0] + " \n");
      log.append("----------\n");
      log.append(std::to_string(temp_child->properties[ii][0]) + " " + std::to_string(temp_child->properties[ii][1]) + " " + std::to_string(temp_child->properties[ii][2]) + " \n");
      if (temp_child->properties[ii][1]==1)
      {
        log.append("SCALAR\n");
        log.append(std::to_string(temp_child->property_scalar[temp_child->properties[ii][2]]) + " \n");
        log.append("SCALAR GUI\n");
        log.append(std::to_string(temp_child->property_scalar_gui[temp_child->properties[ii][2]]) + " \n");
      } else if (temp_child->properties[ii][1]==4)
      {        
        log.append("MATRIX\n");
        for (size_t iii = 0; iii < temp_child->property_matrix[temp_child->properties[ii][2]].size(); iii++)
        {
          for (size_t iv = 0; iv < temp_child->property_matrix[temp_child->properties[ii][2]][iii].size(); iv++)
          {
            log.append(std::to_string(temp_child->property_matrix[temp_child->properties[ii][2]][iii][iv]) + " ");
          }
          log.append("\n");
        }
        log.append("MATRIX GUI\n");
        for (size_t iii = 0; iii < temp_child->property_matrix_gui[temp_child->properties[ii][2]].size(); iii++)
        {
          for (size_t iv = 0; iv < temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii].size(); iv++)
          {
            log.append(std::to_string(temp_child->property_matrix_gui[temp_child->properties[ii][2]][iii][iv]) + " ");
          }
          log.append("\n");
        }
      }
    }
    PRINT_INFO("%s", log.c_str());
  }



  // We must send the Cubit commands through the Claro framework, so first we need to translate
  // the commands into the python form that Claro will understand.
  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    for(int i = 0; i < commands.size(); i++)
      cubit_translator->decode(commands[i]);

    // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
}

void MaterialManagement::on_pushButton_close_clicked(bool)
{
  //log = " clicked close \n";
  //PRINT_INFO("%s", log.c_str());
  this->close();
}

void MaterialManagement::on_pushButton_new_clicked(bool)
{
  //log = " clicked new \n";
  //PRINT_INFO("%s", log.c_str());
  
  QStringList commands;
  bool ok;  
  QString name = QInputDialog::getText(0, "Create Material",
                                         "Material Name:", QLineEdit::Normal,
                                         "", &ok);
  if (ok && !name.isEmpty()){
    commands.push_back("create material \"" + name + "\" property_group \"CalculiX-FEA\"");
  }

  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    for(int i = 0; i < commands.size(); i++)
      cubit_translator->decode(commands[i]);

    // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
}

void MaterialManagement::on_pushButton_delete_clicked(bool)
{
  //log = " clicked delete \n";
  //PRINT_INFO("%s", log.c_str());
  QStringList commands;
  
  QMessageBox::StandardButton msgbox;
  msgbox = QMessageBox::question(this,"Delete Material","Delete Material " + current_material_item->material_name_qstring + "?", QMessageBox::Yes | QMessageBox::No);
  
  if (msgbox == QMessageBox::Yes)
  {
    commands.push_back("delete material " + current_material_item->material_id_qstring);
  }

  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    for(int i = 0; i < commands.size(); i++)
      cubit_translator->decode(commands[i]);

    // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
  this->removeListItems();
  elastic_widget->hide();
  plastic_widget->hide();
  density_widget->hide();
  expansion_widget->hide();
}

void MaterialManagement::on_pushButton_add_clicked(bool)
{
  //log = " clicked add \n";
  //PRINT_INFO("%s", log.c_str());
  switchListItem(list_available, list_used);
}

void MaterialManagement::on_pushButton_remove_clicked(bool)
{
  //log = " clicked remove \n";
  //PRINT_INFO("%s", log.c_str());
  switchListItem(list_used, list_available);
}

void MaterialManagement::material_clicked(QTreeWidgetItem* item, int column)
{
  //log = " material clicked \n";
  //PRINT_INFO("%s", log.c_str());
  MaterialManagementItem* material_item;

  if (material_item = dynamic_cast<MaterialManagementItem*>(item))
  {
    current_material_item = material_item;
    this->removeListItems();
    this->createListItems(material_item);
    elastic_widget->hide();
    plastic_widget->hide();
    density_widget->hide();
    expansion_widget->hide();
    if (current_material_item!=nullptr)
    {
      elastic_widget->update(material_item);
      plastic_widget->update(material_item);
    }
  }
}

void MaterialManagement::material_changed(QTreeWidgetItem* current_item, QTreeWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->material_clicked(current_item,0);
  }
}

void MaterialManagement::material_card_clicked(QListWidgetItem* item)
{
  //log = " material card " + item->text().toStdString() + " clicked \n";
  //PRINT_INFO("%s", log.c_str()); 
  this->selectListItem(item);
  this->loadWidget(item);
}

void MaterialManagement::material_card_doubleclicked(QListWidgetItem* item)
{
  //log = " material card " + item->text().toStdString() + " clicked \n";
  //PRINT_INFO("%s", log.c_str()); 
  if (item->listWidget()==list_used)
  {
    switchListItem(list_used, list_available);
  }else{
    switchListItem(list_available, list_used);
  }
}

void MaterialManagement::material_card_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->selectListItem(current_item);
    this->loadWidget(current_item);
  }
}