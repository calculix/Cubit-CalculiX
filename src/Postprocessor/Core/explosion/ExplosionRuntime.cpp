#include "ExplosionRuntime.hpp"
#include "../representations/ResultMesh.hpp"
#include "../representations/ResultReferenceMesh.hpp"
#include "../selection/ResultProbe.hpp"
#include <vtkCell.h>
#include <vtkAppendPolyData.h>
#include <vtkCellType.h>
#include <vtkVariant.h>
#include <iterator>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkAbstractArray.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkExtractEdges.h>
#include <vtkIdList.h>
#include <vtkIndexedArray.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTypeInt64Array.h>
#include <vtkUnstructuredGrid.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
namespace
{
  bool original_id(vtkDataArray* array,std::size_t index,std::int64_t& value)
  {
    if (!array || array->GetNumberOfComponents() != 1 || index >= static_cast<std::size_t>(array->GetNumberOfTuples()) ||
        array->GetDataType() == VTK_FLOAT || array->GetDataType() == VTK_DOUBLE)
    {
      return false;
    }
    bool valid = false;
    value = array->GetVariantValue(static_cast<vtkIdType>(index)).ToLongLong(&valid);
    return valid && value >= 0;
  }
  template<class T> std::shared_ptr<T> own(T* value)
  {
    return std::shared_ptr<T>(value,[](T* item)
    {
      item->Delete();
    });
  }
  template<typename T> vtkSmartPointer<vtkDataArray> indexed(vtkDataArray* input,vtkIdList* handles,vtkIdType tuples)
  {
    auto result = vtkSmartPointer<vtkIndexedArray<T>>::New();
    result->SetBackend(std::make_shared<vtkIndexedImplicitBackend<T>>(handles,input));
    result->SetNumberOfComponents(input->GetNumberOfComponents());
    result->SetNumberOfTuples(tuples);
    result->SetName(input->GetName());
    return result;
  }
  // Numeric fields remain read-only indexed views of input arrays. Only the
  // tuple/component indirection is allocated; frames/results are not copied.
  void map_arrays(vtkDataSetAttributes* input,vtkDataSetAttributes* output,const std::vector<vtkIdType>& tuples)
  {
    std::map<int,vtkSmartPointer<vtkIdList>> maps;
    for (int i = 0; i < input->GetNumberOfArrays(); ++i)
    {
      auto* array = input->GetAbstractArray(i);
      const int components = array->GetNumberOfComponents();
      if (auto* numeric = vtkDataArray::SafeDownCast(array))
      {
        std::string name;
        if (array->GetName())
        {
          name = array->GetName();
        }
        if (name == "OriginalNodeId" || name == "OriginalElementId" || name == "IntegrationPointId")
        {
          auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
          ids->SetName(array->GetName());
          for (auto tuple : tuples)
          {
            std::int64_t id = -1;
            original_id(numeric,static_cast<std::size_t>(tuple),id);
            ids->InsertNextValue(id);
          }
          output->AddArray(ids);
          continue;
        }
        auto& handles = maps[components];
        if (!handles)
        {
          handles = vtkSmartPointer<vtkIdList>::New();
          handles->SetNumberOfIds(static_cast<vtkIdType>(tuples.size())*components);
          for (std::size_t row = 0; row < tuples.size(); ++row)
          {
            for (int component = 0; component < components; ++component)
            {
              handles->SetId(static_cast<vtkIdType>(row)*components+component,tuples[row]*components+component);
            }
          }
        }
        vtkSmartPointer<vtkDataArray> view;
        switch (numeric->GetDataType())
        {
          vtkTemplateMacro(view = indexed<VTK_TT>(numeric,handles,static_cast<vtkIdType>(tuples.size())));
          case VTK_BIT:
            view = indexed<unsigned char>(numeric,handles,static_cast<vtkIdType>(tuples.size()));
            break;
          default: break;
        }
        if (view)
        {
          output->AddArray(view);
        }
      }
      else
      {
        auto copy = vtkSmartPointer<vtkAbstractArray>::Take(array->NewInstance());
        copy->SetName(array->GetName());
        copy->SetNumberOfComponents(components);
        for (auto tuple : tuples)
        {
          copy->InsertNextTuple(tuple, array);
        }
        output->AddArray(copy);
      }
    }
  }
  std::shared_ptr<vtkPolyData> edges(vtkDataSet* mesh)
  {
    auto extract = vtkSmartPointer<vtkExtractEdges>::New();
    extract->SetInputData(mesh);
    extract->Update();
    auto result = own(vtkPolyData::New());
    result->ShallowCopy(extract->GetOutput());
    return result;
  }
  std::shared_ptr<vtkPolyData> nodes(vtkDataSet* mesh)
  {
    auto result = own(vtkPolyData::New());
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    points->SetNumberOfPoints(mesh->GetNumberOfPoints());
    auto vertices = vtkSmartPointer<vtkCellArray>::New();
    for (vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i)
    {
      double point[3];
      mesh->GetPoint(i,point);
      points->SetPoint(i,point);
      vertices->InsertNextCell(1,&i);
    }
    result->SetPoints(points);
    result->SetVerts(vertices);
    result->GetPointData()->ShallowCopy(mesh->GetPointData());
    return result;
  }
}
namespace Postprocessor
{
  bool prepare_explosion_reference(std::shared_ptr<const SimResults::VtkResultsData> source,ExplosionReference& output,std::string& error)
  {
    error.clear();
    ExplosionReference reference;
    reference.source = source;
    if (!source)
    {
      error = "Explosion source is unavailable.";
      return false;
    }
    const auto first = first_mesh_increment(*source);
    if (!first)
    {
      error = "Explosion requires mesh geometry.";
      return false;
    }
    const auto parts = result_mesh_parts(source->increments[*first].dataset.get());
    const auto catalogue = result_dataset_catalogue(*source);
    std::map<std::size_t,vtkDataSet*> candidates;
    std::vector<ExplosionReferenceBlock> all;
    for (const auto& part : parts)
    {
      const auto metadata = std::find_if(catalogue.begin(),catalogue.end(),[&](const auto& value)
      {
        return value.id == part.first && dataset_in_category(value,DatasetCategory::Blocks);
      });
      if (metadata == catalogue.end())
      {
        continue;
      }
      ExplosionReferenceBlock block;
      block.identity = {metadata->kind,metadata->name,metadata->model_id};
      if (!mesh_bounds(part.second,block.bounds))
      {
        continue;
      }
      for (std::size_t a = 0; a < 3; ++a)
      {
        block.center[a] = block.bounds[2*a]/2+block.bounds[2*a+1]/2;
      }
      for (vtkIdType i = 0; i < part.second->GetNumberOfCells(); ++i)
      {
        std::int64_t id = -1;
        if (original_id(part.second->GetCellData()->GetArray("OriginalElementId"),static_cast<std::size_t>(i),id))
        {
          block.elements.insert(id);
        }
      }
      if (block.elements.empty())
      {
        continue;
      }
      candidates[all.size()] = part.second;
      all.push_back(std::move(block));
    }
    // Strict subsets and identical aliases follow the containing physical block.
    // Partial overlapping primary sets are ambiguous and must be disambiguated.
    for (std::size_t i = 0; i < all.size(); ++i)
    {
      bool subset = false;
      for (std::size_t j = 0; j < all.size(); ++j)
      {
        if (i != j && (all[j].elements.size() > all[i].elements.size() || (all[j].elements == all[i].elements && j < i)) &&
            std::includes(all[j].elements.begin(),all[j].elements.end(),all[i].elements.begin(),all[i].elements.end()))
        {
          subset = true;
        }
      }
      if (subset)
      {
        continue;
      }
      const auto blockIndex = reference.blocks.size();
      reference.blocks.push_back(all[i]);
      auto* mesh = candidates.at(i);
      for (vtkIdType cellId = 0; cellId < mesh->GetNumberOfCells(); ++cellId)
      {
        std::int64_t id = -1;
        if (!original_id(mesh->GetCellData()->GetArray("OriginalElementId"),static_cast<std::size_t>(cellId),id))
        {
          continue;
        }
        if (mesh->GetCellType(cellId) == VTK_POLYHEDRON)
        {
          error = "Polyhedron explosion is not supported; use standard finite elements.";
          return false;
        }
        if (reference.elements.count(id))
        {
          error = "Overlapping primary element sets have ambiguous explosion ownership.";
          return false;
        }
        auto* cell = mesh->GetCell(cellId);
        ExplosionReferenceElement element;
        element.id = id;
        element.block = blockIndex;
        double parametric[3];
        int subId = cell->GetParametricCenter(parametric);
        std::vector<double> weights(static_cast<std::size_t>(cell->GetNumberOfPoints()));
        cell->EvaluateLocation(subId,parametric,element.center.data(),weights.data());
        reference.elements.emplace(id,element);
        for (vtkIdType point = 0; point < cell->GetNumberOfPoints(); ++point)
        {
          std::int64_t node = -1;
          if (original_id(mesh->GetPointData()->GetArray("OriginalNodeId"),static_cast<std::size_t>(cell->GetPointId(point)),node))
          {
            reference.nodes[node].push_back(id);
          }
        }
      }
    }
    for (auto& item : reference.nodes)
    {
      auto& ids = item.second;
      std::sort(ids.begin(),ids.end());
      ids.erase(std::unique(ids.begin(),ids.end()),ids.end());
    }
    output = std::move(reference);
    return true;
  }
  bool explosion_enabled(const ExplosionState& settings,const std::string& source)
  {
    if (settings.enabled && settings.amount > 0 && std::find(settings.sources.begin(),settings.sources.end(),source) != settings.sources.end())
    {
      return true;
    }
    const auto mesh = settings.meshes.find(source);
    return mesh != settings.meshes.end() && mesh->second.enabled && mesh->second.amount > 0;
  }
  bool explode_output(const ExplosionState& settings,const Representation& display,const ExplosionReference& reference,double sceneScale,
    std::shared_ptr<const EvaluatedOutput> input,std::shared_ptr<const EvaluatedOutput>& output,std::string& error)
  {
    error.clear();
    const auto sourceId = input->source_id;
    const bool componentsEnabled = settings.enabled &&
      std::find(settings.sources.begin(),settings.sources.end(),sourceId) != settings.sources.end();
    const auto meshSetting = settings.meshes.find(sourceId);
    bool meshEnabled = meshSetting != settings.meshes.end() && meshSetting->second.enabled && meshSetting->second.amount > 0;
    for (const auto& placement : settings.placements)
    {
      if (componentsEnabled && placement.source_id == sourceId && !placement.block.kind.empty() &&
          std::none_of(reference.blocks.begin(),reference.blocks.end(),[&](const auto& block)
          {
            return same_explosion_block(placement.block,block.identity);
          }))
      {
        error = "A saved exploded block is missing or no longer uniquely resolvable. Recalculate the layout.";
        return false;
      }
    }
    if (meshEnabled)
    {
      for (const auto& selected : meshSetting->second.blocks)
      {
        if (std::none_of(reference.blocks.begin(),reference.blocks.end(),[&](const auto& block)
          {
            return same_explosion_block(selected,block.identity);
          }))
        {
          error = "A selected mesh-explosion block is missing; select an available block or reset explosion.";
          return false;
        }
      }
    }
    const bool needsOwner = meshEnabled || (componentsEnabled && std::any_of(settings.placements.begin(),settings.placements.end(),[&](const auto& piece)
    {
      return piece.source_id == sourceId && !piece.block.kind.empty();
    }));
    const auto selectedElement = [&](const ExplosionReferenceElement& element)
    {
      if (!meshEnabled)
      {
        return false;
      }
      const auto& selected = meshSetting->second.blocks;
      return selected.empty() || std::any_of(selected.begin(),selected.end(),[&](const auto& block)
      {
        return same_explosion_block(block,reference.blocks[element.block].identity);
      });
    };
    const auto translation = [&](std::int64_t elementId,bool components = true,bool elements = true)
    {
      std::array<double,3> delta = {0,0,0};
      const auto element = reference.elements.find(elementId);
      if (componentsEnabled && components)
      {
        for (const auto& placement : settings.placements)
        {
          if (placement.source_id != sourceId)
          {
            continue;
          }
          if (!placement.block.kind.empty() && (element == reference.elements.end() ||
              !same_explosion_block(placement.block,reference.blocks[element->second.block].identity)))
          {
            continue;
          }
          for (std::size_t a = 0; a < 3; ++a)
          {
            delta[a] += settings.amount*(placement.offset[a]+placement.manual[a])/sceneScale;
          }
        }
      }
      if (elements && element != reference.elements.end() && selectedElement(element->second))
      {
        const auto& block = reference.blocks[element->second.block];
        for (std::size_t a = 0; a < 3; ++a)
        {
          delta[a] += meshSetting->second.amount*(element->second.center[a]-block.center[a]);
        }
      }
      return delta;
    };
    std::size_t allocated = 0;
    const auto transform = [&](vtkDataSet* mesh,std::shared_ptr<vtkDataSet>& result) -> bool
    {
      if (!mesh)
      {
        return true;
      }
      auto grid = own(vtkUnstructuredGrid::New());
      auto points = vtkSmartPointer<vtkPoints>::New();
      points->SetDataTypeToDouble();
      auto basePoints = vtkSmartPointer<vtkDoubleArray>::New();
      auto componentOffsets = vtkSmartPointer<vtkDoubleArray>::New();
      auto meshOffsets = vtkSmartPointer<vtkDoubleArray>::New();
      basePoints->SetName("PostprocessorPulseBase");
      componentOffsets->SetName("PostprocessorPulseComponents");
      meshOffsets->SetName("PostprocessorPulseMesh");
      for (auto* array : {basePoints.GetPointer(),componentOffsets.GetPointer(),meshOffsets.GetPointer()})
      {
        array->SetNumberOfComponents(3);
      }
      auto owners = vtkSmartPointer<vtkTypeInt64Array>::New();
      owners->SetName(explosion_owner_array);
      auto blockOwners = vtkSmartPointer<vtkTypeInt64Array>::New();
      blockOwners->SetName(explosion_block_array);
      auto cellOwners = vtkSmartPointer<vtkTypeInt64Array>::New();
      cellOwners->SetName(explosion_owner_array);
      std::vector<vtkIdType> pointMap,cellMap;
      std::map<std::pair<vtkIdType,std::int64_t>,vtkIdType> reused;
      const auto appendCell = [&](int type,const std::vector<vtkIdType>& original,vtkIdType originalCell,std::int64_t owner) -> bool
      {
        if (allocated > ExplosionDefaults::maximum_points || original.size() > ExplosionDefaults::maximum_points-allocated)
        {
          error = "Exploded geometry exceeds the 10 million displayed-point limit; reduce the selected blocks.";
          return false;
        }
        allocated += original.size();
        const auto delta = translation(owner);
        const auto componentDelta = translation(owner,true,false);
        const auto meshDelta = translation(owner,false,true);
        const auto element = reference.elements.find(owner);
        std::int64_t blockId = -1;
        if (element != reference.elements.end())
        {
          blockId = static_cast<std::int64_t>(element->second.block);
        }
        auto ids = vtkSmartPointer<vtkIdList>::New();
        for (auto originalPoint : original)
        {
          double point[3];
          mesh->GetPoint(originalPoint,point);
          const std::array<double,3> base = {point[0],point[1],point[2]};
          for (std::size_t a = 0; a < 3; ++a)
          {
            point[a] += delta[a];
            if (!std::isfinite(point[a]))
            {
              error = "Exploded coordinates overflow.";
              return false;
            }
          }
          std::int64_t copy = blockId;
          if (element != reference.elements.end() && selectedElement(element->second))
          {
            copy = owner;
          }
          // The discriminator separates element IDs from canonical block indices.
          bool perElement = element != reference.elements.end() && selectedElement(element->second);
          std::int64_t key = -blockId-2;
          if (perElement)
          {
            key = copy;
          }
          const auto inserted = reused.emplace(std::make_pair(originalPoint,key),points->GetNumberOfPoints());
          if (inserted.second)
          {
            points->InsertNextPoint(point);
            basePoints->InsertNextTuple(base.data());
            componentOffsets->InsertNextTuple(componentDelta.data());
            meshOffsets->InsertNextTuple(meshDelta.data());
            pointMap.push_back(originalPoint);
            owners->InsertNextValue(owner);
            blockOwners->InsertNextValue(blockId);
          }
          ids->InsertNextId(inserted.first->second);
        }
        grid->InsertNextCell(type,ids);
        cellOwners->InsertNextValue(owner);
        cellMap.push_back(originalCell);
        return true;
      };
      const bool cloud = mesh->GetNumberOfCells() == 0 || mesh->GetMaxCellSize() <= 1;
      if (cloud)
      {
        for (vtkIdType point = 0; point < mesh->GetNumberOfPoints(); ++point)
        {
          std::int64_t elementId = -1,nodeId = -1;
          std::vector<std::int64_t> candidates;
          if (original_id(mesh->GetPointData()->GetArray("OriginalElementId"),static_cast<std::size_t>(point),elementId))
          {
            candidates.push_back(elementId);
          }
          else if (original_id(mesh->GetPointData()->GetArray("OriginalNodeId"),static_cast<std::size_t>(point),nodeId))
          {
            const auto found = reference.nodes.find(nodeId);
            if (found != reference.nodes.end())
            {
              candidates = found->second;
            }
          }
          if (candidates.empty())
          {
            if (needsOwner)
            {
              error = "A displayed point has no original element/node ownership for explosion.";
              return false;
            }
            candidates.push_back(-1);
          }
          std::set<std::array<double,3>> emitted;
          for (auto owner : candidates)
          {
            if (emitted.insert(translation(owner)).second && !appendCell(VTK_VERTEX,{point},point,owner))
            {
              return false;
            }
          }
        }
      }
      else
      {
        for (vtkIdType cellId = 0; cellId < mesh->GetNumberOfCells(); ++cellId)
        {
          auto* cell = mesh->GetCell(cellId);
          std::int64_t owner = -1;
          std::vector<std::int64_t> cellCandidates;
          original_id(mesh->GetCellData()->GetArray("OriginalElementId"),static_cast<std::size_t>(cellId),owner);
          if (reference.elements.count(owner) == 0 && cell->GetNumberOfPoints() > 0)
          {
            std::vector<std::int64_t> candidates;
            for (vtkIdType p = 0; p < cell->GetNumberOfPoints(); ++p)
            {
              std::int64_t node = -1;
              if (!original_id(mesh->GetPointData()->GetArray("OriginalNodeId"),static_cast<std::size_t>(cell->GetPointId(p)),node))
              {
                candidates.clear();
                break;
              }
              const auto found = reference.nodes.find(node);
              if (found == reference.nodes.end())
              {
                candidates.clear();
                break;
              }
              if (p == 0)
              {
                candidates = found->second;
              }
              else
              {
                std::vector<std::int64_t> common;
                std::set_intersection(candidates.begin(),candidates.end(),found->second.begin(),found->second.end(),std::back_inserter(common));
                candidates = std::move(common);
              }
            }
            cellCandidates = std::move(candidates);
          }
          else if (reference.elements.count(owner))
          {
            cellCandidates.push_back(owner);
          }
          if (cellCandidates.empty())
          {
            if (needsOwner)
            {
              error = "A displayed cell has no original element/node ownership for explosion.";
              return false;
            }
            cellCandidates.push_back(-1);
          }
          std::vector<vtkIdType> ids;
          for (vtkIdType point = 0; point < cell->GetNumberOfPoints(); ++point)
          {
            ids.push_back(cell->GetPointId(point));
          }
          std::set<std::array<double,3>> emitted;
          for (auto candidate : cellCandidates)
          {
            if (emitted.insert(translation(candidate)).second && !appendCell(cell->GetCellType(),ids,cellId,candidate))
            {
              return false;
            }
          }
        }
      }
      grid->SetPoints(points);
      map_arrays(mesh->GetPointData(),grid->GetPointData(),pointMap);
      if (mesh->GetNumberOfCells() > 0 && !cellMap.empty() &&
          *std::max_element(cellMap.begin(),cellMap.end()) < mesh->GetNumberOfCells())
      {
        map_arrays(mesh->GetCellData(),grid->GetCellData(),cellMap);
      }
      grid->GetPointData()->AddArray(owners);
      grid->GetPointData()->AddArray(basePoints);
      grid->GetPointData()->AddArray(componentOffsets);
      grid->GetPointData()->AddArray(meshOffsets);
      grid->GetPointData()->AddArray(blockOwners);
      grid->GetCellData()->AddArray(cellOwners);
      grid->GetFieldData()->ShallowCopy(mesh->GetFieldData());
      result = grid;
      return true;
    };
    auto result = std::make_shared<EvaluatedOutput>();
    result->source = input->source;
    result->source_id = sourceId;
    result->warnings = input->warnings;
    result->exploded = true;
    auto data = std::make_shared<SimResults::VtkResultsData>(*input->data);
    auto collection = own(vtkPartitionedDataSetCollection::New());
    const auto& frame = input->data->increments[display.increment_index];
    for (const auto& part : result_mesh_parts(frame.dataset.get()))
    {
      if (!dataset_visible(display,part.first))
      {
        continue;
      }
      FilteredPart derived;
      derived.dataset = part.first;
      auto original = std::find_if(input->parts.begin(),input->parts.end(),[&](const auto& item)
      {
        return item.dataset == part.first;
      });
      std::shared_ptr<vtkDataSet> moved;
      std::string warning;
      if (!input->parts.empty())
      {
        moved = own(vtkDataSet::SafeDownCast(part.second->NewInstance()));
        moved->ShallowCopy(part.second);
      }
      else if (!deform_mesh(part.second,part.first,frame,display.deformation,moved,warning,error))
      {
        return false;
      }
      vtkDataSet* displayed = part.second;
      if (moved)
      {
        displayed = moved.get();
      }
      if (!transform(displayed,derived.mesh))
      {
        return false;
      }
      derived.nodes = nodes(derived.mesh.get());
      derived.edges = edges(derived.mesh.get());
      if (original != input->parts.end())
      {
        // Filter node proxies contain retained original nodes, whereas cut
        // intersections intentionally have no solver-node identity.
        if (original->nodes)
        {
          std::shared_ptr<vtkDataSet> proxy;
          if (!transform(original->nodes.get(),proxy))
          {
            return false;
          }
          derived.nodes = nodes(proxy.get());
        }
        if (original->edges)
        {
          std::shared_ptr<vtkDataSet> outline;
          if (!transform(original->edges.get(),outline))
          {
            return false;
          }
          derived.edges = edges(outline.get());
        }
      }
      if (original != input->parts.end() && original->removed)
      {
        if (!transform(original->removed.get(),derived.removed))
        {
          return false;
        }
      }
      if (display.deformation.enabled && display.deformation.overlay)
      {
        vtkDataSet* base = part.second;
        if (original != input->parts.end() && original->reference_edges)
        {
          base = original->reference_edges.get();
        }
        std::shared_ptr<vtkDataSet> referenceMesh;
        if (!transform(base,referenceMesh))
        {
          return false;
        }
        derived.reference_edges = edges(referenceMesh.get());
        if (input->parts.empty() && part.second->GetPointData()->HasArray("IntegrationPointId"))
        {
          std::vector<std::shared_ptr<vtkDataSet>> parents;
          if (!integration_point_reference_meshes(frame,part.second,parents,warning))
          {
            error = warning;
            return false;
          }
          if (!parents.empty())
          {
            auto merged = vtkSmartPointer<vtkAppendPolyData>::New();
            for (const auto& parent : parents)
            {
              std::shared_ptr<vtkDataSet> translated;
              if (!transform(parent.get(),translated))
              {
                return false;
              }
              merged->AddInputData(edges(translated.get()).get());
            }
            merged->Update();
            derived.reference_edges = own(vtkPolyData::New());
            derived.reference_edges->ShallowCopy(merged->GetOutput());
          }
        }
      }
      const auto index = static_cast<unsigned int>(part.first);
      if (collection->GetNumberOfPartitionedDataSets() <= index)
      {
        collection->SetNumberOfPartitionedDataSets(index+1);
      }
      collection->SetPartition(index,0,derived.mesh.get());
      result->parts.push_back(std::move(derived));
    }
    data->increments[display.increment_index].dataset = collection;
    data->layout = SimResults::DatasetLayout::Collection;
    result->data = std::move(data);
    output = std::move(result);
    return true;
  }
}
