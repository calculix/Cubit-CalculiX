#include "UserOptionsPanel.hpp"
#include "loadUserOptions.hpp"


UserOptionsPanel::UserOptionsPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  
  int labelWidth = 240;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  HBoxLayout_4 = new QHBoxLayout();
  HBoxLayout_5 = new QHBoxLayout();
  HBoxLayout_6 = new QHBoxLayout();
  HBoxLayout_7 = new QHBoxLayout();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addLayout(HBoxLayout_4);
  VBoxLayout->addLayout(HBoxLayout_5);
  VBoxLayout->addLayout(HBoxLayout_6);
  VBoxLayout->addLayout(HBoxLayout_7);
  VBoxLayout->addItem(vertical_spacer);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(lineEdit_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(lineEdit_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_6->addWidget(label_7);
  HBoxLayout_6->addWidget(lineEdit_7);

  isInitialized = true;
}

UserOptionsPanel::~UserOptionsPanel()
{}

void UserOptionsPanel::refresh_settings()
{
  label_1->setText(ccx_uo.mPathSolverName);
  label_2->setText(ccx_uo.mSolverThreadsName);
  label_3->setText(ccx_uo.mConverterThreadsName);
  label_4->setText(ccx_uo.mPathCGXName);
  label_5->setText(ccx_uo.mPathParaViewName);
  label_6->setText(ccx_uo.mPathIconsName);
  label_7->setText(ccx_uo.mPathPythonInterfaceName);

  lineEdit_1->setText(ccx_uo.mPathSolver);
  lineEdit_2->setText(QString::number(ccx_uo.mSolverThreads));
  lineEdit_3->setText(QString::number(ccx_uo.mConverterThreads));
  lineEdit_4->setText(ccx_uo.mPathCGX);
  lineEdit_5->setText(ccx_uo.mPathParaView);
  lineEdit_6->setText(ccx_uo.mPathIcons);
  lineEdit_7->setText(ccx_uo.mPathPythonInterface);
}

void UserOptionsPanel::save_settings()
{
  ccx_uo.mPathSolver = lineEdit_1->text();
  ccx_uo.mSolverThreads = lineEdit_2->text().toInt();
  ccx_uo.mConverterThreads = lineEdit_3->text().toInt();
  ccx_uo.mPathCGX = lineEdit_4->text();
  ccx_uo.mPathParaView = lineEdit_5->text();
  ccx_uo.mPathIcons = lineEdit_6->text();
  ccx_uo.mPathPythonInterface = lineEdit_7->text();
}
