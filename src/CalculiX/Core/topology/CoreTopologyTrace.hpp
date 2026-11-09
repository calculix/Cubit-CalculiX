#ifndef CORETOPOLOGYTRACE_HPP
#define CORETOPOLOGYTRACE_HPP

/**
 * @brief Write opt-in diagnostic events directly to a file.
 * @note Disabled unless CUBITSIM_TOPOLOGY_TRACE_FILE is set. Each event is
 * flushed independently so the last event remains visible during a UI hang.
 * File-open/write failures do not alter calculation return values.
 */
class CoreTopologyTrace
{
public:
  static void write(const char* event);
};

#endif
