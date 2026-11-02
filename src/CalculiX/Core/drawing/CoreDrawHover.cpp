#include "CoreDrawHover.hpp"
#include "CubitColorTable.hpp"
#include "CubitMessage.hpp"
#include "CubitVector.hpp"
#include "GCubitModelSet.hpp"
#include "GGenericModel.hpp"
#include "GView.hpp"
#include "GWindowFactory.hpp"
#include <algorithm>
#include <exception>
#include <set>

namespace
{
const char HoverSetName[] = "ccx_symbol_hover";
}

void CoreDrawHover::show(const DrawGeometry& geometry, const std::string& color)
{
  auto* factory = GWindowFactory::get_window_factory();
  if (factory == nullptr)
  {
    return;
  }
  int colorId = -1;
  CubitColor red;
  if (!CubitColorTable::string_to_color_id(color.c_str(), colorId) || !CubitColorTable::color_id_to_rgb(colorId, red))
  {
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
    auto* set = models->get_drawing_set(CubitString(HoverSetName));
    if (set == nullptr)
    {
      set = models->create_drawing_set(CubitString(HoverSetName), false);
    }
    if (set == nullptr || set->mGenericModel == nullptr)
    {
      continue;
    }
    auto* model = set->mGenericModel;
    model->clear();
    model->set_line_width(2.0f);
    model->set_point_size(5.0f);
    for (const auto& primitive : geometry.primitives)
    {
      if (primitive.kind == DrawPrimitiveKind::Label)
      {
        continue;
      }
      std::vector<CubitVector> points;
      for (const auto& point : primitive.points)
      {
        if (point.size() >= 3)
        {
          points.emplace_back(point[0], point[1], point[2]);
        }
      }
      if (points.empty() || points.size() != primitive.points.size())
      {
        continue;
      }
      if (primitive.kind == DrawPrimitiveKind::Point)
      {
        model->add_points(static_cast<int>(points.size()), points.data(), red);
      }
      else if (primitive.kind == DrawPrimitiveKind::Line && points.size() >= 2)
      {
        model->add_line(points[0], points[1], red);
      }
      else if (primitive.kind == DrawPrimitiveKind::Polygon && points.size() >= 3)
      {
        // Keep the original face fill; draw only its boundary in red.
        for (std::size_t index = 0; index < points.size(); ++index)
        {
          model->add_line(points[index], points[(index + 1) % points.size()], red);
        }
      }
    }
    const auto& registered = models->models();
    if (std::find(registered.begin(), registered.end(), model) == registered.end())
    {
      models->add_model(model);
    }
    models->set_model_vis(model, true);
    visible = true;
    models->update_model_parts();
    models->render();
  }
}

void CoreDrawHover::clear() noexcept
{
  if (!visible)
  {
    return;
  }
  visible = false;
  try
  {
    auto* factory = GWindowFactory::get_window_factory();
    if (factory == nullptr)
    {
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
      auto* set = models->get_drawing_set(CubitString(HoverSetName));
      if (set != nullptr && set->mGenericModel != nullptr)
      {
        set->mGenericModel->clear();
        models->set_model_vis(set->mGenericModel, false);
        models->update_model_parts();
        models->render();
      }
    }
  }
  catch (const std::exception& error)
  {
    PRINT_WARNING("CalculiX hover cleanup failed: %s\n", error.what());
  }
}

void CoreDrawHover::detach() noexcept
{
  clear();
  try
  {
    auto* factory = GWindowFactory::get_window_factory();
    if (factory == nullptr)
    {
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
      if (auto* set = models->get_drawing_set(CubitString(HoverSetName)))
      {
        models->delete_drawing_set(set);
      }
    }
  }
  catch (const std::exception& error)
  {
    PRINT_WARNING("CalculiX hover shutdown failed: %s\n", error.what());
  }
}
