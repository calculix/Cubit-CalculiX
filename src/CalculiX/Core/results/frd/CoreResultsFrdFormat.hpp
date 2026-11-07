#ifndef CORERESULTSFRDFORMAT_HPP
#define CORERESULTSFRDFORMAT_HPP

#include "Core/results/ResultReadNumbers.hpp"
#include <array>
#include <string>
#include <string_view>

namespace CoreResultsFrdFormat
{
  enum class RecordStatus
  {
    Pending,
    Ready,
    Invalid
  };

  // frdheader.c uses fixed columns: MODAL immediately follows the five-column
  // increment number, so whitespace tokenization cannot identify it reliably.
  inline bool loadcase(std::string_view line, double& time, int& increment, int& analysis)
  {
    return line.size() >= 75 && ResultReadNumbers::trim(line.substr(0, 7)) == "100CL" && line[74] == '1' &&
      ResultReadNumbers::real(line.substr(12, 12), time) &&
      ResultReadNumbers::integer(line.substr(56, 2), analysis) &&
      ResultReadNumbers::integer(line.substr(58, 5), increment);
  }

  /** One logical ASCII row consists of -1/id followed by at most six values
   * per physical line. Continuations start with -2 and have no identifier.
   * The caller dispatches Ready rows only and keeps Pending across batches.
   */
  inline RecordStatus append(std::string_view line, std::size_t components, std::string& record)
  {
    line = ResultReadNumbers::trim(line);
    std::array<std::string_view, ResultReadNumbers::max_fields> fields;
    std::size_t count = 0;
    if (!ResultReadNumbers::fields(line, fields, count) || components == 0)
    {
      return RecordStatus::Invalid;
    }
    if (fields[0] == "-1")
    {
      if (!record.empty() || count < 3 || count > 8)
      {
        return RecordStatus::Invalid;
      }
      record.assign(line.data(), line.size());
    }
    else if (fields[0] == "-2")
    {
      if (record.empty() || count < 2 || count > 7)
      {
        return RecordStatus::Invalid;
      }
      record.push_back(' ');
      record.append(line.substr(2));
    }
    else
    {
      return RecordStatus::Invalid;
    }
    count = ResultReadNumbers::field_count(record);
    if (count < 2 || count - 2 > components)
    {
      return RecordStatus::Invalid;
    }
    return count - 2 == components ? RecordStatus::Ready : RecordStatus::Pending;
  }
}

#endif // CORERESULTSFRDFORMAT_HPP
