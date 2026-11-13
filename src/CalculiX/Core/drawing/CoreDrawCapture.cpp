#include "CoreDrawCapture.hpp"
#include "CoreDrawScope.hpp"
#include "CoreDrawEntityGeometry.hpp"
#include "CoreDrawMeshGeometry.hpp"
#include "CalculiXDrawLabels.hpp"
#include "CalculiXDrawingAdapter.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitInterface.hpp"
#include <QCoreApplication>
#include <QThread>
#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <map>
#include <utility>

namespace
{
bool accepts(CalculiXCaptureContent content, DrawFeature feature)
{
  const auto group = drawing_group(feature);
  return (group == DrawGroup::Loads && (content == CalculiXCaptureContent::Setup || content == CalculiXCaptureContent::Loads)) ||
    (group == DrawGroup::BoundaryConditions && (content == CalculiXCaptureContent::Setup || content == CalculiXCaptureContent::BoundaryConditions));
}
}

CoreDrawCapture::CoreDrawCapture() = default;

bool CoreDrawCapture::validate(const CalculiXCaptureRequest& request, std::string& error)
{
  error.clear();
  auto* app = QCoreApplication::instance();
  if (app == nullptr || QCoreApplication::closingDown() || QThread::currentThread() != app->thread())
  {
    error = "CalculiX capture requires the live Cubit application thread.";
    return false;
  }
  if (request.scope.kind != DrawScopeKind::Model && request.scope.kind != DrawScopeKind::Step)
  {
    error = "Capture requires an entire-model or step scope.";
    return false;
  }
  if (request.scope.kind == DrawScopeKind::Step && !CalculiXStepsInterface().check_step_exists(request.scope.stepId))
  {
    error = "The requested capture step does not exist.";
    return false;
  }
  if (request.content != CalculiXCaptureContent::Model && request.content != CalculiXCaptureContent::Setup &&
      request.content != CalculiXCaptureContent::Loads && request.content != CalculiXCaptureContent::BoundaryConditions)
  {
    error = "Unsupported CalculiX capture content.";
    return false;
  }
  if (request.labels.selectedOnly)
  {
    error = "Capture labels cannot depend on the live selection; use all-entity label settings.";
    return false;
  }
  return request.labels.validate(error);
}

bool CoreDrawCapture::buildFrame(CalculiXCore& core, const CalculiXCaptureRequest& request,
  DrawingFrame& output, std::string& error)
{
  if (!validate(request, error))
  {
    return false;
  }
  try
  {
    DrawingFrame frame;
    frame.style = {"black", "black"};
    frame.layers["model"] = {true, false};
    frame.layers[drawing_group_name(DrawGroup::Loads)] = {true, false};
    frame.layers[drawing_group_name(DrawGroup::BoundaryConditions)] = {true, false};
    frame.selection = {};
    frame.pickingEnabled = false;
    DrawingGeometry mesh;
    // Current undeformed FE mesh only. Reuse the existing element corner-face
    // builder; curved/high-order faces are represented by corner polygons.
    for (const auto* type : {"hex", "tet", "wedge", "pyramid", "face", "edge"})
    {
      for (int id : CubitInterface::parse_cubit_list(type, "all"))
      {
        if (!CoreDrawMeshGeometry::append(type, id, false, "lightgray", mesh))
        {
          error = std::string("Cannot capture mesh entity ") + type + " " + std::to_string(id) + ".";
          return false;
        }
      }
    }
    if (mesh.primitives.empty())
    {
      error = "There is no supported mesh to capture. Mesh the model first.";
      return false;
    }
    std::array<double, 3> minimum = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    std::array<double, 3> maximum = {std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest()};
    for (const auto& primitive : mesh.primitives)
    {
      for (const auto& point : primitive.points)
      {
        for (std::size_t axis = 0; axis < 3; ++axis)
        {
          minimum[axis] = (std::min)(minimum[axis], point[axis]);
          maximum[axis] = (std::max)(maximum[axis], point[axis]);
        }
      }
    }
    const double extent = (std::max)({maximum[0] - minimum[0], maximum[1] - minimum[1], maximum[2] - minimum[2]});
    const double size = std::isfinite(extent) && extent > 0 ? 0.10 * extent : 1.0;
    frame.entities.push_back({{"calculix", "model", 0, ""}, "model", std::move(mesh), false});
    if (request.content != CalculiXCaptureContent::Model)
    {
      ResolvedDrawScope resolved;
      const bool valid = request.scope.kind == DrawScopeKind::Step ?
        CoreDrawScope::resolveStep(core, request.scope.stepId, size, resolved, error) :
        CoreDrawScope::resolveModel(core, resolved, error);
      if (!valid)
      {
        return false;
      }
      // Step geometry already contains inherited/replaced conditions. Only
      // unresolved entities (e.g. trajectory paths) use the definition builder.
      std::map<DrawEntityKey, DrawingGeometry> geometry = std::move(resolved.geometry);
      for (const auto& entity : resolved.entities)
      {
        if (accepts(request.content, entity.feature) && geometry.count(entity) == 0)
        {
          DrawingGeometry built;
          if (!CoreDrawEntityGeometry::build(entity, size, built, error))
          {
            return false;
          }
          geometry.emplace(entity, std::move(built));
        }
      }
      for (auto& entity : geometry)
      {
        if (!accepts(request.content, entity.first.feature))
        {
          continue;
        }
        const auto group = drawing_group(entity.first.feature);
        const bool labels = request.labels.enabled &&
          (group == DrawGroup::Loads ? request.labels.loads : request.labels.boundaryConditions);
        if (!labels)
        {
          auto& primitives = entity.second.primitives;
          primitives.erase(std::remove_if(primitives.begin(), primitives.end(), [](const DrawingPrimitive& primitive)
          {
            return primitive.kind == DrawingPrimitiveKind::Label;
          }), primitives.end());
        }
        else
        {
          for (auto& primitive : entity.second.primitives)
          {
            if (primitive.kind == DrawingPrimitiveKind::Label)
            {
              primitive.color = request.labels.color;
            }
          }
          DrawLabelData label;
          const auto saved = resolved.labels.find(entity.first);
          if (saved != resolved.labels.end())
          {
            label = saved->second;
          }
          else if (!CalculiXDrawLabels::read(entity.first, label, error))
          {
            return false;
          }
          if (request.scope.kind == DrawScopeKind::Step)
          {
            label.stepId = request.scope.stepId;
          }
          if (!CalculiXDrawLabels::append(label, request.labels, size, entity.second, error))
          {
            return false;
          }
        }
        frame.entities.push_back({CalculiXDrawingAdapter::key(entity.first,
          request.scope.kind == DrawScopeKind::Step ? request.scope.stepId : -1),
          drawing_group_name(group), std::move(entity.second), false});
      }
    }
    output = std::move(frame);
    error.clear();
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot build CalculiX capture scene: ") + exception.what();
    return false;
  }
}
