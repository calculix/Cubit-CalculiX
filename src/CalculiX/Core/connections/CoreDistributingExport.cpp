#include "CoreDistributingExport.hpp"
#include "CalculiXConnectionsInterface.hpp"
#include "Core/export/CoreExportNodeAllocator.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/results/vtk/CoreResultsVtkElements.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CubitInterface.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <cmath>
#include <map>

namespace
{
using Vec = std::array<double,3>;
using Side = std::pair<int,int>;
using Corners = std::vector<int>;

Vec subtract(const Vec& a,const Vec& b)
{
  return {a[0]-b[0],a[1]-b[1],a[2]-b[2]};
}

Vec cross(const Vec& a,const Vec& b)
{
  return {a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]};
}

double dot(const Vec& a,const Vec& b)
{
  return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

bool resolve(const DistributingAttachment& a, std::map<Side,Corners>& selected, std::string& error)
{
  CalculiXMeshInterface mesh;
  std::set<Side> requested;
  std::set<Corners> facets;
  if (a.geometry == "sideset")
  {
    if (!mesh.get_export_sides(a.entity,requested,error))
    {
      return false;
    }
  }
  else
  {
    const auto types = CubitInterface::get_geometry_type("surface", {a.entity});
    if (types.empty() || types.front() != "plane")
    {
      error = "Select a planar Cubit surface.";
      return false;
    }
    for (const std::string kind : {"tri","quad"})
    {
      const auto ids = kind == "tri" ? CubitInterface::get_surface_tris(a.entity) : CubitInterface::get_surface_quads(a.entity);
      for (int id : ids)
      {
        auto nodes = CubitInterface::get_connectivity(kind,id);
        const size_t n = kind == "tri" ? 3 : 4;
        if (nodes.size() < n)
        {
          error = "Incomplete face connectivity.";
          return false;
        }
        nodes.resize(n);
        std::sort(nodes.begin(),nodes.end());
        facets.insert(nodes);
      }
    }
  }
  std::map<int,std::string> formulations;
  for (int block : mesh.get_blocks())
  {
    const auto type = mesh.get_ccx_element_type(block);
    for (int id : mesh.get_block_element_ids(block))
    {
      formulations.emplace(id,type);
    }
  }
  std::set<Corners> found;
  for (const auto& row : mesh.get_element_id_type_connectivity())
  {
    if (row.size() < 3 || !formulations.count(row[0]))
    {
      continue;
    }
    const auto& type = formulations.at(row[0]);
    CoreResultsVtkElements::Description desc;
    std::string reason;
    if (!CoreResultsVtkElements::describe(type,desc,reason) || desc.frd_type < 1 || desc.frd_type > 6)
    {
      continue;
    }
    Corners ccx;
    using Order = CoreResultsVtkElements::NodeOrder;
    if (!CoreResultsVtkElements::reorder(desc,Order::Exodus,Order::CalculiX,Corners(row.begin()+2,row.end()),ccx,error))
    {
      return false;
    }
    for (int s = 1; s <= 6; ++s)
    {
      const Side side{row[0],s};
      if (a.geometry == "sideset" && !requested.count(side))
      {
        continue;
      }
      Corners face;
      int vtk = 0;
      if (!CoreResultsVtkElements::surface(desc,type,"S"+std::to_string(s),ccx,vtk,face,reason))
      {
        continue;
      }
      const size_t n = (vtk == 5 || vtk == 22) ? 3 : 4;
      Corners key(face.begin(),face.begin()+n);
      std::sort(key.begin(),key.end());
      if (a.geometry == "surface" && !facets.count(key))
      {
        continue;
      }
      if (!found.insert(key).second)
      {
        error = "Ambiguous internal face. Select a sideset with a single oriented side.";
        return false;
      }
      Corners boundary;
      for (size_t i = 0; i < n; ++i)
      {
        boundary.push_back(face[i]);
        if (face.size() == 2*n)
        {
          boundary.push_back(face[n+i]);
        }
      }
      selected.emplace(side,std::move(boundary));
    }
  }
  if (selected.empty() || (a.geometry == "surface" ? facets.size() : requested.size()) != selected.size())
  {
    error = "Every selected facet must resolve to an exported solid element face.";
    return false;
  }
  return true;
}

// Integrate a piecewise-linear field on triangles from each boundary edge to the face center.
bool area_weights(const std::map<Side,Corners>& faces, std::map<int,double>& weights, std::string& error)
{
  Vec normal{}, origin{};
  double area = 0;
  for (const auto& entry : faces)
  {
    const auto& ids = entry.second;
    std::vector<Vec> coords;
    Vec center{};
    for (int id : ids)
    {
      coords.push_back(CubitInterface::get_nodal_coordinates(id));
      for (int j = 0; j < 3; ++j)
      {
        center[j] += coords.back()[j]/ids.size();
      }
    }
    for (size_t i = 0; i < ids.size(); ++i)
    {
      const auto k = (i+1)%ids.size();
      const auto v = cross(subtract(coords[i],center),subtract(coords[k],center));
      const double length = std::sqrt(dot(v,v));
      if (!(length > 0) || !std::isfinite(length))
      {
        error = "Degenerate distributing face.";
        return false;
      }
      if (area == 0)
      {
        origin = coords[i];
        normal = {v[0]/length,v[1]/length,v[2]/length};
      }
      const auto offset = subtract(coords[i],origin);
      if (dot(v,normal) < (1-1e-9)*length ||
          std::abs(dot(offset,normal)) > 1e-8*(std::sqrt(length)+std::sqrt(dot(offset,offset))))
      {
        error = "Distributing faces must be coplanar and consistently oriented.";
        return false;
      }
      area += length/2;
      weights[ids[i]] += length/6;
      weights[ids[k]] += length/6;
      for (int id : ids)
      {
        weights[id] += length/(6*ids.size());
      }
    }
  }
  for (auto& weight : weights)
  {
    weight.second /= area;
  }
  return true;
}
}

bool CoreDistributingExport::append(const CoreConnections& model, CoreExportNodeAllocator& allocator,
  const std::set<int>& rigid,std::ostream& nodes,std::ostream& cards,std::string& error)
{
  std::set<int> used = rigid;
  for (const auto& entry : model.attachments())
  {
    const auto& a = entry.second;
    std::map<Side,Corners> faces;
    std::map<int,double> weights;
    if (!resolve(a,faces,error) || !area_weights(faces,weights,error))
    {
      error = a.name + ": " + error;
      return false;
    }
    const auto prefix = "CCX_DA_" + std::to_string(a.id);
    const auto refset = "CCX_RP_" + std::to_string(a.point) + "_REF";
    int ref;
    if (!allocator.reserve_set(prefix) || !allocator.next(ref) || !allocator.reserve_set(refset) || !allocator.reserve_set(prefix+"_FACE") ||
        !allocator.reserve_set(prefix+"_NODES"))
    {
      error = "Distributing attachment export name or node ID unavailable.";
      return false;
    }
    Vec position;
    SimUnits::QuantityUnit length;
    if (!CalculiXConnectionsInterface().point_position(a.point,position,error) || !framework_units().quantity("length",length,error))
    {
      return false;
    }
    for (auto& x : position)
    {
      double si;
      if (!framework_units().geometry_to_si(x,si,error) || !framework_units().to_solver(si,length.dimension,"calculix",x,error))
      {
        return false;
      }
    }
    nodes << "*NODE,NSET=" << refset << "\n" << ref << "," << position[0] << "," << position[1] << "," << position[2] << "\n";
    QJsonObject binding{{"id",a.id},{"name",QString::fromStdString(a.name)},{"point",a.point},{"ref",ref},
      {"rp_name",QString::fromStdString(model.points().at(a.point).name)},{"position",QJsonArray{position[0],position[1],position[2]}}};
    nodes << "** CUBIT_SIM_DA " << QJsonDocument(binding).toJson(QJsonDocument::Compact).toBase64().toStdString() << "\n";
    if (a.mean)
    {
      nodes << "*NSET,NSET=" << prefix << "_NODES\n";
    }
    for (const auto& w : weights)
    {
      if (!used.insert(w.first).second)
      {
        error = "Overlapping rigid or distributing attachment at node " + std::to_string(w.first);
        return false;
      }
      if (a.mean)
      {
        nodes << w.first << "\n** CUBIT_SIM_DA_WEIGHT " << a.id << " " << w.first << " " << w.second << "\n";
      }
    }
    cards << "*SURFACE,TYPE=ELEMENT,NAME=" << prefix << "_FACE\n";
    for (const auto& face : faces)
    {
      cards << face.first.first << ",S" << face.first.second << "\n";
    }
    cards << "*COUPLING,CONSTRAINT NAME=" << prefix << ",REF NODE=" << ref << ",SURFACE=" << prefix << "_FACE\n*DISTRIBUTING\n1,3\n";
    for (int i = 0; i < 3; ++i)
    {
      if (a.moments & (1 << i))
      {
        cards << i+4 << "," << i+4 << "\n";
      }
    }
  }
  return true;
}
