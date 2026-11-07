#include "Framework/Commands/UserOptionCommandSupport.hpp"
#include "UserOptionsPanel.hpp"
#include "UserOptions.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include <QIntValidator>
#include <QStringList>
#include <limits>
#include "DrawingOptionsWidget.hpp"

UserOptionsPanel::UserOptionsPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if (isInitialized)
  {
    return;
  }

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
  HBoxLayout_8 = new QHBoxLayout();
  HBoxLayout_9 = new QHBoxLayout();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_8 = new QLabel();
  label_9 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  label_8->setFixedWidth(labelWidth);
  label_9->setFixedWidth(labelWidth);
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();
  checkBox_8 = new QCheckBox();
  lineEdit_9 = new QLineEdit();
  lineEdit_6->setToolTip(tr("Changes to the icons directory take effect after restarting Cubit."));
  lineEdit_7->setToolTip(tr("Changes to the Python interface directory take effect after restarting Cubit."));
  lineEdit_9->setToolTip(tr("Changes to the material library take effect after restarting Cubit."));

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addLayout(HBoxLayout_4);
  VBoxLayout->addLayout(HBoxLayout_5);
  VBoxLayout->addLayout(HBoxLayout_6);
  VBoxLayout->addLayout(HBoxLayout_7);
  VBoxLayout->addLayout(HBoxLayout_8);
  VBoxLayout->addLayout(HBoxLayout_9);
  drawingOptions = new DrawingOptionsWidget(this);
  VBoxLayout->addWidget(drawingOptions);
  restartNotice = new QLabel(this);
  restartNotice->setWordWrap(true);
  restartNotice->hide();
  VBoxLayout->addWidget(restartNotice);
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
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(lineEdit_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(checkBox_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(lineEdit_9);

  lineEdit_2->setValidator(new QIntValidator(1, (std::numeric_limits<int>::max)(), lineEdit_2));
  lineEdit_3->setValidator(new QIntValidator(1, (std::numeric_limits<int>::max)(), lineEdit_3));
  isInitialized = true;
}

UserOptionsPanel::~UserOptionsPanel()
{}

void UserOptionsPanel::refresh_settings()
{
  const auto options = CalculiXApplicationInterface().user_options();
  label_1->setText(options.mPathSolverName);
  label_2->setText(options.mSolverThreadsName);
  label_3->setText(options.mConverterThreadsName);
  label_4->setText(options.mPathCGXName);
  label_5->setText(options.mPathParaViewName);
  label_6->setText(options.mPathIconsName);
  label_7->setText(options.mPathPythonInterfaceName);
  label_8->setText(options.mSaveLoadedResultsName);
  label_9->setText(options.mPathMaterialLibraryName);

  lineEdit_1->setText(options.mPathSolver);
  lineEdit_2->setText(QString::number(options.mSolverThreads));
  lineEdit_3->setText(QString::number(options.mConverterThreads));
  lineEdit_4->setText(options.mPathCGX);
  lineEdit_5->setText(options.mPathParaView);
  lineEdit_6->setText(options.mPathIcons);
  lineEdit_7->setText(options.mPathPythonInterface);
  checkBox_8->setChecked(options.mSaveLoadedResults);
  lineEdit_9->setText(options.mPathMaterialLibrary);
  drawingOptions->setOptions(options.mDrawing);
  QStringList pending;
  for (const auto& label : CalculiXApplicationInterface().user_options_restart_required())
  {
    pending.append(QString::fromStdString(label));
  }
  restartNotice->setText(pending.isEmpty() ? QString() : tr("Saved. Restart Cubit to apply: %1.").arg(pending.join(", ")));
  restartNotice->setVisible(!pending.isEmpty());
  baseline = option_values();
}

QJsonObject UserOptionsPanel::option_values() const
{
  QJsonObject values;
  values["paths/solver"] = lineEdit_1->text();
  values["execution/solver_threads"] = lineEdit_2->text().toInt();
  values["execution/converter_threads"] = lineEdit_3->text().toInt();
  values["paths/cgx"] = lineEdit_4->text();
  values["paths/paraview"] = lineEdit_5->text();
  values["paths/icons"] = lineEdit_6->text();
  values["paths/python_interface"] = lineEdit_7->text();
  values["results/save_loaded_results"] = checkBox_8->isChecked();
  values["paths/material_library"] = lineEdit_9->text();
  const auto drawing = drawingOptions->options();
  values["drawing/size_factor"] = drawing.sizeFactor;
  values["drawing/overlay_visible"] = drawing.overlayVisible;
  values["drawing/boundary_conditions"] = drawing.boundaryConditions;
  values["drawing/loads"] = drawing.loads;
  values["drawing/constraints"] = drawing.constraints;
  values["drawing/orientations"] = drawing.orientations;
  values["drawing/hover_color"] = QString::fromStdString(drawing.hoverColor);
  values["drawing/selection_color"] = QString::fromStdString(drawing.selectionColor);
  return values;
}

bool UserOptionsPanel::save_settings(std::string& error)
{
  error.clear();
  bool validSolver = false;
  bool validConverter = false;
  const int solverThreads = lineEdit_2->text().toInt(&validSolver);
  const int converterThreads = lineEdit_3->text().toInt(&validConverter);
  if (!validSolver || !validConverter || solverThreads < 1 || converterThreads < 1)
  {
    error = "Thread counts must be positive integers.";
    return false;
  }
  const auto edited = option_values();
  std::vector<std::pair<std::string, std::string>> commands;
  if (!UserOptionCommandSupport::edited_commands("ccx useroptions", CalculiXApplicationInterface().user_option_definitions(), edited, baseline, commands, error))
  {
    return false;
  }
  for (const auto& command : commands)
  {
    if (!CalculiXApplicationInterface().cmd(command.second))
    {
      error = "The user-options command failed; see the Cubit error message.";
      return false;
    }
    const auto key = QString::fromStdString(command.first);
    baseline[key] = edited.value(key);
  }
  refresh_settings();
  return true;
}
