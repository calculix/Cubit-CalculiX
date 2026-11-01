#ifndef RESULTREADBUFFER_HPP
#define RESULTREADBUFFER_HPP

#include <cstddef>
#include <cstring>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace ResultsReaderPipeline
{
enum class LineStatus { Line, End, IoError, TooLong };

// Returned views remain valid until next() is called again. Worker batches
// must own their text; append_raw_record() copies each view into the batch.
class BufferedInput
{
public:
  explicit BufferedInput(std::ifstream& file, std::size_t maximum_line_bytes) : file_(file), buffer_(1024 * 1024), maximum_line_bytes_(maximum_line_bytes)
  {
  }

  std::streamoff consumed_bytes() const
  {
    return consumed_bytes_;
  }

  LineStatus next(std::string_view& line)
  {
    line = {};
    spill_.clear();
    while (true)
    {
      if (begin_ == end_)
      {
        const auto status = refill();
        if (status == LineStatus::IoError)
        {
          return status;
        }
        if (status == LineStatus::End)
        {
          if (spill_.empty())
          {
            return LineStatus::End;
          }
          line = spill_;
          return LineStatus::Line;
        }
      }
      const char* first = buffer_.data() + begin_;
      const std::size_t available = end_ - begin_;
      const auto newline = static_cast<const char*>(std::memchr(first, '\n', available));
      const std::size_t length = newline == nullptr ? available : static_cast<std::size_t>(newline - first);
      if (length > maximum_line_bytes_ - spill_.size())
      {
        return LineStatus::TooLong;
      }
      if (newline != nullptr)
      {
        if (spill_.empty())
        {
          line = std::string_view(first, length);
        }
        else
        {
          spill_.append(first, length);
          line = spill_;
        }
        begin_ += length + 1;
        consumed_bytes_ += static_cast<std::streamoff>(length + 1);
        return LineStatus::Line;
      }
      spill_.append(first, length);
      begin_ += length;
      consumed_bytes_ += static_cast<std::streamoff>(length);
    }
  }

private:
  LineStatus refill()
  {
    begin_ = 0;
    end_ = 0;
    if (file_.bad() || (file_.fail() && !file_.eof()))
    {
      return LineStatus::IoError;
    }
    if (file_.eof())
    {
      return LineStatus::End;
    }
    file_.read(buffer_.data(), static_cast<std::streamsize>(buffer_.size()));
    if (file_.bad() || (file_.fail() && !file_.eof()))
    {
      return LineStatus::IoError;
    }
    end_ = static_cast<std::size_t>(file_.gcount());
    return end_ == 0 ? LineStatus::End : LineStatus::Line;
  }

  std::ifstream& file_;
  std::vector<char> buffer_;
  std::string spill_;
  std::size_t begin_ = 0;
  std::size_t end_ = 0;
  std::size_t maximum_line_bytes_;
  std::streamoff consumed_bytes_ = 0;
};
}
#endif // RESULTREADBUFFER_HPP
