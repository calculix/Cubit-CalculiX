#include "ExplosionDefinition.hpp"
#include <cmath>
namespace Postprocessor
{
  double explosion_pulse_factor(const ExplosionState::Pulse& pulse)
  {
    if (!pulse.enabled)
    {
      return 1.0;
    }
    constexpr double pi = 3.14159265358979323846;
    const double phase = std::fmod(pulse.elapsed_seconds,pulse.period_seconds)/pulse.period_seconds;
    const double weight = 0.5+0.5*std::cos(2.0*pi*phase);
    return pulse.minimum+(pulse.maximum-pulse.minimum)*weight;
  }
}
