#pragma once
#include <string>
#include <vector>

class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class CoreLoadsGravity;
class CoreLoadsCentrifugal;
class CoreLoadsFilm;
class CoreLoadsRadiation;
class CoreLoadsSurfaceTraction;
class CoreLoadsBodyHeatflux;
class CoreBCsDisplacements;
class CoreBCsTemperatures;
class HDF5Tool;

enum class CalculiXUnitView { Display, Solver, Report };

class CalculiXPhysicalModelUnits {
public:
  static bool input(const std::string& domain, std::vector<std::string>& options, std::vector<double>& values, const std::vector<int>& markers, std::string& error);
  static bool convert(CoreLoadsForces& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreLoadsPressures& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreLoadsHeatfluxes& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreLoadsGravity& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreLoadsCentrifugal& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreLoadsFilm& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreLoadsRadiation& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreLoadsSurfaceTraction& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreLoadsBodyHeatflux& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreBCsDisplacements& model, CalculiXUnitView view, std::string& error);
  static bool convert(CoreBCsTemperatures& model, CalculiXUnitView view, std::string& error);
  static bool read_basis(HDF5Tool& file, const std::string& group, std::string& basis, std::string& error);
  static bool write_basis(HDF5Tool& file, const std::string& group, const std::string& basis, std::string& error);
};
