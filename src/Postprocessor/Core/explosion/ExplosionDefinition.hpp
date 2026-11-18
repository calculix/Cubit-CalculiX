#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <map>
#include <string>
#include <vector>
namespace Postprocessor
{
  namespace ExplosionDefaults
  {
    inline constexpr double amount = 1;
    inline constexpr double gap = .05;
    inline constexpr double mesh_amount = .10;
    inline constexpr int preview_delay_ms = 40;
    inline constexpr double pulse_period_seconds = 3.0;
    inline constexpr int pulse_interval_ms = 16;
    inline constexpr std::size_t maximum_points = 10000000;
    inline constexpr std::size_t maximum_pieces = 4096;
  }
  enum class ExplosionGrouping
  {
    Automatic, Sources, Blocks
  };
  enum class ExplosionDirection
  {
    Automatic, X, Y, Z, Radial
  };
  struct ExplosionBlockReference
  {
    std::string kind, name;
    int model_id = -1;
  };
  bool same_explosion_block(const ExplosionBlockReference& a,const ExplosionBlockReference& b);
  struct ExplosionPlacement
  {
    std::string source_id;
    // Empty kind identifies a source assembly, otherwise a durable block identity.
    ExplosionBlockReference block;
    std::array<double,3> offset = {0,0,0}, manual = {0,0,0};
  };
  struct MeshExplosion
  {
    bool enabled = false;
    double amount = ExplosionDefaults::mesh_amount;
    std::vector<ExplosionBlockReference> blocks;
  };
  struct ExplosionState
  {
    bool enabled = false;
    double amount = ExplosionDefaults::amount, gap = ExplosionDefaults::gap;
    ExplosionGrouping grouping = ExplosionGrouping::Automatic;
    ExplosionDirection direction = ExplosionDirection::Automatic;
    std::string coordinate_unit;
    std::vector<std::string> sources;
    std::vector<ExplosionPlacement> placements;
    std::map<std::string,MeshExplosion> meshes;
    struct Pulse
    {
      enum class Target
      {
        Components, Mesh, Both
      };
      double period_seconds = ExplosionDefaults::pulse_period_seconds;
      double minimum = 0.0, maximum = 1.0;
      Target target = Target::Both;
      bool enabled = false; // Playback and elapsed time are runtime only.
      double elapsed_seconds = 0.0;
      std::uint64_t settings_revision = 0; // Runtime notification, separate from geometry.
    } pulse;
    std::uint64_t revision = 1; // Runtime only.
  };
  struct ExplosionPieceBounds
  {
    ExplosionPlacement piece;
    std::array<double,6> bounds;
  };
  bool valid_explosion_pulse(const ExplosionState::Pulse& pulse,std::string& error);
  bool valid_explosion(const ExplosionState& value,std::string& error);
  // Validated settings; one cosine cycle starts fully separated, with zero
  // velocity at both extremes. Disabled playback always restores static amounts.
  double explosion_pulse_factor(const ExplosionState::Pulse& pulse);
  // Reference AABBs; inclusive gap is a fraction of their union diagonal.
  // Auto minimizes squared translation among axis layouts. Stable input order
  // breaks equal-center ties. The assembly's arithmetic mean center is retained.
  bool calculate_explosion_layout(const std::vector<ExplosionPieceBounds>& pieces,
    ExplosionDirection direction,double gap,std::vector<ExplosionPlacement>& output,std::string& error);
}
