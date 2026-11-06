#include "DrawingGraphics.hpp"
#include "CubitColorTable.hpp"
#include "CubitVector.hpp"
#include "GCubitModelSet.hpp"
#include "GGenericModel.hpp"
#include "GView.hpp"
#include "GWindowFactory.hpp"
#include <algorithm>
#include <set>

namespace
{
std::string drawingSetName(const std::string& workspace, const std::string& layer)
{
  const char* digits = "0123456789abcdef";
  const auto encode = [digits](const std::string& input)
  {
    std::string output;
    for (unsigned char value : input)
    {
      output.push_back(digits[value >> 4]);
      output.push_back(digits[value & 15]);
    }
    return output;
  };
  return "cubitsim_" + encode(workspace) + "_" + encode(layer);
}
}

bool DrawingGraphics::synchronize(const std::string& workspace, const std::map<std::string, DrawingGeometry>& geometry, const DrawingFrame& state, std::string& error)
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
    bool preview = false;
    for (const auto& layer : state.layers)
    {
      const auto data = geometry.find(layer.first);
      if (state.overlayVisible && layer.second.enabled && layer.second.hideNativeModel &&
          data != geometry.end() && !data->second.primitives.empty())
      {
        preview = true;
      }
    }
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
    for (const auto& owned : ownedSets)
    {
      if (auto* set = models->get_drawing_set(owned.c_str()))
      {
        if (set->mGenericModel != nullptr)
        {
          set->mGenericModel->clear();
          models->set_model_vis(set->mGenericModel, false);
        }
      }
    }
    for (const auto& entry : state.layers)
    {
      const auto ownedName = drawingSetName(workspace, entry.first);
      ownedSets.insert(ownedName);
      const CubitString name(ownedName.c_str());
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
            case DrawingPrimitiveKind::Point: model->add_points(1, points.data(), color); break;
            case DrawingPrimitiveKind::Line: model->add_line(points[0], points[1], color); break;
            case DrawingPrimitiveKind::Polygon: model->add_polygon(static_cast<int>(points.size()), points.data(), color, SVUtil::SHADE); break;
            case DrawingPrimitiveKind::Label: model->add_label(primitive.text.c_str(), points[0], color); break;
          }
        }
      }
      const auto& registered = models->models();
      if (std::find(registered.begin(), registered.end(), model) == registered.end())
      {
        models->add_model(model);
      }
      models->set_model_vis(model, state.overlayVisible && entry.second.enabled);
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

void DrawingGraphics::detach() noexcept
{
  if (ownedSets.empty() && nativeVisibility.empty())
  {
    return;
  }
  auto* factory = GWindowFactory::get_window_factory();
  if (factory == nullptr)
  {
    ownedSets.clear();
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
    for (const auto& name : ownedSets)
    {
      if (auto* set = models->get_drawing_set(CubitString(name.c_str())))
      {
        models->delete_drawing_set(set);
      }
    }
  }
  ownedSets.clear();
  nativeVisibility.clear();
}
