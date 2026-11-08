#include "CameraCommandTiming.hpp"

thread_local CameraCommandTiming* CameraCommandTiming::current = nullptr;

CameraCommandTiming::CameraCommandTiming(bool enabled) noexcept : previous(current)
{
  // Even a disabled nested dispatch masks the outer probe.
  current = this;
  result.captured = enabled;
  if (enabled)
  {
    started = Clock::now();
  }
}

CameraCommandTiming::~CameraCommandTiming()
{
  finish();
}

CameraCommandTiming::Result CameraCommandTiming::finish() noexcept
{
  if (!finished)
  {
    if (result.captured)
    {
      result.dispatchMilliseconds = std::chrono::duration<double, std::milli>(Clock::now() - started).count();
    }
    current = previous;
    finished = true;
  }
  return result;
}

CameraCommandTiming::Handler::Handler() noexcept
{
  if (current != nullptr && current->result.captured && !current->finished)
  {
    owner = current;
    ++owner->result.handlers;
    outermost = owner->handlerDepth++ == 0;
    if (outermost)
    {
      started = Clock::now();
    }
  }
}

CameraCommandTiming::Handler::~Handler()
{
  if (owner != nullptr)
  {
    --owner->handlerDepth;
    if (outermost)
    {
      owner->result.handlerMilliseconds += std::chrono::duration<double, std::milli>(Clock::now() - started).count();
    }
  }
}

CameraCommandTiming::CoreUpdate::CoreUpdate() noexcept
{
  if (current != nullptr && current->result.captured && !current->finished && current->handlerDepth != 0)
  {
    owner = current;
    ++owner->result.coreUpdates;
    outermost = owner->coreDepth++ == 0;
    if (outermost)
    {
      started = Clock::now();
    }
  }
}

CameraCommandTiming::CoreUpdate::~CoreUpdate()
{
  if (owner != nullptr)
  {
    --owner->coreDepth;
    if (outermost)
    {
      owner->result.coreMilliseconds += std::chrono::duration<double, std::milli>(Clock::now() - started).count();
    }
  }
}
