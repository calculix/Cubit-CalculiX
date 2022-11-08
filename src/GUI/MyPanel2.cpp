#include "MyPanel2.hpp"
#include "ui_MyPanel2.h"

#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"

MyPanel2::MyPanel2(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MyPanel2)
{
  ui->setupUi(this);
}

MyPanel2::~MyPanel2()
{
  delete ui;
}

void MyPanel2::on_pbApply_clicked()
{
  QStringList commands;

  commands.push_back("reset");

  // Set some Aprepro variables based on the user's input
  commands.push_back("#{lead_rad=" + ui->leRadius->text() + "}");
  commands.push_back("#{length=" + ui->leLength->text() + "}");
  commands.push_back("#{width=" + ui->leWidth->text() + "}");
  commands.push_back("#{angle_of_attack=" + ui->leAngle->text() + "}");
  commands.push_back("#{trail_edge_loc = length - lead_rad}");

  // Create the airfoil profile
  commands.push_back("create curve arc radius {lead_rad} center location 0 0 0 "
                     "normal 0 0 1 start angle 90 stop angle 270");
  commands.push_back("create vertex {trail_edge_loc} 0 0");
  commands.push_back("create curve vertex 1 3");
  commands.push_back("create curve vertex 2 3");
  commands.push_back("create surface curve all");
  commands.push_back("rotate surface 1 angle {-angle_of_attack} about z");
  commands.push_back("create surface rectangle width {2*length}");
  commands.push_back("surface 2 move x {length/2}");
  commands.push_back("subtract surface 1 from surface 2");

  // Create a mesh for the profile
  if(ui->cbMesh->isChecked())
  {
    commands.push_back("curve 8 9 10 interval 20");
    commands.push_back("create boundary_layer 1");
    commands.push_back("modify boundary_layer 1 uniform height {lead_rad/15} growth 1.2 layers 5");
    commands.push_back("modify boundary_layer 1 add curve 8 9 10 surface 3");
    commands.push_back("modify boundary_layer 1 continuity on");
    commands.push_back("mesh surface all");
  }

  commands.push_back("sweep surface 3 perpendicular distance {width} include_mesh");

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
