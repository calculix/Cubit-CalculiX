#include "JobsMonitorFRD.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"

JobsMonitorFRD::JobsMonitorFRD()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  //this->setGeometry(0,0,700,570);
  this->setWindowTitle("FRD Results");
  gridLayout = new QGridLayout(this);
  boxLayout_window = new QHBoxLayout();
  boxLayout_result_block = new QVBoxLayout();
  boxLayout_component = new QVBoxLayout();
  boxLayout_widget = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_window,2,4, Qt::AlignRight);
  gridLayout->addLayout(boxLayout_result_block,0,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_component,0,1, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_widget,1,0,1,4, Qt::AlignTop);

  // buttons
  pushButton_refresh = new QPushButton();
  pushButton_refresh->setText("OK");
  boxLayout_window->addWidget(pushButton_refresh);

  pushButton_plot = new QPushButton();
  pushButton_plot->setText("Apply");
  boxLayout_window->addWidget(pushButton_plot);

  // labels
  label_result_block = new QLabel();
  //label_material->setGeometry(10,10,80,16);
  label_result_block->setText("Result Blocks");
  boxLayout_result_block->addWidget(label_result_block);

  label_component = new QLabel();
  //label_available->setGeometry(300,10,100,16);
  label_component->setText("Components");
  boxLayout_component->addWidget(label_component);

  // tree/lists
  list_result_block = new QListWidget();
  boxLayout_result_block->addWidget(list_result_block);

  list_component = new QListWidget();
  boxLayout_component->addWidget(list_component);

  // card widgets
  result_frame = new QFrame();
  result_frame->setMinimumSize(700,350);
  result_frame->setLineWidth(1);
  result_frame->setMidLineWidth(0);
  result_frame->setFrameStyle(QFrame::Box | QFrame::Raised);
  
  //elastic_widget = new MaterialManagementElasticCard(card_frame,current_material_item);
  
  boxLayout_widget->addWidget(result_frame);

  //result_frame->show();

  // Signals
  QObject::connect(pushButton_refresh, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_refresh_clicked(bool)));
  QObject::connect(pushButton_plot, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_plot_clicked(bool)));
  QObject::connect(list_result_block, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(result_block_clicked(QListWidgetItem*)));
  QObject::connect(list_result_block, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(result_block_changed(QListWidgetItem*,QListWidgetItem*)));
  //QObject::connect(list_component, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(material_card_clicked(QListWidgetItem*)));
  //QObject::connect(list_component, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(material_card_changed(QListWidgetItem*,QListWidgetItem*)));

  // Update list items and data
  this->update();
}

JobsMonitorFRD::~JobsMonitorFRD()
{}

void JobsMonitorFRD::clear()
{
  // Remove all of our tree items from material tree.
  /*while (tree_material->topLevelItemCount()>0)
  {
    tree_material->removeItemWidget(tree_material->topLevelItem(0),0);
  }
  this->removeListItems();*/
}

void JobsMonitorFRD::update()
{
  if (current_job_id ==-1)
  {
    return;
  }
  
  this->removeListItems();

  std::vector<std::string> result_block_types = ccx_iface->frd_get_result_block_types(current_job_id);
  
  for (size_t i = 0; i < result_block_types.size(); i++)
  {
    this->addListItem(result_block_types[i],list_result_block);
  } 
}

void JobsMonitorFRD::update_component(std::string result_block)
{
  if (current_job_id ==-1)
  {
    return;
  }
  
  this->removeListItems_from_List(list_component);

  std::vector<std::string> components = ccx_iface->frd_get_result_block_components(current_job_id, result_block);
  
  for (size_t i = 0; i < components.size(); i++)
  {
    this->addListItem(components[i],list_component);
  } 
}

void JobsMonitorFRD::set_current_job_id(int job_id)
{
  this->current_job_id = job_id;
  
  std::string log = "Current job id set is " + std::to_string(current_job_id) + "\n";
  PRINT_INFO("%s", log.c_str());

  this->update();
}

void JobsMonitorFRD::addListItem(std::string item_name, QListWidget* parent_list)
{
  QString name;
  name = QString::fromStdString(item_name);
  QListWidgetItem* new_list_item;
  new_list_item = new QListWidgetItem(name,parent_list);


}

void JobsMonitorFRD::removeListItem(std::string item_name, QListWidget* parent_list)
{
  QString name;
  name = QString::fromStdString(item_name);

}



void JobsMonitorFRD::createListItems()
{
  QListWidgetItem* list_conductivity;
  list_conductivity = new QListWidgetItem("Conductivity",list_result_block);

  list_result_block->sortItems();
}

void JobsMonitorFRD::removeListItems()
{
  if (list_result_block->count()>0)
  {
    list_result_block->setCurrentItem(list_result_block->item(0));
    while (list_result_block->currentItem())
    {
      delete list_result_block->currentItem();
    }
  } 

  if (list_component->count()>0)
  {
    list_component->setCurrentItem(list_component->item(0));
    while (list_component->currentItem())
    {
      delete list_component->currentItem();
    }
  } 
}

void JobsMonitorFRD::removeListItems_from_List(QListWidget* list)
{
  if (list->count()>0)
  {
    list->setCurrentItem(list->item(0));
    while (list->currentItem())
    {
      delete list->currentItem();
    }
  }
}

void JobsMonitorFRD::selectListItem(QListWidgetItem* item)
{
  QListWidgetItem* temp_item;

  for (size_t i = 0; i < list_result_block->count(); i++)
  {
    temp_item = list_result_block->item(int(i));
    if (temp_item != item)
    {
      temp_item->setSelected(false);
    }
  }
}

void JobsMonitorFRD::on_pushButton_refresh_clicked(bool)
{
  this->update();  
}

void JobsMonitorFRD::on_pushButton_plot_clicked(bool)
{
}

void JobsMonitorFRD::result_block_clicked(QListWidgetItem* item)
{
 std::string result_block;
 result_block = item->text().toStdString();
 this->update_component(result_block);
}

void JobsMonitorFRD::result_block_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->selectListItem(current_item);
    this->result_block_clicked(current_item);
  }
}