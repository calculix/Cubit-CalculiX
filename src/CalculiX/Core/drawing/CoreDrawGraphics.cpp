#include "CoreDrawGraphics.hpp"
#include "CubitColorTable.hpp"
#include "CubitVector.hpp"
#include "GCubitModelSet.hpp"
#include "GGenericModel.hpp"
#include "GView.hpp"
#include "GWindowFactory.hpp"
#include <algorithm>
#include <set>

bool CoreDrawGraphics::synchronize(const std::map<DrawGroup, DrawGeometry>& geometry, const CalculiXDrawState& state, bool workspaceActive, std::string& error)
{
  error.clear();
  auto* factory = GWindowFactory::get_window_factory();
  if (factory == nullptr || factory->views().empty())
  {
    error = "No Cubit graphics viewport is available.";
    return false;
  }
  std::set<GCubitModelSet*> visited;
  for (const auto& view : factory->views())
  {
    auto* models = view == nullptr ? nullptr : dynamic_cast<GCubitModelSet*>(view->get_model_set());
    if (models == nullptr || !visited.insert(models).second)
    {
      continue;
    }
    const auto topology = state.groups.find(DrawGroup::Topology);
    const auto topologyGeometry = geometry.find(DrawGroup::Topology);
    const bool preview = workspaceActive && state.overlayVisible && topology != state.groups.end() && topology->second.enabled && topologyGeometry != geometry.end() && !topologyGeometry->second.primitives.empty();
    const auto original = nativeVisibility.find(models);
    if (preview)
    {
      if (original == nativeVisibility.end())
      {
        nativeVisibility.emplace(models, NativeVisibility{models->get_geometry_posted(), models->get_mesh_posted(), models->get_sim_posted()});
      }
      models->set_geometry_posted(false);
      models->set_mesh_posted(false);
      models->set_sim_posted(false);
    }
    else if (original != nativeVisibility.end())
    {
      models->set_geometry_posted(original->second.geometry);
      models->set_mesh_posted(original->second.mesh);
      models->set_sim_posted(original->second.simulation);
      nativeVisibility.erase(original);
    }
    for (const auto& entry : state.groups)
    {
      const CubitString name(drawing_group_name(entry.first));
      auto* set = models->get_drawing_set(name);
      const auto data = geometry.find(entry.first);
      if (set == nullptr && data != geometry.end())
      {
        set = models->create_drawing_set(name, false);
      }
      if (set == nullptr || set->mGenericModel == nullptr)
      {
        continue;
      }
      auto* model = set->mGenericModel;
      model->clear();
      if (data != geometry.end())
      {
        for (const auto& primitive : data->second.primitives)
        {
          int id = -1;
          CubitColor color;
          if (!CubitColorTable::string_to_color_id(primitive.color.c_str(), id) || !CubitColorTable::color_id_to_rgb(id, color))
          {
            error = "Unknown drawing color: " + primitive.color;
            return false;
          }
          std::vector<CubitVector> points;
          for (const auto& point : primitive.points)
          {
            points.emplace_back(point[0], point[1], point[2]);
          }
          switch (primitive.kind)
          {
            case DrawPrimitiveKind::Point: model->add_points(1, points.data(), color); break;
            case DrawPrimitiveKind::Line: model->add_line(points[0], points[1], color); break;
            case DrawPrimitiveKind::Polygon: model->add_polygon(static_cast<int>(points.size()), points.data(), color, SVUtil::SHADE); break;
            case DrawPrimitiveKind::Label: model->add_label(primitive.text.c_str(), points[0], color); break;
          }
        }
      }
      const auto& registered = models->models();
      if (std::find(registered.begin(), registered.end(), model) == registered.end())
      {
        models->add_model(model);
      }
      models->set_model_vis(model, workspaceActive && state.overlayVisible && entry.second.enabled);
    }
    models->update_model_parts();
    models->render();
  }
  if (visited.empty())
  {
    error = "No compatible Cubit graphics model set is available.";
    return false;
  }
  return true;
}

void CoreDrawGraphics::detach() noexcept
{
  auto* factory = GWindowFactory::get_window_factory();
  if (factory == nullptr)
  {
    nativeVisibility.clear();
    return;
  }
  std::set<GCubitModelSet*> visited;
  for (const auto& view : factory->views())
  {
    auto* models = view == nullptr ? nullptr : dynamic_cast<GCubitModelSet*>(view->get_model_set());
    if (models == nullptr || !visited.insert(models).second)
    {
      continue;
    }
    const auto original = nativeVisibility.find(models);
    if (original != nativeVisibility.end())
    {
      models->set_geometry_posted(original->second.geometry);
      models->set_mesh_posted(original->second.mesh);
      models->set_sim_posted(original->second.simulation);
      nativeVisibility.erase(original);
    }
    for (auto group : {DrawGroup::BoundaryConditions, DrawGroup::Loads, DrawGroup::Constraints, DrawGroup::Orientations, DrawGroup::Topology})
    {
      if (auto* set = models->get_drawing_set(CubitString(drawing_group_name(group))))
      {
        models->delete_drawing_set(set);
      }
    }
  }
}
