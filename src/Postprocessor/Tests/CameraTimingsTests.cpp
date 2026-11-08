#include "Postprocessor/GUI/Viewport/CameraTimings.hpp"
#include <iostream>
#include <limits>

namespace
{
int failures = 0;
void check(bool value, const char* message)
{
  if (!value)
  {
    ++failures;
    std::cerr << message << '\n';
  }
}
std::size_t index(CameraTimingStage stage)
{
  return static_cast<std::size_t>(stage);
}
}

int main()
{
  CameraTimingMetric metric;
  metric.add(2);
  metric.add(6);
  metric.add(-1);
  metric.add(std::numeric_limits<double>::infinity());
  check(metric.count == 2 && metric.average() == 4 && metric.maximumMilliseconds == 6,
    "Metric must aggregate finite nonnegative samples.");

  CameraTimings disabled;
  disabled.begin(false);
  disabled.input();
  disabled.queue_frame();
  disabled.begin_frame();
  check(disabled.stamp().id == 0 && disabled.frame_stamp().id == 0, "Disabled diagnostics must not arm clocks.");
  disabled.end(false);
  disabled.await_presentation();
  disabled.presented();
  check(!disabled.take_report(), "Disabled diagnostics must not produce reports.");

  CameraTimings timings;
  timings.begin(true);
  const auto first = timings.stamp();
  timings.record(CameraTimingStage::Formatting, first);
  timings.input();
  timings.input();
  timings.consume_input();
  timings.queue_frame();
  timings.begin_frame();
  timings.await_presentation();
  timings.end(false);
  check(!timings.take_report(), "Released gesture must wait for its pending presentation.");
  timings.begin(true);
  const auto second = timings.stamp();
  check(second.id != first.id, "Gesture IDs must never be reused.");
  timings.presented();
  check(!timings.reports_ready(), "Reporting must wait while another gesture is active.");
  timings.end(false);
  const auto firstReport = timings.take_report();
  check(firstReport && firstReport->id == first.id && firstReport->presented == 1 &&
    firstReport->inputs == 2 && firstReport->coalescedInputs == 1 &&
    firstReport->metrics[index(CameraTimingStage::PresentationWait)].count == 1,
    "Late presentation must finish the original gesture.");
  const auto secondReport = timings.take_report();
  check(secondReport && secondReport->id == second.id && secondReport->presented == 0,
    "New gesture must not inherit an old presentation.");
  timings.begin(true);
  timings.record(CameraTimingStage::Formatting, first);
  timings.end(false);
  const auto staleReport = timings.take_report();
  check(staleReport && staleReport->metrics[index(CameraTimingStage::Formatting)].count == 0,
    "Reclaimed report slots must reject stale stamps.");

  CameraTimings closing;
  closing.begin(true);
  closing.queue_frame();
  closing.end(false);
  check(!closing.reports_ready(), "A queued final frame must retain its gesture.");
  closing.begin_frame();
  closing.presented();
  check(!closing.reports_ready(), "Unarmed composition notification must not acknowledge a frame.");
  closing.await_presentation();
  closing.presented();
  check(closing.take_report().has_value(), "Final frame may render after release.");

  CameraTimings hidden;
  hidden.begin(true);
  hidden.queue_frame();
  hidden.begin_frame();
  hidden.await_presentation();
  hidden.queue_frame();
  hidden.end(true);
  hidden.abandon_frames();
  const auto hiddenReport = hidden.take_report();
  check(hiddenReport && hiddenReport->cancelled && hiddenReport->frames == 1 &&
    hiddenReport->unpresented == 1 && hiddenReport->unrendered == 1,
    "Hiding must distinguish rendered-but-unpresented and never-rendered requests.");

  CameraTimings superseded;
  superseded.begin(true);
  const auto old = superseded.stamp();
  superseded.queue_frame();
  superseded.end(false);
  superseded.begin(false);
  superseded.queue_frame();
  superseded.end(false);
  const auto supersededReport = superseded.take_report();
  check(supersededReport && supersededReport->id == old.id && supersededReport->superseded == 1,
    "An untracked new gesture must not render under an old queued ticket.");

  CameraTimings bounded;
  for (unsigned count = 0; count < 9; ++count)
  {
    bounded.begin(true);
    bounded.end(false);
  }
  unsigned reports = 0;
  std::uint64_t dropped = 0;
  while (const auto report = bounded.take_report())
  {
    ++reports;
    dropped += report->droppedGestures;
  }
  check(reports == 8 && dropped == 1, "Recorder must bound memory and account for overflow.");
  CameraTimings paired;
  paired.begin(true);
  const auto pairedStamp = paired.stamp();
  paired.record_dispatch(pairedStamp, {true, 1, 1, 100, 90, 80});
  paired.record_dispatch(pairedStamp, {true, 1, 1, 80, 1, 0.5});
  paired.end(false);
  const auto pairedReport = paired.take_report();
  check(pairedReport && pairedReport->unpairedDispatches == 0 &&
    pairedReport->metrics[index(CameraTimingStage::CommandOutsideHandler)].count == 2 &&
    pairedReport->metrics[index(CameraTimingStage::CommandOutsideHandler)].maximumMilliseconds == 79 &&
    pairedReport->metrics[index(CameraTimingStage::HandlerOutsideCore)].maximumMilliseconds == 10,
    "Residuals must be paired per call, not computed by subtracting unrelated maxima.");

  CameraTimings missing;
  missing.begin(true);
  const auto missingStamp = missing.stamp();
  missing.record_dispatch(missingStamp, {true, 0, 0, 2, 0, 0});
  missing.record_dispatch(missingStamp, {true, 2, 2, 3, 2, 1});
  missing.record_dispatch(missingStamp, {true, 1, 1, 1, 2, 0.5});
  missing.end(false);
  const auto missingReport = missing.take_report();
  check(missingReport && missingReport->unpairedDispatches == 3 &&
    missingReport->metrics[index(CameraTimingStage::CommandDispatch)].count == 3 &&
    missingReport->metrics[index(CameraTimingStage::CommandOutsideHandler)].count == 0,
    "Missing, ambiguous and inconsistent pairs must retain total timing but reject residuals.");

  CameraCommandTiming probe(true);
  {
    const CameraCommandTiming::Handler handler;
    {
      const CameraCommandTiming::CoreUpdate core;
    }
  }
  const auto measured = probe.finish();
  check(measured.captured && measured.handlers == 1 && measured.coreUpdates == 1 &&
    measured.dispatchMilliseconds >= measured.handlerMilliseconds &&
    measured.handlerMilliseconds >= measured.coreMilliseconds,
    "Handler and Core guards must remain inside their dispatch's timing boundary.");
  const auto repeatedFinish = probe.finish();
  check(repeatedFinish.dispatchMilliseconds == measured.dispatchMilliseconds,
    "Finishing twice must preserve the original dispatch duration.");

  CameraCommandTiming outer(true);
  CameraCommandTiming::Result innerResult;
  CameraCommandTiming::Result disabledResult;
  {
    const CameraCommandTiming::Handler handler;
    {
      CameraCommandTiming inner(true);
      {
        const CameraCommandTiming::Handler nestedHandler;
        const CameraCommandTiming::CoreUpdate nestedCore;
      }
      innerResult = inner.finish();
    }
    {
      CameraCommandTiming muted(false);
      {
        const CameraCommandTiming::Handler mutedHandler;
        const CameraCommandTiming::CoreUpdate mutedCore;
      }
      disabledResult = muted.finish();
    }
    {
      const CameraCommandTiming::CoreUpdate core;
    }
  }
  const auto outerResult = outer.finish();
  check(innerResult.handlers == 1 && outerResult.handlers == 1 && outerResult.coreUpdates == 1 &&
    !disabledResult.captured && disabledResult.handlers == 0,
    "Nested enabled/disabled dispatches must isolate handler attribution and restore the outer context.");

  try
  {
    CameraCommandTiming interrupted(true);
    const CameraCommandTiming::Handler handler;
    const CameraCommandTiming::CoreUpdate core;
    throw 1;
  }
  catch (int)
  {
  }
  CameraCommandTiming afterException(true);
  {
    const CameraCommandTiming::Handler handler;
  }
  const auto recovered = afterException.finish();
  check(recovered.handlers == 1 && recovered.coreUpdates == 0,
    "Unwinding must restore the probe context for subsequent commands.");
  return failures == 0 ? 0 : 1;
}
