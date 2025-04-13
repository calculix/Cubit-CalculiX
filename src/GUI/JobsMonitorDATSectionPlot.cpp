#include "JobsMonitorDATSectionPlot.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "PickWidget.hpp"

JobsMonitorDATSectionPlot::JobsMonitorDATSectionPlot()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  //this->setGeometry(0,0,700,570);
  this->setWindowTitle("DAT Section Results Plot");
  int comboBoxWidth = 120;
  gridLayout = new QGridLayout(this);
  boxLayout_x = new QHBoxLayout();
  boxLayout_y = new QHBoxLayout();
  boxLayout_labels = new QVBoxLayout();
  boxLayout_buttons = new QHBoxLayout();
  gridLayout->addLayout(boxLayout_x,0,0, Qt::AlignLeft);
  gridLayout->addLayout(boxLayout_y,1,0, Qt::AlignLeft);
  gridLayout->addLayout(boxLayout_labels,2,0, Qt::AlignTop);
  gridLayout->addLayout(boxLayout_buttons,3,0, Qt::AlignRight);
  horizontal_spacer_x = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  horizontal_spacer_y = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  label_x = new QLabel();
  label_x->setText("X Axis:");
  boxLayout_x->addWidget(label_x);
  label_y = new QLabel();
  label_y->setText("Y Axis:");
  boxLayout_y->addWidget(label_y);

  combobox_x_1 = new QComboBox();
  combobox_x_1->setFixedWidth(comboBoxWidth);
  combobox_x_1->addItem("Section data");
  combobox_x_1->addItem("Time");

  combobox_y_1 = new QComboBox();
  combobox_y_1->setFixedWidth(comboBoxWidth);
  combobox_y_1->addItem("Section data");
  combobox_y_1->addItem("Time");

  boxLayout_x->addWidget(combobox_x_1);
  boxLayout_y->addWidget(combobox_y_1);

  combobox_x_2 = new QComboBox();
  combobox_x_2->setFixedWidth(comboBoxWidth);
  combobox_y_2 = new QComboBox();
  combobox_y_2->setFixedWidth(comboBoxWidth);

  boxLayout_x->addWidget(combobox_x_2);
  boxLayout_y->addWidget(combobox_y_2);

  combobox_x_3 = new QComboBox();
  combobox_x_3->setFixedWidth(comboBoxWidth);
  combobox_y_3 = new QComboBox();
  combobox_y_3->setFixedWidth(comboBoxWidth);
  boxLayout_x->addWidget(combobox_x_3);
  boxLayout_y->addWidget(combobox_y_3);

  boxLayout_x->addItem(horizontal_spacer_x);
  boxLayout_y->addItem(horizontal_spacer_y);

  //custom labels and save path
  label_title = new QLabel();
  label_title->setText("Label Title");
  label_x_axis = new QLabel();
  label_x_axis->setText("Label X Axis");
  label_y_axis = new QLabel();
  label_y_axis->setText("Label Y Axis");
  label_save_path = new QLabel();
  label_save_path->setText("Save Plot to File");
  lineEdit_title = new QLineEdit();
  lineEdit_x_axis = new QLineEdit();
  lineEdit_y_axis = new QLineEdit();
  lineEdit_save_path = new QLineEdit();
  lineEdit_save_path->setPlaceholderText("Plot.png");
  boxLayout_labels->addWidget(label_title);
  boxLayout_labels->addWidget(lineEdit_title);
  boxLayout_labels->addWidget(label_x_axis);
  boxLayout_labels->addWidget(lineEdit_x_axis);
  boxLayout_labels->addWidget(label_y_axis);
  boxLayout_labels->addWidget(lineEdit_y_axis);
  boxLayout_labels->addWidget(label_save_path);
  boxLayout_labels->addWidget(lineEdit_save_path);

  // buttons
  pushButton_reset = new QPushButton();
  pushButton_reset->setText("Reset");
  pushButton_plot = new QPushButton();
  pushButton_plot->setText("Plot");
  pushButton_close = new QPushButton();
  pushButton_close->setText("Close");
  
  boxLayout_buttons->addWidget(pushButton_plot);
  boxLayout_buttons->addWidget(pushButton_reset);
  boxLayout_buttons->addWidget(pushButton_close);
  
  QObject::connect(pushButton_reset, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_reset_clicked(bool)));
  QObject::connect(pushButton_plot, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_plot_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_close_clicked(bool)));
  QObject::connect(combobox_x_1, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_x_1_index_changed(int)));  
  QObject::connect(combobox_x_2, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_x_3_index_changed(int)));
  QObject::connect(combobox_y_1, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_y_1_index_changed(int)));
  QObject::connect(combobox_y_2, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_y_3_index_changed(int)));
  
  combobox_x_1->setCurrentIndex(1);
  combobox_y_1->setCurrentIndex(0);
  this->combobox_x_1_index_changed(1);
  this->combobox_y_1_index_changed(0);
}

JobsMonitorDATSectionPlot::~JobsMonitorDATSectionPlot()
{}

void JobsMonitorDATSectionPlot::reset()
{
  combobox_x_2->clear();
  combobox_y_2->clear();
  combobox_x_3->clear();
  combobox_y_3->clear();

  std::vector<std::string> section_sets = ccx_iface->dat_get_section_set(current_job_id);
  for (size_t i = 0; i < section_sets.size(); i++)
  {
    combobox_x_2->addItem(QString::fromStdString(section_sets[i]));
    combobox_y_2->addItem(QString::fromStdString(section_sets[i]));
  }

  std::vector<std::string> section_label = ccx_iface->dat_get_section_label(current_job_id);
  for (size_t i = 0; i < section_label.size(); i++)
  {
    combobox_x_3->addItem(QString::fromStdString(section_label[i]));
    combobox_y_3->addItem(QString::fromStdString(section_label[i]));
  }

  combobox_x_1->setCurrentIndex(1);
  combobox_y_1->setCurrentIndex(0);
  lineEdit_title->setText("");
  lineEdit_x_axis->setText("");
  lineEdit_y_axis->setText("");
  lineEdit_save_path->setText("");
}

void JobsMonitorDATSectionPlot::combobox_x_1_index_changed(int index)
{
  if (combobox_x_1->currentIndex()==0)
  {
    combobox_x_2->setEnabled(true);
    combobox_x_3->setEnabled(true);
  }else{
    combobox_x_2->setDisabled(true);
    combobox_x_3->setDisabled(true);
    }
}

void JobsMonitorDATSectionPlot::combobox_y_1_index_changed(int index)
{
  if (combobox_y_1->currentIndex()==0)
  {
    combobox_y_2->setEnabled(true);
    combobox_y_3->setEnabled(true);
  }else{
    combobox_y_2->setDisabled(true);
    combobox_y_3->setDisabled(true);
  }
}

void JobsMonitorDATSectionPlot::combobox_x_2_index_changed(int index)
{
  combobox_x_3->clear();
  
  std::vector<std::string> components = ccx_iface->dat_get_section_label(current_job_id);
  
  for (size_t i = 0; i < components.size(); i++)
  {
    combobox_x_3->addItem(QString::fromStdString(components[i]));
  }
}

void JobsMonitorDATSectionPlot::combobox_y_2_index_changed(int index)
{
  combobox_y_3->clear();
  
  std::vector<std::string> components = ccx_iface->dat_get_section_label(current_job_id);
  
  for (size_t i = 0; i < components.size(); i++)
  {
    combobox_y_3->addItem(QString::fromStdString(components[i]));
  }
}

void JobsMonitorDATSectionPlot::on_pushButton_reset_clicked(bool)
{
  this->reset();
}

void JobsMonitorDATSectionPlot::on_pushButton_plot_clicked(bool)
{
  bool push_cmd = true;
  std::string log = "";
  std::string cmd = "";
  cmd.append("ccx result plot job " + std::to_string(this->current_job_id) + " dat section ");

  if (combobox_x_1->currentText()=="Section data")
  {
    cmd.append("x_section_set \'" + combobox_x_2->currentText().toStdString() + "\' ");
    cmd.append("x_section_component \'" + combobox_x_3->currentText().toStdString() + "\' ");
  }else if (combobox_x_1->currentText()=="Time")
  {
    cmd.append("x_time ");
  }
  
  if (combobox_y_1->currentText()=="Section data")
  {
    cmd.append("y_section_set \'" + combobox_y_2->currentText().toStdString() + "\' ");
    cmd.append("y_section_component \'" + combobox_y_3->currentText().toStdString() + "\' ");
  }else if (combobox_y_1->currentText()=="Time")
  {
    cmd.append("y_time ");
  }
  
  if (lineEdit_title->text()!="")
  {
    cmd.append("title \'" + lineEdit_title->text().toStdString() + "\' ");
  }
  if (lineEdit_x_axis->text()!="")
  {
    cmd.append("x_axis \'" + lineEdit_x_axis->text().toStdString() + "\' ");
  }
  if (lineEdit_y_axis->text()!="")
  {
    cmd.append("y_axis \'" + lineEdit_y_axis->text().toStdString() + "\' ");
  }
  if (lineEdit_save_path->text()!="")
  {
    if (".png" == lineEdit_save_path->text().toStdString().substr(lineEdit_save_path->text().toStdString().size()-4))
    {
      cmd.append("save \'" + lineEdit_save_path->text().toStdString() + "\' ");
    }else{
      cmd.append("save \'" + lineEdit_save_path->text().toStdString() + ".png\' ");
    }
  }
 
  if (push_cmd)
  {
    ccx_iface->cmd(cmd);
  }
}

void JobsMonitorDATSectionPlot::on_pushButton_close_clicked(bool)
{
  this->close();
}