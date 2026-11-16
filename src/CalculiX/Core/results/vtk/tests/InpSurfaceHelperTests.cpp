#include "Core/results/vtk/CoreResultsInpReader.hpp"
#include <QTemporaryDir>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
  int failures = 0;
  const std::string marker = "** CUBIT_SIM_SURFACE_HELPER_ELSET\n";
  const std::string meshInput =
    "*NODE\n1,0,0,0\n2,1,0,0\n3,1,1,0\n4,0,1,0\n"
    "5,0,0,1\n6,1,0,1\n7,1,1,1\n8,0,1,1\n"
    "*ELEMENT, TYPE=C3D8, ELSET=Block_1\n1,1,2,3,4,5,6,7,8\n";
  void check(bool condition,const char* message)
  {
    if (!condition)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  }
  bool write(const std::filesystem::path& file,const std::string& contents)
  {
    std::ofstream stream(file);
    stream << contents;
    return static_cast<bool>(stream);
  }
  const CoreResultsVtkGroup* group(const CoreResultsVtkMesh& mesh,const std::string& name)
  {
    const auto found = std::find_if(mesh.groups.begin(),mesh.groups.end(),[&](const CoreResultsVtkGroup& value)
    {
      return value.name == name;
    });
    if (found == mesh.groups.end())
    {
      return nullptr;
    }
    return &*found;
  }
  bool read_case(const std::filesystem::path& directory,const std::string& name,const std::string& body,
    CoreResultsVtkMesh& mesh)
  {
    const auto file = directory/(name+".inp");
    if (!write(file,meshInput+body))
    {
      check(false,"Cannot write INP helper fixture");
      return false;
    }
    CoreResultsInpReader reader;
    std::vector<std::string> files;
    std::string error;
    const bool accepted = reader.read(file.string(),mesh,files,error);
    if (!accepted)
    {
      std::cerr << name << ": " << error << '\n';
    }
    check(accepted,"Cannot read INP helper fixture");
    return accepted;
  }
}
int main()
{
  QTemporaryDir temporary;
  if (!temporary.isValid())
  {
    std::cerr << "Cannot create INP helper test directory\n";
    return 1;
  }
  const std::filesystem::path directory(temporary.path().toStdString());
  CoreResultsVtkMesh mesh;
  if (read_case(directory,"marked",marker+"*ELSET, ELSET=bolt_bottom_S2\n1\n"
      "*SURFACE, NAME=bolt_bottom, TYPE=ELEMENT\nbolt_bottom_S2,S2\n",mesh))
  {
    const auto* surface = group(mesh,"bolt_bottom");
    check(!group(mesh,"bolt_bottom_S2") && group(mesh,"Block_1"),"Helper became a block or removed the real block");
    check(surface && surface->kind == CoreResultsVtkGroupKind::Surface && surface->sides.size() == 1 &&
      surface->sides[0].element_id == 1 && surface->sides[0].side == "S2","Marked helper lost surface face provenance");
    check(mesh.elements.size() == 1 && mesh.nodes.size() == 8,"Helper filtering modified the solver mesh");
  }
  if (read_case(directory,"legacy","*ELSET, ELSET=bolt_bottom_S2\n1\n"
      "*SURFACE, NAME=bolt_bottom, TYPE=ELEMENT\nbolt_bottom_S2,S2\n",mesh))
  {
    check(group(mesh,"bolt_bottom_S2") && group(mesh,"bolt_bottom"),"Unmarked surface-referenced element set was hidden");
  }
  if (read_case(directory,"forward","*SURFACE, NAME=face, TYPE=ELEMENT\nHELPER,S2\n"
      "*ELSET, ELSET=Alias\nHELPER\n"+marker+"*ELSET, ELSET=HELPER, GENERATE\n1,1,1\n",mesh))
  {
    const auto* alias = group(mesh,"Alias");
    check(!group(mesh,"HELPER") && alias && alias->ids == std::vector<int>{1} && group(mesh,"face"),
      "Helper filtering broke forward references, GENERATE, or genuine set aliases");
  }
  if (read_case(directory,"mixed",marker+"*ELSET, ELSET=shared\n1\n"
      "*ELSET, ELSET=SHARED\n1\n",mesh))
  {
    check(group(mesh,"SHARED"),"Ordinary declaration did not preserve a marked set of the same name");
  }
  if (read_case(directory,"mixed_reverse","*ELSET, ELSET=shared\n1\n"+
      marker+"*ELSET, ELSET=SHARED\n1\n",mesh))
  {
    check(group(mesh,"SHARED"),"Helper declaration overrode an earlier ordinary declaration");
  }
  if (read_case(directory,"implicit",marker+"*ELSET, ELSET=shared\n2\n"
      "*ELEMENT, TYPE=C3D8, ELSET=SHARED\n2,1,2,3,4,5,6,7,8\n",mesh))
  {
    check(group(mesh,"SHARED") && mesh.elements.size() == 2,"Implicit ELEMENT/ELSET declaration was hidden");
  }
  check(write(directory/"ordinary.inc","*ELSET, ELSET=Included\n1\n"),"Cannot write ordinary include");
  if (read_case(directory,"parent_marker",marker+"*INCLUDE, INPUT=ordinary.inc\n"
      "*ELSET, ELSET=AfterInclude\n1\n",mesh))
  {
    check(group(mesh,"Included") && group(mesh,"AfterInclude"),"Parent marker leaked through INCLUDE");
  }
  check(write(directory/"helper.inc",marker+"*ELSET, ELSET=IncludedHelper\n1\n"+marker),"Cannot write helper include");
  if (read_case(directory,"child_marker","*INCLUDE, INPUT=helper.inc\n"
      "*ELSET, ELSET=ParentSet\n1\n*SURFACE, NAME=IncludedSurface\nIncludedHelper,S2\n",mesh))
  {
    check(!group(mesh,"IncludedHelper") && group(mesh,"ParentSet") && group(mesh,"IncludedSurface"),
      "Include helper classification or child-marker isolation failed");
  }
  if (read_case(directory,"intervening_data","*ELSET, ELSET=First\n"+marker+"1\n"
      "*ELSET, ELSET=Second\n1\n"+marker+"*NSET, NSET=Nodes\n1\n",mesh))
  {
    check(group(mesh,"First") && group(mesh,"Second") && group(mesh,"Nodes"),"Stale or non-ELSET marker hid a genuine set");
  }
  if (read_case(directory,"continued",marker+"** generated face support\n\n*ELSET,\nELSET=Continued\n1\n"
      "*SURFACE, NAME=ContinuedSurface\nContinued,S2\n",mesh))
  {
    check(!group(mesh,"Continued") && group(mesh,"ContinuedSurface"),"Continuation or intervening comments lost helper classification");
  }
  const auto bad = directory/"invalid.inp";
  check(write(bad,meshInput+marker+"*ELSET, ELSET=InvalidHelper\n999\n"),"Cannot write invalid helper fixture");
  const auto oldNodes = mesh.nodes.size();
  const auto oldElements = mesh.elements.size();
  const auto oldGroups = mesh.groups.size();
  std::vector<std::string> files{"unchanged"};
  std::string error;
  CoreResultsInpReader reader;
  check(!reader.read(bad.string(),mesh,files,error) && mesh.nodes.size() == oldNodes &&
    mesh.elements.size() == oldElements && mesh.groups.size() == oldGroups && files == std::vector<std::string>{"unchanged"},
    "Hidden helper bypassed validation or failure replaced published output");
  if (failures != 0)
  {
    return 1;
  }
  return 0;
}
