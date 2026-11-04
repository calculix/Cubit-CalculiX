#ifndef CORETOPOLOGYEVALUATION_HPP
#define CORETOPOLOGYEVALUATION_HPP

#include <map>

class CoreTopology;
class CoreMaterials;
class CoreJobs;

/** @brief Supply stored applied factors and job checks to numerical objective evaluation. */
class CoreTopologyEvaluation
{
public:
  /**
   * @brief Store physical element energies, then evaluate compliance and raw sensitivities.
   * @param topology Prepared case store receiving energies and the evaluation.
   * @param materials Borrowed store supplying this iteration's applied factors.
   * @param topology_id Existing compliance case.
   * @param iteration_id Positive, unevaluated iteration associated with a physical job.
   * @param energies Complete global-element-ID map of internal strain energies.
   * @return True after energy storage and numerical evaluation both succeed.
   * @note C=2*sum(Ue). Applied-factor sensitivities use -2*Ue/s_hat; continuous
   * SIMP multiplication is a surrogate for grouped solves. No filtering occurs.
   * If evaluation fails after energy storage, those energies remain stored; a
   * retry is not guaranteed to be accepted as an unevaluated operation.
   */
  static bool evaluate_compliance(CoreTopology& topology, const CoreMaterials& materials, int topology_id, int iteration_id, const std::map<int, double>& energies);
  /**
   * @brief Validate job/element coverage and commit the raw displacement evaluation.
   * @param topology Prepared displacement case store receiving the evaluation.
   * @param materials Borrowed store supplying applied factors and grouping metadata.
   * @param jobs Borrowed job store; the physical job must be finished.
   * @param topology_id Existing displacement case.
   * @param iteration_id Positive, unevaluated iteration with a stored design.
   * @param value Finite, nonnegative maximum displacement objective.
   * @param node_id Positive controlling node ID selected from the monitored nodeset.
   * @param element_integrals Complete global-element-ID map of physical S : adjoint ME integrals.
   * @return True after the numerical evaluation is committed; false on invalid data.
   * @note A positive objective additionally requires a finished adjoint job. Zero
   * objective requires zero integrals and does not require an adjoint. No factor
   * of one half enters the bilinear integral. No filtering occurs here.
   */
  static bool evaluate_displacement(CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs, int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals);
};

#endif
