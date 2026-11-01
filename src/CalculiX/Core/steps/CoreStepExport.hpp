#ifndef CORESTEPEXPORT_HPP
#define CORESTEPEXPORT_HPP

#include <string>

class CoreSteps;
class CoreCustomLines;
class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreHistoryOutputs;
class CoreFieldOutputs;
class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class CoreLoadsGravity;
class CoreLoadsCentrifugal;
class CoreLoadsFilm;
class CoreLoadsRadiation;
class CoreLoadsSurfaceTraction;
class CoreLoadsBodyHeatflux;
class CoreSidesetFaces;
class MeshExportInterface;

struct CoreStepExportData
{
  CoreSteps& steps;
  CoreCustomLines& customlines;
  CoreBCsDisplacements& bcsdisplacements;
  CoreBCsTemperatures& bcstemperatures;
  CoreHistoryOutputs& historyoutputs;
  CoreFieldOutputs& fieldoutputs;
  CoreLoadsForces& loadsforces;
  CoreLoadsPressures& loadspressures;
  CoreLoadsHeatfluxes& loadsheatfluxes;
  CoreLoadsGravity& loadsgravity;
  CoreLoadsCentrifugal& loadscentrifugal;
  CoreLoadsFilm& loadsfilm;
  CoreLoadsRadiation& loadsradiation;
  CoreLoadsSurfaceTraction& loadssurfacetraction;
  CoreLoadsBodyHeatflux& loadsbodyheatflux;
  CoreSidesetFaces& sideset_faces;
  MeshExportInterface& mesh;
};

class CoreStepExport
{
public:
  static std::string get_export(const CoreStepExportData& data);
};

#endif
