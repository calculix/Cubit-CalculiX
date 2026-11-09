#ifndef CORETOPOLOGYLEVELSETUPDATE_HPP
#define CORETOPOLOGYLEVELSETUPDATE_HPP

#include "CoreTopologyLevelSetStructs.hpp"
#include <map>
#include <memory>

struct TopologyLevelSetDesign;

class CoreTopologyLevelSetUpdate
{
public:
  /// Calculate the full-move QP once; optionally reuse the completed sensitivity cache.
  /// The successful transient proposal takes ownership; source checks always query Cubit.
  static bool build_proposal(const TopologyLevelSetDesign& source, const TopologyLevelSetEvaluation& evaluation, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, double target_volume_fraction, TopologyLevelSetProposal& proposal, std::shared_ptr<CoreTopologyLevelSetGeometryCache> source_cache = {});
  /// Accept a proposal from this solved source by actual-volume backtracking.
  static bool accept_proposal(const TopologyLevelSetDesign& source, const TopologyLevelSetProposal& proposal, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, TopologyLevelSetDesign& next, std::map<int, double>& factors, TopologyLevelSetUpdateDiagnostics& diagnostics);

  /**
   * @brief Build a regularized, bounded outgoing field using a solved source design.
   * @return False with empty outputs and a Cubit error on failure.
   * @note Aliasing errors leave inputs intact rather than clearing aliased outputs.
   * Other failures clear outputs. Output records/maps must also be distinct.
   * Uses a positive element-clique graph
   * penalty and a diagonal volume-based proximal metric, not an FE Laplacian or
   * an OpenPisco update. A convex box QP enforces a linearized volume constraint;
   * backtracking re-evaluates actual Cubit-volume-weighted fractions. Infeasible
   * initial designs must reduce actual volume; feasible designs must remain
   * feasible within Core tolerance. No additional physical solve is performed.
   * The ordinary proposal residual belongs to source.iteration_id and uses its
   * full configured move limit, before backtracking. Incoming diagnostics on
   * next must not certify convergence of next. Graph regularization is part of
   * the local model; no monotone physical compliance or global optimum is promised.
   * Current mesh/domain and source evaluation must match. No model, material,
   * job, selection or iteration store is modified. Public execution remains gated.
   */
  static bool build_update(const TopologyLevelSetDesign& source, const TopologyLevelSetEvaluation& evaluation, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, double target_volume_fraction, TopologyLevelSetDesign& next, std::map<int, double>& factors, TopologyLevelSetUpdateDiagnostics& diagnostics);
};

#endif
