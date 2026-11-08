#include "CameraTimings.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

void CameraTimingMetric::add(double milliseconds) noexcept
{
  if (!std::isfinite(milliseconds) || milliseconds < 0)
  {
    return;
  }
  ++count;
  totalMilliseconds += milliseconds;
  maximumMilliseconds = std::max(maximumMilliseconds, milliseconds);
}

double CameraTimingMetric::average() const noexcept
{
  return count == 0 ? 0 : totalMilliseconds / static_cast<double>(count);
}

CameraTimings::Report* CameraTimings::find(Id id) noexcept
{
  if (id != 0)
  {
    for (auto& report : reports)
    {
      if (report.id == id)
      {
        return &report;
      }
    }
  }
  return nullptr;
}

bool CameraTimings::contains(Id id) const noexcept
{
  return id != 0 && std::any_of(reports.begin(), reports.end(), [id](const Report& report)
  {
    return report.id == id;
  });
}

void CameraTimings::begin(bool enabled) noexcept
{
  end(true);
  gestureOpen = true;
  if (!enabled)
  {
    return;
  }
  for (auto& report : reports)
  {
    if (report.id == 0 && nextId != std::numeric_limits<Id>::max())
    {
      report = {};
      report.id = nextId++;
      active = report.id;
      return;
    }
  }
  ++droppedGestures;
}

void CameraTimings::end(bool cancelled) noexcept
{
  if (auto* report = find(active))
  {
    report->closed = true;
    report->cancelled = cancelled;
    report->inputPending = false;
  }
  active = 0;
  gestureOpen = false;
}

CameraTimings::Stamp CameraTimings::stamp() const noexcept
{
  return contains(active) ? Stamp{active, Clock::now()} : Stamp{};
}

CameraTimings::Stamp CameraTimings::frame_stamp() const noexcept
{
  return contains(inFlight) ? Stamp{inFlight, Clock::now()} : Stamp{};
}

void CameraTimings::record(CameraTimingStage stage, Stamp started) noexcept
{
  const auto index = static_cast<std::size_t>(stage);
  if (auto* report = find(started.id))
  {
    if (index < report->metrics.size())
    {
      report->metrics[index].add(std::chrono::duration<double, std::milli>(Clock::now() - started.time).count());
    }
  }
}

void CameraTimings::record_dispatch(Stamp dispatch, const CameraCommandTiming::Result& result) noexcept
{
  auto* report = find(dispatch.id);
  if (report == nullptr)
  {
    return;
  }
  if (!result.captured || !std::isfinite(result.dispatchMilliseconds) || result.dispatchMilliseconds < 0)
  {
    ++report->unpairedDispatches;
    return;
  }
  report->metrics[static_cast<std::size_t>(CameraTimingStage::CommandDispatch)].add(result.dispatchMilliseconds);
  // Derive residuals from a single matched invocation, never from aggregates.
  // Multiple/reentrant handlers are explicitly unpaired rather than double-counted.
  if (result.handlers != 1 || result.coreUpdates > 1 ||
      !std::isfinite(result.handlerMilliseconds) || !std::isfinite(result.coreMilliseconds) ||
      result.coreMilliseconds < 0 || result.handlerMilliseconds < result.coreMilliseconds ||
      result.dispatchMilliseconds < result.handlerMilliseconds)
  {
    ++report->unpairedDispatches;
    return;
  }
  report->metrics[static_cast<std::size_t>(CameraTimingStage::CommandHandler)].add(result.handlerMilliseconds);
  report->metrics[static_cast<std::size_t>(CameraTimingStage::CommandOutsideHandler)].add(
    result.dispatchMilliseconds - result.handlerMilliseconds);
  if (result.coreUpdates == 1)
  {
    report->metrics[static_cast<std::size_t>(CameraTimingStage::CoreCameraUpdate)].add(result.coreMilliseconds);
    report->metrics[static_cast<std::size_t>(CameraTimingStage::HandlerOutsideCore)].add(
      result.handlerMilliseconds - result.coreMilliseconds);
  }
}

void CameraTimings::input() noexcept
{
  if (auto* report = find(active))
  {
    ++report->inputs;
    if (report->inputPending)
    {
      ++report->coalescedInputs;
    }
    report->latestInput = Clock::now();
    report->inputPending = true;
  }
}

void CameraTimings::consume_input() noexcept
{
  if (auto* report = find(active))
  {
    if (report->inputPending)
    {
      record(CameraTimingStage::InputWait, {active, report->latestInput});
      report->inputPending = false;
    }
  }
}

void CameraTimings::release(Id id, bool unpresented) noexcept
{
  if (auto* report = find(id))
  {
    if (report->references > 0)
    {
      --report->references;
    }
    if (unpresented)
    {
      ++report->unpresented;
    }
  }
}

void CameraTimings::queue_frame() noexcept
{
  if (active == 0 && gestureOpen && queued != 0)
  {
    if (auto* previous = find(queued))
    {
      ++previous->superseded;
    }
    release(queued, false);
    queued = 0;
  }
  if (active == 0 || active == queued)
  {
    return;
  }
  if (auto* previous = find(queued))
  {
    ++previous->superseded;
    release(queued, false);
  }
  queued = active;
  if (auto* report = find(queued))
  {
    ++report->references;
  }
}

void CameraTimings::begin_frame() noexcept
{
  // Normally prevented by the viewport's presentation gate. Do not misattribute
  // an old frame if resize/context recreation interrupts that gate.
  if (inFlight != 0)
  {
    release(inFlight, true);
  }
  presentation = {};
  inFlight = queued;
  queued = 0;
  if (auto* report = find(inFlight))
  {
    ++report->frames;
    const auto now = Clock::now();
    if (report->hasPreviousFrame)
    {
      record(CameraTimingStage::FrameInterval, {inFlight, report->previousFrame});
    }
    report->previousFrame = now;
    report->hasPreviousFrame = true;
  }
}

void CameraTimings::await_presentation() noexcept
{
  presentation = frame_stamp();
}

void CameraTimings::presented() noexcept
{
  if (inFlight == 0 || presentation.id != inFlight)
  {
    return;
  }
  if (auto* report = find(inFlight))
  {
    record(CameraTimingStage::PresentationWait, presentation);
    ++report->presented;
    release(inFlight, false);
  }
  inFlight = 0;
  presentation = {};
}

void CameraTimings::abandon_frames() noexcept
{
  if (auto* report = find(queued))
  {
    ++report->unrendered;
  }
  release(queued, false);
  release(inFlight, true);
  queued = 0;
  inFlight = 0;
  presentation = {};
}

std::optional<CameraTimings::Report> CameraTimings::take_report() noexcept
{
  if (gestureOpen)
  {
    return std::nullopt;
  }
  for (auto& report : reports)
  {
    if (report.id != 0 && report.closed && report.references == 0)
    {
      auto result = report;
      report = {};
      result.droppedGestures = droppedGestures;
      droppedGestures = 0;
      return result;
    }
  }
  return std::nullopt;
}

bool CameraTimings::reports_ready() const noexcept
{
  return !gestureOpen && std::any_of(reports.begin(), reports.end(), [](const Report& report)
  {
    return report.id != 0 && report.closed && report.references == 0;
  });
}

const char* CameraTimings::name(CameraTimingStage stage) noexcept
{
  switch (stage)
  {
    case CameraTimingStage::InputWait:
      return "input wait";
    case CameraTimingStage::ApplicationCameraUpdate:
      return "direct camera update";
    case CameraTimingStage::Formatting:
      return "formatting";
    case CameraTimingStage::CommandDispatch:
      return "command dispatch";
    case CameraTimingStage::CommandHandler:
      return "camera handler";
    case CameraTimingStage::CoreCameraUpdate:
      return "Core camera update";
    case CameraTimingStage::CommandOutsideHandler:
      return "dispatch outside handler";
    case CameraTimingStage::HandlerOutsideCore:
      return "handler outside Core";
    case CameraTimingStage::SceneSynchronization:
      return "scene sync";
    case CameraTimingStage::TriadUpdate:
      return "triad";
    case CameraTimingStage::RenderCall:
      return "Render wall time";
    case CameraTimingStage::PresentationWait:
      return "presentation wait";
    case CameraTimingStage::FrameInterval:
      return "frame interval";
    default:
      return "unknown";
  }
}
