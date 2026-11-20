#include "CoreTopologyConsole.hpp"
#include "CoreTopology.hpp"
#include "Core/loadCalculiXCore.hpp"
#include <array>
#include <cstdarg>
#include <cstdio>
#include <deque>
#include <map>
#include <mutex>
#include <utility>

namespace
{
  constexpr std::size_t maximumEntries = 4000;
  constexpr std::size_t maximumLine = 4096;
  thread_local CoreTopologyConsole::Context activeContext;

  struct CaseLog
  {
    std::uint64_t generation = 0;
    std::uint64_t sequence = 0;
    std::deque<CoreTopologyConsole::Entry> entries;
  };
  struct JobLog
  {
    CoreTopologyConsole::Context context;
    std::string partial;
    bool truncated = false;
  };
  std::mutex mutex;
  std::map<int,CaseLog> cases;
  std::map<int,JobLog> jobs;
  std::uint64_t nextGeneration = 0;

  void append(const CoreTopologyConsole::Context& context, const char* severity, std::string line)
  {
    if (!line.empty() && line.back() == '\r')
    {
      line.pop_back();
    }
    std::string prefix;
    if (context.iteration > 0)
    {
      prefix = "Iteration " + std::to_string(context.iteration) + " | ";
    }
    if (context.role == CoreTopologyConsole::Role::Optimizer)
    {
      prefix += "Optimizer";
    }
    else
    {
      if (context.role == CoreTopologyConsole::Role::Adjoint)
      {
        prefix += "Adjoint job ";
      }
      else
      {
        prefix += "Physical job ";
      }
      prefix += std::to_string(context.job);
    }
    auto& log = cases[context.topology];
    if (!log.generation)
    {
      log.generation = ++nextGeneration;
    }
    log.entries.push_back({++log.sequence,prefix + " | " + severity + line});
    if (log.entries.size() > maximumEntries)
    {
      log.entries.pop_front();
    }
  }

  void flush(JobLog& job)
  {
    if (job.truncated)
    {
      job.partial += " [line truncated]";
    }
    append(job.context,"",std::move(job.partial));
    job.partial.clear();
    job.truncated = false;
  }
}

CoreTopologyConsole::CoreTopologyConsole(Context context) noexcept : previous(activeContext)
{
  activeContext = context;
}

CoreTopologyConsole::~CoreTopologyConsole()
{
  activeContext = previous;
}

CoreTopologyConsole::Context CoreTopologyConsole::job_context(int job)
{
  if (!ccx_core.topology || job <= 0)
  {
    return {};
  }
  for (const auto& item : ccx_core.topology->topology_cases)
  {
    for (const auto& iteration : item.second.iteration_jobs)
    {
      if (iteration.second == job)
      {
        return {item.first,iteration.first,job,Role::Physical};
      }
    }
    for (const auto& iteration : item.second.iteration_adjoint_jobs)
    {
      if (iteration.second == job)
      {
        return {item.first,iteration.first,job,Role::Adjoint};
      }
    }
  }
  return {};
}

void CoreTopologyConsole::register_job(Context context)
{
  if (context.topology > 0 && context.job > 0)
  {
    const std::lock_guard<std::mutex> guard(mutex);
    jobs.insert_or_assign(context.job,JobLog{context,{}});
  }
}

void CoreTopologyConsole::solver_output(int job, const std::string& text)
{
  const std::lock_guard<std::mutex> guard(mutex);
  const auto found = jobs.find(job);
  if (found == jobs.end())
  {
    return;
  }
  auto& state = found->second;
  for (char character : text)
  {
    if (character == '\n')
    {
      flush(state);
    }
    else if (state.partial.size() < maximumLine)
    {
      state.partial += character;
    }
    else
    {
      state.truncated = true;
    }
  }
}

void CoreTopologyConsole::finish_solver_output(int job)
{
  const std::lock_guard<std::mutex> guard(mutex);
  const auto found = jobs.find(job);
  if (found != jobs.end() && !found->second.partial.empty())
  {
    flush(found->second);
  }
}

void CoreTopologyConsole::forget_job(int job)
{
  const std::lock_guard<std::mutex> guard(mutex);
  const auto found = jobs.find(job);
  if (found != jobs.end())
  {
    if (!found->second.partial.empty())
    {
      flush(found->second);
    }
    jobs.erase(found);
  }
}

void CoreTopologyConsole::erase_case(int topology)
{
  const std::lock_guard<std::mutex> guard(mutex);
  cases.erase(topology);
  for (auto job = jobs.begin(); job != jobs.end();)
  {
    if (job->second.context.topology == topology)
    {
      job = jobs.erase(job);
    }
    else
    {
      ++job;
    }
  }
}

void CoreTopologyConsole::clear()
{
  const std::lock_guard<std::mutex> guard(mutex);
  cases.clear();
  jobs.clear();
}

CoreTopologyConsole::Batch CoreTopologyConsole::read(int topology, std::uint64_t generation, std::uint64_t after)
{
  const std::lock_guard<std::mutex> guard(mutex);
  Batch output;
  const auto found = cases.find(topology);
  if (found != cases.end())
  {
    output.generation = found->second.generation;
    for (const auto& entry : found->second.entries)
    {
      if (generation != output.generation || entry.sequence > after)
      {
        output.entries.push_back(entry);
      }
    }
  }
  return output;
}

bool CoreTopologyConsole::active() noexcept
{
  return activeContext.topology > 0;
}

void CoreTopologyConsole::write(const char* severity, const char* format, ...)
{
  std::array<char,maximumLine+1> buffer{};
  va_list arguments;
  va_start(arguments,format);
  const int length = std::vsnprintf(buffer.data(),buffer.size(),format,arguments);
  va_end(arguments);
  if (length < 0)
  {
    return;
  }
  std::string message(buffer.data());
  if (static_cast<std::size_t>(length) > maximumLine)
  {
    message += " [message truncated]";
  }
  const std::lock_guard<std::mutex> guard(mutex);
  std::size_t begin = 0;
  while (begin < message.size())
  {
    const auto end = message.find('\n',begin);
    append(activeContext,severity,message.substr(begin,end-begin));
    if (end == std::string::npos)
    {
      break;
    }
    begin = end + 1;
  }
}
