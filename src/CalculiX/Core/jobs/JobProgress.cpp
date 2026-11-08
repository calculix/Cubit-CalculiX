#include "JobProgress.hpp"
#include <algorithm>
#include <atomic>
#include <charconv>
#include <cmath>
#include <fstream>
#include <locale>
#include <sstream>
#include <utility>

namespace
{
  bool prefix(const std::vector<std::string>& old, const std::vector<std::string>& current)
  {
    return old.size() <= current.size() && std::equal(old.begin(), old.end(), current.begin());
  }

  bool integer(const std::string& text, int& result)
  {
    const auto converted = std::from_chars(text.data(), text.data() + text.size(), result);
    return converted.ec == std::errc{} && converted.ptr == text.data() + text.size();
  }

  bool real(std::string text, double& result)
  {
    for (auto& character : text)
    {
      if (character == 'D' || character == 'd')
      {
        character = 'E';
      }
    }
    // Fortran can omit E for a three-digit exponent in a narrow output field.
    if (text.find_first_of("Ee") == std::string::npos)
    {
      const auto exponent = text.find_first_of("+-", 1);
      if (exponent != std::string::npos)
      {
        text.insert(exponent, "E");
      }
    }
    std::istringstream stream(text);
    stream.imbue(std::locale::classic());
    stream >> result;
    return stream && stream.peek() == std::char_traits<char>::eof() && std::isfinite(result);
  }

  void parse(const std::string& line, std::size_t number, bool status, CalculiXJobProgress::Data& data)
  {
    std::istringstream stream(line);
    stream.imbue(std::locale::classic());
    std::vector<std::string> words;
    std::string word;
    while (stream >> word)
    {
      words.push_back(word);
    }
    if (words.empty() || words[0] == "SUMMARY" || words[0] == "STEP" ||
      (!status && (words[0] == "EL." || words[0] == "(#)")))
    {
      return;
    }
    bool valid = false;
    if (status && words.size() == 7)
    {
      CalculiXJobProgress::StatusRow row;
      row.unconverged = !words[2].empty() && words[2].back() == 'U';
      if (row.unconverged)
      {
        words[2].pop_back();
      }
      valid = integer(words[0], row.step) && integer(words[1], row.increment) &&
        integer(words[2], row.attempt) && integer(words[3], row.iterations) &&
        row.step > 0 && row.increment > 0 && row.attempt > 0 && row.iterations >= 0;
      for (std::size_t i = 0; i < row.time.size(); ++i)
      {
        valid = real(words[i + 4], row.time[i]) && valid;
      }
      if (valid)
      {
        data.status.push_back(row);
      }
    }
    else if (!status && words.size() == 9)
    {
      CalculiXJobProgress::ConvergenceRow row;
      valid = integer(words[0], row.step) && integer(words[1], row.increment) &&
        integer(words[2], row.attempt) && integer(words[3], row.iteration) &&
        integer(words[4], row.contact_elements) && row.step > 0 && row.increment > 0 &&
        row.attempt > 0 && row.iteration > 0 && row.contact_elements >= 0;
      for (std::size_t i = 0; i < row.measures.size(); ++i)
      {
        valid = real(words[i + 5], row.measures[i]) && valid;
      }
      if (valid)
      {
        data.convergence.push_back(row);
      }
    }
    if (!valid)
    {
      data.unparsed.push_back({status ? "STA" : "CVG", number, line});
    }
  }
}

namespace CalculiXJobProgress
{
  Parser::Parser()
  {
    reset();
  }

  void Parser::reset()
  {
    static std::atomic<std::uint64_t> generation{0};
    parsed = {};
    parsed.generation = ++generation;
    previous_cvg.clear();
    previous_sta.clear();
  }

  void Parser::update(const std::vector<std::string>& cvg, const std::vector<std::string>& sta)
  {
    if (!prefix(previous_cvg, cvg) || !prefix(previous_sta, sta))
    {
      reset();
    }
    if (previous_cvg.size() == cvg.size() && previous_sta.size() == sta.size())
    {
      return;
    }
    for (std::size_t i = previous_cvg.size(); i < cvg.size(); ++i)
    {
      parse(cvg[i], i + 1, false, parsed);
    }
    for (std::size_t i = previous_sta.size(); i < sta.size(); ++i)
    {
      parse(sta[i], i + 1, true, parsed);
    }
    previous_cvg.insert(previous_cvg.end(), cvg.begin() + previous_cvg.size(), cvg.end());
    previous_sta.insert(previous_sta.end(), sta.begin() + previous_sta.size(), sta.end());
    ++parsed.revision;
  }

  const Data& Parser::data() const
  {
    return parsed;
  }

  bool FileReader::read(const std::string& filename, std::vector<std::string>& lines,
    bool& restarted, std::string& error)
  {
    restarted = false;
    error.clear();
    std::error_code code;
    const auto file_path = std::filesystem::u8path(filename);
    const auto size = std::filesystem::file_size(file_path, code);
    if (code)
    {
      // Saved job monitoring data remains inspectable without its source files.
      if (code == std::errc::no_such_file_or_directory)
      {
        return true;
      }
      error = "Cannot inspect job progress file: " + filename;
      return false;
    }
    const auto time = std::filesystem::last_write_time(file_path, code);
    std::ifstream input(file_path, std::ios::binary);
    if (code || !input)
    {
      error = "Cannot open job progress file: " + filename;
      return false;
    }
    bool replace = path != filename || size < position || (size == position && time != modified);
    if (!replace && !anchor.empty())
    {
      input.seekg(static_cast<std::streamoff>(position - anchor.size()));
      std::string check(anchor.size(), '\0');
      input.read(check.data(), static_cast<std::streamsize>(check.size()));
      replace = check != anchor || !input;
    }
    if (replace)
    {
      path = filename;
      position = 0;
      pending.clear();
      anchor.clear();
      lines.clear();
      restarted = true;
    }
    input.clear();
    input.seekg(static_cast<std::streamoff>(position));
    char buffer[8192];
    while (input.read(buffer, sizeof(buffer)) || input.gcount() > 0)
    {
      const auto count = static_cast<std::size_t>(input.gcount());
      pending.append(buffer, count);
      position += count;
      std::size_t begin = 0;
      auto end = pending.find('\n');
      while (end != std::string::npos)
      {
        auto line = pending.substr(begin, end - begin);
        if (!line.empty() && line.back() == '\r')
        {
          line.pop_back();
        }
        lines.push_back(std::move(line));
        begin = end + 1;
        end = pending.find('\n', begin);
      }
      pending.erase(0, begin);
    }
    if (input.bad())
    {
      error = "Cannot read job progress file: " + filename;
      return false;
    }
    input.clear();
    const auto length = (std::min)(position, std::uintmax_t{64});
    input.seekg(static_cast<std::streamoff>(position - length));
    anchor.resize(static_cast<std::size_t>(length));
    input.read(anchor.data(), static_cast<std::streamsize>(length));
    if (static_cast<std::uintmax_t>(input.gcount()) != length)
    {
      anchor.clear();
      error = "Job progress file changed while reading: " + filename;
      return false;
    }
    modified = std::filesystem::last_write_time(file_path, code);
    if (code)
    {
      error = "Cannot inspect job progress file after reading: " + filename;
      return false;
    }
    return true;
  }
}
