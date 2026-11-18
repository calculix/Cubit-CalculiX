#include "ExplosionDefinition.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
namespace Postprocessor
{
  bool same_explosion_block(const ExplosionBlockReference& a,const ExplosionBlockReference& b)
  {
    if (a.kind != b.kind)
    {
      return false;
    }
    if (a.model_id >= 0 && b.model_id >= 0)
    {
      return a.model_id == b.model_id;
    }
    return a.name == b.name;
  }
  bool valid_explosion_pulse(const ExplosionState::Pulse& pulse,std::string& error)
  {
    error.clear();
    if (!std::isfinite(pulse.period_seconds) || pulse.period_seconds < 0.1 || pulse.period_seconds > 3600 ||
        !std::isfinite(pulse.minimum) || !std::isfinite(pulse.maximum) || pulse.minimum < 0 ||
        pulse.maximum > 1 || pulse.minimum > pulse.maximum || !std::isfinite(pulse.elapsed_seconds) || pulse.elapsed_seconds < 0 ||
        static_cast<int>(pulse.target) < 0 || static_cast<int>(pulse.target) > 2)
    {
      error = "Pulse period must be 0.1..3600 seconds and range must satisfy 0 <= minimum <= maximum <= 1.";
      return false;
    }
    return true;
  }
  bool valid_explosion(const ExplosionState& value,std::string& error)
  {
    if (!valid_explosion_pulse(value.pulse,error))
    {
      return false;
    }
    if (!std::isfinite(value.amount) || value.amount < 0 || value.amount > 100 || !std::isfinite(value.gap) || value.gap < 0 || value.gap > 1 ||
        value.sources.size() > ExplosionDefaults::maximum_pieces || value.placements.size() > ExplosionDefaults::maximum_pieces ||
        value.meshes.size() > ExplosionDefaults::maximum_pieces ||
        static_cast<int>(value.grouping) < 0 || static_cast<int>(value.grouping) > 2 ||
        static_cast<int>(value.direction) < 0 || static_cast<int>(value.direction) > 4)
    {
      error = "Explosion amounts must be finite (0..100), gap 0..1, with at most 4096 pieces.";
      return false;
    }
    for (const auto& item : value.meshes)
    {
      if (item.first.empty() || !std::isfinite(item.second.amount) || item.second.amount < 0 || item.second.amount > 100 ||
          item.second.blocks.size() > ExplosionDefaults::maximum_pieces)
      {
        error = "Invalid mesh explosion scope or amount.";
        return false;
      }
    }
    for (const auto& piece : value.placements)
    {
      if (piece.source_id.empty() || piece.block.model_id < -1)
      {
        error = "Explosion pieces require valid source and block identities.";
        return false;
      }
      for (std::size_t axis = 0; axis < 3; ++axis)
      {
        if (!std::isfinite(piece.offset[axis]) || !std::isfinite(piece.manual[axis]) ||
            !std::isfinite(value.amount*(piece.offset[axis]+piece.manual[axis])))
        {
          error = "Explosion offsets must be finite.";
          return false;
        }
      }
    }
    return true;
  }
  bool calculate_explosion_layout(const std::vector<ExplosionPieceBounds>& pieces,ExplosionDirection direction,double gap,
    std::vector<ExplosionPlacement>& output,std::string& error)
  {
    error.clear();
    if (pieces.size() < 2 || pieces.size() > ExplosionDefaults::maximum_pieces || !std::isfinite(gap) || gap < 0 || gap > 1)
    {
      error = "Component explosion needs at least two pieces and a finite gap (0..1).";
      return false;
    }
    auto bounds = pieces.front().bounds;
    std::array<double,3> center = {0,0,0};
    for (const auto& piece : pieces)
    {
      for (std::size_t a = 0; a < 3; ++a)
      {
        if (!std::isfinite(piece.bounds[2*a]) || !std::isfinite(piece.bounds[2*a+1]) || piece.bounds[2*a] > piece.bounds[2*a+1])
        {
          error = "Explosion reference bounds are invalid.";
          return false;
        }
        bounds[2*a] = std::min(bounds[2*a],piece.bounds[2*a]);
        bounds[2*a+1] = std::max(bounds[2*a+1],piece.bounds[2*a+1]);
        center[a] += (piece.bounds[2*a]/2+piece.bounds[2*a+1]/2)/static_cast<double>(pieces.size());
      }
    }
    const double spacing = gap*std::hypot(bounds[1]-bounds[0],bounds[3]-bounds[2],bounds[5]-bounds[4]);
    if (!std::isfinite(spacing))
    {
      error = "Explosion reference extents overflow.";
      return false;
    }
    std::vector<ExplosionPlacement> best;
    double bestCost = std::numeric_limits<double>::infinity();
    for (int axis : {2,0,1})
    {
      if (direction != ExplosionDirection::Automatic && direction != ExplosionDirection::Radial && static_cast<int>(direction) != axis+1)
      {
        continue;
      }
      std::vector<ExplosionPlacement> candidate;
      for (const auto& piece : pieces)
      {
        candidate.push_back(piece.piece);
        candidate.back().offset = {0,0,0};
      }
      if (direction == ExplosionDirection::Radial)
      {
        for (std::size_t i = 0; i < pieces.size(); ++i)
        {
          for (std::size_t a = 0; a < 3; ++a)
          {
            candidate[i].offset[a] = pieces[i].bounds[2*a]/2+pieces[i].bounds[2*a+1]/2-center[a];
          }
        }
        best = std::move(candidate);
        break;
      }
      std::vector<std::size_t> order(pieces.size());
      std::iota(order.begin(),order.end(),0);
      std::stable_sort(order.begin(),order.end(),[&](auto a,auto b)
      {
        return pieces[a].bounds[2*axis]/2+pieces[a].bounds[2*axis+1]/2 < pieces[b].bounds[2*axis]/2+pieces[b].bounds[2*axis+1]/2;
      });
      // Fixed center order: shift[j]-shift[j-1] >= high[j-1]+gap-low[j].
      // Subtract cumulative required spacings to obtain an isotonic regression
      // of -cumulative. Pool-adjacent-violators solves the least-squares problem
      // exactly for this order, including independently separated clusters.
      std::vector<double> cumulative(order.size(),0);
      struct Pool
      {
        std::size_t first, last;
        double mean;
      };
      std::vector<Pool> pools;
      for (std::size_t j = 0; j < order.size(); ++j)
      {
        if (j > 0)
        {
          cumulative[j] = cumulative[j-1]+pieces[order[j-1]].bounds[2*axis+1]+spacing-pieces[order[j]].bounds[2*axis];
        }
        pools.push_back({j,j,-cumulative[j]});
        while (pools.size() > 1 && pools[pools.size()-2].mean > pools.back().mean)
        {
          const auto right = pools.back();
          pools.pop_back();
          auto& left = pools.back();
          const double leftCount = static_cast<double>(left.last-left.first+1);
          const double rightCount = static_cast<double>(right.last-right.first+1);
          const double weight = leftCount/(leftCount+rightCount);
          left.mean = left.mean*weight+right.mean*(1-weight);
          left.last = right.last;
        }
      }
      double cost = 0;
      for (const auto& pool : pools)
      {
        for (auto j = pool.first; j <= pool.last; ++j)
        {
          const auto shift = cumulative[j]+pool.mean;
          candidate[order[j]].offset[axis] = shift;
          cost += shift*shift;
        }
      }
      if (std::isfinite(cost) && cost < bestCost)
      {
        bestCost = cost;
        best = std::move(candidate);
      }
    }
    if (best.empty())
    {
      error = "Cannot calculate a finite explosion layout.";
      return false;
    }
    output = std::move(best);
    return true;
  }
}
