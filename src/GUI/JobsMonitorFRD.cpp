#include "JobsMonitorFRD.hpp"
#include "CalculiXCoreInterface.hpp"
#include "JobsMonitorFRDPlot.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "PickWidget.hpp"

JobsMonitorFRD::JobsMonitorFRD()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  //this->setGeometry(0,0,700,570);
  this->setWindowTitle("FRD Results");
  gridLayout = new QGridLayout(this);
  boxLayout_result_block = new QVBoxLayout();
  boxLayout_component = new QVBoxLayout();
  boxLayout_increment = new QVBoxLayout();
  boxLayout_filter =  new  QVBoxLayout() ;
  boxLayout_filter_by_set = new QVBoxLayout();
  boxLayout_filter_by_node = new QHBoxLayout();
  boxLayout_result_buttons = new QHBoxLayout();
  boxLayout_widget = new QVBoxLayout();
  boxLayout_pages = new QHBoxLayout();
  gridLayout->addLayout(boxLayout_result_block,0,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_component,0,1, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_increment,0,2, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_filter,0,3, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_widget,1,0,1,4, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_pages,2,0,1,4, Qt::AlignTop);

  // buttons
  pushButton_reset = new QPushButton();
  pushButton_reset->setText("Reset");
  pushButton_plot = new QPushButton();
  pushButton_plot->setText("Plot");
  pushButton_query_results = new QPushButton("Get Results");

  boxLayout_result_buttons->addWidget(pushButton_reset);
  boxLayout_result_buttons->addWidget(pushButton_query_results);
  boxLayout_result_buttons->addWidget(pushButton_plot);

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

  label_filter_by_set = new QLabel();
  label_filter_by_set->setText("Filter by Set");
  boxLayout_filter_by_set->addWidget(label_filter_by_set);

  label_filter_by_node = new QLabel();
  label_filter_by_node->setText("Filter by Node");
  label_filter_node_id = new QLabel();
  label_filter_node_id->setText("Node ID");
  boxLayout_filter_by_node->addWidget(label_filter_node_id);


  // lists
  list_result_block = new QListWidget();
  boxLayout_result_block->addWidget(list_result_block);

  list_component = new QListWidget();
  boxLayout_component->addWidget(list_component);

  list_increment = new QListWidget();
  list_increment->setFrameShape(QFrame::Box);
  list_increment->setFrameShadow(QFrame::Raised);
  boxLayout_increment->addWidget(list_increment);

  //filter
  combobox_filter_by_set = new QComboBox();
  combobox_filter_by_set->addItem("none");
  combobox_filter_by_set->addItem(ccx_iface->getIcon2("BlocksTree"),"Block");
  combobox_filter_by_set->addItem(ccx_iface->getIcon2("NodesetTree"),"Nodeset");
  combobox_filter_by_set->addItem(ccx_iface->getIcon2("SidesetTree"),"Sideset");
  boxLayout_filter_by_set->addWidget(combobox_filter_by_set);
  table_filter_by_set = new QTableWidget();
  table_filter_by_set->setLineWidth(1);
  table_filter_by_set->setMidLineWidth(0);
  table_filter_by_set->setFrameStyle(QFrame::Box | QFrame::Raised);
  
  boxLayout_filter_by_set->addWidget(table_filter_by_set);

  PickWidget_filter_node_id = new PickWidget();
  PickWidget_filter_node_id->setPickType(PickWidget::Node);
  PickWidget_filter_node_id->activate();
  boxLayout_filter_by_node->addWidget(PickWidget_filter_node_id);
  
  boxLayout_filter->addLayout(boxLayout_filter_by_set);
  boxLayout_filter->addWidget(label_filter_by_node);
  boxLayout_filter->addLayout(boxLayout_filter_by_node);
  boxLayout_filter->addLayout(boxLayout_result_buttons);

  //table
  table_result = new QTableWidget();
  table_result->setMinimumSize(700,350);
  table_result->setLineWidth(1);
  table_result->setMidLineWidth(0);
  table_result->setFrameStyle(QFrame::Box | QFrame::Raised);
  table_counter = new QLabel;
  table_counter->setText("");
  pushButton_prev = new QPushButton("Previous Results");
  pushButton_next = new QPushButton("Next Results");
  pushButton_prev->setDisabled(true);
  pushButton_next->setDisabled(true);
  boxLayout_widget->addWidget(table_result);
  boxLayout_pages->addWidget(pushButton_prev);
  boxLayout_pages->addWidget(table_counter);
  boxLayout_pages->addWidget(pushButton_next);

  PlotWidget = new JobsMonitorFRDPlot();
  PlotWidget->hide();

  // Signals
  QObject::connect(pushButton_reset, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_reset_clicked(bool)));
  QObject::connect(pushButton_plot, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_plot_clicked(bool)));
  QObject::connect(pushButton_query_results, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_query_results_clicked(bool)));
  QObject::connect(list_result_block, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(result_block_clicked(QListWidgetItem*)));
  QObject::connect(list_result_block, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(result_block_changed(QListWidgetItem*,QListWidgetItem*)));
  QObject::connect(list_component, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(component_clicked(QListWidgetItem*)));
  QObject::connect(list_component, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(component_changed(QListWidgetItem*,QListWidgetItem*)));
  QObject::connect(list_increment, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(increment_clicked(QListWidgetItem*)));
  QObject::connect(list_increment, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(increment_changed(QListWidgetItem*,QListWidgetItem*)));
  QObject::connect(pushButton_prev, SIGNAL(clicked(bool)), this, SLOT(on_pushButton_previous_clicked(bool)));
  QObject::connect(pushButton_next, SIGNAL(clicked(bool)), this, SLOT(on_pushButton_next_clicked(bool)));
  QObject::connect(combobox_filter_by_set, SIGNAL(currentIndexChanged(int)), this, SLOT(update_filter_by_set(int)));

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
  pushButton_prev->setDisabled(true);
  pushButton_next->setDisabled(true);
  table_counter->setText("");
  combobox_filter_by_set->setCurrentIndex(0);
  table_filter_by_set->setRowCount(0);
  table_filter_by_set->setColumnCount(0);
  PickWidget_filter_node_id->setText("");
    
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
  
  this->addListItem("all", list_increment);
  for (size_t i = 0; i < increments.size(); i++)
  {
    this->addListItem(std::to_string(increments[i]),list_increment);
  }
  
  list_increment->item(0)->setSelected(true);
  this->increment_clicked(list_increment->item(0));
}

void JobsMonitorFRD::update_filter_by_set(int index)
{
  std::string filter_set = combobox_filter_by_set->currentText().toStdString();

  table_filter_by_set->setRowCount(0);
  table_filter_by_set->setColumnCount(0);

  if (filter_set=="Block")
  {
    std::vector<int> block_ids = ccx_iface->get_blocks(); 
    table_filter_by_set->setRowCount(int(block_ids.size()));
    table_filter_by_set->setColumnCount(2);
  
    for (size_t i = 0; i < block_ids.size(); i++)
    {
      std::string name = ccx_iface->get_block_name(block_ids[i]);
      QTableWidgetItem* item_name = new QTableWidgetItem;
      item_name->setData(Qt::DisplayRole, QString::fromStdString(name));
      table_filter_by_set->setItem(int(i), 0, item_name);
      QTableWidgetItem* item_id = new QTableWidgetItem;
      item_id->setData(Qt::DisplayRole, QString::number(block_ids[i]));
      table_filter_by_set->setItem(int(i), 1, item_id);
    }
  }else if (filter_set=="Nodeset")
  {
    std::vector<int> nodeset_ids = CubitInterface::parse_cubit_list("nodeset","all"); 
    table_filter_by_set->setRowCount(int(nodeset_ids.size()));
    table_filter_by_set->setColumnCount(2);
  
    for (size_t i = 0; i < nodeset_ids.size(); i++)
    {
      std::string name = ccx_iface->get_nodeset_name(nodeset_ids[i]);
      QTableWidgetItem* item_name = new QTableWidgetItem;
      item_name->setData(Qt::DisplayRole, QString::fromStdString(name));
      table_filter_by_set->setItem(int(i), 0, item_name);
      QTableWidgetItem* item_id = new QTableWidgetItem;
      item_id->setData(Qt::DisplayRole, QString::number(nodeset_ids[i]));
      table_filter_by_set->setItem(int(i), 1, item_id);
    }
  }else if (filter_set=="Sideset")
  {
    std::vector<int> sideset_ids = CubitInterface::parse_cubit_list("sideset","all"); 
    table_filter_by_set->setRowCount(int(sideset_ids.size()));
    table_filter_by_set->setColumnCount(2);

    for (size_t i = 0; i < sideset_ids.size(); i++)
    {
      std::string name = ccx_iface->get_sideset_name(sideset_ids[i]);
      QTableWidgetItem* item_name = new QTableWidgetItem;
      item_name->setData(Qt::DisplayRole, QString::fromStdString(name));
      table_filter_by_set->setItem(int(i), 0, item_name);
      QTableWidgetItem* item_id = new QTableWidgetItem;
      item_id->setData(Qt::DisplayRole, QString::number(sideset_ids[i]));
      table_filter_by_set->setItem(int(i), 1, item_id);
    }
  }
  
  QStringList header_horizontal;
  header_horizontal.push_back("Name");
  header_horizontal.push_back("ID");
  table_filter_by_set->setHorizontalHeaderLabels(header_horizontal);
  table_filter_by_set->setSortingEnabled(true);
  table_filter_by_set->setEditTriggers(QAbstractItemView::NoEditTriggers);
  
      
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

  // check if filter was chosen
  std::string filter_set = combobox_filter_by_set->currentText().toStdString();
  QTableWidgetItem* filter_item = table_filter_by_set->currentItem();
  if (filter_item!=nullptr)
  {
    int row = filter_item->row();
    filter_item = table_filter_by_set->item(row,1);

    if (filter_set=="Block")
    {
      std::vector<int> node_ids = CubitInterface::parse_cubit_list("node","all in block " + filter_item->text().toStdString());; 
      for (size_t i = 0; i < node_ids.size(); i++)
      {
        if (ccx_iface->frd_check_node_exists(current_job_id, node_ids[i]))
        {
          nodes.push_back(node_ids[i]);
        }
      }
    }else if (filter_set=="Nodeset")
    {
      std::vector<int> node_ids = CubitInterface::parse_cubit_list("node","all in nodeset " + filter_item->text().toStdString());; 
      for (size_t i = 0; i < node_ids.size(); i++)
      {
        if (ccx_iface->frd_check_node_exists(current_job_id, node_ids[i]))
        {
          nodes.push_back(node_ids[i]);
        }
      }
    }else if (filter_set=="Sideset")
    {
      std::vector<int> node_ids = CubitInterface::parse_cubit_list("node","all in sideset " + filter_item->text().toStdString());; 
      for (size_t i = 0; i < node_ids.size(); i++)
      {
        if (ccx_iface->frd_check_node_exists(current_job_id, node_ids[i]))
        {
          nodes.push_back(node_ids[i]);
        }
      }
    }
  }
  
  int node_id = PickWidget_filter_node_id->text().toInt();
  if (node_id > 0)
  {
    if (ccx_iface->frd_check_node_exists(current_job_id, node_id))
    {
        nodes.push_back(node_id);
    }else
    {
      log = "Can't find node id " + std::to_string(node_id) + " in frd data -> reference points for example are not written into frd \n";
      PRINT_INFO("%s", log.c_str());
    }  
  }

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
  this->results_size = int(nodes.size()*increments.size());
  if (this->current_page==0)
  {
    pushButton_prev->setDisabled(true);
  }else{
    pushButton_prev->setEnabled(true);
  }
  if (end < int(nodes.size()*increments.size()-1))
  {
    pushButton_next->setEnabled(true);
  }else{
    pushButton_next->setDisabled(true);
  }
  
  

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
  table_result->setColumnCount(int(results[0].size()));
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
        table_result->setItem(int(i), int(ii), item);
      } else {
        QTableWidgetItem* item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, results[i][ii]);
        table_result->setItem(int(i), int(ii), item);
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
  PlotWidget->current_job_id = job_id;
  //std::string log = "Current job id set is " + std::to_string(current_job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());
  this->update();
  PlotWidget->reset();
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

void JobsMonitorFRD::on_pushButton_reset_clicked(bool)
{
  this->update();  
}

void JobsMonitorFRD::on_pushButton_plot_clicked(bool)
{
  PlotWidget->show();
}

void JobsMonitorFRD::on_pushButton_query_results_clicked(bool)
{ // Clemens, dies if not everything chosen
  if(current_job_id == -1)
  {
    return;
  }

  this->current_page = 0;

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