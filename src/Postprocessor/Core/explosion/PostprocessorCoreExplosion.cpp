#include "../application/PostprocessorCore.hpp"
#include "../representations/ResultMesh.hpp"
#include <iterator>
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <set>
#include <tuple>
#include <utility>
namespace Postprocessor
{
  std::shared_ptr<const ExplosionReference> PostprocessorCore::explosion_reference(const std::string& source,std::string& error) const
  {
    const auto data = result_data(source);
    const auto found = explosion_references.find(source);
    if (found != explosion_references.end() && found->second->source == data)
    {
      return found->second;
    }
    auto reference = std::make_shared<ExplosionReference>();
    if (!prepare_explosion_reference(data,*reference,error))
    {
      return {};
    }
    explosion_references[source] = reference;
    return reference;
  }
  bool PostprocessorCore::explosion_active(const Representation& display) const
  {
    const auto view = state.views.find(display.view_id);
    return view != state.views.end() && explosion_enabled(view->second.explosion,output_source(display.input));
  }
  bool PostprocessorCore::evaluate_output(const Representation& display,std::shared_ptr<const EvaluatedOutput>& output,std::string& error) const
  {
    std::shared_ptr<const EvaluatedOutput> input;
    if (!evaluate_unexploded_output(display,input,error))
    {
      return false;
    }
    if (!explosion_active(display))
    {
      output = std::move(input);
      return true;
    }
    const auto& view = state.views.at(display.view_id);
    const auto& settings = view.explosion;
    if (settings.enabled && !settings.placements.empty() && settings.coordinate_unit != view.scene_unit)
    {
      error = "The explosion scene length basis changed. Recalculate the layout before displaying it.";
      return false;
    }
    const auto found = explosion_cache.find(display.id);
    if (found != explosion_cache.end() && found->second.input->data == input->data && found->second.input->source == input->source &&
        found->second.revision == settings.revision && found->second.frame == display.increment_index &&
        same_deformation(found->second.deformation,display.deformation) && found->second.hidden == display.hidden_datasets)
    {
      output = found->second.output;
      return true;
    }
    const auto reference = explosion_reference(input->source_id,error);
    double factor = 1;
    bool known = false;
    if (!reference || !scene_scale(display,factor,known,error) ||
        !explode_output(settings,display,*reference,factor,input,output,error))
    {
      return false;
    }
    apply_explosion_pulse(*output,settings.pulse);
    explosion_cache[display.id] = {input,output,settings.revision,display.increment_index,display.deformation,display.hidden_datasets};
    return true;
  }
  bool PostprocessorCore::change_explosion(const std::string& viewId,const std::vector<std::string>& requestedSources,
    const std::string& action,double value,const std::string& option,const std::string& blockName,
    const std::array<double,3>& offset,const std::string& unit,std::string& error)
  {
    error.clear();
    const auto view = state.views.find(viewId);
    if (source_operation_active || view == state.views.end() || scene_revision == std::numeric_limits<std::uint64_t>::max() ||
        view->second.explosion.revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Explosion view is unavailable during this operation.";
      return false;
    }
    try
    {
      if (action.compare(0,6,"pulse-") == 0)
      {
        auto pulse = view->second.explosion.pulse;
        if (action == "pulse-advance")
        {
          if (!std::isfinite(value) || value < 0)
          {
            error = "Pulse elapsed time must be finite and nonnegative.";
            return false;
          }
          if (!pulse.enabled)
          {
            return true;
          }
          pulse.elapsed_seconds = std::fmod(pulse.elapsed_seconds+std::fmod(value,pulse.period_seconds),pulse.period_seconds);
        }
        else
        {
          if (action == "pulse-on" || action == "pulse-off")
          {
            pulse.enabled = action == "pulse-on";
            pulse.elapsed_seconds = 0;
          }
          else if (action == "pulse-period")
          {
            const double phase = pulse.elapsed_seconds/pulse.period_seconds;
            pulse.period_seconds = value;
            pulse.elapsed_seconds = phase*value;
          }
          else if (action == "pulse-range")
          {
            pulse.minimum = offset[0];
            pulse.maximum = offset[1];
          }
          else if (action == "pulse-target")
          {
            const std::vector<std::string> targets = {"components","mesh","both"};
            const auto found = std::find(targets.begin(),targets.end(),option);
            if (found == targets.end())
            {
              error = "Unknown explosion pulse target.";
              return false;
            }
            pulse.target = static_cast<ExplosionState::Pulse::Target>(found-targets.begin());
          }
          else
          {
            error = "Unknown explosion pulse operation.";
            return false;
          }
          if (!valid_explosion_pulse(pulse,error))
          {
            return false;
          }
          if (pulse.settings_revision == std::numeric_limits<std::uint64_t>::max())
          {
            error = "Explosion pulse settings revision is exhausted.";
            return false;
          }
          ++pulse.settings_revision;
        }
        // Pulse-only edits and ticks reuse displayed topology and displacement
        // arrays. Geometry commands still validate and replace their outputs.
        view->second.explosion.pulse = pulse;
        for (const auto& item : state.representations)
        {
          if (item.second.view_id == viewId)
          {
            const auto cached = explosion_cache.find(item.first);
            if (cached != explosion_cache.end())
            {
              apply_explosion_pulse(*cached->second.output,pulse);
            }
          }
        }
        return true;
      }
      auto next = view->second.explosion;
      std::vector<std::string> scope = requestedSources;
      if (scope.empty() && action != "on" && action != "mesh-on")
      {
        scope = next.sources;
      }
      if (scope.empty())
      {
        for (const auto& item : state.representations)
        {
          if (item.second.view_id == viewId && item.second.visible)
          {
            const auto source = output_source(item.second.input);
            if (!source.empty() && result_data(source))
            {
              scope.push_back(source);
            }
          }
        }
      }
      std::sort(scope.begin(),scope.end());
      scope.erase(std::unique(scope.begin(),scope.end()),scope.end());
      bool rebuild = action == "recalculate";
      const bool wasEnabled = next.enabled || std::any_of(next.meshes.begin(),next.meshes.end(),[](const auto& item)
      {
        return item.second.enabled;
      });
      if (action == "reset")
      {
        next = ExplosionState{};
      }
      else if (action == "off" && requestedSources.empty())
      {
        next.enabled = false;
        for (auto& mesh : next.meshes)
        {
          mesh.second.enabled = false;
        }
      }
      else if (action == "components-off")
      {
        if (requestedSources.empty())
        {
          next.enabled = false;
        }
        else
        {
          for (const auto& source : scope)
          {
            next.sources.erase(std::remove(next.sources.begin(),next.sources.end(),source),next.sources.end());
          }
          next.enabled = next.enabled && !next.sources.empty();
        }
      }
      else if (action == "off")
      {
        for (const auto& source : scope)
        {
          next.sources.erase(std::remove(next.sources.begin(),next.sources.end(),source),next.sources.end());
          next.meshes[source].enabled = false;
        }
      }
      else if (action == "amount")
      {
        next.amount = value;
      }
      else if (action == "gap")
      {
        next.gap = value;
        rebuild = !next.placements.empty();
      }
      else if (action == "direction")
      {
        const std::vector<std::string> names = {"auto","x","y","z","radial"};
        const auto found = std::find(names.begin(),names.end(),option);
        if (found == names.end())
        {
          error = "Unknown explosion direction.";
          return false;
        }
        next.direction = static_cast<ExplosionDirection>(found-names.begin());
        rebuild = !next.placements.empty();
      }
      else if (action == "grouping")
      {
        const std::vector<std::string> names = {"auto","sources","blocks"};
        const auto found = std::find(names.begin(),names.end(),option);
        if (found == names.end())
        {
          error = "Unknown explosion grouping.";
          return false;
        }
        next.grouping = static_cast<ExplosionGrouping>(found-names.begin());
        rebuild = !next.placements.empty();
      }
      else if (action == "mesh-on" || action == "mesh-off" || action == "mesh-amount")
      {
        if (scope.empty())
        {
          error = "Choose an available mesh source before exploding elements.";
          return false;
        }
        for (const auto& source : scope)
        {
          auto& mesh = next.meshes[source];
          if (action == "mesh-amount")
          {
            mesh.amount = value;
          }
          else
          {
            mesh.enabled = action == "mesh-on";
          }
          if (action != "mesh-off")
          {
            const auto reference = explosion_reference(source,error);
            if (!reference || reference->blocks.empty())
            {
              error = "Mesh explosion requires blocks with original element IDs. "+error;
              return false;
            }
            if (blockName.empty() && action == "mesh-on")
            {
              mesh.blocks.clear();
            }
            if (!blockName.empty())
            {
              std::vector<ExplosionBlockReference> matches;
              for (const auto& block : reference->blocks)
              {
                if (block.identity.name == blockName)
                {
                  matches.push_back(block.identity);
                }
              }
              if (matches.size() != 1)
              {
                error = "Exploded block name is missing or ambiguous.";
                return false;
              }
              mesh.blocks = std::move(matches);
            }
          }
        }
      }
      else if (action == "on" || action == "components-on")
      {
        if (scope.empty())
        {
          error = "No visible mesh sources can be exploded.";
          return false;
        }
        for (const auto& source : scope)
        {
          if (std::find(next.sources.begin(),next.sources.end(),source) == next.sources.end())
          {
            next.sources.push_back(source);
            rebuild = true;
          }
        }
        next.enabled = true;
        rebuild = rebuild || next.placements.empty();
      }
      else if (action != "recalculate" && action != "offset")
      {
        error = "Unknown explosion operation.";
        return false;
      }
      if (rebuild)
      {
        if (next.sources.empty())
        {
          next.sources = scope;
        }
        std::vector<ExplosionPieceBounds> pieces;
        bool sourcesMode = next.grouping == ExplosionGrouping::Sources ||
          (next.grouping == ExplosionGrouping::Automatic && next.sources.size() > 1);
        for (const auto& source : next.sources)
        {
          const auto reference = explosion_reference(source,error);
          if (!reference)
          {
            return false;
          }
          const auto sourceData = result_data(source);
          const auto first = first_mesh_increment(*sourceData);
          const auto display = std::find_if(state.representations.begin(),state.representations.end(),[&](const auto& item)
          {
            return item.second.view_id == viewId && output_source(item.second.input) == source;
          });
          if (display == state.representations.end())
          {
            error = "The exploded source has no representation in this view.";
            return false;
          }
          double factor = 1;
          bool known = false;
          if (!scene_scale(display->second,factor,known,error))
          {
            return false;
          }
          if (next.sources.size() > 1 && !known)
          {
            error = "Cross-source explosion requires compatible known coordinate units.";
            return false;
          }
          if (sourcesMode)
          {
            bool found = false;
            ExplosionPieceBounds piece;
            piece.piece.source_id = source;
            for (const auto& part : result_mesh_parts(sourceData->increments[*first].dataset.get()))
            {
              std::array<double,6> bounds;
              if (!mesh_bounds(part.second,bounds))
              {
                continue;
              }
              for (auto& coordinate : bounds)
              {
                coordinate *= factor;
              }
              if (!found)
              {
                piece.bounds = bounds;
                found = true;
              }
              else
              {
                for (std::size_t a = 0; a < 3; ++a)
                {
                  piece.bounds[2*a] = std::min(piece.bounds[2*a],bounds[2*a]);
                  piece.bounds[2*a+1] = std::max(piece.bounds[2*a+1],bounds[2*a+1]);
                }
              }
            }
            if (found)
            {
              pieces.push_back(piece);
            }
          }
          else
          {
            for (const auto& block : reference->blocks)
            {
              ExplosionPieceBounds piece;
              piece.piece.source_id = source;
              piece.piece.block = block.identity;
              piece.bounds = block.bounds;
              for (auto& coordinate : piece.bounds)
              {
                coordinate *= factor;
              }
              pieces.push_back(piece);
            }
          }
        }
        std::stable_sort(pieces.begin(),pieces.end(),[](const auto& a,const auto& b)
        {
          return std::tie(a.piece.source_id,a.piece.block.kind,a.piece.block.model_id,a.piece.block.name) <
            std::tie(b.piece.source_id,b.piece.block.kind,b.piece.block.model_id,b.piece.block.name);
        });
        if (pieces.size() < 2 && action == "on" && next.sources.size() == 1)
        {
          const auto reference = explosion_reference(next.sources.front(),error);
          if (!reference || reference->blocks.empty())
          {
            error = "The source has no blocks with original element identities for element explosion.";
            return false;
          }
          next.enabled = false;
          next.meshes[next.sources.front()].enabled = true;
        }
        else
        {
          std::vector<ExplosionPlacement> placements;
          if (!calculate_explosion_layout(pieces,next.direction,next.gap,placements,error))
          {
            return false;
          }
          for (auto& piece : placements)
          {
            const auto old = std::find_if(next.placements.begin(),next.placements.end(),[&](const auto& item)
            {
              return piece.source_id == item.source_id && same_explosion_block(piece.block,item.block);
            });
            if (old != next.placements.end())
            {
              piece.manual = old->manual;
            }
          }
          next.placements = std::move(placements);
        }
        next.coordinate_unit = view->second.scene_unit;
      }
      if (action == "offset")
      {
        if (scope.size() != 1 || blockName.empty())
        {
          error = "Manual block offsets require exactly one source and a quoted block name.";
          return false;
        }
        const auto reference = explosion_reference(scope.front(),error);
        if (!reference)
        {
          return false;
        }
        std::vector<ExplosionBlockReference> matches;
        for (const auto& block : reference->blocks)
        {
          if (block.identity.name == blockName)
          {
            matches.push_back(block.identity);
          }
        }
        if (matches.size() != 1)
        {
          error = "Manual offset block is missing or ambiguous.";
          return false;
        }
        std::array<double,3> converted = offset;
        if (!unit.empty())
        {
          if (view->second.scene_unit.empty())
          {
            error = "An explicit offset unit requires a calibrated view.";
            return false;
          }
          for (std::size_t a = 0; a < 3; ++a)
          {
            if (!framework_units().convert(offset[a],unit,view->second.scene_unit,converted[a],error))
            {
              return false;
            }
          }
        }
        auto piece = std::find_if(next.placements.begin(),next.placements.end(),[&](const auto& item)
        {
          return item.source_id == scope.front() && same_explosion_block(item.block,matches.front());
        });
        if (piece == next.placements.end())
        {
          ExplosionPlacement placement;
          placement.source_id = scope.front();
          placement.block = matches.front();
          next.placements.push_back(placement);
          piece = std::prev(next.placements.end());
        }
        piece->manual = converted;
        if (std::find(next.sources.begin(),next.sources.end(),scope.front()) == next.sources.end())
        {
          next.sources.push_back(scope.front());
        }
        next.coordinate_unit = view->second.scene_unit;
      }
      if (next.enabled && !next.placements.empty() && next.coordinate_unit != view->second.scene_unit)
      {
        error = "The explosion scene length basis changed. Recalculate or reset the layout.";
        return false;
      }
      if (!valid_explosion(next,error))
      {
        return false;
      }
      std::map<std::string,CachedExplosionOutput> prepared;
      // Validate all visible outputs before publishing any settings or camera fit.
      for (const auto& item : state.representations)
      {
        const auto& display = item.second;
        const auto source = output_source(display.input);
        if (display.view_id != viewId || !display.visible || !explosion_enabled(next,source))
        {
          continue;
        }
        std::shared_ptr<const EvaluatedOutput> input,checked;
        const auto reference = explosion_reference(source,error);
        double factor = 1;
        bool known = false;
        if (!reference || !evaluate_unexploded_output(display,input,error) || !scene_scale(display,factor,known,error) ||
            !explode_output(next,display,*reference,factor,input,checked,error))
        {
          return false;
        }
        apply_explosion_pulse(*checked,next.pulse);
        prepared[display.id] = {input,checked,next.revision+1,display.increment_index,display.deformation,display.hidden_datasets};
      }
      next.revision = view->second.explosion.revision+1;
      auto staged = state;
      auto& target = staged.views.at(viewId);
      target.explosion = std::move(next);
      const bool enabled = target.explosion.enabled || std::any_of(target.explosion.meshes.begin(),target.explosion.meshes.end(),[](const auto& item)
      {
        return item.second.enabled;
      });
      if (!wasEnabled && enabled)
      {
        if (target.fit_request == std::numeric_limits<std::uint64_t>::max())
        {
          error = "Camera fit revision is exhausted.";
          return false;
        }
        target.fit_pending = true;
        ++target.fit_request;
      }
      state.swap(staged);
      explosion_cache = std::move(prepared);
      ++scene_revision;
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot change exploded view: ")+exception.what();
      return false;
    }
  }
}
