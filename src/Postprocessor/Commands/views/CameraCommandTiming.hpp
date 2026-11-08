#ifndef POSTPROCESSOR_CAMERACOMMANDTIMING_HPP
#define POSTPROCESSOR_CAMERACOMMANDTIMING_HPP

#include <chrono>
#include <cstdint>

// A probe for one synchronous camera dispatch, shared by its caller and handler.
// Thread-local scopes isolate nested dispatches. No Core/GUI objects are retained.
// All guards must end before their enclosing dispatch; copies are forbidden.
class CameraCommandTiming
{
public:
  using Clock = std::chrono::steady_clock;
  struct Result
  {
    bool captured = false;
    std::uint64_t handlers = 0;
    std::uint64_t coreUpdates = 0;
    double dispatchMilliseconds = 0;
    double handlerMilliseconds = 0;
    double coreMilliseconds = 0;
  };

  explicit CameraCommandTiming(bool enabled) noexcept;
  ~CameraCommandTiming();
  CameraCommandTiming(const CameraCommandTiming&) = delete;
  CameraCommandTiming& operator=(const CameraCommandTiming&) = delete;
  Result finish() noexcept;

  class Handler
  {
  public:
    Handler() noexcept;
    ~Handler();
    Handler(const Handler&) = delete;
    Handler& operator=(const Handler&) = delete;
  private:
    CameraCommandTiming* owner = nullptr;
    Clock::time_point started;
    bool outermost = false;
  };

  class CoreUpdate
  {
  public:
    CoreUpdate() noexcept;
    ~CoreUpdate();
    CoreUpdate(const CoreUpdate&) = delete;
    CoreUpdate& operator=(const CoreUpdate&) = delete;
  private:
    CameraCommandTiming* owner = nullptr;
    Clock::time_point started;
    bool outermost = false;
  };

private:
  static thread_local CameraCommandTiming* current;
  CameraCommandTiming* previous = nullptr;
  Clock::time_point started;
  Result result;
  unsigned handlerDepth = 0;
  unsigned coreDepth = 0;
  bool finished = false;
};
#endif
