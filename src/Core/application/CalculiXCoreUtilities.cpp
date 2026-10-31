#include "Core/CalculiXCore.hpp"
#include "CoreIdSelection.hpp"
#include "Core/export/CoreCsvWriter.hpp"
#include "ScientificNumber.hpp"

bool CalculiXCore::export_to_csv(std::string path, std::vector<std::string> header, std::vector<std::vector<double>> data, bool overwrite)
{
  return CoreCsvWriter::write(path, header, data, overwrite);
}

std::string CalculiXCore::to_string_scientific(double value, int precision)
{
  return ScientificNumber::format(value, precision);
}

double CalculiXCore::string_scientific_to_double(std::string value, int precision)
{
  return ScientificNumber::parse_with_precision(value, precision);
}

std::vector<int> CalculiXCore::extractIntegers(std::string str)
{
  return CoreIdSelection::extract_integers(str);
}
