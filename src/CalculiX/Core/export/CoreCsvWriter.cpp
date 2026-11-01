#include "CoreCsvWriter.hpp"
#include "CubitMessage.hpp"

#include <cstddef>
#include <fstream>

#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

bool CoreCsvWriter::write(
  const std::string& path,
  const std::vector<std::string>& header,
  const std::vector<std::vector<double>>& data,
  bool overwrite)
{
  std::string filename = path;

  // check if file already exists
  if (!overwrite)
  {
    #ifdef WIN32
      if (_access(filename.c_str(), 0) == 0)
      {
        PRINT_ERROR("Output File already exists!\n");
        return false;
      }
    #else
      if (access(filename.c_str(), F_OK) == 0)
      {
        PRINT_ERROR("Output File already exists!\n");
        return false;
      }
    #endif
  }

  std::ofstream file(filename);

  if (!file.is_open())
  {
    PRINT_ERROR("Can't open CSV output file: %s\n", filename.c_str());
    return false;
  }

  for (std::size_t i = 0; i < header.size(); ++i)
  {
    if (i > 0)
    {
      file << ",";
    }

    file << header[i];
  }

  file << "\n";

  for (const auto& row : data)
  {
    for (std::size_t i = 0; i < row.size(); ++i)
    {
      if (i > 0)
      {
        file << ",";
      }

      file << row[i];
    }

    file << "\n";
  }

  file.close();

  if (!file)
  {
    PRINT_ERROR("Can't write CSV output file: %s\n", filename.c_str());
    return false;
  }

  return true;
}
