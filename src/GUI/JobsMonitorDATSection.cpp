#include "JobsMonitorDATSection.hpp"
#include "CalculiXCoreInterface.hpp"
#include "JobsMonitorDATSectionPlot.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "PickWidget.hpp"

JobsMonitorDATSection::JobsMonitorDATSection()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  //this->setGeometry(0,0,700,570);
  this->setWindowTitle("DAT Section Results");
  gridLayout = new QGridLayout(this);
  boxLayout_result_set = new QVBoxLayout();
  boxLayout_time = new QVBoxLayout();
  boxLayout_result_buttons = new QHBoxLayout();
  boxLayout_widget = new QVBoxLayout();
  boxLayout_pages = new QHBoxLayout();
  gridLayout->addLayout(boxLayout_result_set,0,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_time,0,1, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_widget,1,0,1,2, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_pages,2,0,1,2, Qt::AlignTop);

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
  label_result_set = new QLabel();
  label_result_set->setText("Result Sets");
  boxLayout_result_set->addWidget(label_result_set);

  label_time = new QLabel();
  label_time->setText("Times");
  boxLayout_time->addWidget(label_time);

  // lists
  list_result_set = new QListWidget();
  boxLayout_result_set->addWidget(list_result_set);

  list_time = new QListWidget();
  boxLayout_time->addWidget(list_time);

  boxLayout_time->addLayout(boxLayout_result_buttons);

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
  pushButton_export = new QPushButton("Export to CSV");
  boxLayout_widget->addWidget(table_result);
  boxLayout_pages->addWidget(pushButton_prev);
  boxLayout_pages->addWidget(table_counter);
  boxLayout_pages->addWidget(pushButton_next);
  boxLayout_pages->addWidget(pushButton_export);
  

  PlotWidget = new JobsMonitorDATSectionPlot();
  PlotWidget->hide();

  // Signals
  QObject::connect(pushButton_reset, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_reset_clicked(bool)));
  QObject::connect(pushButton_plot, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_plot_clicked(bool)));
  QObject::connect(pushButton_query_results, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_query_results_clicked(bool)));
  QObject::connect(list_result_set, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(result_set_clicked(QListWidgetItem*)));
  QObject::connect(list_result_set, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(result_set_changed(QListWidgetItem*,QListWidgetItem*)));
  QObject::connect(list_time, SIGNAL(itemClicked(QListWidgetItem*)),this,  SLOT(time_clicked(QListWidgetItem*)));
  QObject::connect(list_time, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,  SLOT(time_changed(QListWidgetItem*,QListWidgetItem*)));
  QObject::connect(pushButton_prev, SIGNAL(clicked(bool)), this, SLOT(on_pushButton_previous_clicked(bool)));
  QObject::connect(pushButton_next, SIGNAL(clicked(bool)), this, SLOT(on_pushButton_next_clicked(bool)));
  QObject::connect(pushButton_export, SIGNAL(clicked(bool)),this, SLOT(on_pushButton_export_clicked(bool)));

  // Update list items and data
  this->update();
}

JobsMonitorDATSection::~JobsMonitorDATSection()
{}

void JobsMonitorDATSection::clear()
{
}

void JobsMonitorDATSection::update()
{
  if (current_job_id ==-1)
  {
    return;
  }

  list_result_set->clear();
  list_time->clear();
  table_result->setRowCount(0);
  table_result->setColumnCount(0);
  pushButton_prev->setDisabled(true);
  pushButton_next->setDisabled(true);
  pushButton_export->setDisabled(true);
  table_counter->setText("");
    
  std::vector<std::string> result_section_sets = ccx_iface->dat_get_section_set(current_job_id);
  
  if (result_section_sets.size() == 0)
  {
    return;
  }

  for (size_t i = 0; i < result_section_sets.size(); i++)
  {
    this->addListItem(result_section_sets[i],list_result_set);
  }
  
  list_result_set->item(0)->setSelected(true);
  this->result_set_clicked(list_result_set->item(0));
}

void JobsMonitorDATSection::update_time()
{
  if (current_job_id ==-1)
  {
    return;
  }
  if (current_result_set==nullptr)
  {
    return;
  }
  
  list_time->clear();
  this->current_time = nullptr;
  std::string result_set;
  result_set = current_result_set->text().toStdString();

  std::vector<std::vector<double>> section_data = ccx_iface->dat_get_section_data(current_job_id,result_set);
  std::vector<double> times;

  for (size_t i = 0; i < section_data.size(); i++)
  {
    times.push_back(section_data[i][0]);
  }
  
  if (times.size()>0)
  {
    this->addListItem("all", list_time);
    for (size_t i = 0; i < times.size(); i++)
    {
      this->addListItem(std::to_string(times[i]),list_time);
    }
    
    list_time->item(0)->setSelected(true);
    this->time_clicked(list_time->item(0));
  }  
}


void JobsMonitorDATSection::update_result()
{
  std::string log;

  table_result->setRowCount(0);
  table_result->setColumnCount(0);

  if(current_job_id == -1)
  {
    log = "Can't update results -> no job set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }
  if (this->current_result_set==nullptr)
  {
    log = "Can't update results -> no result_set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }
  if (current_time==nullptr)
  {
    log = "Can't update results -> no increment set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }

  // get results
  std::string str_current_result_set = current_result_set->text().toStdString();
  std::vector<std::vector<double>> section_data = ccx_iface->dat_get_section_data(current_job_id,str_current_result_set);

  //prepare components
  std::vector<std::string> components;
  components = ccx_iface->dat_get_section_label(current_job_id);

  //prepare times
  std::vector<double> times;
  if (current_time->text().toStdString()=="all")
  {
    for (size_t i = 0; i < section_data.size(); i++)
    {
      times.push_back(section_data[i][0]);
    }
  }else{
    times.push_back(current_time->text().toDouble());
  }

  //filter results by time
  std::vector<std::vector<double>> section_results;
  
  if ((section_data.size()>0)&&(components.size()>0)&&(times.size()>0)) //check if data can be queried
  {
    bool bool_time = false;
    for (size_t i = 0; i < section_data.size(); i++)
    {
      for (size_t ii = 0; ii < times.size(); ii++)
      {
        if (section_data[i][0]==times[ii])
        {
          bool_time = true;
          break;
        }
      }

      if (bool_time)// check if items are in range
      {
        section_results.push_back(section_data[i]);
      }
      bool_time = false;
    }
  }

  int start = 0;
  int end = 0;

  if (section_results.size()>0) // section results
  {
    start = this->current_page * items_per_page;
    end = start + this->items_per_page - 1;
    end = std::min(end, int(section_results.size()-1));
    this->results_size = int(section_results.size());
    if (this->current_page==0)
    {
      pushButton_prev->setDisabled(true);
    }else{
      pushButton_prev->setEnabled(true);
    }
    if (end < int(section_results.size()-1))
    {
      pushButton_next->setEnabled(true);
    }else{
      pushButton_next->setDisabled(true);
    }
    pushButton_export->setEnabled(true);
  }

  // filter per range
  std::vector<std::vector<double>> section_results_range;
  if ((section_results.size()>0)&&(components.size()>0)&&(times.size()>0))
  {
    int ic = 0;
    for (size_t i = 0; i < section_results.size(); i++)
    {
      if ((ic>=start)&&(ic<=end))// check if items are in range
      {
        section_results_range.push_back(section_results[ic]);
        //std::string log = "range " + std::to_string(ic) + " start " + std::to_string(start) + " end " + std::to_string(end) + " results size " + std::to_string(section_results.size()) + "\n";
        //PRINT_INFO("%s", log.c_str());
      }
      ++ic;

      if (ic>end)
      {
        break;
      }
    }
  }

  table_result->setSortingEnabled(true);
  if (section_results_range.size()>0)
  {
    table_result->setRowCount(std::min(50,int(section_results_range.size())));
    table_result->setColumnCount(int(section_results_range[0].size()));    
  }
  table_result->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table_counter->setText(QString::fromStdString("Results " + std::to_string(start+1) + "-" + std::to_string(end+1) + " of " + std::to_string(results_size)));

  //fill table
  for (size_t i = 0; i < section_results_range.size(); i++)
  {
    if (i==0) // label headers
    {
      QStringList header_horizontal;
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
    for (size_t ii = 0; ii < section_results_range[i].size(); ii++)
    {
      if (ii > 2)
      {
        QString formatted_result_value = QString::number(section_results_range[i][ii], 'f', 6);
        QTableWidgetItem* item = new QTableWidgetItem(formatted_result_value);
        item->setData(Qt::DisplayRole, section_results_range[i][ii]);
        table_result->setItem(int(i), int(ii), item);
      } else {
        QTableWidgetItem* item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole, section_results_range[i][ii]);
        table_result->setItem(int(i), int(ii), item);
      }
    }
  }
}

void JobsMonitorDATSection::set_current_job_id(int job_id)
{
  this->current_job_id = job_id;
  PlotWidget->current_job_id = job_id;
  //std::string log = "Current job id set is " + std::to_string(current_job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());
  this->update();
  PlotWidget->reset();
}

void JobsMonitorDATSection::addListItem(std::string item_name, QListWidget* parent_list)
{
  QString name;
  name = QString::fromStdString(item_name);
  QListWidgetItem* new_list_item;
  new_list_item = new QListWidgetItem(name,parent_list);
}

void JobsMonitorDATSection::addTableItem(std::string item_name, QTableWidget* parent_list)
{
  QString name;
  name = QString::fromStdString(item_name);
  QTableWidgetItem* new_list_item;
  new_list_item = new QTableWidgetItem(name);
}

void JobsMonitorDATSection::on_pushButton_reset_clicked(bool)
{
  this->update();  
}

void JobsMonitorDATSection::on_pushButton_plot_clicked(bool)
{
  PlotWidget->show();
}

void JobsMonitorDATSection::on_pushButton_query_results_clicked(bool)
{ // Clemens, dies if not everything chosen
  if(current_job_id == -1)
  {
    return;
  }

  this->current_page = 0;

  this->update_result();
}

void JobsMonitorDATSection::result_set_clicked(QListWidgetItem* item)
{
  this->current_result_set = item;
  this->update_time();
}

void JobsMonitorDATSection::result_set_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->result_set_clicked(current_item);
  }
}

void JobsMonitorDATSection::time_clicked(QListWidgetItem* item)
{
  this->current_time = item;
}

void JobsMonitorDATSection::time_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item)
{
  if (current_item!=nullptr)
  {
    this->time_clicked(current_item);
  }
}

void JobsMonitorDATSection::on_pushButton_previous_clicked(bool)
{
  if (current_page>0)
  {
    this->current_page = current_page-1;
    update_result();
  }
}

void JobsMonitorDATSection::on_pushButton_next_clicked(bool)
{
  if (current_page < std::ceil(results_size/50))
  {
    current_page = current_page+1;
    update_result();
  }
}

void JobsMonitorDATSection::on_pushButton_export_clicked(bool)
{
  int node_id=-1;
  int element_id=-1;
  std::string block_set="";
  std::string time = "";
  bool overwrite = false;
  std::string save_filepath;

  std::string log;
  if(current_job_id == -1)
  {
    log = "Can't export results -> no job set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }
  if (this->current_result_set==nullptr)
  {
    log = "Can't export results -> no result_set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }
  if (current_time==nullptr)
  {
    log = "Can't export results -> no increment set \n";
    PRINT_INFO("%s", log.c_str());
    return;
  }
  
  block_set = current_result_set->text().toStdString();
  time = current_time->text().toStdString();

  QString fileName;

  QFileDialog dialog;
  dialog.setWindowTitle("Save CSV");
  dialog.setDirectory(QDir::homePath());
  dialog.setNameFilter("CSV Files (*.csv);;All Files (*)");
  fileName = dialog.getSaveFileName();

  if(fileName.isEmpty())
  {
    log = "File save canceled.\n";
    PRINT_INFO("%s", log.c_str());
    return;
  }

  if (!fileName.endsWith(".csv", Qt::CaseInsensitive)) 
  {
    fileName += ".csv";
  }

  if (QFileInfo::exists(fileName)) 
  {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "File Exists", "The file already exists. Do you want to overwrite it?", QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
      overwrite = true;
    }
  }

  std::string cmd = "";
  cmd.append("ccx result csv job " + std::to_string(this->current_job_id) + " dat section ");
  cmd.append("block_set \'" + block_set + "\' ");
  cmd.append("inc_time \'" + time + "\' ");
  cmd.append("save \'" + fileName.toStdString() + "\' ");
  if (overwrite)
  {
    cmd.append("overwrite ");
  }
 
  ccx_iface->cmd(cmd);

  return;
}