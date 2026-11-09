#ifndef POSTPROCESSOR_ANIMATIONDEFINITION_HPP
#define POSTPROCESSOR_ANIMATIONDEFINITION_HPP
namespace Postprocessor
{
  namespace AnimationDefaults
  {
    inline constexpr int frames_per_second = 10;
    inline constexpr int minimum_fps = 1;
    inline constexpr int maximum_fps = 120;
    inline constexpr bool loop = false;
  }
  struct AnimationDefinition
  {
    int frames_per_second = AnimationDefaults::frames_per_second;
    bool loop = AnimationDefaults::loop;
    // Transient playback request; never persist as an automatically running session.
    bool playing = false;
  };
}
#endif
