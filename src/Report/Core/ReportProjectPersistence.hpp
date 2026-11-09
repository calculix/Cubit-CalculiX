#pragma once
#include <string>
class HDF5Tool;
class ReportSession;

class ReportProjectPersistence
{
public:
  ReportProjectPersistence();
  ~ReportProjectPersistence();
  bool read(HDF5Tool& file, ReportSession& output, std::string& error) const;
  bool write(HDF5Tool& file, const ReportSession& session, std::string& error) const;
};
