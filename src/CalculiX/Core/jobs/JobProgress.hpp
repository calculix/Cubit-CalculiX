#ifndef CALCULIX_JOBPROGRESS_HPP
#define CALCULIX_JOBPROGRESS_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace CalculiXJobProgress
{
  struct StatusRow
  {
    int step = 0;
    int increment = 0;
    int attempt = 0;
    int iterations = 0;
    bool unconverged = false;
    // Solver total time, step time and attempted increment size; not wall time.
    std::array<double, 3> time{};
  };

  struct ConvergenceRow
  {
    int step = 0;
    int increment = 0;
    int attempt = 0;
    int iteration = 0;
    int contact_elements = 0;
    // CCX-reported percentages: force residual, displacement correction,
    // flux residual, temperature correction. No generic convergence threshold.
    std::array<double, 4> measures{};
  };

  struct UnparsedLine
  {
    std::string source;
    std::size_t line = 0;
    std::string text;
  };

  struct Data
  {
    std::uint64_t generation = 0;
    std::uint64_t revision = 0;
    std::vector<StatusRow> status;
    std::vector<ConvergenceRow> convergence;
    std::vector<UnparsedLine> unparsed;
  };

  class Parser
  {
  public:
    Parser();
    void reset();
    // Append complete records only. Replacement/truncation starts a generation.
    // Supports CCX 2.23 STA (including ATT suffix U) and nine-column CVG.
    // Invalid/overflow/nonfinite records are retained verbatim in unparsed.
    void update(const std::vector<std::string>& cvg, const std::vector<std::string>& sta);
    const Data& data() const;

  private:
    Data parsed;
    std::vector<std::string> previous_cvg;
    std::vector<std::string> previous_sta;
  };

  class FileReader
  {
  public:
    // Missing files preserve saved lines. Only newline-terminated records are
    // published; an interrupted final fragment remains pending. Reads append
    // bytes incrementally and detects truncation, changed tails and replacement.
    // Callers discard readers on managed solver restart or project reload.
    bool read(const std::string& path, std::vector<std::string>& lines,
      bool& restarted, std::string& error);

  private:
    std::string path;
    std::uintmax_t position = 0;
    std::string pending;
    std::string anchor;
    std::filesystem::file_time_type modified{};
  };

  struct State
  {
    FileReader cvg;
    FileReader sta;
    Parser parser;
    std::string error;
  };
}

#endif
