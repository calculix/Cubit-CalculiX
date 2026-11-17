#include "VectorPlotRuntime.hpp"
#include "ResultVectors.hpp"
#include "ResultMesh.hpp"
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkVariant.h>


#include <vtkCell.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <set>
#include <map>
namespace
{
  std::uint64_t mix(std::uint64_t value)
  {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value^(value>>30))*0xbf58476d1ce4e5b9ULL;
    value = (value^(value>>27))*0x94d049bb133111ebULL;
    return value^(value>>31);
  }
  long long identity(vtkDataSetAttributes* attributes,const char* name,vtkIdType tuple)
  {
    auto* array = attributes->GetArray(name);
    if (!array || tuple >= array->GetNumberOfTuples() || array->GetNumberOfComponents() != 1)
    {
      return -1;
    }
    bool valid = false;
    const auto value = array->GetVariantValue(tuple).ToLongLong(&valid);
    if (!valid || value < 0)
    {
      return -1;
    }
    return value;
  }
}
namespace Postprocessor
{
  bool prepare_vector_plot(const SimResults::VtkResultsData& source,const SimResults::VtkResultsData& displayed,
    const Representation& display,double referenceMagnitude,VectorPlotData& output,std::string& error,
    const SimUnits::WorkspaceUnitContext* presentation)
  {
    output = {};
    error.clear();
    if (!display.vectors.enabled || !display.vectors.field)
    {
      return true;
    }
    if (!valid_vector_plot(display.vectors,error))
    {
      return false;
    }
    if (display.increment_index >= displayed.increments.size())
    {
      error = "The vector frame is unavailable.";
      return false;
    }
    // Use the first spatial source frame, independent of the animated frame,
    // deformation, camera, filtering and glyph bounds.
    std::array<double,6> reference = {1,-1,1,-1,1,-1};
    for (const auto& frame : source.increments)
    {
      for (const auto& part : result_mesh_parts(frame.dataset.get()))
      {
        if (!dataset_visible(display,part.first) || !part.second->GetNumberOfPoints())
        {
          continue;
        }
        double bounds[6];
        part.second->GetBounds(bounds);
        for (std::size_t axis = 0; axis < 3; ++axis)
        {
          if (reference[0] > reference[1] && axis == 0)
          {
            reference = {bounds[0],bounds[1],bounds[2],bounds[3],bounds[4],bounds[5]};
          }
          reference[2*axis] = std::min(reference[2*axis],bounds[2*axis]);
          reference[2*axis+1] = std::max(reference[2*axis+1],bounds[2*axis+1]);
        }
      }
      if (reference[0] <= reference[1])
      {
        break;
      }
    }
    if (reference[0] > reference[1])
    {
      return true;
    }
    output.reference_length = 0.05*std::hypot(reference[1]-reference[0],reference[3]-reference[2],reference[5]-reference[4]);
    output.reference_magnitude = referenceMagnitude;
    const auto& frame = displayed.increments[display.increment_index];
    bool found = false;
    for (const auto& part : result_mesh_parts(frame.dataset.get()))
    {
      auto selection = *display.vectors.field;
      if (!dataset_visible(display,part.first) || (selection.dataset_id && *selection.dataset_id != part.first))
      {
        continue;
      }
      selection.dataset_id = part.first;
      const auto* field = find_field(frame,selection);
      if (!field || !spatial_vector(*field))
      {
        continue;
      }
      found = true;
      std::shared_ptr<vtkDataSet> deformed;
      std::string warning;
      if (display.input.owner == OutputOwner::Source && !deform_mesh(part.second,part.first,frame,display.deformation,deformed,warning,error))
      {
        return false;
      }
      auto* mesh = part.second;
      if (deformed)
      {
        mesh = deformed.get();
      }
      const bool element = field->association == SimResults::Association::Element;
      vtkDataSetAttributes* attributes = mesh->GetPointData();
      vtkIdType count = mesh->GetNumberOfPoints();
      if (element)
      {
        attributes = mesh->GetCellData();
        count = mesh->GetNumberOfCells();
      }
      std::set<vtkIdType> exterior;
      if (display.vectors.surface_only && field->association != SimResults::Association::IntegrationPoint)
      {
        // Count original cell faces rather than passing nonlinear surfaces
        // through tessellation: synthesized points must never become node IDs.
        std::map<std::vector<vtkIdType>,std::pair<unsigned int,vtkIdType>> faces;
        for (vtkIdType cellId = 0; cellId < mesh->GetNumberOfCells(); ++cellId)
        {
          auto* cell = mesh->GetCell(cellId);
          if (cell->GetCellDimension() < 3)
          {
            if (element)
            {
              exterior.insert(cellId);
            }
            else
            {
              for (vtkIdType j = 0; j < cell->GetNumberOfPoints(); ++j)
              {
                exterior.insert(cell->GetPointId(j));
              }
            }
            continue;
          }
          for (int faceId = 0; faceId < cell->GetNumberOfFaces(); ++faceId)
          {
            auto* face = cell->GetFace(faceId);
            std::vector<vtkIdType> ids;
            for (vtkIdType j = 0; j < face->GetNumberOfPoints(); ++j)
            {
              ids.push_back(face->GetPointId(j));
            }
            std::sort(ids.begin(),ids.end());
            auto& use = faces[ids];
            ++use.first;
            use.second = cellId;
          }
        }
        for (const auto& face : faces)
        {
          if (face.second.first != 1)
          {
            continue;
          }
          if (element)
          {
            exterior.insert(face.second.second);
          }
          else
          {
            exterior.insert(face.first.begin(),face.first.end());
          }
        }
      }
      for (vtkIdType i = 0; i < count; ++i)
      {
        if (display.vectors.surface_only && field->association != SimResults::Association::IntegrationPoint && !exterior.count(i))
        {
          continue;
        }
        VectorPlotSample sample;
        sample.dataset = part.first;
        sample.entity = identity(attributes,"OriginalNodeId",i);
        if (element || field->association == SimResults::Association::IntegrationPoint)
        {
          sample.entity = identity(attributes,"OriginalElementId",i);
        }
        sample.integration_point = identity(attributes,"IntegrationPointId",i);
        if (element)
        {
          auto* cell = mesh->GetCell(i);
          std::vector<double> weights(static_cast<std::size_t>(cell->GetNumberOfPoints()));
          double pcoords[3];
          int subId = cell->GetParametricCenter(pcoords);
          cell->EvaluateLocation(subId,pcoords,sample.position.data(),weights.data());
        }
        else
        {
          mesh->GetPoint(i,sample.position.data());
        }
        sample.key = mix(part.first)^mix(static_cast<std::uint64_t>(sample.entity))^mix(static_cast<std::uint64_t>(sample.integration_point));
        if (sample.entity < 0)
        {
          // Interpolated section/clip points have no solver node identity.
          for (double coordinate : sample.position)
          {
            std::uint64_t bits = 0;
            std::memcpy(&bits,&coordinate,sizeof(bits));
            sample.key = mix(sample.key^bits);
          }
        }
        if (sample.key%display.vectors.stride != 0)
        {
          continue;
        }
        std::string unit,issue;
        if (!vector_sample(mesh,*field,static_cast<std::size_t>(i),sample.direction,unit,issue,presentation))
        {
          if (!issue.empty())
          {
            output.warning = issue;
          }
          continue;
        }
        output.unit = unit;
        sample.magnitude = std::hypot(sample.direction[0],sample.direction[1],sample.direction[2]);
        if (!std::isfinite(sample.magnitude) || sample.magnitude <= 0 ||
            !std::all_of(sample.position.begin(),sample.position.end(),[](double v) { return std::isfinite(v); }))
        {
          continue;
        }
        sample.length = output.reference_length*display.vectors.scale;
        if (display.vectors.proportional)
        {
          if (!std::isfinite(referenceMagnitude) || referenceMagnitude <= 0)
          {
            continue;
          }
          sample.length *= sample.magnitude/referenceMagnitude;
        }
        if (!std::isfinite(sample.length) || sample.length <= 0)
        {
          continue;
        }
        for (auto& component : sample.direction)
        {
          component /= sample.magnitude;
        }
        const auto order = [](const auto& a,const auto& b) { return a.key < b.key; };
        if (display.vectors.maximum && output.samples.size() == display.vectors.maximum)
        {
          if (sample.key >= output.samples.front().key)
          {
            continue;
          }
          std::pop_heap(output.samples.begin(),output.samples.end(),order);
          output.samples.pop_back();
        }
        output.samples.push_back(sample);
        if (display.vectors.maximum)
        {
          std::push_heap(output.samples.begin(),output.samples.end(),order);
        }
      }
    }
    if (!found)
    {
      output.warning = "Selected vector field unavailable in this frame; arrows are hidden.";
    }
    const auto order = [](const auto& a,const auto& b) { return a.key < b.key; };
    std::sort(output.samples.begin(),output.samples.end(),order);
    bool first = true;
    for (const auto& sample : output.samples)
    {
      // Conservative radius includes the arrow shaft and head at every direction.
      for (std::size_t axis = 0; axis < 3; ++axis)
      {
        const double tip = sample.position[axis]+sample.direction[axis]*sample.length;
        const double low = std::min(sample.position[axis],tip)-0.1*sample.length;
        const double high = std::max(sample.position[axis],tip)+0.1*sample.length;
        if (first)
        {
          output.bounds[2*axis] = low;
          output.bounds[2*axis+1] = high;
        }
        else
        {
          output.bounds[2*axis] = std::min(output.bounds[2*axis],low);
          output.bounds[2*axis+1] = std::max(output.bounds[2*axis+1],high);
        }
      }
      first = false;
    }
    return true;
  }
}
