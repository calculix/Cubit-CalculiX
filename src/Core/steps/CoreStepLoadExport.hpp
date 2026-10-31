#ifndef CORESTEPLOADEXPORT_HPP
#define CORESTEPLOADEXPORT_HPP

#include <string>
#include <vector>

class CoreSteps;
class CoreCustomLines;
class CoreSidesetFaces;
class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class CoreLoadsGravity;
class CoreLoadsCentrifugal;
class CoreLoadsFilm;
class CoreLoadsRadiation;
class CoreLoadsSurfaceTraction;
class CoreLoadsBodyHeatflux;

class CoreStepLoadExport
{
public:
  static bool append(CoreSteps& steps, CoreLoadsForces& loads, CoreCustomLines& customlines, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
  static bool append(CoreSteps& steps, CoreLoadsPressures& loads, CoreCustomLines& customlines, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
  static bool append(CoreSteps& steps, CoreLoadsHeatfluxes& loads, CoreCustomLines& customlines, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
  static bool append(CoreSteps& steps, CoreLoadsGravity& loads, CoreCustomLines& customlines, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
  static bool append(CoreSteps& steps, CoreLoadsCentrifugal& loads, CoreCustomLines& customlines, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
  static bool append(CoreSteps& steps, CoreLoadsFilm& loads, CoreCustomLines& customlines, CoreSidesetFaces& sideset_faces, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
  static bool append(CoreSteps& steps, CoreLoadsRadiation& loads, CoreCustomLines& customlines, CoreSidesetFaces& sideset_faces, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
  static bool append(CoreSteps& steps, CoreLoadsSurfaceTraction& loads, CoreCustomLines& customlines, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
  static bool append(CoreSteps& steps, CoreLoadsBodyHeatflux& loads, CoreCustomLines& customlines, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
};

#endif
