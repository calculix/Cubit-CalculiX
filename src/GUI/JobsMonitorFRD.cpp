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
  boxLayout_increment = new QVBoxLayout();
  boxLayout_filter = new QVBoxLayout();
  boxLayout_widget = new QVBoxLayout();
  boxLayout_pages = new QHBoxLayout();
  gridLayout->addLayout(boxLayout_window,2,4, Qt::AlignRight);
  gridLayout->addLayout(boxLayout_result_block,0,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_component,0,1, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_increment,0,2, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_filter,0,3, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_widget,1,0,1,4, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_pages,2,0,1,4, Qt::AlignTop);

  // buttons
  pushButton_refresh = new QPushButton();
  pushButton_refresh->setText("Refresh");
  boxLayout_window->addWidget(pushButton_refresh);

  pushButton_plot = new QPushButton();
  pushButton_plot->setText("Plot");
  boxLayout_window->addWidget(pushButton_plot);

  // labels
  label_result_block = new QLabel();
  label_result_block->setText("Result Blocks");
  boxLayout_result_block->addWidget(label_result_block);

  label_component = new QLabel();
  label_component->setText("Components");
  boxLayout_component->addWidget(label_component);

  label_increment = new QLabel();
  label_increment->setText("Increments");
  boxLayout_increment->addWidget(label_increment);

  label_filter = new QLabel();
  label_filter->setText("Filter");
  boxLayout_filter->addWidget(label_filter);

  // lists
  list_result_block = new QListWidget();
  boxLayout_result_block->addWidget(list_result_block);

  list_component = new QListWidget();
  boxLayout_component->addWidget(list_component);

  list_filter = new QListWidget();
  list_filter->setFrameShape(QFrame::Box);
  list_filter->setFrameShadow(QFrame::Raised);

  list_increment = new QListWidget();
  list_increment->setFrameShape(QFrame::Box);
  list_increment->setFrameShadow(QFrame::Raised);
  boxLayout_increment->addWidget(list_increment);

  combobox_filter1 = new QComboBox();
  combobox_filter1->addItem(" ");
  combobox_filter1->addItem("Block");
  combobox_filter1->addItem("Sideset");
  combobox_filter1->addItem("Nodeset");
  combobox_filter1->addItem("Node");
  combobox_filter2 = new QComboBox();
  boxLayout_filter->addWidget(combobox_filter1);

  pushButton_apply_filter = new QPushButton("Apply Filter");
  boxLayout_filter->addWidget(pushButton_apply_filter);

  //table
  table_result = new QTableWidget();
  table_result->setMinimumSize(700,350);
  table_result->setLineWidth(1);
  table_result->setMidLineWidth(0);
  table_result->setFrameStyle(QFrame::Box | QFrame::Raised);
  table_counter = new QLabel;
  pushButton_prev = new QPushButton("Previous Results");
  pushButton_next = new QPushButton("Next Results");
  boxLayout_widget->addWidget(table_result);
  boxLayout_pages->addWidget(pushButton_prev);
  boxLayout_pages->addWidget(table_counter);
  boxLayout_pages->addWidget(pushButton_next);

  // Signals
  QObject::connect(pushButton_refresh, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_refresh_clicked(bool)));
  QObject::connect(pushButton_plot, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_plot_clicked(bool)));
  QObject::connect(pushButton_apply_filter, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_filter_clicked(bool)));
  QObject::connect(list_result_block, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(result_block_clicked(QListWidgetItem*)));
  QObject::connect(list_result_block, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(result_block_changed(QListWidgetItem*,QListWidgetItem*)));
  QObject::connect(list_component, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(component_clicked(QListWidgetItem*)));
  QObject::connect(list_component, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(component_changed(QListWidgetItem*,QListWidgetItem*)));
  QObject::connect(list_increment, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(increment_clicked(QListWidgetItem*)));
  QObject::connect(list_increment, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(increment_changed(QListWidgetItem*,QListWidgetItem*)));
  QObject::connect(pushButton_prev, SIGNAL(clicked(bool)), this, SLOT(on_pushButton_previous_clicked(bool)));
  QObject::connect(pushButton_next, SIGNAL(clicked(bool)), this, SLOT(on_pushButton_next_clicked(bool)));
  QObject::connect(combobox_filter1, SIGNAL(currentIndexChanged(int)), this, SLOT(update_filter(int)));

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

  list_result_block->clear();
  list_component->clear();
  list_increment->clear();
  table_result->setRowCount(0);
  table_result->setColumnCount(0);
  this->current_result_block = nullptr;
  this->current_result_component = nullptr;

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
  
  list_component->clear();
  list_increment->clear();

  std::vector<std::string> components = ccx_iface->frd_get_result_block_components(current_job_id, result_block);
  components.insert(components.begin(), "all");

  for (size_t i = 0; i < components.size(); i++)
  {
    this->addListItem(components[i],list_component);
  }

  list_component->item(0)->setSelected(true);
  this->component_clicked(list_component->item(0));
}

void JobsMonitorFRD::update_increment()
{
  if (current_job_id ==-1)
  {
    return;
  }
  
  list_increment->clear();

  std::vector<int> increments = ccx_iface->frd_get_total_increments(current_job_id);
  std::vector<std::variant<int, std::string>> total_increments;
  total_increments.push_back("all");
  for (int increment : increments)
  {
    total_increments.push_back(increment);
  }
  for (const auto& item : total_increments)
  {
    if (std::holds_alternative<int>(item))
    {
      this->addListItem(QString::number(std::get<int>(item)).toStdString(), list_increment);
    } else {
      this->addListItem(std::get<std::string>(item), list_increment);
    }
  }
  list_increment->item(0)->setSelected(true);
  this->increment_clicked(list_increment->item(0));
}

void JobsMonitorFRD::update_filter(int index) //Clemens updatet nicht, wenn filter geändert wird 
{
  /* if (this->combobox2_exists==true)
  {
    this->combobox2_exists = false;
    this->textFields_exists = false;
    removeWidgetFromLayout(boxLayout_filter, combobox_filter2);
  } else if (this->textFields_exists==true)
  {
    this->combobox2_exists = false;
    this->textFields_exists = false;
    removeWidgetFromLayout(boxLayout_filter, combobox_filter2);
    removeWidgetFromLayout(boxLayout_filter, textField1);
    removeWidgetFromLayout(boxLayout_filter, textField2);
    removeWidgetFromLayout(boxLayout_filter, label_filter_lower);
    removeWidgetFromLayout(boxLayout_filter, label_filter_upper);
  } else {
    return;
  } */

  std::string applied_filter = combobox_filter1->currentText().toStdString();
  if (applied_filter==" ")
  {
    combobox2_exists = false;
    textFields_exists = false;
    return;
  } else if (applied_filter=="Block")
  {
    combobox2_exists = true;
    textFields_exists = false;
    combobox_filter2->addItem(" ");
    std::vector<int> blocks = ccx_iface->get_blocks();
    for (size_t i = 1; i < blocks.size(); i++)
    {
      QString block_name = QString::fromStdString(ccx_iface->get_block_name(i));
      combobox_filter2->addItem(block_name);
    }
    boxLayout_filter->addWidget(combobox_filter2);
  } else if (applied_filter=="Sideset")
  {
    combobox2_exists = true;
    textFields_exists = false;
    combobox_filter2->addItem(" ");
    std::vector<int> sidesets = CubitInterface::get_sideset_id_list();
    for (size_t i = 1; i < sidesets.size(); i++)
    {
      QString sideset_name = QString::fromStdString(ccx_iface->get_sideset_name(i));
      combobox_filter2->addItem(sideset_name);
    }
    boxLayout_filter->addWidget(combobox_filter2);
  } else if (applied_filter=="Nodeset")
  {
    combobox2_exists = true;
    textFields_exists = false;
    combobox_filter2->addItem(" ");
    std::vector<int> nodesets = CubitInterface::get_nodeset_id_list();
    for (size_t i = 1; i < nodesets.size(); i++)
    {
      QString nodeset_name = QString::fromStdString(ccx_iface->get_nodeset_name(i));
      combobox_filter2->addItem(nodeset_name);
    }
    boxLayout_filter->addWidget(combobox_filter2);
  } else if (applied_filter=="Node")
  {
    combobox2_exists = false;
    textFields_exists = true;
    textField1 = new QSpinBox;
    textField2 = new QSpinBox;
    label_filter_lower = new QLabel;
    label_filter_lower->setText("From");
    label_filter_upper = new QLabel;
    label_filter_upper->setText("Until");
    boxLayout_filter->addWidget(label_filter_lower);
    boxLayout_filter->addWidget(textField1);
    boxLayout_filter->addWidget(label_filter_upper);
    boxLayout_filter->addWidget(textField2);
  }
}

void JobsMonitorFRD::update_result()
{
  std::string log;
  if(current_job_id == -1)
  {
    log = "Can't update results -> no job set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }
  if (this->current_result_block==nullptr)
  {
    log = "Can't update results -> no result block set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }
  if(this->current_result_component==nullptr)
  {
    log = "Can't update results -> no result component set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }

  if (current_increment==nullptr)
  {
    log = "Can't update results -> no increment set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }

  std::vector<int> nodes;
  std::vector<int> frd_nodes = ccx_iface->frd_get_nodes(current_job_id);
  table_result->setRowCount(0);
  table_result->setColumnCount(0);
/*
  std::string applied_filter = combobox_filter1->currentText().toStdString();
  
  if (applied_filter==" ")
  {
    //return;
  }
  if (this->combobox2_exists==true)
  {
    std::string applied_node = combobox_filter2->currentText().toStdString();
    if (applied_node==" ")
    {
      //return;
    }
    if (applied_filter=="Block")
    {
      std::string name = combobox_filter2->currentText().toStdString();
      int id = CubitInterface::get_id_from_name(name);
    } else if (applied_filter=="Sideset")
    {
      std::string name = combobox_filter2->currentText().toStdString();
      int id = CubitInterface::get_id_from_name(name);
      std::string int_list = "all in sideset " + std::to_string(id);
      nodes = CubitInterface::parse_cubit_list("node",int_list);
    } else if (applied_filter=="Nodeset")
    {
      std::string name = combobox_filter2->currentText().toStdString();
      int id = CubitInterface::get_id_from_name(name);
      nodes = CubitInterface::get_nodeset_nodes_inclusive(id);
    } else {
      //return;
    }
  }
  
  if (this->textFields_exists==true)
  {
    QString string_filter_lower = textField1->text();
    QString string_filter_upper = textField2->text();
    
    double filter_lower = string_filter_lower.toInt();
    double filter_upper = string_filter_upper.toInt();

    if ((!filter_lower==0)&&(!filter_upper==0))
    {
      for (size_t i = filter_lower; i <= filter_upper; i++)
      {
        nodes.push_back(i);
      }
    } else if ((filter_lower==0)&&(!filter_upper==0))
    {
      for (size_t i = 1; i <= filter_upper; i++)
      {
        nodes.push_back(i);
      }
    } else if ((!filter_lower==0)&&(filter_upper==0))
    {
      std::vector<int> total_nodes = CubitInterface::get_nodeset_id_list();
      if(!total_nodes.empty())
      {
        auto max_node = std::max_element(total_nodes.begin(), total_nodes.end());
        int node_max = *max_node;
        for (size_t i = filter_lower; i <= node_max; i++)
        {
          nodes.push_back(i);
        }
      } else {
        //return;
      }
    } else if ((filter_lower==0)&&(filter_upper==0))
    {
      std::vector<int> total_nodes = CubitInterface::get_nodeset_id_list();
      if(!total_nodes.empty())
      {
        auto max_node = std::max_element(total_nodes.begin(), total_nodes.end());
        int node_max = *max_node;
        for (size_t i = 1; i <= node_max; i++)
        {
          nodes.push_back(i);
        }
      } else {
        //return;
      }
    } else {
      //return;
    }
  }
*/
  if (nodes.size()==0) // this means no filter for sets was applied
  {
    nodes = frd_nodes;
  }
  
  //prepare components
  std::vector<std::string> components;
  if (current_result_component->text().toStdString()=="all")
  {
    components = ccx_iface->frd_get_result_block_components(current_job_id, current_result_block->text().toStdString());
  }else{
    components.push_back(current_result_component->text().toStdString());
  }

  //prepare increments
  std::vector<int> increments;
  if (current_increment->text().toStdString()=="all")
  {
    increments = ccx_iface->frd_get_total_increments(current_job_id);
  }else{
    increments.push_back(current_increment->text().toInt());
  }
  
  std::vector<std::vector<double>> results;
  std::string str_current_result_block = current_result_block->text().toStdString();
  int start = this->current_page * items_per_page;
  int end = start + this->items_per_page - 1;
  end = std::min(end, int(nodes.size()*increments.size()-1));
  this->results_size = nodes.size()*increments.size();

  if ((nodes.size()>0)&&(components.size()>0)&&(increments.size()>0)) //check if data can be queried
  {
    int ic = 0;
    for (size_t i = 0; i < nodes.size(); i++)
    {
      for (size_t ii = 0; ii < increments.size(); ii++)
      {
        if ((ic>=start)&&(ic<=end))// check if items are in range
        {
          std::vector<double> tmp_result;
          double increment_time = ccx_iface->frd_get_time_from_total_increment(current_job_id, increments[ii]);
          tmp_result.push_back(double(nodes[i]));
          tmp_result.push_back(double(increments[ii]));
          tmp_result.push_back(increment_time);
          for (size_t iii = 0; iii < components.size(); iii++)
          {
            double node_result = ccx_iface->frd_get_node_value(current_job_id, nodes[i] , increments[ii], str_current_result_block, components[iii]);
            tmp_result.push_back(node_result);
          }
          results.push_back(tmp_result);
        }
        ++ic;
      }
      if (ic>end)
      {
        break;
      }
    }
  }
  //log = "update result \n";
  //log.append("start " + std::to_string(start) + " end " + std::to_string(end) + "\n");
  //log.append("result size " + std::to_string(results.size()) + "\n");
  //PRINT_INFO("%s", log.c_str());
  /*
  if (current_result_component->text().toStdString()=="all")
  {

  } else {
    if (current_increment->text().toStdString()=="all")
    {
      std::vector<int> increments = ccx_iface->frd_get_total_increments(current_job_id);
      if (!increments.empty())
      {
        for (int i : nodes)
        {
          for (int j : increments)
          {
            results.push_back({double(i),double(j)});
          }
        }

        if (results.size()==0)
        {
          return;
        }

        for (size_t i = 0; i < results.size(); i++)
        {
          double increment_time = ccx_iface->frd_get_time_from_total_increment(current_job_id, results[i][0]);
          results[i].push_back(increment_time);

          double node_result = ccx_iface->frd_get_node_value(current_job_id, results[i][0] , current_increment->text().toInt(), current_result_block->text().toStdString(), current_result_component->text().toStdString());
          results[i].push_back(node_result);
        }
      } else {
        return;
      }
    } else {
      for (size_t i = 0; i < nodes.size(); i++)
      {
        std::vector<double> temp;
        temp.push_back(nodes[i]);
        results.push_back(temp);
      }

      if (results.size()==0)
      {
        return;
      }

      for (size_t i = 0; i < results.size(); i++)
      {
        double node_result = ccx_iface->frd_get_node_value(current_job_id, results[i][0] , current_increment->text().toInt(), current_result_block->text().toStdString(), current_result_component->text().toStdString());
        results[i].push_back(node_result);
      }
    }
  }
  */

  table_result->setSortingEnabled(true);
  table_result->setRowCount(std::min(50,int(results.size())));
  table_result->setColumnCount(results[0].size());
  table_result->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_counter->setText(QString::fromStdString("Results " + std::to_string(start+1) + "-" + std::to_string(end+1) + " of " + std::to_string(results_size)));

  //fill table
  for (size_t i = 0; i < results.size(); i++)
  {
    if (i==0) // label headers
    {
      QStringList header_horizontal;
      header_horizontal.push_back("Node ID");
      header_horizontal.push_back("Increment");
      header_horizontal.push_back("Time");
      for (size_t ii = 0; ii < components.size(); ii++)
      {
        header_horizontal.push_back(QString::fromStdString(components[ii]));
      }
      table_result->setHorizontalHeaderLabels(header_horizontal);
      
      QStringList header_vertical;
      for (size_t ii = start; ii < end+1; ii++)
      {
        header_vertical.push_back(QString::number(ii+1));
      }
      table_result->setVerticalHeaderLabels(header_vertical);
    }
    //push data
    for (size_t ii = 0; ii < results[i].size(); ii++)
    {
      if (ii > 2)
      {
        QString formatted_result_value = QString::number(results[i][ii], 'f', 6);
        QTableWidgetItem* item = new QTableWidgetItem(formatted_result_value);
        item->setData(Qt::DisplayRole, results[i][ii]);
        table_result->setItem(i, ii, item);
      } else {
        QTableWidgetItem* item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, results[i][ii]);
        table_result->setItem(i, ii, item);
      }
    }
  }
  

/*
  for (size_t i = start; i < end; i++)
  {
    for (size_t ii = 0; ii < results[0].size(); ii++)
    {
      if (current_increment->text().toStdString()=="all")
      {
        if (ii > 1)
        {
          QString formatted_result_value = QString::number(results[i][ii], 'f', 6);

          QTableWidgetItem* item = new QTableWidgetItem(formatted_result_value);
          item->setData(Qt::DisplayRole, results[i][ii]);
          table_result->setItem(i-start, ii, item);
        } else {
          QTableWidgetItem* item = new QTableWidgetItem;
          item->setData(Qt::DisplayRole, results[i][ii]);
          table_result->setItem(i-start, ii, item);
        }
        table_result->setHorizontalHeaderLabels(QStringList() << "Node Id" << "Increment Id" << "Increment Time" << current_result_component->text());
      } else {
        if (ii > 1)
        {
          QString formatted_result_value = QString::number(results[i][ii], 'f', 6);

          QTableWidgetItem* item = new QTableWidgetItem(formatted_result_value);
          item->setData(Qt::DisplayRole, results[i][ii]);
          table_result->setItem(i-start, ii, item);
        } else {
          QTableWidgetItem* item = new QTableWidgetItem;
          item->setData(Qt::DisplayRole, results[i][ii]);
          table_result->setItem(i-start, ii, item);
        }
        table_result->setHorizontalHeaderLabels(QStringList() << "Node Id" << current_result_component->text());
      }
    }
  }
  */
}

void JobsMonitorFRD::set_current_job_id(int job_id)
{
  this->current_job_id = job_id;
  //std::string log = "Current job id set is " + std::to_string(current_job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());
  this->update();
}

void JobsMonitorFRD::addListItem(std::string item_name, QListWidget* parent_list)
{
  QString name;
  name = QString::fromStdString(item_name);
  QListWidgetItem* new_list_item;
  new_list_item = new QListWidgetItem(name,parent_list);
}

void JobsMonitorFRD::addTableItem(std::string item_name, QTableWidget* parent_list)
{
  QString name;
  name = QString::fromStdString(item_name);
  QTableWidgetItem* new_list_item;
  new_list_item = new QTableWidgetItem(name);
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

void JobsMonitorFRD::on_pushButton_refresh_clicked(bool)
{
  this->update();  
}

void JobsMonitorFRD::on_pushButton_plot_clicked(bool)
{

}

void JobsMonitorFRD::on_pushButton_apply_filter_clicked(bool)
{ // Clemens, dies if not everything chosen
  if(current_job_id == -1)
  {
    return;
  }

  this->current_page = 0;
  //filter.clear();

  this->update_result();
}

void JobsMonitorFRD::result_block_clicked(QListWidgetItem* item)
{
 std::string result_block;
 result_block = item->text().toStdString();
 this->update_component(result_block);
 this->current_result_block = item;
}

void JobsMonitorFRD::result_block_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->result_block_clicked(current_item);
  }
}

void JobsMonitorFRD::component_clicked(QListWidgetItem* item)
{
  this->update_increment();
  this->current_result_component = item;
}

void JobsMonitorFRD::component_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->component_clicked(current_item);
  }
}

void JobsMonitorFRD::increment_clicked(QListWidgetItem* item)
{
  this->current_increment = item;
}

void JobsMonitorFRD::increment_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->increment_clicked(current_item);
  }
}

void JobsMonitorFRD::on_pushButton_previous_clicked(bool)
{
  if (current_page>0)
  {
    this->current_page = current_page-1;
    update_result();
  }
}

void JobsMonitorFRD::on_pushButton_next_clicked(bool)
{
  if (current_page < std::ceil(results_size/50))
  {
    current_page = current_page+1;
    update_result();
  }
}

void JobsMonitorFRD::removeWidgetFromLayout(QVBoxLayout* layout, QWidget* widget) {
  for (int i = 0; i < layout->count(); ++i) {
    QLayoutItem* item = layout->itemAt(i);
    if (item->widget() == widget) {
      layout->takeAt(i);
      delete item->widget();
      delete item;
      break;
    }
  }
}