#include "Core/results/vtk/CoreDiscreteElementHistory.hpp"
#include "Core/results/vtk/CoreResultsInpReader.hpp"
#include "Core/results/vtk/CoreResultsVtkLink.hpp"
#include <QJsonDocument>
#include <QTemporaryDir>
#include "Core/connections/CoreMovingJointExport.hpp"
#include "Core/connections/CoreConnections.hpp"
#include "Core/export/CoreExportNodeAllocator.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

namespace
{
bool check(bool condition, const std::string& message)
{
  if (!condition)
  {
    std::cerr << message << '\n';
  }
  return condition;
}

std::string marker(int id, int element, const QString& type)
{
  QJsonObject data{{"id",id},{"element",element},{"name",type},{"type",type},
    {"a",1},{"b",type == "SPRINGA" ? 2 : 0},{"direction",type == "SPRING1" ? 2 : 0},
    {"value",200.0},{"position_a",QJsonArray{0.,0.,0.}}};
  if (type == "SPRINGA")
  {
    data.insert("position_b",QJsonArray{1.,0.,0.});
  }
  return "** CUBIT_SIM_DISCRETE " + QJsonDocument(data).toJson(QJsonDocument::Compact).toBase64().toStdString() + "\n";
}

bool moving_joint(const std::string& filename, const std::string& type)
{
  CoreMovingJointExport writer;
  CoreExportNodeAllocator allocator;
  std::ostringstream nodes, a, b, springs, constraints;
  nodes.precision(17);
  std::string error;
  const CoreMovingJointExport::Position origin{}, axis{1,0,0};
  ConnectionJoint joint{1,"Bearing",type,1,2,axis};
  std::set<std::pair<int,int>> dependent;
  QJsonObject binding{{"id",1},{"name","Bearing"},{"type",QString::fromStdString(type)},
    {"a",1},{"b",2},{"axis",QJsonArray{1,0,0}},{"history",QJsonArray{1}}};
  if (!writer.add_markers(true,origin,axis,2.,allocator,nodes,b,error) ||
      !writer.add_markers(false,origin,axis,1.,allocator,nodes,a,error) ||
      !writer.append(joint,101,103,allocator,nodes,springs,constraints,dependent,binding,error))
  {
    return check(false,error);
  }
  std::ofstream out(filename);
  out << nodes.str() << "*NODE\n101,0,0,0\n102,0,0,0\n103,0,0,0\n104,0,0,0\n";
  out << "*NODE\n201,0,0,0\n202,1,0,0\n203,0,1,0\n204,0,0,1\n"
      << "*ELEMENT,TYPE=C3D4,ELSET=physical\n100,201,202,203,204\n" << springs.str();
  for (int id : {1,2})
  {
    const int ref = 99+2*id;
    out << "*NSET,NSET=CCX_RB_" << id << "_MEMBERS\n" << (id == 1 ? a.str() : b.str())
        << "*RIGID BODY,NSET=CCX_RB_" << id << "_MEMBERS,REF NODE=" << ref << ",ROT NODE=" << ref+1 << "\n"
        << "*NSET,NSET=CCX_RP_" << id << "_REF\n" << ref << "\n*NSET,NSET=CCX_RP_" << id << "_ROT\n" << ref+1 << "\n";
    QJsonObject rp{{"id",id},{"name","RP"},{"ref",ref},{"rot",ref+1},{"position",QJsonArray{0,0,0}},
      {"body",id},{"body_name","Body"},{"generated_ref",true},{"generated_rot",true},{"history",QJsonArray{1}}};
    out << "** CUBIT_SIM_RP " << QJsonDocument(rp).toJson(QJsonDocument::Compact).toBase64().toStdString() << "\n";
  }
  out << constraints.str() << "** CUBIT_SIM_JOINT " << QJsonDocument(binding).toJson(QJsonDocument::Compact).toBase64().toStdString()
      << "\n*ELSET,ELSET=MIXED\n100,1\n";
  out.close();
  CoreResultsVtkMesh mesh;
  ConnectionRunMap restored;
  std::vector<std::string> files;
  if (!CoreResultsInpReader().read(filename,mesh,files,error) || !restored.restore(mesh.connections.json(),error))
  {
    return check(false,error);
  }
  const auto count = type == "revolute" ? 12u : 24u;
  if (!check(restored.joint_elements.size() == count && restored.discrete.empty() &&
      mesh.elements.size() == 1 && mesh.elements.front().id == 100,"Joint helpers leaked into physical elements"))
  {
    return false;
  }
  auto observed = mesh;
  observed.connections = {};
  for (const auto& entry : restored.joint_elements)
  {
    observed.elements.push_back({entry.first,11,{},{entry.second[0],entry.second[1]}});
  }
  std::sort(observed.elements.begin(),observed.elements.end(),[](const auto& a,const auto& b)
  {
    return a.id < b.id;
  });
  if (!check(CoreResultsVtkLink::compatible(observed,mesh,{},error),error))
  {
    return false;
  }
  std::swap(observed.elements.front().connectivity[0],observed.elements.front().connectivity[1]);
  if (!check(!CoreResultsVtkLink::compatible(observed,mesh,{},error),"Linked conflicting FRD helper endpoints"))
  {
    return false;
  }
  bool mixed = false;
  for (const auto& group : mesh.groups)
  {
    if (group.kind == CoreResultsVtkGroupKind::Block)
    {
      if (!check(group.ids == std::vector<int>{100},"Joint element published in a block"))
      {
        return false;
      }
      mixed = mixed || group.name == "MIXED";
    }
  }
  return check(mixed,"Lost physical portion of mixed set");
}
}

int main()
{
  QTemporaryDir temporary;
  if (!temporary.isValid())
  {
    return 1;
  }
  const auto filename = temporary.filePath("discrete.inp").toStdString();
  {
    std::ofstream out(filename);
    out << "*NODE\n1,0,0,0\n2,1,0,0\n"
      << marker(1,11,"MASS") << marker(1,12,"SPRINGA") << marker(2,13,"SPRING1")
      << "** CUBIT_SIM_DISCRETE_HISTORY MASS 1 1 1\n"
      << "** CUBIT_SIM_DISCRETE_HISTORY SPRINGA 1 1 1\n"
      << "** CUBIT_SIM_DISCRETE_HISTORY SPRING1 2 1 1\n"
      << "*NSET,NSET=CCX_DISCRETE_STEP_1\n1,2\n"
      << "*ELEMENT,TYPE=MASS,ELSET=CCX_POINT_MASS_1\n11,1\n*MASS,ELSET=CCX_POINT_MASS_1\n200.\n"
      << "*ELEMENT,TYPE=SPRINGA,ELSET=CCX_SPRING_1\n12,1,2\n*SPRING,ELSET=CCX_SPRING_1\n\n200.\n"
      << "*ELEMENT,TYPE=SPRING1,ELSET=CCX_SPRING_2\n13,1\n*SPRING,ELSET=CCX_SPRING_2\n2\n200.\n";
    if (!out)
    {
      return 1;
    }
  }
  std::vector<std::string> files;
  std::string error;
  CoreResultsVtkMesh model;
  if (!CoreResultsInpReader().read(filename,model,files,error))
  {
    std::cerr << error << '\n';
    return 1;
  }
  ConnectionRunMap archive;
  if (!archive.restore(model.connections.json(),error) || archive.discrete.size() != 3)
  {
    std::cerr << "Discrete element archive round trip failed: " << error << '\n';
    return 1;
  }
  CoreResultsVtkMesh frd;
  frd.nodes = model.nodes;
  frd.elements.push_back({12,11,{}, {1,2}});
  bool ok = check(CoreResultsVtkLink::compatible(frd,model,{},error),"FRD omitting MASS and SPRING1 must match");
  frd.elements[0].connectivity = {2,1};
  ok &= check(!CoreResultsVtkLink::compatible(frd,model,{},error),"Reversed archived endpoint order must fail");

  frd.elements[0].connectivity = {1,2};
  auto remeshed = model;
  remeshed.nodes[1].coordinates[0] = 2.0;
  ok &= check(!CoreResultsVtkLink::compatible(frd,remeshed,{},error),"Remeshed endpoint matched the archived spring");
  auto collision = model;
  collision.elements.push_back(frd.elements.front());
  ok &= check(!CoreResultsVtkLink::compatible(frd,collision,{},error),"Reused solver element ID matched a structural element");
  ConnectionRunMap duplicate = archive;
  ok &= check(!duplicate.read_marker(marker(3,11,"MASS"),error),"Duplicate solver element ID replaced its archived binding");

  const CoreDiscreteElementHistory::Columns zero = {{"Extension",0.0},{"Derived axial force",0.0}};
  // Rigid 90 degree rotation of a unit spring plus a common translation.
  const std::map<int,CoreDiscreteElementHistory::Sample> motion{{1,{2.,3.,0.}},{2,{1.,4.,0.}}};
  const auto& axial = archive.discrete.at(12);
  ok &= check(CoreDiscreteElementHistory::evaluate(axial,1,motion) == zero,"Rigid rotation produced spring force");
  ok &= check(CoreDiscreteElementHistory::evaluate(axial,2,motion).empty(),"History appeared in an unassigned step");
  ok &= check(CoreDiscreteElementHistory::evaluate(axial,1,{{1,{0.,0.,0.}}}).empty(),"Missing spring endpoint became zero motion");
  const CoreDiscreteElementHistory::Columns ground = {{"Travel",3.0},{"Derived restoring force",-600.0}};
  ok &= check(CoreDiscreteElementHistory::evaluate(archive.discrete.at(13),1,motion) == ground,"Incorrect ground direction or sign");
  ok &= moving_joint(temporary.filePath("hinge.inp").toStdString(),"revolute");
  ok &= moving_joint(temporary.filePath("cylinder.inp").toStdString(),"cylindrical");
  return ok ? 0 : 1;
}
