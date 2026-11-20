#pragma once
#include <cstdint>
#include <string>
#include <vector>

class CoreTopologyConsole
{
public:
  enum class Role { Optimizer, Physical, Adjoint };
  struct Context
  {
    int topology = -1;
    int iteration = -1;
    int job = -1;
    Role role = Role::Optimizer;
  };
  struct Entry
  {
    std::uint64_t sequence;
    std::string text;
  };
  struct Batch
  {
    std::uint64_t generation = 0;
    std::vector<Entry> entries;
  };

  explicit CoreTopologyConsole(Context context) noexcept;
  ~CoreTopologyConsole();
  CoreTopologyConsole(const CoreTopologyConsole&) = delete;
  CoreTopologyConsole& operator=(const CoreTopologyConsole&) = delete;

  // Resolve ownership on the host thread. Pipe readers only use the registered context.
  static Context job_context(int job);
  static void register_job(Context context);
  static void solver_output(int job, const std::string& text);
  static void finish_solver_output(int job);
  static void forget_job(int job);
  static void erase_case(int topology);
  static void clear();
  static Batch read(int topology, std::uint64_t generation, std::uint64_t after);
  static bool active() noexcept;
  static void write(const char* severity, const char* format, ...);

private:
  Context previous;
};
