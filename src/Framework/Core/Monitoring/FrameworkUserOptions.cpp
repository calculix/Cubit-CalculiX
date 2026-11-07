#include "FrameworkUserOptions.hpp"
#include "Framework/Interfaces/UserOptionsInterface.hpp"
#include <cmath>
#include <algorithm>
#include <exception>

namespace
{
bool validColor(const UserOptionValue& value)
{
  const auto* color = std::get_if<std::string>(&value);
  return color != nullptr && color->size() == 7 && color->front() == '#' &&
    std::all_of(color->begin() + 1, color->end(), [](char character)
    {
      return (character >= '0' && character <= '9') ||
        (character >= 'a' && character <= 'f') || (character >= 'A' && character <= 'F');
    });
}

FrameworkOptions settings(const UserOptionValues& values)
{
  FrameworkOptions result;
  result.monitorEnabled = std::get<bool>(values.at("monitor/enabled"));
  result.intervalSeconds = std::get<int>(values.at("monitor/interval_seconds"));
  result.showCpu = std::get<bool>(values.at("monitor/show_cpu"));
  result.showMemory = std::get<bool>(values.at("monitor/show_memory"));
  result.showCubit = std::get<bool>(values.at("monitor/show_cubit"));
  result.showJobs = std::get<bool>(values.at("monitor/show_jobs"));
  result.showJobAttribution = std::get<bool>(values.at("monitor/show_job_attribution"));
  result.systemColorRgb = static_cast<std::uint32_t>(std::stoul(std::get<std::string>(values.at("monitor/system_color")).substr(1), nullptr, 16));
  result.cubitColorRgb = static_cast<std::uint32_t>(std::stoul(std::get<std::string>(values.at("monitor/cubit_color")).substr(1), nullptr, 16));
  result.jobsColorRgb = static_cast<std::uint32_t>(std::stoul(std::get<std::string>(values.at("monitor/jobs_color")).substr(1), nullptr, 16));
  return result;
}
}

std::vector<UserOption> FrameworkUserOptions::definitions()
{
  // Authoritative Framework defaults. The plain snapshot above is empty state.
  return {
    {"monitor/enabled", UserOptionKind::Boolean, true},
    {"monitor/interval_seconds", UserOptionKind::Integer, 2, [](const UserOptionValue& value)
      {
        const auto* number = std::get_if<int>(&value);
        return number != nullptr && *number >= 1 && *number <= 60;
      }},
    {"monitor/show_cpu", UserOptionKind::Boolean, true},
    {"monitor/show_memory", UserOptionKind::Boolean, true},
    {"monitor/show_cubit", UserOptionKind::Boolean, true},
    {"monitor/show_jobs", UserOptionKind::Boolean, true},
    {"monitor/show_job_attribution", UserOptionKind::Boolean, true},
    {"monitor/system_color", UserOptionKind::Text, std::string("#5CB85C"), validColor},
    {"monitor/cubit_color", UserOptionKind::Text, std::string("#5B9BD5"), validColor},
    {"monitor/jobs_color", UserOptionKind::Text, std::string("#E6A23C"), validColor}
  };
}

bool FrameworkUserOptions::initialize(UserOptionsInterface& options, std::string& error)
{
  error.clear();
  try
  {
    UserOptionValues values;
    if (!options.load("framework", definitions(), values, error))
    {
      return false;
    }
    const auto candidate = settings(values);
    configured = candidate;
    store = &options;
    return true;
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
    return false;
  }
}

const FrameworkOptions& FrameworkUserOptions::current() const noexcept
{
  return configured;
}

bool FrameworkUserOptions::apply(const UserOptionValues& changes, std::string& error)
{
  error.clear();
  if (store == nullptr)
  {
    error = "Framework preferences are unavailable.";
    return false;
  }
  try
  {
    const auto schema = definitions();
    UserOptionValues values;
    if (!store->load("framework", schema, values, error) ||
      !merge_user_option_changes(schema, changes, values, error))
    {
      return false;
    }
    const auto candidate = settings(values);
    if (!store->save("framework", schema, values, error))
    {
      return false;
    }
    // Trivial assignment cannot throw after persistence succeeds.
    configured = candidate;
    return true;
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
    return false;
  }
}

bool FrameworkUserOptions::values(UserOptionValues& snapshot, std::string& error) const
{
  if (store == nullptr)
  {
    error = "Framework preferences are unavailable.";
    return false;
  }
  return store->load("framework", definitions(), snapshot, error);
}
