#ifndef POSTPROCESSOR_CAMERATIMINGS_HPP
#define POSTPROCESSOR_CAMERATIMINGS_HPP

#include "Postprocessor/Commands/views/CameraCommandTiming.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <optional>

enum class CameraTimingStage
{
  InputWait,
  ApplicationCameraUpdate,
  Formatting,
  CommandDispatch,
  CommandHandler,
  CoreCameraUpdate,
  CommandOutsideHandler,
  HandlerOutsideCore,
  SceneSynchronization,
  TriadUpdate,
  RenderCall,
  PresentationWait,
  FrameInterval,
  Count
};

struct CameraTimingMetric
{
  std::uint64_t count = 0;
  double totalMilliseconds = 0;
  double maximumMilliseconds = 0;
  void add(double milliseconds) noexcept;
  double average() const noexcept;
};

// GUI-owner-thread recorder. Fixed storage, no per-frame allocation or output.
// IDs are never reused: late callbacks cannot write into a newer gesture.
// RenderCall is CPU wall time (including driver waits), not GPU execution time.
class CameraTimings
{
public:
  using Clock = std::chrono::steady_clock;
  using Id = std::uint64_t;
  struct Stamp
  {
    Id id = 0;
    Clock::time_point time;
  };
  struct Report
  {
    Id id = 0;
    bool closed = false;
    bool cancelled = false;
    unsigned references = 0;
    std::uint64_t inputs = 0;
    std::uint64_t coalescedInputs = 0;
    std::uint64_t frames = 0;
    std::uint64_t presented = 0;
    std::uint64_t unpresented = 0;
    std::uint64_t unrendered = 0;
    std::uint64_t superseded = 0;
    std::uint64_t droppedGestures = 0;
    std::uint64_t unpairedDispatches = 0;
    std::array<CameraTimingMetric, static_cast<std::size_t>(CameraTimingStage::Count)> metrics;
    Clock::time_point latestInput;
    Clock::time_point previousFrame;
    bool inputPending = false;
    bool hasPreviousFrame = false;
  };

  void begin(bool enabled) noexcept;
  void end(bool cancelled) noexcept;
  void input() noexcept;
  void consume_input() noexcept;
  Stamp stamp() const noexcept;
  Stamp frame_stamp() const noexcept;
  void record(CameraTimingStage stage, Stamp started) noexcept;
  void record_dispatch(Stamp dispatch, const CameraCommandTiming::Result& result) noexcept;
  void queue_frame() noexcept;
  void begin_frame() noexcept;
  void await_presentation() noexcept;
  void presented() noexcept;
  void abandon_frames() noexcept;
  bool reports_ready() const noexcept;
  std::optional<Report> take_report() noexcept;
  static const char* name(CameraTimingStage stage) noexcept;

private:
  Report* find(Id id) noexcept;
  bool contains(Id id) const noexcept;
  void release(Id id, bool unpresented) noexcept;
  std::array<Report, 8> reports = {};
  Id nextId = 1;
  Id active = 0;
  Id queued = 0;
  Id inFlight = 0;
  Stamp presentation;
  bool gestureOpen = false;
  std::uint64_t droppedGestures = 0;
};
#endif
