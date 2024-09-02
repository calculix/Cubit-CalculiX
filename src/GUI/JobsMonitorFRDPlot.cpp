#include "JobsMonitorFRDPlot.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "PickWidget.hpp"

JobsMonitorFRDPlot::JobsMonitorFRDPlot()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  //this->setGeometry(0,0,700,570);
  this->setWindowTitle("FRD Results Plot");
  gridLayout = new QGridLayout(this);
  boxLayout_x = new QHBoxLayout();
  boxLayout_y = new QHBoxLayout();
  boxLayout_buttons = new QHBoxLayout();
  gridLayout->addLayout(boxLayout_x,0,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_y,1,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_buttons,2,0, Qt::AlignRight);
  
  label_x = new QLabel();
  label_x->setText("X Axis:");
  boxLayout_x->addWidget(label_x);
  label_y = new QLabel();
  label_y->setText("Y Axis:");
  boxLayout_y->addWidget(label_y);

  combobox_x_1 = new QComboBox();
  combobox_x_1->addItem("Node data");
  combobox_x_1->addItem("Increment");
  combobox_x_1->addItem("Time");

  combobox_y_1 = new QComboBox();
  combobox_y_1->addItem("Node data");
  combobox_y_1->addItem("Increment");
  combobox_y_1->addItem("Time");

  boxLayout_x->addWidget(combobox_x_1);
  boxLayout_y->addWidget(combobox_y_1);


  label_node_x = new QLabel();
  label_node_x->setText("Node ID");
  boxLayout_x->addWidget(label_node_x);
  label_node_y = new QLabel();
  label_node_y->setText("Node ID");
  boxLayout_y->addWidget(label_node_y);

  PickWidget_node_x = new PickWidget();
  PickWidget_node_x->setPickType(PickWidget::Node);
  PickWidget_node_x->activate();
  boxLayout_x->addWidget(PickWidget_node_x);
  PickWidget_node_y = new PickWidget();
  PickWidget_node_y->setPickType(PickWidget::Node);
  PickWidget_node_y->activate();
  boxLayout_y->addWidget(PickWidget_node_y);

  combobox_x_2 = new QComboBox();
  combobox_y_2 = new QComboBox();

  boxLayout_x->addWidget(combobox_x_2);
  boxLayout_y->addWidget(combobox_y_2);

  combobox_x_3 = new QComboBox();
  combobox_y_3 = new QComboBox();
  boxLayout_x->addWidget(combobox_x_3);
  boxLayout_y->addWidget(combobox_y_3);

  // buttons
  pushButton_reset = new QPushButton();
  pushButton_reset->setText("Reset");
  pushButton_plot = new QPushButton();
  pushButton_plot->setText("Plot");

  boxLayout_buttons->addWidget(pushButton_reset);
  boxLayout_buttons->addWidget(pushButton_plot);
  
  QObject::connect(pushButton_reset, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_reset_clicked(bool)));
  QObject::connect(pushButton_plot, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_plot_clicked(bool)));
  QObject::connect(combobox_x_1, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_x_1_index_changed(int)));
  QObject::connect(combobox_y_1, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_y_1_index_changed(int)));
  QObject::connect(combobox_x_2, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_x_2_index_changed(int)));
  QObject::connect(combobox_y_2, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_y_2_index_changed(int)));
  
  combobox_x_1->setCurrentIndex(2);
  combobox_y_1->setCurrentIndex(0);
}

JobsMonitorFRDPlot::~JobsMonitorFRDPlot()
{}

void JobsMonitorFRDPlot::reset()
{
  combobox_x_2->clear();
  combobox_y_2->clear();

  std::vector<std::string> result_block_types = ccx_iface->frd_get_result_block_types(current_job_id);
  
  for (size_t i = 0; i < result_block_types.size(); i++)
  {
    combobox_x_2->addItem(QString::fromStdString(result_block_types[i]));
    combobox_y_2->addItem(QString::fromStdString(result_block_types[i]));
  }

  combobox_x_1->setCurrentIndex(2);
  combobox_y_1->setCurrentIndex(0);
  PickWidget_node_x->setText("");
  PickWidget_node_y->setText("");
}

void JobsMonitorFRDPlot::combobox_x_1_index_changed(int index)
{
  if (combobox_x_1->currentIndex()!=0)
  {
    PickWidget_node_x->deactivate();
    PickWidget_node_x->setDisabled(true);
    combobox_x_2->setDisabled(true);
    combobox_x_3->setDisabled(true);
  }else{
    PickWidget_node_x->activate();
    PickWidget_node_x->setEnabled(true);
    combobox_x_2->setEnabled(true);
    combobox_x_3->setEnabled(true);
  }
}

void JobsMonitorFRDPlot::combobox_y_1_index_changed(int index)
{
  if (combobox_y_1->currentIndex()!=0)
  {
    PickWidget_node_y->deactivate();
    PickWidget_node_y->setDisabled(true);
    combobox_y_2->setDisabled(true);
    combobox_y_3->setDisabled(true);
  }else{
    PickWidget_node_y->activate();
    PickWidget_node_y->setEnabled(true);
    combobox_y_2->setEnabled(true);
    combobox_y_3->setEnabled(true);
  }
}

void JobsMonitorFRDPlot::combobox_x_2_index_changed(int index)
{
  combobox_x_3->clear();
  
  std::vector<std::string> components = ccx_iface->frd_get_result_block_components(current_job_id, combobox_x_2->currentText().toStdString());
  
  for (size_t i = 0; i < components.size(); i++)
  {
    combobox_x_3->addItem(QString::fromStdString(components[i]));
  }
}

void JobsMonitorFRDPlot::combobox_y_2_index_changed(int index)
{
  combobox_y_3->clear();
  
  std::vector<std::string> components = ccx_iface->frd_get_result_block_components(current_job_id, combobox_y_2->currentText().toStdString());
  
  for (size_t i = 0; i < components.size(); i++)
  {
    combobox_y_3->addItem(QString::fromStdString(components[i]));
  }
}

void JobsMonitorFRDPlot::on_pushButton_reset_clicked(bool)
{
  this->reset();  
}

void JobsMonitorFRDPlot::on_pushButton_plot_clicked(bool)
{
  bool push_cmd = true;
  std::string log = "";
  std::string cmd = "";
  cmd.append("ccx result plot job " + std::to_string(this->current_job_id) + " frd ");
  //ccx result plot job <job_id> frd {x_node_id <x_node_id> x_block_type <x_block_type> x_block_component <x_block_type>|x_increment|x_time} {y_node_id <y_node_id> y_block_type <y_block_type> y_block_component <y_block_type>|y_increment|y_time}[title <title>] [x_axis <x_axis>] [y_axis <y_axis>] [save <save_filepath>] 

  if (combobox_x_1->currentText()=="Node data")
  {
    int node_id_x = PickWidget_node_x->text().toInt();
    if (node_id_x > 0)
    {
      if (ccx_iface->frd_check_node_exists(current_job_id, node_id_x))
      {
        cmd.append("x_node_id " + std::to_string(node_id_x) + " ");
      }else{
        log = "Can't find node id " + std::to_string(node_id_x) + " in frd data -> reference points for example are not written into frd \n";
        PRINT_INFO("%s", log.c_str());
        push_cmd = false;
      }
    }else{
      push_cmd = false;
    }
    cmd.append("x_block_type " + combobox_x_2->currentText().toStdString() + " ");
    cmd.append("x_block_component " + combobox_x_3->currentText().toStdString() + " ");
  }else if (combobox_x_1->currentText()=="Increment")
  {
    cmd.append("x_increment ");
  }else if (combobox_x_1->currentText()=="Time")
  {
    cmd.append("x_time ");
  }
  
  if (combobox_y_1->currentText()=="Node data")
  {
    int node_id_y = PickWidget_node_y->text().toInt();
    if (node_id_y > 0)
    {
      if (ccx_iface->frd_check_node_exists(current_job_id, node_id_y))
      {
        cmd.append("y_node_id " + std::to_string(node_id_y) + " ");
      }else{
        log = "Can't find node id " + std::to_string(node_id_y) + " in frd data -> reference points for example are not written into frd \n";
        PRINT_INFO("%s", log.c_str());
        push_cmd = false;
      }
    }else{
      push_cmd = false;
    }
    cmd.append("y_block_type " + combobox_y_2->currentText().toStdString() + " ");
    cmd.append("y_block_component " + combobox_y_3->currentText().toStdString() + " ");
  }else if (combobox_y_1->currentText()=="Increment")
  {
    cmd.append("y_increment ");
  }else if (combobox_y_1->currentText()=="Time")
  {
    cmd.append("y_time ");
  }
  
  
 
  if (push_cmd)
  {
    ccx_iface->cmd(cmd);
  }
}